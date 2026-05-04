#ifndef COMPILER_PROJECT_COMMON_SYMBOL_H
#define COMPILER_PROJECT_COMMON_SYMBOL_H

#include "compiler_project/common/type.h"

typedef enum {
    SYMBOL_VARIABLE,
    SYMBOL_FUNCTION,
    SYMBOL_PARAMETER,
    SYMBOL_TYPEDEF_NAME,
    SYMBOL_FIELD,
    SYMBOL_TEMP,
    SYMBOL_CONSTANT
} SymbolCategory;

typedef enum {
    SCOPE_GLOBAL,
    SCOPE_FUNCTION,
    SCOPE_BLOCK,
    SCOPE_STRUCT,
    SCOPE_PARAM
} ScopeKind;

typedef struct {
    int id;
    char name[64];
    SymbolCategory category;
    TypeExpr type_expr;
    int offset;
    int scope_id;
    int decl_line;
    int decl_column;
    int initialized;
    char attributes[128];
} SymbolEntry;

const char *cp_symbol_category_name(SymbolCategory category);
const char *cp_scope_kind_name(ScopeKind kind);

#endif
