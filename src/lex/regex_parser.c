#include "compiler_project/lex/regex_parser.h"

#include <stdio.h>
#include <string.h>

RegexResult cp_parse_regex(const char *regex_text) {
    RegexResult result;
    RESULT_SUCCESS(&result.base);
    snprintf(result.data, sizeof(result.data), "%s", regex_text == NULL ? "" : regex_text);
    return result;
}
