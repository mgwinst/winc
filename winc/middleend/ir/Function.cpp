#include "IR.hpp"
#include "utils/casting.hpp"

Argument::Argument(Type* type, std::string_view name) :
    Value{ValueKind::Argument, type, "%" + std::string(name)} {}

Function::Function(Type* type, std::string_view name) :
    Value{ValueKind::Function, type, name} {}

Function::~Function()
{
    for (Instruction* user : static_cast_view<Instruction>(users_)) {
        for (auto* operand : user->operands_) {
            if (operand == this) {
                operand = nullptr;
            }
        }
    }

    // must destroy blocks in post order

    // actually this is a perfect reason why
    // destructors shouldn't be responsible for modifying the graph
    // just make sure that nothing references the thing you are about to destroy
    // and remove the references separately

    // issue is ~Block() is modifying graph

    while (!blocks_.empty()) {
        blocks_.pop_back();
    }
}

BasicBlock* Function::get_entry_block()
{
    assert(!blocks_.empty());
    auto* entry = blocks_.front().get();
    assert(entry->name_ == "entry" && entry->predecessors().empty());
    return entry;
}

BasicBlock* Function::insert(BasicBlock* block)
{
    block->parent_ = this;
    blocks_.push_back(std::unique_ptr<BasicBlock>{ block });
    return block;
}

Argument* Function::insert(Argument* arg)
{
    arg->parent_ = this;
    args_.push_back(std::unique_ptr<Argument>{ arg });
    return arg;
}
