#ifndef COMPILER_PROJECT_LEX_REGEX_PARSER_H
#define COMPILER_PROJECT_LEX_REGEX_PARSER_H

#include "compiler_project/common/result.h"

typedef struct {
    ResultBase base;
    char data[128];
} RegexResult;

RegexResult cp_parse_regex(const char *regex_text);

#endif
