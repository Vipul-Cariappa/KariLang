cmake_minimum_required(VERSION 3.20)
project(KariLang C)

set(CMAKE_C_STANDARD 11)

find_package(BISON 3.0 REQUIRED)

bison_target(PARSER "src/parser.y" "src/parser.tab.c" DEFINES_FILE "src/parser.tab.h" COMPILE_FLAGS "-Wall -Wcounterexamples")

find_package(FLEX 2.0 REQUIRED)

flex_target(LEXER "src/lexer.l" "src/lex.yy.c")

add_executable(KariLang src/main.c
        src/semantics.c
        src/interpreter.c
        src/parser.tab.c
        src/lex.yy.c
        src/DS.h
        src/common.h
        src/cli_interpreter.h)
