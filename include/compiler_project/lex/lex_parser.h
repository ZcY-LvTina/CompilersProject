#ifndef COMPILER_PROJECT_LEX_LEX_PARSER_H
#define COMPILER_PROJECT_LEX_LEX_PARSER_H

#include "compiler_project/common/result.h"

#define CP_MAX_LEX_RULES 128
#define CP_MAX_LEX_DEFINITIONS 64

typedef struct {
    char key[64];
    char value[128];
} LexDefinition;

typedef struct {
    char name[64];
    char pattern[128];
    char token_type[32];
    int skip;
} LexRule;

typedef struct {
    LexDefinition definitions[CP_MAX_LEX_DEFINITIONS];
    int definition_count;
    LexRule rules[CP_MAX_LEX_RULES];
    int rule_count;
} LexSpec;

typedef struct {
    ResultBase base;
    LexSpec data;
} LexSpecResult;

LexSpecResult cp_parse_lex_spec(const char *path);

#endif
