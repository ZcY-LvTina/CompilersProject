#include "compiler_project/lex/lexer_runtime.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "compiler_project/common/error.h"
#include "compiler_project/lex/automata.h"

static TokenType coarse_type(const char *token_type) {
    if (strcmp(token_type, "ID") == 0) return TOKEN_IDENTIFIER;
    if (strcmp(token_type, "NUM") == 0) return TOKEN_INTEGER_LITERAL;
    if (strcmp(token_type, "INT") == 0 || strcmp(token_type, "RETURN") == 0) return TOKEN_KEYWORD;
    if (strcmp(token_type, "ASSIGN") == 0 || strcmp(token_type, "PLUS") == 0) return TOKEN_OPERATOR;
    if (strcmp(token_type, "SEMI") == 0) return TOKEN_DELIMITER;
    if (strcmp(token_type, "EOF") == 0) return TOKEN_END_OF_FILE;
    return TOKEN_UNKNOWN;
}

static void advance_position(const char *text, int length, int *line, int *column) {
    int i;
    for (i = 0; i < length; ++i) {
        if (text[i] == '\n') {
            (*line)++;
            *column = 1;
        } else {
            (*column)++;
        }
    }
}

static int append_token(
    TokenStreamResult *result,
    const char *kind,
    const char *lexeme_start,
    int lexeme_length,
    int line,
    int column
) {
    Token *token;
    if (result->data.count >= CP_MAX_TOKENS) {
        CompilerError error;
        cp_make_error(&error, 5002, "Too many tokens", line, column, "lex-runtime");
        RESULT_FAILURE(&result->base, error);
        return 0;
    }
    token = &result->data.tokens[result->data.count++];
    memset(token, 0, sizeof(*token));
    token->type = coarse_type(kind);
    snprintf(token->kind, sizeof(token->kind), "%s", kind);
    snprintf(token->lexeme, sizeof(token->lexeme), "%.*s", lexeme_length, lexeme_start);
    token->line = line;
    token->column = column;
    token->length = lexeme_length;
    token->symbol_id = -1;
    if (token->type == TOKEN_INTEGER_LITERAL) {
        char value[128];
        snprintf(value, sizeof(value), "%.*s", lexeme_length, lexeme_start);
        snprintf(token->attr, sizeof(token->attr), "value=%.121s", value);
    }
    return 1;
}

TokenStreamResult cp_tokenize_source(const LexSpec *spec, const char *source_code) {
    TokenStreamResult result;
    DFAResult dfa_result;
    size_t i = 0;
    int line = 1;
    int column = 1;

    RESULT_SUCCESS(&result.base);
    memset(&result.data, 0, sizeof(result.data));

    if (source_code == NULL) {
        CompilerError error;
        cp_make_error(&error, 5000, "Missing source code", -1, -1, "lex-runtime");
        RESULT_FAILURE(&result.base, error);
        return result;
    }

    dfa_result = cp_build_dfa_from_lex_spec(spec);
    if (!dfa_result.base.ok) {
        RESULT_FAILURE(&result.base, dfa_result.base.errors[0]);
        return result;
    }

    while (source_code[i] != '\0') {
        int state = dfa_result.data.start_state;
        int last_accept_state = -1;
        int last_accept_length = 0;
        int length = 0;
        int start_line = line;
        int start_column = column;

        /* Walk until the DFA has no transition, remembering the best accepting prefix. */
        while (source_code[i + length] != '\0') {
            unsigned char ch = (unsigned char) source_code[i + length];
            int next;
            if (ch >= CP_AUTOMATA_CHAR_COUNT) {
                break;
            }
            next = dfa_result.data.transitions[state][ch];
            if (next < 0) {
                break;
            }
            state = next;
            length++;
            if (dfa_result.data.accepts[state].accepting) {
                last_accept_state = state;
                last_accept_length = length;
            }
        }

        if (last_accept_state < 0) {
            CompilerError error;
            cp_make_error(&error, 5001, "Unexpected character in source", line, column, "lex-runtime");
            RESULT_FAILURE(&result.base, error);
            return result;
        }

        if (!dfa_result.data.accepts[last_accept_state].skip) {
            if (!append_token(
                &result,
                dfa_result.data.accepts[last_accept_state].token_kind,
                &source_code[i],
                last_accept_length,
                start_line,
                start_column
            )) {
                return result;
            }
        }
        advance_position(&source_code[i], last_accept_length, &line, &column);
        i += (size_t) last_accept_length;
    }

    append_token(&result, "EOF", "", 0, line, column);
    return result;
}
