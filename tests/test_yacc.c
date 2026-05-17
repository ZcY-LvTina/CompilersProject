#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "compiler_project/common/token.h"
#include "compiler_project/yacc/grammar.h"
#include "compiler_project/yacc/lr_builder.h"
#include "compiler_project/yacc/parser_runtime.h"
#include "compiler_project/yacc/yacc_parser.h"

static void add_token(TokenStream *stream, const char *kind, const char *lexeme, int line, int column) {
    Token *token = &stream->tokens[stream->count++];
    memset(token, 0, sizeof(*token));
    snprintf(token->kind, sizeof(token->kind), "%s", kind);
    snprintf(token->lexeme, sizeof(token->lexeme), "%s", lexeme);
    token->line = line;
    token->column = column;
    token->length = (int) strlen(lexeme);
}

static TokenStream make_decl_tokens(void) {
    TokenStream stream;
    memset(&stream, 0, sizeof(stream));
    add_token(&stream, "INT", "int", 1, 1);
    add_token(&stream, "ID", "sum", 1, 5);
    add_token(&stream, "ASSIGN", "=", 1, 9);
    add_token(&stream, "NUM", "12", 1, 11);
    add_token(&stream, "SEMI", ";", 1, 13);
    add_token(&stream, "EOF", "", 1, 14);
    return stream;
}

static void test_parse_yacc_spec(void) {
    GrammarResult result = cp_parse_yacc_spec("samples/yacc/minic.y");
    assert(result.base.ok);
    assert(strcmp(result.data.start_symbol, "program") == 0);
    assert(result.data.production_count >= 4);
    assert(result.data.token_count >= 7);
}

static void test_first_and_follow(void) {
    GrammarResult grammar = cp_parse_yacc_spec("samples/yacc/minic.y");
    GrammarAnalysisResult analysis = cp_analyze_grammar(&grammar.data);
    int program_index;
    int decl_index;
    assert(analysis.base.ok);
    program_index = cp_grammar_find_symbol(&analysis.data.symbols, "program");
    decl_index = cp_grammar_find_symbol(&analysis.data.symbols, "decl");
    assert(program_index >= 0);
    assert(decl_index >= 0);
    assert(cp_first_set_contains(&analysis.data.first[program_index], "INT"));
    assert(cp_first_set_contains(&analysis.data.follow[program_index], "EOF"));
    assert(cp_first_set_contains(&analysis.data.follow[decl_index], "EOF"));
}

static void test_lr_table(void) {
    GrammarResult grammar = cp_parse_yacc_spec("samples/yacc/minic.y");
    LRTableResult table = cp_build_lr_table(&grammar.data);
    int eof_index;
    int int_index;
    int accept_state = -1;
    int i;
    assert(table.base.ok);
    assert(table.data.state_count > 1);
    eof_index = cp_grammar_find_symbol(&table.data.symbols, "EOF");
    int_index = cp_grammar_find_symbol(&table.data.symbols, "INT");
    assert(eof_index >= 0);
    assert(int_index >= 0);
    for (i = 0; i < table.data.state_count; ++i) {
        if (table.data.action[i][eof_index].kind == CP_ACT_ACCEPT) {
            accept_state = i;
            break;
        }
    }
    assert(accept_state >= 0);
    assert(table.data.action[0][int_index].kind == CP_ACT_SHIFT);
}

static void test_parse_valid_program(void) {
    GrammarResult grammar = cp_parse_yacc_spec("samples/yacc/minic.y");
    TokenStream tokens = make_decl_tokens();
    ASTNodeResult parse = cp_parse_tokens(&grammar.data, &tokens);
    const ParseTrace *trace = cp_last_parse_trace();
    assert(parse.base.ok);
    assert(strcmp(parse.data.symbol_name, "program") == 0);
    assert(parse.data.child_count == 1);
    assert(trace->step_count > 0);
    assert(trace->reduced_count > 0);
}

static void test_parse_syntax_error(void) {
    GrammarResult grammar = cp_parse_yacc_spec("samples/yacc/minic.y");
    TokenStream stream;
    ASTNodeResult parse;
    memset(&stream, 0, sizeof(stream));
    add_token(&stream, "ID", "sum", 1, 1);
    add_token(&stream, "EOF", "", 1, 5);
    parse = cp_parse_tokens(&grammar.data, &stream);
    assert(!parse.base.ok);
    assert(parse.base.errors[0].code == 6002);
}

static void test_parse_two_decls(void) {
    GrammarResult grammar = cp_parse_yacc_spec("samples/yacc/minic.y");
    TokenStream stream;
    ASTNodeResult parse;
    memset(&stream, 0, sizeof(stream));
    add_token(&stream, "INT", "int", 1, 1);
    add_token(&stream, "ID", "a", 1, 5);
    add_token(&stream, "ASSIGN", "=", 1, 7);
    add_token(&stream, "NUM", "1", 1, 9);
    add_token(&stream, "SEMI", ";", 1, 10);
    add_token(&stream, "INT", "int", 2, 1);
    add_token(&stream, "ID", "b", 2, 5);
    add_token(&stream, "ASSIGN", "=", 2, 7);
    add_token(&stream, "NUM", "2", 2, 9);
    add_token(&stream, "SEMI", ";", 2, 10);
    add_token(&stream, "EOF", "", 2, 11);
    parse = cp_parse_tokens(&grammar.data, &stream);
    assert(parse.base.ok);
    assert(strcmp(parse.data.symbol_name, "program") == 0);
}

int main(void) {
    test_parse_yacc_spec();
    test_first_and_follow();
    test_lr_table();
    test_parse_valid_program();
    test_parse_syntax_error();
    test_parse_two_decls();
    return 0;
}
