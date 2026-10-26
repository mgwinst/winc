#include "cleanup_cfg.hpp"
#include "utils/casting.hpp"

void remove_unreachable_blocks(std::unique_ptr<Function>& function, bool& changed)
{
    auto it = function->blocks_.begin();
    auto end = function->blocks_.end();

    while (it != end) {
        auto* block = it->get();

        if (block->predecessors().empty() && block != function->get_entry_block()) {
            it = function->blocks_.erase(it);
            changed = true;
        }

        ++it;
    }
}

void remove_redundant_branches(std::unique_ptr<Function>& function, bool& changed)
{
    for (auto& block : function->blocks_) {
        auto& insts = block->instructions_;

        if (insts.size() >= 2) {
            auto a = std::prev(insts.end(), 2);
            auto b = std::prev(insts.end());

            if (isa<Branch>(*a) && isa<Branch>(*b)) {
                insts.pop_back();
                changed = true;
            }
        }
    }
}

bool collapsible(BasicBlock* block)
{
    if (block->successors().size() == 1) {
        auto successor = block->successors()[0];
        if (successor->predecessors().size() == 1) {
            assert(block == successor->predecessors()[0]);
            return true;
        }
    }

    return false;
}

// return the successor so we can remove from function at the call site (hack b/c no intrusive list)
BasicBlock* combine_with_successor(BasicBlock* block)
{
    auto* successor = block->successors()[0];

    for (auto& inst : successor->instructions_) {
        inst->parent_ = block;
    }

    // remove the terminator branch block1(..., -> [br] <-) + block2(inst, inst, ...) before merging
    block->instructions_.pop_back();

    block->instructions_.splice(block->instructions_.end(), successor->instructions_);

    // this is just an empty block now, so return it and remove from function that owns it
    return successor;
}

void merge_linear_blocks(std::unique_ptr<Function>& function, bool& changed)
{
    std::unordered_set<BasicBlock*> dead;

    auto it = function->blocks_.begin();
    auto end = function->blocks_.end();

    while (it != end) {
        auto* block = it->get();

        if (dead.contains(block)) {
            ++it;
            continue;
        }

        if (collapsible(block)) {
            auto* dead_block = combine_with_successor(block);
            dead.insert(dead_block);
            changed = true;
        }

        ++it;
    }

    function->blocks_.remove_if([&](auto& b) {
        return dead.contains(b.get());
    });
}

void cleanup_cfg(IRProgram& program)
{
    for (auto& function : program.functions_) {

        bool changed = true;

        while (changed) {
            changed = false;

            remove_redundant_branches(function, changed);
            remove_unreachable_blocks(function, changed);
            merge_linear_blocks(function, changed);
        }
    }
}
