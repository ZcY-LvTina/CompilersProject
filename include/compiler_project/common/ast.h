#ifndef COMPILER_PROJECT_COMMON_AST_H
#define COMPILER_PROJECT_COMMON_AST_H

#include "compiler_project/common/result.h"

#define CP_MAX_AST_CHILDREN 16

typedef struct ASTNode {
    char kind[64];
    char value[128];
    struct ASTNode *children[CP_MAX_AST_CHILDREN];
    int child_count;
} ASTNode;

typedef struct {
    ResultBase base;
    ASTNode data;
} ASTNodeResult;

#endif
