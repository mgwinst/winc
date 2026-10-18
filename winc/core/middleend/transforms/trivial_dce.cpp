#include "trivial_dce.hpp"

#include "middleend/ir/IR.hpp"

bool may_have_side_effect(const std::unique_ptr<Value>& inst)
{
    switch (inst->kind_) {
        case ValueKind::Store: // mutating memory state is a side effect
        case ValueKind::Call:
        case ValueKind::Return:
        case ValueKind::Branch:
            return true;
        default:
            return false;
    }
}

void trivial_dce(IRProgram& program)
{
    std::vector<Value*> dead;

    for (auto& function : program.functions_) {
        for (auto& block : function->blocks_) {
            for (auto it = block->instructions_.rbegin(); it != block->instructions_.rend(); ++it) {
                if ((*it)->users_.empty() && !may_have_side_effect(*it)) {
                    dead.push_back(it->get());
                }
            }

            block->instructions_.remove_if([&dead](std::unique_ptr<Value>& inst) {
                return std::ranges::contains(dead, inst.get());
            });

            dead.clear();
        }
    }
}