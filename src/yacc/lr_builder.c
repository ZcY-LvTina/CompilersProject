#include "compiler_project/yacc/lr_builder.h"

#include <stdio.h>

LRTableResult cp_build_lr_table(const Grammar *grammar) {
    LRTableResult result;
    RESULT_SUCCESS(&result.base);
    result.data.state_count = 1;
    snprintf(result.data.states[0], sizeof(result.data.states[0]), "I0");
    (void) grammar;
    return result;
}
