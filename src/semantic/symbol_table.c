#include "compiler_project/semantic/symbol_table.h"

#include <stdio.h>
#include <string.h>

static void cp_scope_init(Scope *scope, int id, ScopeKind kind, const char *name, int parent_scope_id) {
    if (scope == NULL) {
        return;
    }
    memset(scope, 0, sizeof(*scope));
    scope->id = id;
    scope->kind = kind;
    scope->parent_scope_id = parent_scope_id;
    scope->next_offset = 0;
    snprintf(scope->name, sizeof(scope->name), "%s", name == NULL ? "" : name);
}

static int cp_find_scope_index_by_id(const SymbolTable *table, int scope_id) {
    int index;
    if (table == NULL) {
        return -1;
    }
    for (index = 0; index < table->scope_count; ++index) {
        if (table->scopes[index].id == scope_id) {
            return index;
        }
    }
    return -1;
}

Scope *cp_symbol_table_current_scope(SymbolTable *table) {
    if (table == NULL || table->scope_count <= 0 || table->current_scope_index < 0) {
        return NULL;
    }
    return &table->scopes[table->current_scope_index];
}

const Scope *cp_symbol_table_current_scope_const(const SymbolTable *table) {
    if (table == NULL || table->scope_count <= 0 || table->current_scope_index < 0) {
        return NULL;
    }
    return &table->scopes[table->current_scope_index];
}

void cp_symbol_table_init(SymbolTable *table) {
    if (table == NULL) {
        return;
    }
    memset(table, 0, sizeof(*table));
    table->scope_count = 1;
    table->current_scope_index = 0;
    table->next_scope_id = 1;
    table->next_symbol_id = 1;
    cp_scope_init(&table->scopes[0], 0, SCOPE_GLOBAL, "global", -1);
}

int cp_symbol_table_enter_scope(SymbolTable *table, ScopeKind kind, const char *name) {
    Scope *current;
    Scope *next;
    int new_index;
    if (table == NULL || table->scope_count >= CP_MAX_SCOPES) {
        return -1;
    }
    current = cp_symbol_table_current_scope(table);
    new_index = table->scope_count++;
    next = &table->scopes[new_index];
    cp_scope_init(next, table->next_scope_id++, kind, name, current == NULL ? -1 : current->id);
    table->current_scope_index = new_index;
    return next->id;
}

void cp_symbol_table_exit_scope(SymbolTable *table) {
    const Scope *current;
    int index;
    if (table == NULL) {
        return;
    }
    current = cp_symbol_table_current_scope_const(table);
    if (current == NULL || current->parent_scope_id < 0) {
        return;
    }
    for (index = table->scope_count - 1; index >= 0; --index) {
        if (table->scopes[index].id == current->parent_scope_id) {
            table->current_scope_index = index;
            return;
        }
    }
}

int cp_symbol_table_insert(SymbolTable *table, const SymbolEntry *entry) {
    Scope *scope;
    int entry_index;
    if (table == NULL || entry == NULL || table->scope_count <= 0) {
        return 0;
    }
    scope = cp_symbol_table_current_scope(table);
    if (scope == NULL) {
        return 0;
    }
    if (scope->count >= CP_MAX_SYMBOLS_PER_SCOPE) {
        return 0;
    }
    for (entry_index = 0; entry_index < scope->count; ++entry_index) {
        if (strcmp(scope->entries[entry_index].name, entry->name) == 0) {
            return 0;
        }
    }
    scope->entries[scope->count] = *entry;
    scope->entries[scope->count].id = table->next_symbol_id++;
    scope->entries[scope->count].scope_id = scope->id;
    scope->entries[scope->count].offset = scope->next_offset;
    scope->next_offset += entry->type_expr.width;
    ++scope->count;
    return 1;
}

const SymbolEntry *cp_symbol_table_lookup_current(const SymbolTable *table, const char *name) {
    int entry_index;
    const Scope *scope = cp_symbol_table_current_scope_const(table);
    if (scope == NULL || name == NULL) {
        return NULL;
    }
    for (entry_index = 0; entry_index < scope->count; ++entry_index) {
        if (strcmp(scope->entries[entry_index].name, name) == 0) {
            return &scope->entries[entry_index];
        }
    }
    return NULL;
}

const SymbolEntry *cp_symbol_table_lookup(const SymbolTable *table, const char *name) {
    int scope_index;
    if (table == NULL || name == NULL) {
        return NULL;
    }
    scope_index = table->current_scope_index;
    while (scope_index >= 0) {
        int entry_index;
        const Scope *scope = &table->scopes[scope_index];
        for (entry_index = 0; entry_index < scope->count; ++entry_index) {
            if (strcmp(scope->entries[entry_index].name, name) == 0) {
                return &scope->entries[entry_index];
            }
        }
        scope_index = cp_find_scope_index_by_id(table, scope->parent_scope_id);
    }
    return NULL;
}
