#include <functional>
#include <ranges>

#include "analysis/dominator.hpp"
#include "middleend/transforms/cse.hpp"
#include "middleend/ir/IR.hpp"
#include "utils/structures/ScopedHashTable.hpp"

bool is_pure(const std::unique_ptr<Value>& inst) {
    return inst->kind_ >= ValueKind::Add && inst->kind_ <= ValueKind::PtrAdd;
}

uint64_t hash(auto& value)
{
    // impl base class casting for Value
    auto* inst = static_cast<Instruction*>(value.get());
    auto hashed = inst->operands_ | std::views::transform([](auto* x) { return std::hash<decltype(x)>{}(x); });
    return std::ranges::fold_left(hashed, 0ull, std::bit_xor{});
}

using ScopedTable = ScopedHashTable<uint64_t, Value*>;

void cse(BasicBlock* block, ScopedTable& table, DominatorTree& dt, bool changed)
{
    table.push_scope();

    std::vector<Value*> dead;

    for (auto& inst : block->instructions_) {
        if (!is_pure(inst)) {
            continue;
        }

        auto key = hash(inst);
        if (auto existing = table.lookup(key)) {
            inst->replace_uses_with(*existing);
            dead.push_back(inst.get());
            changed = true;
        } else {
            table.emplace(key, inst.get());
        }
    }

    block->instructions_.remove_if([&dead](std::unique_ptr<Value>& inst) {
        return std::ranges::contains(dead, inst.get());
    });

    for (auto* child : dt[block]) {
        cse(child, table, dt, changed);
    }

    table.pop_scope();
}

void cse(IRProgram& program)
{
    ScopedTable table;

    for (auto& function : program.functions_) {
        DominatorTree dt{function.get()};
        auto* entry_block = function->get_entry_block();

        auto changed = false;
        while (changed) {
            cse(entry_block, table, dt, changed);
        }
    }
}
