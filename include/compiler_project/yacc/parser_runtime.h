#ifndef COMPILER_PROJECT_YACC_PARSER_RUNTIME_H
#define COMPILER_PROJECT_YACC_PARSER_RUNTIME_H

#include "compiler_project/common/ast.h"
#include "compiler_project/common/token.h"
#include "compiler_project/yacc/grammar.h"

ASTNodeResult cp_parse_tokens(const Grammar *grammar, const TokenStream *stream);

#endif
