#include "compiler_project/common/error.h"

#include <stdio.h>
#include <string.h>

void cp_make_error(
    CompilerError *error,
    int code,
    const char *message,
    int line,
    int column,
    const char *stage
) {
    if (error == NULL) {
        return;
    }
    error->code = code;
    error->line = line;
    error->column = column;
    snprintf(error->message, sizeof(error->message), "%s", message == NULL ? "" : message);
    snprintf(error->stage, sizeof(error->stage), "%s", stage == NULL ? "unknown" : stage);
}
