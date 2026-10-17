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
        auto value_count = 0;
        for (auto& block : function->blocks_) {
            for (auto& inst : block->instructions_) {
                if (is_nameable(inst.get())) {
                    if (!isa<Const>(inst))
                        inst->name_ = "%" + std::to_string(value_count++);
                    else
                        inst->name_ = std::to_string(value_count++);
                }
            }
        }
    }
}
