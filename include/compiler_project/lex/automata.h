#ifndef COMPILER_PROJECT_LEX_AUTOMATA_H
#define COMPILER_PROJECT_LEX_AUTOMATA_H

#include "compiler_project/common/result.h"

typedef struct {
    int state_count;
} NFA;

typedef struct {
    int state_count;
} DFA;

typedef struct {
    ResultBase base;
    NFA data;
} NFAResult;

typedef struct {
    ResultBase base;
    DFA data;
} DFAResult;

NFAResult cp_build_nfa(const char *regex_ast, const char *token_kind);
DFAResult cp_determinize(const NFA *nfa);
DFAResult cp_minimize_dfa(const DFA *dfa);

#endif
