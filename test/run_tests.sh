#!/bin/bash

clang++ -std=c++23 \
    -I../ \
    -I../winc \
    -I/opt/homebrew/include \
    test_main.cpp \
    test_lexer.cpp \
    test_parser_declarations.cpp \
    test_parser_statements.cpp \
    test_parser_expressions.cpp \
    test_type_interning.cpp \
    test_def_use.cpp \
    test_basic_block.cpp \
    test_lowering.cpp \
    test_trivial_dce.cpp \
    test_dse.cpp \
    ../build/debug/libwinc_lib.a \
    && ./a.out
