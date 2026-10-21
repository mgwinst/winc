#include "compiler/compiler.hpp"
#include "compiler/compile_options.hpp"
#include "frontend/translation_unit.hpp"
#include "frontend/parser/parser.hpp"
#include "frontend/sema/analyzer.hpp"
#include "middleend/lowering/LoweringEngine.hpp"
#include "middleend/transforms/TransformPassManager.hpp"
#include "utils/temp_ir_naming.hpp"
#include "utils/print/print.hpp"

void Compiler::run(std::string_view input_file, const CompileOptions& options)
{
    TranslationUnit translation_unit{input_file};

    auto ast = parse(translation_unit);

    analyze(translation_unit, ast);

    if (options.flags_ & DUMP_AST) {
        print(ast);
    }

    auto ir_program = lower(translation_unit, ast);

    if (options.flags_ & DUMP_IR) {
        print(ir_program);
    }

    optimize(ir_program);

    if (options.flags_ & DUMP_OPT) {
        print(ir_program);
    }
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

IRProgram Compiler::lower(TranslationUnit& translation_unit, AST& ast)
{
    auto program = LoweringEngine{translation_unit, ast}.run();
    name_values(program);
    return program;
}

void Compiler::optimize(IRProgram& program)
{
    TransformPassManager{{
        Transforms::TRIVIAL_DCE,
        Transforms::DSE,
        Transforms::CFG_CLEANUP,
        Transforms::MEM2REG,
        Transforms::CSE
        // Transforms::TRIVIAL_DCE
    }}.run(program);

    name_values(program);
}
