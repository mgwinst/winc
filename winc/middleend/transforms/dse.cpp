#include "dse.hpp"
#include "analysis/escape.hpp"
#include "utils/casting.hpp"

template <typename T>
concept LoadOrStore = std::same_as<T, Load> || std::same_as<T, Store>;

Alloca* get_alloca_operand(LoadOrStore auto* inst)
{
    if (inst) {
        return dyn_cast<Alloca>(inst->operands_[0]);
    }

    return nullptr;
}

void remove_dead_stores(IRProgram& program)
{
    std::unordered_map<Alloca*, Store*> last_store;
    std::vector<Store*> dead;

    for (auto& function : program.functions_) {
        auto non_escaping = non_escaping_allocas(function.get());
        for (auto& block : function->blocks_) {
            for (auto& inst : std::views::reverse(block->instructions_)) {
                if (auto* store = dyn_cast<Store>(inst)) {
                    auto* a = get_alloca_operand(store);
                    if (non_escaping.contains(a)) {
                        if (last_store.contains(a)) {
                            dead.push_back(store);
                        }
                        last_store[a] = store;
                    }
                } else if (auto* load = dyn_cast<Load>(inst)) {
                    auto* a = get_alloca_operand(load);
                    if (non_escaping.contains(a)) {
                        last_store.erase(a);
                    }
                }
            }

            block->instructions_.remove_if([&dead](std::unique_ptr<Value>& inst) {
                return std::ranges::contains(dead, inst.get());
            });

            last_store.clear();
            dead.clear();
        }
    }
}
