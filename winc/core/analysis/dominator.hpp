#pragma once

#include "middleend/ir/IR.hpp"

class DominanceFrontier
{
public:
    DominanceFrontier(Function* function);

    std::unordered_set<BasicBlock*>& operator[](BasicBlock* block);

private:
    std::unordered_map<BasicBlock*, std::unordered_set<BasicBlock*>> frontier;
};

class DominatorTree
{
public:
    DominatorTree(Function* function);

    std::unordered_set<BasicBlock*>& operator[](BasicBlock* block);

private:
    std::unordered_map<BasicBlock*, std::unordered_set<BasicBlock*>> tree;
};