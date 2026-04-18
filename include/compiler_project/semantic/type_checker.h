#ifndef COMPILER_PROJECT_SEMANTIC_TYPE_CHECKER_H
#define COMPILER_PROJECT_SEMANTIC_TYPE_CHECKER_H

#include "compiler_project/common/ast.h"
#include "compiler_project/common/result.h"

typedef struct {
    ResultBase base;
    int data;
} BoolResult;

BoolResult cp_check_types(const ASTNode *ast);

#endif
