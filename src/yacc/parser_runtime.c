#include "compiler_project/yacc/parser_runtime.h"

#include <stdio.h>
#include <string.h>

ASTNodeResult cp_parse_tokens(const Grammar *grammar, const TokenStream *stream) {
    ASTNodeResult result;
    RESULT_SUCCESS(&result.base);
    cp_ast_init(
        &result.data,
        AST_PROGRAM,
        grammar != NULL && grammar->start_symbol[0] != '\0' ? grammar->start_symbol : "Program",
        "",
        1,
        1
    );
    snprintf(result.data.value, sizeof(result.data.value), "token_count=%d", stream == NULL ? 0 : stream->count);
    return result;
}
