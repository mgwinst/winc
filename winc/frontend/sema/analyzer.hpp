#pragma once

#include "frontend/ast/ast.hpp"
#include "frontend/translation_unit.hpp"

class SemanticAnalyzer
{
public:
    SemanticAnalyzer(TranslationUnit& translation_unit, AST& ast) :
        translation_unit_{ translation_unit },
        ast_{ ast } {}

    void run()
    {
        decorate(ast_.root_);
        translation_unit_.diagnostics_.report();

        check_type(ast_.root_);
        translation_unit_.diagnostics_.report();

        desugar(ast_.root_);
    }

private:
    TranslationUnit& translation_unit_;
    AST& ast_;

    void decorate(ASTNode* node);
    Type* check_type(ASTNode* node);
    void desugar(ASTNode*& node);

    void register_error(auto&&... args)
    {
        translation_unit_.diagnostics_.register_error(std::forward<decltype(args)>(args)...);
    }
};
