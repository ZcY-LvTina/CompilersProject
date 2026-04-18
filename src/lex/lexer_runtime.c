#include "compiler_project/lex/lexer_runtime.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "compiler_project/common/error.h"

static TokenType coarse_type(const char *token_type) {
    if (strcmp(token_type, "ID") == 0) return TOKEN_IDENTIFIER;
    if (strcmp(token_type, "NUM") == 0) return TOKEN_INTEGER_LITERAL;
    if (strcmp(token_type, "INT") == 0 || strcmp(token_type, "RETURN") == 0) return TOKEN_KEYWORD;
    if (strcmp(token_type, "ASSIGN") == 0 || strcmp(token_type, "PLUS") == 0) return TOKEN_OPERATOR;
    if (strcmp(token_type, "SEMI") == 0) return TOKEN_DELIMITER;
    return TOKEN_UNKNOWN;
}

static int is_identifier_start(char ch) {
    return isalpha((unsigned char) ch) || ch == '_';
}

static int is_identifier_char(char ch) {
    return isalnum((unsigned char) ch) || ch == '_';
}

static void push_token(
    TokenStreamResult *result,
    TokenType type,
    const char *kind,
    const char *lexeme,
    int line,
    int column
) {
    Token *token = &result->data.tokens[result->data.count++];
    token->type = type;
    snprintf(token->kind, sizeof(token->kind), "%s", kind);
    snprintf(token->lexeme, sizeof(token->lexeme), "%s", lexeme);
    token->line = line;
    token->column = column;
    token->length = (int) strlen(lexeme);
    token->symbol_id = -1;
    token->attr[0] = '\0';
}

TokenStreamResult cp_tokenize_source(const LexSpec *spec, const char *source_code) {
    TokenStreamResult result;
    size_t i = 0;
    int line = 1;
    int column = 1;

    (void) spec;
    RESULT_SUCCESS(&result.base);
    memset(&result.data, 0, sizeof(result.data));

    while (source_code[i] != '\0') {
        char ch = source_code[i];
        int start_column = column;
        if (isspace((unsigned char) ch)) {
            if (ch == '\n') {
                line++;
                column = 1;
            } else {
                column++;
            }
            i++;
            continue;
        }
        if (strncmp(&source_code[i], "int", 3) == 0 && !is_identifier_char(source_code[i + 3])) {
            push_token(&result, coarse_type("INT"), "INT", "int", line, start_column);
            i += 3;
            column += 3;
            continue;
        }
        if (strncmp(&source_code[i], "return", 6) == 0 && !is_identifier_char(source_code[i + 6])) {
            push_token(&result, coarse_type("RETURN"), "RETURN", "return", line, start_column);
            i += 6;
            column += 6;
            continue;
        }
        if (is_identifier_start(ch)) {
            char lexeme[128];
            size_t j = 0;
            while (is_identifier_char(source_code[i]) && j + 1 < sizeof(lexeme)) {
                lexeme[j++] = source_code[i++];
                column++;
            }
            lexeme[j] = '\0';
            push_token(&result, coarse_type("ID"), "ID", lexeme, line, start_column);
            continue;
        }
        if (isdigit((unsigned char) ch)) {
            char lexeme[128];
            size_t j = 0;
            while (isdigit((unsigned char) source_code[i]) && j + 1 < sizeof(lexeme)) {
                lexeme[j++] = source_code[i++];
                column++;
            }
            lexeme[j] = '\0';
            push_token(&result, coarse_type("NUM"), "NUM", lexeme, line, start_column);
            continue;
        }
        if (ch == '=') {
            push_token(&result, coarse_type("ASSIGN"), "ASSIGN", "=", line, start_column);
            i++;
            column++;
            continue;
        }
        if (ch == '+') {
            push_token(&result, coarse_type("PLUS"), "PLUS", "+", line, start_column);
            i++;
            column++;
            continue;
        }
        if (ch == ';') {
            push_token(&result, coarse_type("SEMI"), "SEMI", ";", line, start_column);
            i++;
            column++;
            continue;
        }
        {
            CompilerError error;
            cp_make_error(&error, 5001, "Unexpected character in source", line, column, "lex-runtime");
            RESULT_FAILURE(&result.base, error);
            return result;
        }
    }

    push_token(&result, TOKEN_END_OF_FILE, "EOF", "", line, column);
    return result;
}
