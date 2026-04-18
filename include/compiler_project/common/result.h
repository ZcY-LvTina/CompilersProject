#ifndef COMPILER_PROJECT_COMMON_RESULT_H
#define COMPILER_PROJECT_COMMON_RESULT_H

#include "compiler_project/common/error.h"

typedef struct {
    int ok;
    int warning_count;
    char warnings[8][128];
    CompilerError errors[8];
    int error_count;
} ResultBase;

#define RESULT_SUCCESS(result_ptr) \
    do { \
        (result_ptr)->ok = 1; \
        (result_ptr)->warning_count = 0; \
        (result_ptr)->error_count = 0; \
    } while (0)

#define RESULT_FAILURE(result_ptr, error_value) \
    do { \
        (result_ptr)->ok = 0; \
        (result_ptr)->warning_count = 0; \
        (result_ptr)->errors[0] = (error_value); \
        (result_ptr)->error_count = 1; \
    } while (0)

#endif
