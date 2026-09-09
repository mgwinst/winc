#pragma once

#include <string_view>

#include "compiler/compile_options.hpp"
#include "frontend/translation_unit.hpp"
#include "frontend/ast/ast.hpp"

class Compiler
{
public:
    void run(std::string_view input_file, const CompileOptions& options);

private:
    AST parse(TranslationUnit& translation_unit);
    // SemaTree analyze(ModuleContext& ctx, AST& ast);
    // Program lower(ModuleContext& ctx, const SemaTree& tree);
    // void optimize(Program& program);
};
