#ifndef COMPILER_PROJECT_COMMON_TOKEN_H
#define COMPILER_PROJECT_COMMON_TOKEN_H

#include "compiler_project/common/result.h"

#define CP_MAX_TOKENS 512

typedef enum {
    TOKEN_IDENTIFIER,
    TOKEN_KEYWORD,
    TOKEN_INTEGER_LITERAL,
    TOKEN_FLOAT_LITERAL,
    TOKEN_CHAR_LITERAL,
    TOKEN_STRING_LITERAL,
    TOKEN_OPERATOR,
    TOKEN_DELIMITER,
    TOKEN_END_OF_FILE,
    TOKEN_UNKNOWN
} TokenType;

typedef struct {
    TokenType type;
    char kind[32];
    char lexeme[128];
    int line;
    int column;
    int length;
    int symbol_id;
    char attr[128];
} Token;

typedef struct {
    Token tokens[CP_MAX_TOKENS];
    int count;
} TokenStream;

typedef struct {
    ResultBase base;
    TokenStream data;
} TokenStreamResult;

const char *cp_token_type_name(TokenType type);

#endif
