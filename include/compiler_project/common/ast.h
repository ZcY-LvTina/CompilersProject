#ifndef COMPILER_PROJECT_COMMON_AST_H
#define COMPILER_PROJECT_COMMON_AST_H

#include "compiler_project/common/result.h"

#define CP_MAX_AST_CHILDREN 16

typedef enum {
    AST_TERMINAL,
    AST_NON_TERMINAL,
    AST_ACTION,
    AST_PROGRAM,
    AST_DECLARATION,
    AST_FUNCTION_DEF,
    AST_PARAM_LIST,
    AST_BLOCK,
    AST_STATEMENT,
    AST_EXPRESSION,
    AST_TYPE_NAME,
    AST_IDENTIFIER,
    AST_LITERAL,
    AST_UNKNOWN
} ASTNodeType;

typedef struct {
    char type_name[64];
    char place[64];
    int symbol_id;
    int quad_begin;
    int quad_end;
    int is_lvalue;
} ASTAttribute;

typedef struct ASTNode {
    int id;
    ASTNodeType node_type;
    char symbol_name[64];
    char lexeme[128];
    int production_id;
    int line;
    int column;
    ASTAttribute attr;
    struct ASTNode *children[CP_MAX_AST_CHILDREN];
    int child_count;
    char kind[64];
    char value[128];
} ASTNode;

typedef struct {
    ResultBase base;
    ASTNode data;
} ASTNodeResult;

void cp_ast_init(ASTNode *node, ASTNodeType node_type, const char *symbol_name, const char *lexeme, int line, int column);
int cp_ast_add_child(ASTNode *parent, ASTNode *child);
const char *cp_ast_node_type_name(ASTNodeType node_type);

#endif
