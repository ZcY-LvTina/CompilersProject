#include "compiler_project/semantic/semantic_action.h"

#include <stdio.h>
#include <string.h>

static TypeKind cp_type_kind_from_name(const char *name) {
    if (name == NULL) {
        return TYPE_ERROR_T;
    }
    if (strcmp(name, "void") == 0) return TYPE_VOID_T;
    if (strcmp(name, "char") == 0) return TYPE_CHAR_T;
    if (strcmp(name, "short") == 0) return TYPE_SHORT_T;
    if (strcmp(name, "int") == 0) return TYPE_INT_T;
    if (strcmp(name, "long") == 0) return TYPE_LONG_T;
    if (strcmp(name, "float") == 0) return TYPE_FLOAT_T;
    if (strcmp(name, "double") == 0) return TYPE_DOUBLE_T;
    if (strcmp(name, "bool") == 0) return TYPE_BOOL_T;
    return TYPE_ERROR_T;
}

static TypeExpr *cp_shared_primitive_type(TypeKind kind) {
    static TypeExpr cache[8];
    static int initialized[8];
    if (kind < TYPE_VOID_T || kind > TYPE_BOOL_T) {
        return NULL;
    }
    if (!initialized[kind]) {
        cp_type_init_primitive(&cache[kind], kind);
        initialized[kind] = 1;
    }
    return &cache[kind];
}

static void cp_type_from_type_node(const ASTNode *type_node, TypeExpr *type) {
    TypeKind kind = cp_type_kind_from_name(type_node == NULL ? NULL : type_node->lexeme);
    if (kind == TYPE_ERROR_T) {
        cp_type_init_error(type);
        return;
    }
    cp_type_init_primitive(type, kind);
}

static int cp_insert_declaration(SymbolTable *table, const ASTNode *decl, SymbolCategory category) {
    SymbolEntry entry;
    if (table == NULL || decl == NULL || decl->child_count < 2) {
        return 1;
    }
    if (decl->children[0] == NULL || decl->children[1] == NULL) {
        return 1;
    }
    memset(&entry, 0, sizeof(entry));
    snprintf(entry.name, sizeof(entry.name), "%s", decl->children[1]->lexeme);
    entry.category = category;
    cp_type_from_type_node(decl->children[0], &entry.type_expr);
    entry.decl_line = decl->line;
    entry.decl_column = decl->column;
    entry.initialized = decl->child_count >= 3;
    return cp_symbol_table_insert(table, &entry);
}

static int cp_walk_semantic(const ASTNode *node, SymbolTable *table) {
    int index;
    if (node == NULL || table == NULL) {
        return 1;
    }
    if (node->node_type == AST_DECLARATION) {
        return cp_insert_declaration(table, node, SYMBOL_VARIABLE);
    }
    if (node->node_type == AST_FUNCTION_DEF) {
        SymbolEntry fn_entry;
        const ASTNode *param_list = node->child_count > 2 ? node->children[2] : NULL;
        TypeExpr *param_ptrs[CP_MAX_TYPE_PARAMS];
        int param_count = 0;
        TypeKind return_kind = TYPE_VOID_T;
        memset(&fn_entry, 0, sizeof(fn_entry));
        if (node->child_count >= 2 && node->children[0] != NULL && node->children[1] != NULL) {
            return_kind = cp_type_kind_from_name(node->children[0]->lexeme);
            if (param_list != NULL && param_list->node_type == AST_PARAM_LIST) {
                for (index = 0; index < param_list->child_count && index < CP_MAX_TYPE_PARAMS; ++index) {
                    param_ptrs[index] = cp_shared_primitive_type(
                        cp_type_kind_from_name(param_list->children[index]->children[0]->lexeme)
                    );
                    ++param_count;
                }
            }
            snprintf(fn_entry.name, sizeof(fn_entry.name), "%s", node->children[1]->lexeme);
            fn_entry.category = SYMBOL_FUNCTION;
            cp_type_init_function(&fn_entry.type_expr, cp_shared_primitive_type(return_kind), param_ptrs, param_count);
            fn_entry.decl_line = node->line;
            fn_entry.decl_column = node->column;
            if (!cp_symbol_table_insert(table, &fn_entry)) {
                return 0;
            }
        }
        cp_symbol_table_enter_scope(table, SCOPE_FUNCTION, node->child_count > 1 ? node->children[1]->lexeme : "function");
        if (param_list != NULL && param_list->node_type == AST_PARAM_LIST) {
            for (index = 0; index < param_list->child_count; ++index) {
                if (!cp_insert_declaration(table, param_list->children[index], SYMBOL_PARAMETER)) {
                    cp_symbol_table_exit_scope(table);
                    return 0;
                }
            }
        }
        for (index = 3; index < node->child_count; ++index) {
            if (!cp_walk_semantic(node->children[index], table)) {
                cp_symbol_table_exit_scope(table);
                return 0;
            }
        }
        cp_symbol_table_exit_scope(table);
        return 1;
    }
    if (node->node_type == AST_BLOCK) {
        cp_symbol_table_enter_scope(table, SCOPE_BLOCK, node->symbol_name[0] == '\0' ? "block" : node->symbol_name);
        for (index = 0; index < node->child_count; ++index) {
            if (!cp_walk_semantic(node->children[index], table)) {
                cp_symbol_table_exit_scope(table);
                return 0;
            }
        }
        cp_symbol_table_exit_scope(table);
        return 1;
    }
    for (index = 0; index < node->child_count; ++index) {
        if (!cp_walk_semantic(node->children[index], table)) {
            return 0;
        }
    }
    return 1;
}

SymbolTableResult cp_run_semantic_actions(const ASTNode *ast) {
    SymbolTableResult result;
    RESULT_SUCCESS(&result.base);
    memset(&result.data, 0, sizeof(result.data));
    cp_symbol_table_init(&result.data);
    if (ast != NULL && !cp_walk_semantic(ast, &result.data)) {
        CompilerError error;
        cp_make_error(&error, 7001, "symbol redefinition in current scope", ast->line, ast->column, "semantic");
        RESULT_FAILURE(&result.base, error);
    }
    return result;
}
