#include "compiler_project/lex/automata.h"

#include <stddef.h>

NFAResult cp_build_nfa(const char *regex_ast, const char *token_kind) {
    NFAResult result;
    (void) regex_ast;
    (void) token_kind;
    RESULT_SUCCESS(&result.base);
    result.data.state_count = 2;
    return result;
}

DFAResult cp_determinize(const NFA *nfa) {
    DFAResult result;
    RESULT_SUCCESS(&result.base);
    result.data.state_count = nfa == NULL ? 0 : nfa->state_count;
    return result;
}

DFAResult cp_minimize_dfa(const DFA *dfa) {
    DFAResult result;
    RESULT_SUCCESS(&result.base);
    result.data.state_count = dfa == NULL ? 0 : dfa->state_count;
    return result;
}
