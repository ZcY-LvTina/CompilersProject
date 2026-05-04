#ifndef COMPILER_PROJECT_SEMANTIC_IR_BUILDER_H
#define COMPILER_PROJECT_SEMANTIC_IR_BUILDER_H

#include "compiler_project/common/ast.h"
#include "compiler_project/common/ir.h"
#include "compiler_project/common/result.h"

typedef struct {
    ResultBase base;
    QuadrupleList data;
} QuadrupleListResult;

void cp_ir_list_init(QuadrupleList *list);
int cp_ir_emit(QuadrupleList *list, IROp op, const char *arg1, const char *arg2, const char *result);
QuadrupleListResult cp_build_ir(const ASTNode *ast);

#endif
