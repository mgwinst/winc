#include "compiler/compiler.hpp"
#include "frontend/translation_unit.hpp"
#include "frontend/parser/parser.hpp"
#include "frontend/sema/analyzer.hpp"
#include "utils/print/print.hpp"

void Compiler::run(std::string_view input_file, const CompileOptions& options)
{
    TranslationUnit translation_unit{input_file};

    auto ast = parse(translation_unit);

    analyze(translation_unit, ast);

    if (options.flags_ & DUMP_AST) {
        print(ast);
    }

    // auto ir_program = lower(ast);


}

AST Compiler::parse(TranslationUnit& translation_unit)
{
    auto ast = Parser{translation_unit}.run();

    translation_unit.diagnostics_.report();

    return ast;
}

void Compiler::analyze(TranslationUnit& translation_unit, AST& ast)
{
    SemanticAnalyzer{translation_unit, ast}.run();

    translation_unit.diagnostics_.report();
}
