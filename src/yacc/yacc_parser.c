#include "compiler_project/yacc/yacc_parser.h"

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

GrammarResult cp_parse_yacc_spec(const char *path) {
    GrammarResult result;
    FILE *file = fopen(path, "r");
    char line[512];
    int line_no = 0;
    int in_rules = 0;

    RESULT_SUCCESS(&result.base);
    memset(&result.data, 0, sizeof(result.data));

    if (file == NULL) {
        CompilerError error;
        cp_make_error(&error, 3000, "Cannot open yacc spec file", -1, -1, "yacc");
        RESULT_FAILURE(&result.base, error);
        return result;
    }

    while (fgets(line, sizeof(line), file) != NULL) {
        char raw[512];
        line_no++;
        snprintf(raw, sizeof(raw), "%s", line);
        trim(raw);
        if (raw[0] == '\0' || raw[0] == '#') {
            continue;
        }
        if (strcmp(raw, "%%") == 0) {
            in_rules = !in_rules;
            continue;
        }
        if (!in_rules) {
            if (strncmp(raw, "%token", 6) == 0) {
                char *token = strtok(raw + 6, " \t");
                while (token != NULL) {
                    snprintf(result.data.tokens[result.data.token_count++], 32, "%s", token);
                    token = strtok(NULL, " \t");
                }
            }
            continue;
        }
        {
            char *colon = strchr(raw, ':');
            char *rhs_token;
            Production *production;
            if (colon == NULL) {
                CompilerError error;
                cp_make_error(&error, 3001, "Production must contain ':'", line_no, 1, "yacc");
                RESULT_FAILURE(&result.base, error);
                fclose(file);
                return result;
            }
            *colon = '\0';
            production = &result.data.productions[result.data.production_count++];
            trim(raw);
            snprintf(production->lhs, sizeof(production->lhs), "%s", raw);
            trim(colon + 1);
            rhs_token = strtok(colon + 1, " \t");
            while (rhs_token != NULL) {
                if (strcmp(rhs_token, ";") != 0) {
                    snprintf(production->rhs[production->rhs_count++], 64, "%s", rhs_token);
                }
                rhs_token = strtok(NULL, " \t");
            }
            if (result.data.start_symbol[0] == '\0') {
                snprintf(result.data.start_symbol, sizeof(result.data.start_symbol), "%s", production->lhs);
            }
        }
    }

    fclose(file);
    return result;
}
