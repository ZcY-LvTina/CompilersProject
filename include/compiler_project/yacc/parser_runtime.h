#ifndef COMPILER_PROJECT_YACC_PARSER_RUNTIME_H
#define COMPILER_PROJECT_YACC_PARSER_RUNTIME_H

#include "compiler_project/common/ast.h"
#include "compiler_project/common/token.h"
#include "compiler_project/yacc/grammar.h"

typedef struct {
    char steps[256][256];
    int step_count;
    int reduced_count;
} ParseTrace;

ASTNodeResult cp_parse_tokens(const Grammar *grammar, const TokenStream *stream);
const ParseTrace *cp_last_parse_trace(void);

#endif
