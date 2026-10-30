#pragma once

#include "IR.hpp"

struct IRBuilder
{
    IRProgram* program_;
    Function* current_function_;
    BasicBlock* current_block_;

    IRBuilder(IRProgram* program) :
        program_{ program },
        current_function_{ nullptr },
        current_block_{ nullptr } {}

    template <DerivedFromValue T, typename... Args>
    T* create(Args&&... args)
    {
        if constexpr (std::same_as<T, Const>) {
            static_assert(sizeof...(args) == 1);
            auto [it, _] = program_->constant_pool_.try_insert(args...);
            return it->second.get();
        }

        T* value = new T{ std::forward<Args>(args)... };
        if (!value) {
            return nullptr;
        }

        if constexpr (std::same_as<T, Function>) {
            return program_ ? program_->insert(value) : value;
        } else if constexpr (std::same_as<T, BasicBlock> || std::same_as<T, Argument>) {
            return current_function_ ? current_function_->insert(value) : value;
        } else {
            return current_block_ ? current_block_->insert(value) : value;
        }
    }
};
