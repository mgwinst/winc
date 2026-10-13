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
    void analyze(TranslationUnit& translation_unit, AST& ast);
    // IRProgram lower(TranslationUnit& translation_unit, AST& ast);
    // void optimize(IRProgram& program);
};
