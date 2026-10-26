#include <utility>
#include <queue>
#include <cassert>
#include <print>

#include "licm.hpp"
#include "analysis/dominator.hpp"
#include "middleend/ir/IR.hpp"
#include "middleend/transforms/helpers.hpp"
#include "utils/casting.hpp"
#include "utils/print/print.hpp"

auto find_natural_loops(Function* function)
{
    DominatorTree dt{function};

    std::vector<std::pair<BasicBlock*, BasicBlock*>> back_edges;

    for (auto* block : post_order(function, true)) {
        for (auto* succ : block->successors()) {
            if (dt.dominates(succ, block)) {
                back_edges.push_back({succ, block});
            }
        }
    }

    std::vector<std::unordered_set<BasicBlock*>> loops;

    for (auto [header, latch] : back_edges) {
        std::unordered_set<BasicBlock*> loop{header, latch};

        std::queue<BasicBlock*> worklist;
        if (header != latch) {
            worklist.push(latch);
        }

        while (!worklist.empty()) {
            auto* block = worklist.front();
            worklist.pop();

            for (auto* pred : block->predecessors()) {
                if (!loop.contains(pred)) {
                    loop.insert(pred);
                }

                if (pred != header) {
                    worklist.push(pred);
                }
            }
        }

        header->name_ += ".header"; // temporary hack for header tracking

        loops.emplace_back(std::move(loop));
    }

    return loops;
}

BasicBlock* add_preheader(const std::unordered_set<BasicBlock*>& loop) {
    assert(!loop.empty());

    auto* preheader = new BasicBlock{"preheader"};

    auto it = std::ranges::find_if(loop, [](BasicBlock* block) {
        return block->name_.ends_with("header");
    });

    assert(it != loop.end());

    auto* header = *it;

    for (auto* pred : header->predecessors()) {
        pred->instructions_.pop_back(); // remove the branch that lead to header
        pred->insert(new Branch{preheader});
    }

    preheader->insert(new Branch{header});

    auto* function = cast<Function>(header->parent_);
    function->insert(preheader);

    return preheader;
}

auto find_invariants(const std::unordered_set<BasicBlock*>& loop)
{
    std::unordered_set<Value*> invariants;

    bool changed = true;
    while (changed) {
        changed = false;

        for (auto* block : loop) {
            for (auto& instruction : block->instructions_) {
                if (invariants.contains(instruction.get())) {
                    continue;
                }

                if (is_pure(instruction)) {
                    auto* inst = static_cast<Instruction*>(instruction.get());

                    auto is_invariant = std::ranges::all_of(inst->operands_, [&](auto* operand) {
                        return !loop.contains(dyn_cast<BasicBlock>(operand->parent_)) || invariants.contains(operand); });

                    if (is_invariant) {
                        invariants.insert(inst);
                        changed = true;
                    }
                }
            }
        }
    }

    return invariants;
}

void licm(Function* function, bool changed)
{
    auto loops = find_natural_loops(function);

    // initial hoistable property (more to handle later)
    auto safe_to_speculate = [](auto value) {
        return !isa<Div>(value);
    };

    // process innermost loop first
    for (auto& loop : std::views::reverse(loops)) {
        auto* preheader = add_preheader(loop);

        auto hoistable = find_invariants(loop) | std::views::filter(safe_to_speculate);

        std::println("hoistable instructions: \n");
        for (auto* value : hoistable) {
            std::println("{}", ir_value_to_str(value));
        }
    }
}

void licm(IRProgram& program)
{
    for (auto& function : program.functions_) {
        auto changed = true;
        while (changed) {
            changed = false;
            licm(function.get(), changed);
        }
    }
}
