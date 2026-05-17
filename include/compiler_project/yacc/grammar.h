#ifndef COMPILER_PROJECT_YACC_GRAMMAR_H
#define COMPILER_PROJECT_YACC_GRAMMAR_H

#include "compiler_project/common/result.h"

#define CP_MAX_GRAMMAR_TOKENS 128
#define CP_MAX_PRODUCTIONS 256
#define CP_MAX_SYMBOLS_PER_RULE 32
#define CP_MAX_GRAMMAR_SYMBOLS 128
#define CP_MAX_SYMBOL_SET_ITEMS 128

typedef struct {
    int id;
    char lhs[64];
    char rhs[CP_MAX_SYMBOLS_PER_RULE][64];
    int rhs_count;
    char action_text[256];
} Production;

typedef struct {
    char symbol[32];
    int level;
    char assoc[16];
} GrammarPrecedence;

typedef struct {
    char tokens[CP_MAX_GRAMMAR_TOKENS][32];
    int token_count;
    Production productions[CP_MAX_PRODUCTIONS];
    int production_count;
    char start_symbol[64];
    char non_terminals[CP_MAX_GRAMMAR_SYMBOLS][32];
    int non_terminal_count;
    GrammarPrecedence precedence[CP_MAX_GRAMMAR_SYMBOLS];
    int precedence_count;
} Grammar;

typedef struct {
    char items[CP_MAX_SYMBOL_SET_ITEMS][32];
    int count;
} GrammarSymbolSet;

typedef struct {
    char names[CP_MAX_GRAMMAR_SYMBOLS][32];
    int count;
} GrammarSymbolTable;

typedef struct {
    GrammarSymbolTable symbols;
    GrammarSymbolSet first[CP_MAX_GRAMMAR_SYMBOLS];
    GrammarSymbolSet follow[CP_MAX_GRAMMAR_SYMBOLS];
} GrammarAnalysis;

typedef struct {
    ResultBase base;
    Grammar data;
} GrammarResult;

typedef struct {
    ResultBase base;
    GrammarAnalysis data;
} GrammarAnalysisResult;

GrammarAnalysisResult cp_analyze_grammar(const Grammar *grammar);
int cp_grammar_find_symbol(const GrammarSymbolTable *symbols, const char *name);
int cp_first_set_contains(const GrammarSymbolSet *set, const char *symbol);

#endif
