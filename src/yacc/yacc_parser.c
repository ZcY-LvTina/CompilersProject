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

static int add_non_terminal(Grammar *grammar, const char *name) {
    int i;
    for (i = 0; i < grammar->non_terminal_count; ++i) {
        if (strcmp(grammar->non_terminals[i], name) == 0) {
            return i;
        }
    }
    if (grammar->non_terminal_count >= CP_MAX_GRAMMAR_SYMBOLS) {
        return -1;
    }
    snprintf(grammar->non_terminals[grammar->non_terminal_count], sizeof(grammar->non_terminals[0]), "%s", name);
    return grammar->non_terminal_count++;
}

static void parse_precedence_line(Grammar *grammar, const char *raw) {
    char line[512];
    char assoc[16];
    char *token;
    int level;
    if (grammar == NULL || raw == NULL) {
        return;
    }
    snprintf(line, sizeof(line), "%s", raw);
    if (strncmp(line, "%left", 5) == 0) {
        snprintf(assoc, sizeof(assoc), "left");
        token = strtok(line + 5, " \t");
    } else if (strncmp(line, "%right", 6) == 0) {
        snprintf(assoc, sizeof(assoc), "right");
        token = strtok(line + 6, " \t");
    } else {
        snprintf(assoc, sizeof(assoc), "nonassoc");
        token = strtok(line + 9, " \t");
    }
    level = grammar->precedence_count + 1;
    while (token != NULL && grammar->precedence_count < CP_MAX_GRAMMAR_SYMBOLS) {
        snprintf(grammar->precedence[grammar->precedence_count].symbol, sizeof(grammar->precedence[0].symbol), "%s", token);
        snprintf(grammar->precedence[grammar->precedence_count].assoc, sizeof(grammar->precedence[0].assoc), "%s", assoc);
        grammar->precedence[grammar->precedence_count].level = level;
        grammar->precedence_count++;
        token = strtok(NULL, " \t");
    }
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
            if (strncmp(raw, "%left", 5) == 0 || strncmp(raw, "%right", 6) == 0 || strncmp(raw, "%nonassoc", 9) == 0) {
                parse_precedence_line(&result.data, raw);
                continue;
            }
            continue;
        }
        {
            char *colon = strchr(raw, ':');
            char *rhs_token;
            Production *production;
            char *action_begin;
            if (colon == NULL) {
                CompilerError error;
                cp_make_error(&error, 3001, "Production must contain ':'", line_no, 1, "yacc");
                RESULT_FAILURE(&result.base, error);
                fclose(file);
                return result;
            }
            *colon = '\0';
            production = &result.data.productions[result.data.production_count++];
            memset(production, 0, sizeof(*production));
            production->id = result.data.production_count;
            trim(raw);
            snprintf(production->lhs, sizeof(production->lhs), "%s", raw);
            add_non_terminal(&result.data, production->lhs);
            trim(colon + 1);
            action_begin = strchr(colon + 1, '{');
            if (action_begin != NULL) {
                char *action_end = strrchr(action_begin, '}');
                if (action_end != NULL && action_end > action_begin) {
                    *action_end = '\0';
                    snprintf(production->action_text, sizeof(production->action_text), "%s", action_begin + 1);
                }
                *action_begin = '\0';
                trim(colon + 1);
            }
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
