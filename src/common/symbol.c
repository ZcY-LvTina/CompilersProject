#include "compiler_project/common/symbol.h"

const char *cp_symbol_category_name(SymbolCategory category) {
    switch (category) {
        case SYMBOL_VARIABLE: return "VARIABLE";
        case SYMBOL_FUNCTION: return "FUNCTION";
        case SYMBOL_PARAMETER: return "PARAMETER";
        case SYMBOL_TYPEDEF_NAME: return "TYPEDEF_NAME";
        case SYMBOL_FIELD: return "FIELD";
        case SYMBOL_TEMP: return "TEMP";
        case SYMBOL_CONSTANT: return "CONSTANT";
    }
    return "VARIABLE";
}

const char *cp_scope_kind_name(ScopeKind kind) {
    switch (kind) {
        case SCOPE_GLOBAL: return "GLOBAL_SCOPE";
        case SCOPE_FUNCTION: return "FUNCTION_SCOPE";
        case SCOPE_BLOCK: return "BLOCK_SCOPE";
        case SCOPE_STRUCT: return "STRUCT_SCOPE";
        case SCOPE_PARAM: return "PARAM_SCOPE";
    }
    return "GLOBAL_SCOPE";
}
