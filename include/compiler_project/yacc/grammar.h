#ifndef COMPILER_PROJECT_YACC_GRAMMAR_H
#define COMPILER_PROJECT_YACC_GRAMMAR_H

#include "compiler_project/common/result.h"

#define CP_MAX_GRAMMAR_TOKENS 128
#define CP_MAX_PRODUCTIONS 256
#define CP_MAX_SYMBOLS_PER_RULE 32

typedef struct {
    char lhs[64];
    char rhs[CP_MAX_SYMBOLS_PER_RULE][64];
    int rhs_count;
} Production;

typedef struct {
    char tokens[CP_MAX_GRAMMAR_TOKENS][32];
    int token_count;
    Production productions[CP_MAX_PRODUCTIONS];
    int production_count;
    char start_symbol[64];
} Grammar;

typedef struct {
    ResultBase base;
    Grammar data;
} GrammarResult;

#endif
