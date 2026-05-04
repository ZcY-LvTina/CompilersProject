#include "compiler_project/common/type.h"

#include <stdio.h>
#include <string.h>

static void cp_type_reset(TypeExpr *type) {
    if (type == NULL) {
        return;
    }
    memset(type, 0, sizeof(*type));
    type->array_size = -1;
}

const char *cp_type_kind_name(TypeKind kind) {
    switch (kind) {
        case TYPE_VOID_T: return "void";
        case TYPE_CHAR_T: return "char";
        case TYPE_SHORT_T: return "short";
        case TYPE_INT_T: return "int";
        case TYPE_LONG_T: return "long";
        case TYPE_FLOAT_T: return "float";
        case TYPE_DOUBLE_T: return "double";
        case TYPE_BOOL_T: return "bool";
        case TYPE_POINTER_T: return "pointer";
        case TYPE_ARRAY_T: return "array";
        case TYPE_FUNCTION_T: return "function";
        case TYPE_STRUCT_T: return "struct";
        case TYPE_ENUM_T: return "enum";
        case TYPE_ERROR_T: return "type_error";
    }
    return "type_error";
}

void cp_type_init_primitive(TypeExpr *type, TypeKind kind) {
    int width = 0;
    cp_type_reset(type);
    if (type == NULL) {
        return;
    }
    type->kind = kind;
    switch (kind) {
        case TYPE_CHAR_T: width = 1; break;
        case TYPE_SHORT_T: width = 2; break;
        case TYPE_INT_T: width = 4; break;
        case TYPE_LONG_T: width = 8; break;
        case TYPE_FLOAT_T: width = 4; break;
        case TYPE_DOUBLE_T: width = 8; break;
        case TYPE_BOOL_T: width = 1; break;
        case TYPE_VOID_T: width = 0; break;
        default: width = 0; break;
    }
    type->width = width;
    snprintf(type->text, sizeof(type->text), "%s", cp_type_kind_name(kind));
}

void cp_type_init_pointer(TypeExpr *type, TypeExpr *base) {
    cp_type_reset(type);
    if (type == NULL) {
        return;
    }
    type->kind = TYPE_POINTER_T;
    type->base = base;
    type->width = 8;
    snprintf(type->text, sizeof(type->text), "pointer(%s)", base == NULL ? "unknown" : base->text);
}

void cp_type_init_array(TypeExpr *type, TypeExpr *base, int array_size) {
    cp_type_reset(type);
    if (type == NULL) {
        return;
    }
    type->kind = TYPE_ARRAY_T;
    type->base = base;
    type->array_size = array_size;
    type->width = (base == NULL || array_size < 0) ? 0 : base->width * array_size;
    snprintf(type->text, sizeof(type->text), "array(%s,%d)", base == NULL ? "unknown" : base->text, array_size);
}

void cp_type_init_function(TypeExpr *type, TypeExpr *return_type, TypeExpr **param_types, int param_count) {
    int index;
    cp_type_reset(type);
    if (type == NULL) {
        return;
    }
    type->kind = TYPE_FUNCTION_T;
    type->return_type = return_type;
    type->param_count = param_count < 0 ? 0 : (param_count > CP_MAX_TYPE_PARAMS ? CP_MAX_TYPE_PARAMS : param_count);
    for (index = 0; index < type->param_count; ++index) {
        type->param_types[index] = param_types == NULL ? NULL : param_types[index];
    }
    snprintf(type->text, sizeof(type->text), "function(%s,%d)", return_type == NULL ? "void" : return_type->text, type->param_count);
}

void cp_type_init_error(TypeExpr *type) {
    cp_type_reset(type);
    if (type == NULL) {
        return;
    }
    type->kind = TYPE_ERROR_T;
    snprintf(type->text, sizeof(type->text), "type_error");
}

int cp_type_is_same(const TypeExpr *lhs, const TypeExpr *rhs) {
    if (lhs == NULL || rhs == NULL) {
        return 0;
    }
    if (lhs->kind != rhs->kind) {
        return 0;
    }
    if (lhs->kind == TYPE_POINTER_T || lhs->kind == TYPE_ARRAY_T) {
        if (lhs->base == NULL || rhs->base == NULL) {
            return lhs->base == rhs->base && lhs->array_size == rhs->array_size;
        }
        return cp_type_is_same(lhs->base, rhs->base) && lhs->array_size == rhs->array_size;
    }
    if (lhs->kind == TYPE_FUNCTION_T) {
        return lhs->param_count == rhs->param_count
            && ((lhs->return_type == NULL && rhs->return_type == NULL)
                || (lhs->return_type != NULL && rhs->return_type != NULL && cp_type_is_same(lhs->return_type, rhs->return_type)));
    }
    return strcmp(lhs->text, rhs->text) == 0;
}

int cp_type_is_numeric(const TypeExpr *type) {
    if (type == NULL) {
        return 0;
    }
    return type->kind == TYPE_CHAR_T
        || type->kind == TYPE_SHORT_T
        || type->kind == TYPE_INT_T
        || type->kind == TYPE_LONG_T
        || type->kind == TYPE_FLOAT_T
        || type->kind == TYPE_DOUBLE_T
        || type->kind == TYPE_BOOL_T;
}

int cp_type_can_assign(const TypeExpr *target, const TypeExpr *source) {
    int target_rank;
    int source_rank;
    if (target == NULL || source == NULL) {
        return 0;
    }
    if (cp_type_is_same(target, source)) {
        return 1;
    }
    if (cp_type_is_numeric(target) && cp_type_is_numeric(source)) {
        switch (target->kind) {
            case TYPE_BOOL_T: target_rank = 0; break;
            case TYPE_CHAR_T: target_rank = 1; break;
            case TYPE_SHORT_T: target_rank = 2; break;
            case TYPE_INT_T: target_rank = 3; break;
            case TYPE_LONG_T: target_rank = 4; break;
            case TYPE_FLOAT_T: target_rank = 5; break;
            case TYPE_DOUBLE_T: target_rank = 6; break;
            default: target_rank = -1; break;
        }
        switch (source->kind) {
            case TYPE_BOOL_T: source_rank = 0; break;
            case TYPE_CHAR_T: source_rank = 1; break;
            case TYPE_SHORT_T: source_rank = 2; break;
            case TYPE_INT_T: source_rank = 3; break;
            case TYPE_LONG_T: source_rank = 4; break;
            case TYPE_FLOAT_T: source_rank = 5; break;
            case TYPE_DOUBLE_T: source_rank = 6; break;
            default: source_rank = -1; break;
        }
        return target_rank >= source_rank;
    }
    return 0;
}
