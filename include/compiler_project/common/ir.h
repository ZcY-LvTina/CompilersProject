#ifndef COMPILER_PROJECT_COMMON_IR_H
#define COMPILER_PROJECT_COMMON_IR_H

#define CP_MAX_QUADRUPLES 256

typedef enum {
    IR_ADD,
    IR_SUB,
    IR_MUL,
    IR_DIV,
    IR_MOD,
    IR_ASSIGN,
    IR_GOTO,
    IR_IF_LT,
    IR_IF_LE,
    IR_IF_GT,
    IR_IF_GE,
    IR_IF_EQ,
    IR_IF_NE,
    IR_PARAM,
    IR_CALL,
    IR_FUNC_BEGIN,
    IR_FUNC_END,
    IR_RET,
    IR_LABEL,
    IR_LOAD,
    IR_STORE,
    IR_CAST
} IROp;

typedef struct {
    int index;
    IROp op;
    char arg1[64];
    char arg2[64];
    char result[64];
} Quadruple;

typedef struct {
    Quadruple items[CP_MAX_QUADRUPLES];
    int count;
} QuadrupleList;

const char *cp_ir_op_name(IROp op);

#endif
