#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "compiler_project/common/token.h"
#include "compiler_project/lex/lex_parser.h"
#include "compiler_project/lex/lexer_runtime.h"
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
    print_usage();
    return 1;
}
