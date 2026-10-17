#include <stack>

#include "frontend/translation_unit.hpp"
#include "frontend/ast/ast.hpp"
#include "middleend/ir/IRBuilder.hpp"

struct LoopContext
{
    BasicBlock *preheader_, *header_, *body_, *end_;
};

class LoweringEngine
{
public:
    LoweringEngine(TranslationUnit& translation_unit, const AST& ast) :
        translation_unit_{ translation_unit },
        ast_{ ast },
        program_{ },
        builder_{ &program_ } {}

    IRProgram run()
    {
        lower(ast_.root_);
        return std::move(program_);
    }

private:
    TranslationUnit& translation_unit_;
    const AST& ast_;
    IRProgram program_;
    IRBuilder builder_;

    std::unordered_map<std::string, Alloca*> alloca_map_; // must outlive LoweringEngine object
    std::unordered_map<std::string, Function*> function_map_;
    std::stack<LoopContext> loop_context_stack_;

    Value* lower(ASTNode* node);

    Function* current_function() const;
    BasicBlock* current_block() const;
    void set_current_function(Function* function);
    void set_current_block(BasicBlock* block);
    Value* get_value(ReferenceExpr* ref); // {ref.name : Value*} -> get alloca ptr for this variable
    void push_loop_context(BasicBlock* preheader, BasicBlock* header, BasicBlock* body, BasicBlock* end);
    void pop_loop_context();
    LoopContext& get_loop_context();
};
