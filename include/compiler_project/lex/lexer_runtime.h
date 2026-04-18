#ifndef COMPILER_PROJECT_LEX_LEXER_RUNTIME_H
#define COMPILER_PROJECT_LEX_LEXER_RUNTIME_H

#include "compiler_project/common/token.h"
#include "compiler_project/lex/lex_parser.h"

TokenStreamResult cp_tokenize_source(const LexSpec *spec, const char *source_code);

#endif
