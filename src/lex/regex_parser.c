#include "compiler_project/lex/regex_parser.h"

#include <stdio.h>
#include <string.h>

#include "compiler_project/common/error.h"

static int validate_regex(const char *text, CompilerError *error) {
    int depth = 0;
    int in_class = 0;
    size_t i;
    if (text == NULL || text[0] == '\0') {
        cp_make_error(error, 2000, "Empty regex", -1, -1, "regex");
        return 0;
    }
    for (i = 0; text[i] != '\0'; ++i) {
        char ch = text[i];
        if (ch == '\\') {
            if (text[i + 1] == '\0') {
                cp_make_error(error, 2001, "Dangling escape in regex", -1, (int) i + 1, "regex");
                return 0;
            }
            i++;
            continue;
        }
        if (in_class) {
            if (ch == ']') {
                in_class = 0;
            }
            continue;
        }
        if (ch == '[') {
            in_class = 1;
            continue;
        }
        if (ch == '(') {
            depth++;
            continue;
        }
        if (ch == ')') {
            depth--;
            if (depth < 0) {
                cp_make_error(error, 2002, "Unmatched ')' in regex", -1, (int) i + 1, "regex");
                return 0;
            }
        }
    }
    if (in_class) {
        cp_make_error(error, 2003, "Unclosed character class in regex", -1, -1, "regex");
        return 0;
    }
    if (depth != 0) {
        cp_make_error(error, 2004, "Unclosed group in regex", -1, -1, "regex");
        return 0;
    }
    return 1;
}

RegexResult cp_parse_regex(const char *regex_text) {
    RegexResult result;
    CompilerError error;
    RESULT_SUCCESS(&result.base);
    memset(&result.data, 0, sizeof(result.data));
    if (!validate_regex(regex_text, &error)) {
        RESULT_FAILURE(&result.base, error);
        return result;
    }
    snprintf(result.data, sizeof(result.data), "%s", regex_text == NULL ? "" : regex_text);
    return result;
}
