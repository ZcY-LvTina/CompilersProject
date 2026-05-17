#ifndef COMPILER_PROJECT_YACC_LR_BUILDER_H
#define COMPILER_PROJECT_YACC_LR_BUILDER_H

#include "compiler_project/common/result.h"
#include "compiler_project/yacc/grammar.h"

#define CP_MAX_LR_STATES 256

typedef enum {
    CP_ACT_ERROR,
    CP_ACT_SHIFT,
    CP_ACT_REDUCE,
    CP_ACT_ACCEPT
} LRActionKind;

typedef struct {
    LRActionKind kind;
    int target;
} LRAction;

typedef struct {
    GrammarSymbolTable symbols;
    char states[CP_MAX_LR_STATES][32];
    int state_count;
    LRAction action[CP_MAX_LR_STATES][CP_MAX_GRAMMAR_SYMBOLS];
    int go_to[CP_MAX_LR_STATES][CP_MAX_GRAMMAR_SYMBOLS];
} LRTable;

typedef struct {
    ResultBase base;
    LRTable data;
} LRTableResult;

LRTableResult cp_build_lr_table(const Grammar *grammar);

#endif
