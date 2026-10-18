#include "doctest/doctest.h"

#include "frontend/parser/parser.hpp"
#include "frontend/sema/analyzer.hpp"
#include "frontend/translation_unit.hpp"
#include "middleend/lowering/LoweringEngine.hpp"
#include "middleend/ir/IRBuilder.hpp"
#include "utils/casting.hpp"

// check source location for one of the examples

inline AST parse(std::string_view file_path)
{
    TranslationUnit translation_unit{file_path};
    return Parser{translation_unit}.run();
}

inline std::pair<TranslationUnit, AST> analyze(std::string_view file_path)
{
    TranslationUnit translation_unit{file_path};
    auto ast = parse(file_path);
    SemanticAnalyzer{translation_unit, ast}.run();
    return {std::move(translation_unit), std::move(ast)};
}

inline IRProgram lower(const std::string& source)
{
    auto [translation_unit, sema_tree] = analyze(source);
    return LoweringEngine{translation_unit, sema_tree}.run();
}
