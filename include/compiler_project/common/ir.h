#ifndef COMPILER_PROJECT_COMMON_IR_H
#define COMPILER_PROJECT_COMMON_IR_H

#define CP_MAX_QUADRUPLES 256

typedef struct {
    char op[32];
    char arg1[64];
    char arg2[64];
    char result[64];
} Quadruple;

typedef struct {
    Quadruple items[CP_MAX_QUADRUPLES];
    int count;
} QuadrupleList;

#endif
