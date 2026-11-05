#pragma once

#include "middleend/ir/IR.hpp"
#include "casting.hpp"

inline bool is_nameable(Value* value)
{
    return value->kind_ >= ValueKind::Load && value->kind_ <= ValueKind::Phi;
}

inline void name_values(IRProgram& program)
{
    for (auto& function : program.functions_) {
        auto block_count = 0;
        auto value_count = 0;
        for (auto* block : post_order(function.get(), true)) {
            block->name_ = std::format("bb.{}", block_count++);
            for (auto& inst : block->instructions_) {
                if (is_nameable(inst.get())) {
                    if (!isa<Const>(inst))
                        inst->name_ = std::format("%{}", value_count++);
                    else
                        inst->name_ = std::to_string(value_count++);
                }
            }
        }
    }
}
