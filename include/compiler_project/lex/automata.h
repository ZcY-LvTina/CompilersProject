#ifndef COMPILER_PROJECT_LEX_AUTOMATA_H
#define COMPILER_PROJECT_LEX_AUTOMATA_H

#include "compiler_project/common/result.h"
#include "compiler_project/lex/lex_parser.h"

#define CP_MAX_AUTOMATA_STATES 256
#define CP_MAX_AUTOMATA_TRANSITIONS 2048
#define CP_AUTOMATA_CHAR_COUNT 128

typedef struct {
    int from;
    int to;
    int min_char;
    int max_char;
    int epsilon;
} AutomataTransition;

typedef struct {
    int accepting;
    int skip;
    int priority;
    char token_kind[32];
} AutomataAccept;

typedef struct {
    int state_count;
    int start_state;
    AutomataTransition transitions[CP_MAX_AUTOMATA_TRANSITIONS];
    int transition_count;
    AutomataAccept accepts[CP_MAX_AUTOMATA_STATES];
} NFA;

typedef struct {
    int state_count;
    int start_state;
    int transitions[CP_MAX_AUTOMATA_STATES][CP_AUTOMATA_CHAR_COUNT];
    AutomataAccept accepts[CP_MAX_AUTOMATA_STATES];
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
NFAResult cp_build_combined_nfa_from_lex_spec(const LexSpec *spec);
DFAResult cp_determinize(const NFA *nfa);
DFAResult cp_minimize_dfa(const DFA *dfa);
DFAResult cp_build_dfa_from_lex_spec(const LexSpec *spec);
void cp_dump_nfa_text(const NFA *nfa, char *buffer, int buffer_size);
void cp_dump_dfa_text(const DFA *dfa, char *buffer, int buffer_size);

#endif
