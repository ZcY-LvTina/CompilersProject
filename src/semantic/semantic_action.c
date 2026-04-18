#include "compiler_project/semantic/semantic_action.h"

#include <string.h>

SymbolTableResult cp_run_semantic_actions(const ASTNode *ast) {
    SymbolTableResult result;
    (void) ast;
    RESULT_SUCCESS(&result.base);
    memset(&result.data, 0, sizeof(result.data));
    cp_symbol_table_init(&result.data);
    return result;
}
