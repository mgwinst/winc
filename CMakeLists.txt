cmake_minimum_required(VERSION 4.0)

project(winc VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Boost REQUIRED)

add_library(winc_lib STATIC
    winc/driver/driver.cpp
    winc/compiler/compiler.cpp
    winc/frontend/error/diagnostics.cpp
    winc/frontend/lexer/lexer.cpp
    winc/frontend/parser/parser.cpp
    winc/frontend/sema/types/type_table.cpp
    winc/frontend/sema/analyzer.cpp
    winc/middleend/ir/BasicBlock.cpp
    winc/middleend/ir/Function.cpp
    winc/middleend/ir/Instruction.cpp
    winc/middleend/ir/IRProgram.cpp
    winc/middleend/ir/Value.cpp
    winc/middleend/lowering/LoweringEngine.cpp
    winc/analysis/escape.cpp
    winc/analysis/dominator.cpp
    winc/middleend/transforms/trivial_dce.cpp
    winc/middleend/transforms/dse.cpp
    winc/middleend/transforms/cleanup_cfg.cpp
    winc/middleend/transforms/mem2reg.cpp
    winc/middleend/transforms/cse.cpp
    winc/utils/print/print.cpp
)

target_include_directories(winc_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/winc
)

target_link_libraries(winc_lib PUBLIC Boost::boost)

add_executable(${PROJECT_NAME} winc/main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE winc_lib)

target_compile_options(${PROJECT_NAME} PRIVATE
    -march=native -fcolor-diagnostics -Wall
    -Wextra -Wpedantic -Wconversion -Wsign-conversion
    -Wshadow -Wundef -Werror=return-type # -Werror
)

target_compile_options(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:Debug>:
        -O0 -g3
        -fno-omit-frame-pointer
        -fsanitize=address,undefined #leak
        -fsanitize-address-use-after-scope
        -fno-sanitize-recover=all
    >
    $<$<CONFIG:Release>:
        -O3
        -flto=thin # faster incremental builds
        -DNDEBUG
    >
)

target_link_options(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:Debug>:
        -fsanitize=address,undefined #leak
        -fsanitize-address-use-after-scope
        -fno-sanitize-recover=all
    >
    $<$<CONFIG:Release>:
        -flto=thin
    >
)
