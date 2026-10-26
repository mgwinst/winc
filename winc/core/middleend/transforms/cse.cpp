#include <functional>
#include <ranges>

#include "analysis/dominator.hpp"
#include "middleend/transforms/cse.hpp"
#include "middleend/ir/IR.hpp"
#include "middleend/transforms/helpers.hpp"
#include "utils/structures/ScopedHashTable.hpp"

uint64_t hash(auto& value)
{
    // impl base class casting for Value
    auto* inst = static_cast<Instruction*>(value.get());
    auto hashed = inst->operands_ | std::views::transform([](auto* x) { return std::hash<decltype(x)>{}(x); });
    return std::ranges::fold_left(hashed, 0ull, std::bit_xor{});
}

void cse(BasicBlock* block, ScopedHashTable<uint64_t, Value*>& table, DominatorTree& dt, bool& changed)
{
    table.push_scope();

    for (auto it = block->instructions_.begin(); it != block->instructions_.end(); ) {
        if (!is_pure(*it)) {
            ++it;
            continue;
        }

        auto key = hash(*it);
        if (auto existing = table.lookup(key)) {
            (*it)->replace_uses_with(*existing);
            it = block->instructions_.erase(it);
            changed = true;
        } else {
            table.emplace(key, (*it).get());
            ++it;
        }
    }

    for (auto* child : dt[block]) {
        cse(child, table, dt, changed);
    }

    table.pop_scope();
}

void cse(IRProgram& program)
{
    ScopedHashTable<uint64_t, Value*> table;

    for (auto& function : program.functions_) {
        DominatorTree dt{function.get()};
        auto* entry_block = function->get_entry_block();

        bool changed = true;
        while (changed) {
            changed = false;
            cse(entry_block, table, dt, changed);
        }
    }
}
