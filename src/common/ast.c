#include "compiler_project/common/ast.h"

#include <stdio.h>
#include <string.h>

void cp_ast_init(ASTNode *node, ASTNodeType node_type, const char *symbol_name, const char *lexeme, int line, int column) {
    if (node == NULL) {
        return;
    }
    memset(node, 0, sizeof(*node));
    node->node_type = node_type;
    node->id = -1;
    node->production_id = -1;
    node->line = line;
    node->column = column;
    node->attr.symbol_id = -1;
    node->attr.quad_begin = -1;
    node->attr.quad_end = -1;
    snprintf(node->symbol_name, sizeof(node->symbol_name), "%s", symbol_name == NULL ? "" : symbol_name);
    snprintf(node->lexeme, sizeof(node->lexeme), "%s", lexeme == NULL ? "" : lexeme);
    snprintf(node->kind, sizeof(node->kind), "%s", node->symbol_name);
    snprintf(node->value, sizeof(node->value), "%s", node->lexeme);
}

int cp_ast_add_child(ASTNode *parent, ASTNode *child) {
    if (parent == NULL || child == NULL || parent->child_count >= CP_MAX_AST_CHILDREN) {
        return 0;
    }
    parent->children[parent->child_count++] = child;
    return 1;
}

const char *cp_ast_node_type_name(ASTNodeType node_type) {
    switch (node_type) {
        case AST_TERMINAL: return "TERMINAL";
        case AST_NON_TERMINAL: return "NON_TERMINAL";
        case AST_ACTION: return "ACTION";
        case AST_PROGRAM: return "PROGRAM";
        case AST_DECLARATION: return "DECLARATION";
        case AST_FUNCTION_DEF: return "FUNCTION_DEF";
        case AST_PARAM_LIST: return "PARAM_LIST";
        case AST_BLOCK: return "BLOCK";
        case AST_STATEMENT: return "STATEMENT";
        case AST_EXPRESSION: return "EXPRESSION";
        case AST_TYPE_NAME: return "TYPE_NAME";
        case AST_IDENTIFIER: return "IDENTIFIER";
        case AST_LITERAL: return "LITERAL";
        case AST_UNKNOWN: return "UNKNOWN";
    }
    return "UNKNOWN";
}
