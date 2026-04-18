#include "compiler_project/semantic/ir_builder.h"

#include <string.h>

QuadrupleListResult cp_build_ir(const ASTNode *ast) {
    QuadrupleListResult result;
    (void) ast;
    RESULT_SUCCESS(&result.base);
    memset(&result.data, 0, sizeof(result.data));
    return result;
}
