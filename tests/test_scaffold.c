#include <assert.h>
#include <string.h>

#include "compiler_project/common/ast.h"
#include "compiler_project/common/symbol.h"
#include "compiler_project/lex/automata.h"
#include "compiler_project/lex/lex_parser.h"
#include "compiler_project/lex/lexer_runtime.h"
#include "compiler_project/lex/regex_parser.h"
#include "compiler_project/semantic/symbol_table.h"
#include "compiler_project/yacc/yacc_parser.h"

static void test_parse_lex_spec(void) {
    LexSpecResult result = cp_parse_lex_spec("samples/lex/minic.l");
    assert(result.base.ok);
    assert(result.data.rule_count >= 3);
    assert(result.data.rules[0].skip);
    assert(result.data.rules[0].priority == 0);
}

static void test_tokenize_source(void) {
    LexSpecResult spec = cp_parse_lex_spec("samples/lex/minic.l");
    TokenStreamResult result = cp_tokenize_source(&spec.data, "int sum = 12;");
    assert(result.base.ok);
    assert(strcmp(result.data.tokens[0].kind, "INT") == 0);
    assert(strcmp(result.data.tokens[1].kind, "ID") == 0);
    assert(strcmp(result.data.tokens[1].lexeme, "sum") == 0);
    assert(strcmp(result.data.tokens[3].kind, "NUM") == 0);
    assert(strcmp(result.data.tokens[5].kind, "EOF") == 0);
}

static void test_keyword_longest_match(void) {
    LexSpecResult spec = cp_parse_lex_spec("samples/lex/minic.l");
    TokenStreamResult result = cp_tokenize_source(&spec.data, "int intx returnx return");
    assert(result.base.ok);
    assert(strcmp(result.data.tokens[0].kind, "INT") == 0);
    assert(strcmp(result.data.tokens[1].kind, "ID") == 0);
    assert(strcmp(result.data.tokens[1].lexeme, "intx") == 0);
    assert(strcmp(result.data.tokens[2].kind, "ID") == 0);
    assert(strcmp(result.data.tokens[2].lexeme, "returnx") == 0);
    assert(strcmp(result.data.tokens[3].kind, "RETURN") == 0);
}

static void test_token_positions_and_skip(void) {
    LexSpecResult spec = cp_parse_lex_spec("samples/lex/minic.l");
    TokenStreamResult result = cp_tokenize_source(&spec.data, "int\n  sum = 12;");
    assert(result.base.ok);
    assert(result.data.tokens[0].line == 1);
    assert(result.data.tokens[0].column == 1);
    assert(result.data.tokens[1].line == 2);
    assert(result.data.tokens[1].column == 3);
}

static void test_tokenize_error(void) {
    LexSpecResult spec = cp_parse_lex_spec("samples/lex/minic.l");
    TokenStreamResult result = cp_tokenize_source(&spec.data, "int @;");
    assert(!result.base.ok);
    assert(result.base.errors[0].code == 5001);
    assert(result.base.errors[0].line == 1);
    assert(result.base.errors[0].column == 5);
}

static void test_regex_validation_and_dfa(void) {
    LexSpecResult spec = cp_parse_lex_spec("samples/lex/minic.l");
    RegexResult bad = cp_parse_regex("[A-");
    DFAResult dfa = cp_build_dfa_from_lex_spec(&spec.data);
    assert(!bad.base.ok);
    assert(dfa.base.ok);
    assert(dfa.data.state_count > 0);
}

static void test_parse_yacc_spec(void) {
    GrammarResult result = cp_parse_yacc_spec("samples/yacc/minic.y");
    assert(result.base.ok);
    assert(strcmp(result.data.start_symbol, "program") == 0);
}

static void test_symbol_table(void) {
    SymbolTable table;
    SymbolEntry entry;
    cp_symbol_table_init(&table);
    memset(&entry, 0, sizeof(entry));
    strcpy(entry.name, "sum");
    strcpy(entry.category, "variable");
    strcpy(entry.type_expr.name, "int");
    assert(cp_symbol_table_insert(&table, &entry));
    assert(cp_symbol_table_lookup(&table, "sum") != NULL);
}

int main(void) {
    test_parse_lex_spec();
    test_tokenize_source();
    test_keyword_longest_match();
    test_token_positions_and_skip();
    test_tokenize_error();
    test_regex_validation_and_dfa();
    test_parse_yacc_spec();
    test_symbol_table();
    return 0;
}
