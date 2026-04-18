cmake_minimum_required(VERSION 3.16)
project(compiler_project C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

add_library(compiler_core
    src/common/error.c
    src/common/token.c
    src/common/ast.c
    src/common/type.c
    src/common/symbol.c
    src/common/ir.c
    src/lex/lex_parser.c
    src/lex/regex_parser.c
    src/lex/automata.c
    src/lex/lexer_runtime.c
    src/yacc/grammar.c
    src/yacc/yacc_parser.c
    src/yacc/lr_builder.c
    src/yacc/parser_runtime.c
    src/semantic/symbol_table.c
    src/semantic/type_checker.c
    src/semantic/semantic_action.c
    src/semantic/ir_builder.c
)

target_include_directories(compiler_core PUBLIC include)

add_executable(compiler_project src/main/main.c)
target_link_libraries(compiler_project PRIVATE compiler_core)

enable_testing()

add_executable(test_scaffold tests/test_scaffold.c)
target_link_libraries(test_scaffold PRIVATE compiler_core)
target_include_directories(test_scaffold PRIVATE include)

add_test(NAME scaffold_tests COMMAND test_scaffold)
