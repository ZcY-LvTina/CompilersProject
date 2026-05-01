#include "compiler_project/lex/lex_parser.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "compiler_project/common/error.h"

static void trim(char *text) {
    size_t start = 0;
    size_t end = strlen(text);
    while (text[start] != '\0' && isspace((unsigned char) text[start])) {
        start++;
    }
    while (end > start && isspace((unsigned char) text[end - 1])) {
        end--;
    }
    if (start > 0) {
        memmove(text, text + start, end - start);
    }
    text[end - start] = '\0';
}

static int has_capacity(int count, int limit) {
    return count < limit;
}

LexSpecResult cp_parse_lex_spec(const char *path) {
    LexSpecResult result;
    char line[512];
    int line_no = 0;
    FILE *file = fopen(path, "r");

    RESULT_SUCCESS(&result.base);
    memset(&result.data, 0, sizeof(result.data));

    if (file == NULL) {
        CompilerError error;
        cp_make_error(&error, 1000, "Cannot open lex spec file", -1, -1, "lex");
        RESULT_FAILURE(&result.base, error);
        return result;
    }

    while (fgets(line, sizeof(line), file) != NULL) {
        char raw[512];
        char *token;
        line_no++;
        snprintf(raw, sizeof(raw), "%s", line);
        trim(raw);
        if (raw[0] == '\0' || raw[0] == '#') {
            continue;
        }
        if (strncmp(raw, "%define", 7) == 0) {
            char key[64];
            char value[128];
            if (!has_capacity(result.data.definition_count, CP_MAX_LEX_DEFINITIONS)) {
                CompilerError error;
                cp_make_error(&error, 1004, "Too many lex definitions", line_no, 1, "lex");
                RESULT_FAILURE(&result.base, error);
                fclose(file);
                return result;
            }
            if (sscanf(raw, "%%define %63s %127[^\n]", key, value) != 2) {
                CompilerError error;
                cp_make_error(&error, 1001, "Invalid %define syntax", line_no, 1, "lex");
                RESULT_FAILURE(&result.base, error);
                fclose(file);
                return result;
            }
            snprintf(result.data.definitions[result.data.definition_count].key, 64, "%s", key);
            snprintf(result.data.definitions[result.data.definition_count].value, 128, "%s", value);
            result.data.definition_count++;
            continue;
        }

        token = strtok(raw, " \t");
        if (token == NULL) {
            continue;
        }
        if (!has_capacity(result.data.rule_count, CP_MAX_LEX_RULES)) {
            CompilerError error;
            cp_make_error(&error, 1005, "Too many lex rules", line_no, 1, "lex");
            RESULT_FAILURE(&result.base, error);
            fclose(file);
            return result;
        }
        snprintf(result.data.rules[result.data.rule_count].name, 64, "%s", token);
        token = strtok(NULL, " \t");
        if (token == NULL) {
            CompilerError error;
            cp_make_error(&error, 1002, "Missing rule pattern", line_no, 1, "lex");
            RESULT_FAILURE(&result.base, error);
            fclose(file);
            return result;
        }
        snprintf(result.data.rules[result.data.rule_count].pattern, 128, "%s", token);
        token = strtok(NULL, " \t");
        if (token == NULL) {
            CompilerError error;
            cp_make_error(&error, 1003, "Missing token type", line_no, 1, "lex");
            RESULT_FAILURE(&result.base, error);
            fclose(file);
            return result;
        }
        snprintf(result.data.rules[result.data.rule_count].token_type, 32, "%s", token);
        token = strtok(NULL, " \t");
        result.data.rules[result.data.rule_count].skip = token != NULL && strcmp(token, "skip") == 0;
        result.data.rules[result.data.rule_count].priority = result.data.rule_count;
        result.data.rule_count++;
    }

    fclose(file);
    return result;
}
