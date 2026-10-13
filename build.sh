#!/bin/bash

if [ ! -d ./build/debug ]; then
    mkdir -p build/debug
fi

CXX=clang++ cmake -B build/debug -S . -G Ninja \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_COLOR_DIAGNOSTICS=ON
