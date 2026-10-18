#include "escape.hpp"
#include "utils/casting.hpp"

bool escapes_via(Value* value, Value* target, std::unordered_set<Value*>& visited)
{
    if (visited.contains(value)) {
        return false;
    }

    visited.insert(value);

    if (auto* store = dyn_cast<Store>(value)) {
        return store->operands_[1] == target; // are we storing this alloca ptr, (since we are storing, we lost value flow because it has left ssa world)
    }

    else if (auto* ret = dyn_cast<Return>(value)) {
        return ret->operands_[0] == target; // this is okay because this case is checking for returning the alloca itself, not a load of hte alloca, otherwise this operand would be a load instruction, not the alloca. the lowering engine will handle this case
    }

    else if (auto* ptradd = dyn_cast<PtrAdd>(value)) {
        if (ptradd->operands_[0] == target) {
            for (auto* use : ptradd->users_) {
                if (escapes_via(use, ptradd, visited)) {
                    return true;
                }
            }
        }
        return false;
    }

    else if (auto* call = dyn_cast<Call>(value)) {
        for (auto* arg : call->operands_) {
            if (arg == target) {
                return true;
            }
        }
    }

    else {
        return false;
    }
}

bool escapes(Alloca* alloca)
{
    std::unordered_set<Value*> v;

    for (auto* use : alloca->users_) {
        if (escapes_via(use, alloca, v)) {
            return true;
        }
    }

    return false;
}

std::unordered_set<Alloca*> non_escaping_allocas(Function* function)
{
    std::unordered_set<Alloca*> allocas;

    for (auto& block : function->blocks_) {
        for (auto& inst : block->instructions_) {
            if (auto* alloca = dyn_cast<Alloca>(inst)) {
                if (!escapes(alloca)) {
                    allocas.insert(alloca);
                }
            }
        }
    }

    return allocas;
}
