#include "dominator.hpp"

#define REVERSE (true)

auto compute_rpo_index_map(const std::vector<BasicBlock*>& blocks) 
{
    std::unordered_map<BasicBlock*, uint64_t> rpo_index;

    for (auto i = 0uz; i < blocks.size(); ++i) {
        rpo_index[blocks[i]] = i;
    }

    return rpo_index;
}

auto compute_immediate_dominators(Function* function)
{
    std::unordered_map<BasicBlock*, BasicBlock*> idom;
    
    auto blocks = post_order(function, REVERSE);

    auto rpo_index = compute_rpo_index_map(blocks);

    auto* entry = function->get_entry_block();
    
    idom[entry] = entry;

    for (auto* b : blocks) {
        if (b != entry) {
            idom[b] = nullptr;
        }
    }

    auto intersect = [&](BasicBlock* b1, BasicBlock* b2) {
        while (b1 != b2) {
            while (rpo_index[b1] > rpo_index[b2]) {
                b1 = idom[b1];
            }
            while (rpo_index[b2] > rpo_index[b1]) {
                b2 = idom[b2];
            }
        }
        return b1;
    };

    bool changed = true;
    while (changed) {
        changed = false;
        for (auto i = 0uz; i < blocks.size(); ++i) {
            BasicBlock* b = blocks[i];

            if (b == entry) 
                continue;

            BasicBlock* new_idom = nullptr;

            for (BasicBlock* p : b->predecessors()) {
                if (idom[p] != nullptr) {
                    if (new_idom == nullptr)
                        new_idom = p;
                    else
                        new_idom = intersect(p, new_idom);
                }
            }

            if (idom[b] != new_idom) {
                idom[b] = new_idom;
                changed = true;
            }
        }
    }

    return idom;
}

DominanceFrontier::DominanceFrontier(Function* function)
{
    auto idom = compute_immediate_dominators(function);

    auto blocks = post_order(function, REVERSE);

    for (auto* b : blocks) {
        if (b->predecessors().size() >= 2) {
            for (auto* p : b->predecessors()) {
                auto* runner = p;
                while (runner != idom[b]) {
                    frontier[runner].insert(b);
                    runner = idom[runner];
                }
            }
        }
    }
}

std::unordered_set<BasicBlock*>& DominanceFrontier::operator[](BasicBlock* block)
{
    return frontier[block];
}

DominatorTree::DominatorTree(Function* function)
{
    auto blocks = post_order(function, REVERSE);  

    auto idom = compute_immediate_dominators(function);

    for (auto* b : blocks) {
        auto* p = idom[b];

        if (p != b) {
            tree[p].insert(b);
        }
    }
}

std::unordered_set<BasicBlock*>& DominatorTree::operator[](BasicBlock* block)
{
    return tree[block];
}