#pragma once

#include "middleend/ir/IR.hpp"
#include <unordered_map>

class DominanceFrontier
{
public:
    DominanceFrontier(Function* function);

    std::unordered_set<BasicBlock*>& operator[](BasicBlock* block);

private:
    std::unordered_map<BasicBlock*, std::unordered_set<BasicBlock*>> frontier_;
};

// have to cache immediate dominators dominance queries

class DominatorTree
{
public:
    DominatorTree(Function* function);

    std::unordered_set<BasicBlock*>& operator[](BasicBlock* block);

    // does b1 dominate b2
    bool dominates(BasicBlock* b1, BasicBlock* b2);

private:
    std::unordered_map<BasicBlock*, std::unordered_set<BasicBlock*>> tree_;
    std::unordered_map<BasicBlock*, BasicBlock*> idom_;
};
