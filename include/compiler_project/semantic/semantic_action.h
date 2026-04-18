#ifndef COMPILER_PROJECT_SEMANTIC_SEMANTIC_ACTION_H
#define COMPILER_PROJECT_SEMANTIC_SEMANTIC_ACTION_H

#include "compiler_project/common/ast.h"
#include "compiler_project/semantic/symbol_table.h"

SymbolTableResult cp_run_semantic_actions(const ASTNode *ast);

#endif
