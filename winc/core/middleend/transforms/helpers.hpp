#pragma once

#include <concepts>

#include "middleend/ir/IR.hpp"

template <typename T>
concept LoadOrStore = std::same_as<T, Load> || std::same_as<T, Store>;

inline Alloca* get_alloca_operand(LoadOrStore auto* inst)
{
    if (inst) {
        return dyn_cast<Alloca>(inst->operands_[0]);
    }

    return nullptr;
}
