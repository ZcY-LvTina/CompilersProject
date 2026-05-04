#include "compiler_project/semantic/ir_builder.h"

#include <stdio.h>
#include <string.h>

typedef struct {
    QuadrupleList *list;
    int next_temp_id;
} IRContext;

void cp_ir_list_init(QuadrupleList *list) {
    if (list == NULL) {
        return;
    }
    memset(list, 0, sizeof(*list));
}

int cp_ir_emit(QuadrupleList *list, IROp op, const char *arg1, const char *arg2, const char *result) {
    Quadruple *quad;
    if (list == NULL || list->count >= CP_MAX_QUADRUPLES) {
        return -1;
    }
    quad = &list->items[list->count];
    quad->index = 100 + list->count;
    quad->op = op;
    snprintf(quad->arg1, sizeof(quad->arg1), "%s", arg1 == NULL ? "_" : arg1);
    snprintf(quad->arg2, sizeof(quad->arg2), "%s", arg2 == NULL ? "_" : arg2);
    snprintf(quad->result, sizeof(quad->result), "%s", result == NULL ? "_" : result);
    ++list->count;
    return quad->index;
}

static void cp_new_temp(IRContext *ctx, char *buffer, size_t buffer_size) {
    snprintf(buffer, buffer_size, "T%d", ++ctx->next_temp_id);
}

static void cp_gen_expr(const ASTNode *node, IRContext *ctx, char *out_place, size_t out_place_size);

static void cp_gen_stmt(const ASTNode *node, IRContext *ctx) {
    int index;
    char value[64];
    if (node == NULL || ctx == NULL) {
        return;
    }
    if (node->node_type == AST_DECLARATION) {
        if (node->child_count >= 3 && node->children[1] != NULL) {
            cp_gen_expr(node->children[2], ctx, value, sizeof(value));
            cp_ir_emit(ctx->list, IR_ASSIGN, value, "_", node->children[1]->lexeme);
        }
        return;
    }
    if (node->node_type == AST_STATEMENT && strcmp(node->symbol_name, "return") == 0 && node->child_count >= 1) {
        cp_gen_expr(node->children[0], ctx, value, sizeof(value));
        cp_ir_emit(ctx->list, IR_RET, value, "_", "_");
        return;
    }
    if (node->node_type == AST_STATEMENT && strcmp(node->symbol_name, "expr") == 0 && node->child_count >= 1) {
        cp_gen_expr(node->children[0], ctx, value, sizeof(value));
        return;
    }
    for (index = 0; index < node->child_count; ++index) {
        cp_gen_stmt(node->children[index], ctx);
    }
}

static void cp_gen_expr(const ASTNode *node, IRContext *ctx, char *out_place, size_t out_place_size) {
    char lhs[64];
    char rhs[64];
    if (node == NULL || ctx == NULL || out_place == NULL) {
        return;
    }
    if (node->node_type == AST_IDENTIFIER || node->node_type == AST_LITERAL) {
        snprintf(out_place, out_place_size, "%s", node->lexeme);
        return;
    }
    if (node->node_type == AST_EXPRESSION && strcmp(node->symbol_name, "assign") == 0 && node->child_count >= 2) {
        cp_gen_expr(node->children[1], ctx, rhs, sizeof(rhs));
        cp_ir_emit(ctx->list, IR_ASSIGN, rhs, "_", node->children[0]->lexeme);
        snprintf(out_place, out_place_size, "%s", node->children[0]->lexeme);
        return;
    }
    if (node->node_type == AST_EXPRESSION && node->child_count >= 2) {
        IROp op = IR_ADD;
        cp_gen_expr(node->children[0], ctx, lhs, sizeof(lhs));
        cp_gen_expr(node->children[1], ctx, rhs, sizeof(rhs));
        if (strcmp(node->symbol_name, "sub") == 0) op = IR_SUB;
        else if (strcmp(node->symbol_name, "mul") == 0) op = IR_MUL;
        else if (strcmp(node->symbol_name, "div") == 0) op = IR_DIV;
        cp_new_temp(ctx, out_place, out_place_size);
        cp_ir_emit(ctx->list, op, lhs, rhs, out_place);
        return;
    }
    snprintf(out_place, out_place_size, "_");
}

QuadrupleListResult cp_build_ir(const ASTNode *ast) {
    QuadrupleListResult result;
    IRContext ctx;
    RESULT_SUCCESS(&result.base);
    cp_ir_list_init(&result.data);
    ctx.list = &result.data;
    ctx.next_temp_id = 0;
    cp_gen_stmt(ast, &ctx);
    return result;
}
