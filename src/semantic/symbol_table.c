#include "compiler_project/semantic/symbol_table.h"

#include <string.h>

void cp_symbol_table_init(SymbolTable *table) {
    if (table == NULL) {
        return;
    }
    memset(table, 0, sizeof(*table));
    table->scope_count = 1;
}

int cp_symbol_table_insert(SymbolTable *table, const SymbolEntry *entry) {
    Scope *scope;
    if (table == NULL || entry == NULL || table->scope_count <= 0) {
        return 0;
    }
    scope = &table->scopes[table->scope_count - 1];
    if (scope->count >= CP_MAX_SYMBOLS_PER_SCOPE) {
        return 0;
    }
    scope->entries[scope->count++] = *entry;
    return 1;
}

const SymbolEntry *cp_symbol_table_lookup(const SymbolTable *table, const char *name) {
    int scope_index;
    if (table == NULL || name == NULL) {
        return NULL;
    }
    for (scope_index = table->scope_count - 1; scope_index >= 0; --scope_index) {
        int entry_index;
        const Scope *scope = &table->scopes[scope_index];
        for (entry_index = 0; entry_index < scope->count; ++entry_index) {
            if (strcmp(scope->entries[entry_index].name, name) == 0) {
                return &scope->entries[entry_index];
            }
        }
    }
    return NULL;
}
