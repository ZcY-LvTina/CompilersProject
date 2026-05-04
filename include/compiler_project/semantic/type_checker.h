#ifndef COMPILER_PROJECT_SEMANTIC_TYPE_CHECKER_H
#define COMPILER_PROJECT_SEMANTIC_TYPE_CHECKER_H

#include "compiler_project/common/ast.h"
#include "compiler_project/common/result.h"
#include "compiler_project/common/type.h"
#include "compiler_project/semantic/symbol_table.h"

typedef struct {
    ResultBase base;
    int data;
} BoolResult;

typedef struct {
    ResultBase base;
    TypeExpr data;
} TypeExprResult;

TypeExprResult cp_synthesize_expr_type(const ASTNode *expr, SymbolTable *table);
BoolResult cp_check_assignment(const ASTNode *lhs, const ASTNode *rhs, SymbolTable *table);
BoolResult cp_check_function_call(const ASTNode *call, SymbolTable *table);
BoolResult cp_check_statement(const ASTNode *stmt, SymbolTable *table);
BoolResult cp_check_types(const ASTNode *ast);

#endif
