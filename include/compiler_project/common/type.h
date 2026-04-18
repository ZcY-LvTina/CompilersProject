#ifndef COMPILER_PROJECT_COMMON_TYPE_H
#define COMPILER_PROJECT_COMMON_TYPE_H

typedef struct {
    char name[64];
    int width;
    char metadata[128];
} TypeExpr;

#endif
