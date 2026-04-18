#ifndef COMPILER_PROJECT_COMMON_SYMBOL_H
#define COMPILER_PROJECT_COMMON_SYMBOL_H

#include "compiler_project/common/type.h"

typedef struct {
    char name[64];
    char category[32];
    TypeExpr type_expr;
    int scope_level;
    int offset;
    char attributes[128];
} SymbolEntry;

#endif
