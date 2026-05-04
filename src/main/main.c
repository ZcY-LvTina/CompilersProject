#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "compiler_project/common/token.h"
#include "compiler_project/lex/lex_parser.h"
#include "compiler_project/lex/lexer_runtime.h"
#include "compiler_project/semantic/ir_builder.h"
#include "compiler_project/semantic/semantic_action.h"
#include "compiler_project/semantic/type_checker.h"
#include "compiler_project/yacc/parser_runtime.h"
#include "compiler_project/yacc/yacc_parser.h"

static int read_file(const char *path, char *buffer, size_t size) {
    FILE *file = fopen(path, "r");
    size_t read_size;
    if (file == NULL) {
        return 0;
    }
    read_size = fread(buffer, 1, size - 1, file);
    buffer[read_size] = '\0';
    fclose(file);
    return 1;
}

static void print_usage(void) {
    printf("Usage:\n");
    printf("  compiler_project check-layout\n");
    printf("  compiler_project lex --spec <path> --source <path>\n");
    printf("  compiler_project yacc --spec <path>\n");
    printf("  compiler_project semantic --lex-spec <path> --yacc-spec <path> --source <path>\n");
}

static int cmd_check_layout(void) {
    const char *paths[] = {
        "docs",
        "samples",
        "include/compiler_project/common",
        "include/compiler_project/lex",
        "include/compiler_project/yacc",
        "include/compiler_project/semantic",
        "src",
        "tests",
        "output"
    };
    size_t i;
    for (i = 0; i < sizeof(paths) / sizeof(paths[0]); ++i) {
        FILE *probe = fopen(paths[i], "r");
        if (probe != NULL) {
            fclose(probe);
            continue;
        }
        if (access(paths[i], 0) != 0) {
            printf("{\"ok\":false,\"missing\":\"%s\"}\n", paths[i]);
            return 1;
        }
    }
    printf("{\"ok\":true}\n");
    return 0;
}

static int cmd_lex(const char *spec_path, const char *source_path) {
    LexSpecResult spec_result;
    TokenStreamResult token_result;
    char source[4096];
    int i;

    spec_result = cp_parse_lex_spec(spec_path);
    if (!spec_result.base.ok) {
        printf("lex spec error: %s\n", spec_result.base.errors[0].message);
        return 1;
    }
    if (!read_file(source_path, source, sizeof(source))) {
        printf("cannot read source file\n");
        return 1;
    }
    token_result = cp_tokenize_source(&spec_result.data, source);
    if (!token_result.base.ok) {
        printf("tokenize error: %s\n", token_result.base.errors[0].message);
        return 1;
    }
    for (i = 0; i < token_result.data.count; ++i) {
        const Token *token = &token_result.data.tokens[i];
        printf("(%s, \"%s\", %d, %d, %s)\n",
            token->kind,
            token->lexeme,
            token->line,
            token->column,
            token->attr[0] == '\0' ? "null" : token->attr);
    }
    return 0;
}

static int cmd_yacc(const char *spec_path) {
    GrammarResult grammar_result = cp_parse_yacc_spec(spec_path);
    int i;
    if (!grammar_result.base.ok) {
        printf("yacc spec error: %s\n", grammar_result.base.errors[0].message);
        return 1;
    }
    printf("start_symbol: %s\n", grammar_result.data.start_symbol);
    for (i = 0; i < grammar_result.data.production_count; ++i) {
        int j;
        const Production *production = &grammar_result.data.productions[i];
        printf("%s ->", production->lhs);
        for (j = 0; j < production->rhs_count; ++j) {
            printf(" %s", production->rhs[j]);
        }
        printf("\n");
    }
    return 0;
}

