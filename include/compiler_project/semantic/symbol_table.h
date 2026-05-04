#ifndef COMPILER_PROJECT_SEMANTIC_SYMBOL_TABLE_H
#define COMPILER_PROJECT_SEMANTIC_SYMBOL_TABLE_H

#include "compiler_project/common/result.h"
#include "compiler_project/common/symbol.h"

#define CP_MAX_SCOPES 32
#define CP_MAX_SYMBOLS_PER_SCOPE 128

typedef struct {
    int id;
    ScopeKind kind;
    char name[64];
    int parent_scope_id;
    SymbolEntry entries[CP_MAX_SYMBOLS_PER_SCOPE];
    int count;
    int next_offset;
} Scope;

typedef struct {
    Scope scopes[CP_MAX_SCOPES];
    int scope_count;
    int current_scope_index;
    int next_scope_id;
    int next_symbol_id;
} SymbolTable;

typedef struct {
    ResultBase base;
    SymbolTable data;
} SymbolTableResult;

void cp_symbol_table_init(SymbolTable *table);
int cp_symbol_table_enter_scope(SymbolTable *table, ScopeKind kind, const char *name);
void cp_symbol_table_exit_scope(SymbolTable *table);
int cp_symbol_table_insert(SymbolTable *table, const SymbolEntry *entry);
const SymbolEntry *cp_symbol_table_lookup_current(const SymbolTable *table, const char *name);
const SymbolEntry *cp_symbol_table_lookup(const SymbolTable *table, const char *name);
Scope *cp_symbol_table_current_scope(SymbolTable *table);
const Scope *cp_symbol_table_current_scope_const(const SymbolTable *table);

#endif
