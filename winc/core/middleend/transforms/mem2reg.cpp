#include <queue>
#include <stack>

#include "analysis/dominator.hpp"
#include "analysis/escape.hpp"
#include "middleend/transforms/helpers.hpp"
#include "utils/casting.hpp"

/*

handle trivial cases before full mem2reg

1. an alloca with a single store and multiple loads
2. an alloca whose loads are all in the same block as and dominated by the stores feeding them

*/

// also must run dead store elimination in case of immediately overwritten phi via store in block

std::queue<BasicBlock*> def_blocks(Function* function, Alloca* alloca)
{
    std::queue<BasicBlock*> def_blocks;

    for (auto& block : function->blocks_) {
        for (auto& inst : block->instructions_) {
            if (auto* store = dyn_cast<Store>(inst)) {
                if (get_alloca_operand(store) == alloca) {
                    def_blocks.push(block.get());
                    break;
                }
            }
        }
    }

    return def_blocks;
}

void rename(BasicBlock* block, std::unordered_map<Alloca*, std::stack<Value*>>& def_stack, DominatorTree& dt)
{
    std::unordered_map<Alloca*, std::size_t> sizes;
    for (auto [alloca, stack] : def_stack) {
        sizes[alloca] = stack.size();
    }

    std::vector<Value*> to_remove;

    for (auto& inst : block->instructions_) {
        if (auto* phi = dyn_cast<Phi>(inst)) {
            def_stack[phi->alloca_].push(phi);
        }

        else if (auto* load = dyn_cast<Load>(inst)) {
            if (def_stack[get_alloca_operand(load)].size() == 0)
                continue;
            auto* v = def_stack[get_alloca_operand(load)].top();
            inst->replace_uses_with(v);
            to_remove.push_back(load);
        }

        else if (auto* store = dyn_cast<Store>(inst)) {
            def_stack[get_alloca_operand(store)].push(store->operands_[1]);
            to_remove.push_back(store);
        }
    }

    block->instructions_.remove_if([&to_remove](auto& inst) {
        return std::ranges::contains(to_remove, inst.get());
    });

    for (auto* succ : block->successors()) {
        for (auto& inst : succ->instructions_) {
            if (auto* phi = dyn_cast<Phi>(inst)) {
                auto& def = def_stack[phi->alloca_].top();
                def->add_use(phi);
                phi->operands_.emplace_back(def, block);
            }
        }
    }

    for (auto* child : dt[block]) {
        rename(child, def_stack, dt);
    }

    for (auto [alloca, size] : sizes) {
        auto& stack = def_stack[alloca];
        while (stack.size() > size) {
            stack.pop();
        }
    }
}

void promote_memory_to_register(Function* function)
{
    auto dt = DominatorTree{function};
    auto df = DominanceFrontier{function};

    auto promotable = [](Alloca* a) {
        for (auto* user : a->users_) {
            if (auto* store = dyn_cast<Store>(user)) {
                if (store->operands_[0] != a || store->operands_[1] == a) {
                    return false;
                }
            } else if (!isa<Load>(user)) {
                return false;
            }
        }
        return true;
    };

    auto promotable_allocas = non_escaping_allocas(function)
        | std::views::filter(promotable)
        | std::ranges::to<std::vector>();

    for (auto* alloca : promotable_allocas) {
        std::queue worklist = def_blocks(function, alloca);
        std::unordered_set<BasicBlock*> phi_blocks;

        while (!worklist.empty()) {
            auto* block = worklist.front();
            worklist.pop();
            for (auto* d : df[block]) {
                if (!phi_blocks.contains(d)) {
                    phi_blocks.insert(d);
                    worklist.push(d);
                }
            }
        }

        for (auto* block : phi_blocks) {
            block->instructions_.emplace_front(new Phi{alloca});
        }
    }

    std::unordered_map<Alloca*, std::stack<Value*>> def_stack;
    for (auto* alloca : promotable_allocas) {
        def_stack.emplace(alloca, std::stack<Value*>{});
    }

    rename(function->get_entry_block(), def_stack, dt);

    // delete all promotable allocas after the rename walk
    function->get_entry_block()->instructions_.remove_if([&promotable_allocas](auto& inst) {
        return std::ranges::contains(promotable_allocas, inst.get());
    });
}

void mem2reg(IRProgram& program)
{
    for (auto& function : program.functions_) {
        promote_memory_to_register(function.get());
    }
}
