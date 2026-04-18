#ifndef COMPILER_PROJECT_COMMON_ERROR_H
#define COMPILER_PROJECT_COMMON_ERROR_H

typedef struct {
    int code;
    char message[256];
    int line;
    int column;
    char stage[32];
} CompilerError;

void cp_make_error(
    CompilerError *error,
    int code,
    const char *message,
    int line,
    int column,
    const char *stage
);

#endif
