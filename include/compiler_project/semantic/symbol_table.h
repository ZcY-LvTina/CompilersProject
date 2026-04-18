#ifndef COMPILER_PROJECT_SEMANTIC_SYMBOL_TABLE_H
#define COMPILER_PROJECT_SEMANTIC_SYMBOL_TABLE_H

#include "compiler_project/common/result.h"
#include "compiler_project/common/symbol.h"

#define CP_MAX_SCOPES 16
#define CP_MAX_SYMBOLS_PER_SCOPE 128

typedef struct {
    SymbolEntry entries[CP_MAX_SYMBOLS_PER_SCOPE];
    int count;
} Scope;

typedef struct {
    Scope scopes[CP_MAX_SCOPES];
    int scope_count;
} SymbolTable;

typedef struct {
    ResultBase base;
    SymbolTable data;
} SymbolTableResult;

void cp_symbol_table_init(SymbolTable *table);
int cp_symbol_table_insert(SymbolTable *table, const SymbolEntry *entry);
const SymbolEntry *cp_symbol_table_lookup(const SymbolTable *table, const char *name);

#endif
