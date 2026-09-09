#include "compiler/compiler.hpp"
#include "frontend/translation_unit.hpp"
#include "frontend/parser/parser.hpp"

void Compiler::run(std::string_view input_file, const CompileOptions& options)
{
    TranslationUnit translation_unit{input_file};

    auto ast = parse(translation_unit);
}

AST Compiler::parse(TranslationUnit& translation_unit)
{
    auto ast = Parser{translation_unit}.run();

    translation_unit.diagnostics_.report();

    return ast;
}