static void print_symbol_table(const SymbolTable *table) {
    int i;
    if (table == NULL) {
        return;
    }
    for (i = 0; i < table->scope_count; ++i) {
        int j;
        const Scope *scope = &table->scopes[i];
        printf("[scope %d] %s name=%s parent=%d entries=%d\n",
            scope->id,
            cp_scope_kind_name(scope->kind),
            scope->name,
            scope->parent_scope_id,
            scope->count);
        for (j = 0; j < scope->count; ++j) {
            const SymbolEntry *entry = &scope->entries[j];
            printf("  - %s %s offset=%d scope=%d initialized=%d\n",
                entry->type_expr.text,
                entry->name,
                entry->offset,
                entry->scope_id,
                entry->initialized);
        }
    }
}

static void print_ir(const QuadrupleList *ir) {
    int i;
    if (ir == NULL) {
        return;
    }
    for (i = 0; i < ir->count; ++i) {
        const Quadruple *quad = &ir->items[i];
        printf("(%d) (%s, %s, %s, %s)\n",
            quad->index,
            cp_ir_op_name(quad->op),
            quad->arg1,
            quad->arg2,
            quad->result);
    }
}

static int cmd_semantic(const char *lex_spec_path, const char *yacc_spec_path, const char *source_path) {
    LexSpecResult lex_spec;
    GrammarResult grammar;
    TokenStreamResult tokens;
    ASTNodeResult ast;
    SymbolTableResult symbols;
    BoolResult type_check;
    QuadrupleListResult ir;
    char source[4096];

    lex_spec = cp_parse_lex_spec(lex_spec_path);
    if (!lex_spec.base.ok) {
        printf("lex spec error: %s\n", lex_spec.base.errors[0].message);
        return 1;
    }
    grammar = cp_parse_yacc_spec(yacc_spec_path);
    if (!grammar.base.ok) {
        printf("yacc spec error: %s\n", grammar.base.errors[0].message);
        return 1;
    }
    if (!read_file(source_path, source, sizeof(source))) {
        printf("cannot read source file\n");
        return 1;
    }
    tokens = cp_tokenize_source(&lex_spec.data, source);
    if (!tokens.base.ok) {
        printf("tokenize error: %s\n", tokens.base.errors[0].message);
        return 1;
    }
    ast = cp_parse_tokens(&grammar.data, &tokens.data);
    if (!ast.base.ok) {
        printf("parse error: %s\n", ast.base.errors[0].message);
        return 1;
    }
    symbols = cp_run_semantic_actions(&ast.data);
    if (!symbols.base.ok) {
        printf("semantic action error: %s\n", symbols.base.errors[0].message);
        return 1;
    }
    type_check = cp_check_types(&ast.data);
    if (!type_check.base.ok) {
        printf("type error: %s\n", type_check.base.errors[0].message);
        return 1;
    }
    ir = cp_build_ir(&ast.data);
    if (!ir.base.ok) {
        printf("ir error: %s\n", ir.base.errors[0].message);
        return 1;
    }
    printf("AST root: %s (%s)\n", ast.data.symbol_name, cp_ast_node_type_name(ast.data.node_type));
    print_symbol_table(&symbols.data);
    print_ir(&ir.data);
    return 0;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        print_usage();
        return 1;
    }
    if (strcmp(argv[1], "check-layout") == 0) {
        return cmd_check_layout();
    }
    if (strcmp(argv[1], "lex") == 0 && argc == 6 && strcmp(argv[2], "--spec") == 0 && strcmp(argv[4], "--source") == 0) {
        return cmd_lex(argv[3], argv[5]);
    }
    if (strcmp(argv[1], "yacc") == 0 && argc == 4 && strcmp(argv[2], "--spec") == 0) {
        return cmd_yacc(argv[3]);
    }
    if (strcmp(argv[1], "semantic") == 0
        && argc == 8
        && strcmp(argv[2], "--lex-spec") == 0
        && strcmp(argv[4], "--yacc-spec") == 0
        && strcmp(argv[6], "--source") == 0) {
        return cmd_semantic(argv[3], argv[5], argv[7]);
    }
    print_usage();
    return 1;
}
