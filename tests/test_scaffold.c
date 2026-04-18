#include <assert.h>
#include <string.h>

#include "compiler_project/common/ast.h"
#include "compiler_project/common/symbol.h"
#include "compiler_project/lex/lex_parser.h"
#include "compiler_project/lex/lexer_runtime.h"
#include "compiler_project/semantic/symbol_table.h"
#include "compiler_project/yacc/yacc_parser.h"

static void test_parse_lex_spec(void) {
    LexSpecResult result = cp_parse_lex_spec("samples/lex/minic.l");
    assert(result.base.ok);
    assert(result.data.rule_count >= 3);
}

static void test_tokenize_source(void) {
    LexSpecResult spec = cp_parse_lex_spec("samples/lex/minic.l");
    TokenStreamResult result = cp_tokenize_source(&spec.data, "int sum = 12;");
    assert(result.base.ok);
    assert(strcmp(result.data.tokens[0].kind, "INT") == 0);
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
    test_parse_yacc_spec();
    test_symbol_table();
    return 0;
}
