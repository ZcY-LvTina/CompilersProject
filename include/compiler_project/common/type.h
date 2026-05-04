#ifndef COMPILER_PROJECT_COMMON_TYPE_H
#define COMPILER_PROJECT_COMMON_TYPE_H

#define CP_MAX_TYPE_TEXT 64
#define CP_MAX_TYPE_PARAMS 8

typedef enum {
    TYPE_VOID_T,
    TYPE_CHAR_T,
    TYPE_SHORT_T,
    TYPE_INT_T,
    TYPE_LONG_T,
    TYPE_FLOAT_T,
    TYPE_DOUBLE_T,
    TYPE_BOOL_T,
    TYPE_POINTER_T,
    TYPE_ARRAY_T,
    TYPE_FUNCTION_T,
    TYPE_STRUCT_T,
    TYPE_ENUM_T,
    TYPE_ERROR_T
} TypeKind;

typedef struct TypeExpr {
    TypeKind kind;
    char text[CP_MAX_TYPE_TEXT];
    struct TypeExpr *base;
    int array_size;
    struct TypeExpr *param_types[CP_MAX_TYPE_PARAMS];
    int param_count;
    struct TypeExpr *return_type;
    int width;
} TypeExpr;

void cp_type_init_primitive(TypeExpr *type, TypeKind kind);
void cp_type_init_pointer(TypeExpr *type, TypeExpr *base);
void cp_type_init_array(TypeExpr *type, TypeExpr *base, int array_size);
void cp_type_init_function(TypeExpr *type, TypeExpr *return_type, TypeExpr **param_types, int param_count);
void cp_type_init_error(TypeExpr *type);
int cp_type_is_same(const TypeExpr *lhs, const TypeExpr *rhs);
int cp_type_is_numeric(const TypeExpr *type);
int cp_type_can_assign(const TypeExpr *target, const TypeExpr *source);
const char *cp_type_kind_name(TypeKind kind);

#endif
