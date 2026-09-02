#include "compiler/compiler.hpp"
#include "frontend/translation_unit.hpp"
// #include "frontend/parser/parser.hpp"

void Compiler::run(std::string_view input_file, const CompileOptions& options)
{
    TranslationUnit translation_unit{input_file};
    // AST ast = parse(translation_unit);

}

/*
AST Compiler::parse(TranslationUnit& translation_unit)
{
    Parser parser{translation_unit};
    AST ast = parser.run();

    translation_unit.diagnostics_.report();

    return ast;
}
*/
