#include "compiler_project/common/ir.h"

const char *cp_ir_op_name(IROp op) {
    switch (op) {
        case IR_ADD: return "ADD";
        case IR_SUB: return "SUB";
        case IR_MUL: return "MUL";
        case IR_DIV: return "DIV";
        case IR_MOD: return "MOD";
        case IR_ASSIGN: return "ASSIGN";
        case IR_GOTO: return "GOTO";
        case IR_IF_LT: return "IF_LT";
        case IR_IF_LE: return "IF_LE";
        case IR_IF_GT: return "IF_GT";
        case IR_IF_GE: return "IF_GE";
        case IR_IF_EQ: return "IF_EQ";
        case IR_IF_NE: return "IF_NE";
        case IR_PARAM: return "PARAM";
        case IR_CALL: return "CALL";
        case IR_RET: return "RET";
        case IR_LABEL: return "LABEL";
        case IR_LOAD: return "LOAD";
        case IR_STORE: return "STORE";
        case IR_CAST: return "CAST";
    }
    return "ASSIGN";
}
