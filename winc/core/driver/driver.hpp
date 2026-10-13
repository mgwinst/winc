#pragma once

#include <vector>

#include "compiler/compile_options.hpp"

// Compilation orchestrator (target, options, pipeline, toolchain)
class Driver
{
public:
    Driver() = default; // init CompileOptions?

    void parse_command_args(int argc, const char** argv);
    void run();

private:
    std::vector<std::string> input_files_; // encapsulate (input/output files)
    CompileOptions compile_options_;

    void compile();
};
