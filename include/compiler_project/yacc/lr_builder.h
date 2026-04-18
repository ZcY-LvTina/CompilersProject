#ifndef COMPILER_PROJECT_YACC_LR_BUILDER_H
#define COMPILER_PROJECT_YACC_LR_BUILDER_H

#include "compiler_project/common/result.h"
#include "compiler_project/yacc/grammar.h"

typedef struct {
    char states[64][32];
    int state_count;
} LRTable;

typedef struct {
    ResultBase base;
    LRTable data;
} LRTableResult;

LRTableResult cp_build_lr_table(const Grammar *grammar);

#endif
