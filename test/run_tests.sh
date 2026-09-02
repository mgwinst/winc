#!/bin/bash

clang++ -std=c++23 \
    -I../ \
    -I../winc/core \
    -I/opt/homebrew/include \
    test_main.cpp \
    test_lexer.cpp \
    ../build/debug/libwinc_lib.a \
    && ./a.out
