#!/bin/bash

clang++ -std=c++23 \
    -I../ \
    -I../winc/core \
    -I/opt/homebrew/include \
    test_main.cpp \
    test_lexer.cpp \
    test_parser_declarations.cpp \
    test_parser_expressions.cpp \
    test_parser_statements.cpp \
    test_type_interning.cpp \
    ../build/debug/libwinc_lib.a \
    && ./a.out
