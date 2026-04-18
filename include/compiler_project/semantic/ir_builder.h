#ifndef COMPILER_PROJECT_SEMANTIC_IR_BUILDER_H
#define COMPILER_PROJECT_SEMANTIC_IR_BUILDER_H

#include "compiler_project/common/ast.h"
#include "compiler_project/common/ir.h"
#include "compiler_project/common/result.h"

typedef struct {
    ResultBase base;
    QuadrupleList data;
} QuadrupleListResult;

QuadrupleListResult cp_build_ir(const ASTNode *ast);

#endif
