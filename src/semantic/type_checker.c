#include "compiler_project/semantic/type_checker.h"

#include <stddef.h>

BoolResult cp_check_types(const ASTNode *ast) {
    BoolResult result;
    RESULT_SUCCESS(&result.base);
    result.data = ast != NULL;
    return result;
}
