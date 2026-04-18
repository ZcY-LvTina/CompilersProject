#include "compiler_project/common/token.h"

const char *cp_token_type_name(TokenType type) {
    switch (type) {
        case TOKEN_IDENTIFIER: return "IDENTIFIER";
        case TOKEN_KEYWORD: return "KEYWORD";
        case TOKEN_INTEGER_LITERAL: return "INTEGER_LITERAL";
        case TOKEN_FLOAT_LITERAL: return "FLOAT_LITERAL";
        case TOKEN_CHAR_LITERAL: return "CHAR_LITERAL";
        case TOKEN_STRING_LITERAL: return "STRING_LITERAL";
        case TOKEN_OPERATOR: return "OPERATOR";
        case TOKEN_DELIMITER: return "DELIMITER";
        case TOKEN_END_OF_FILE: return "END_OF_FILE";
        case TOKEN_UNKNOWN: return "UNKNOWN";
    }
    return "UNKNOWN";
}
