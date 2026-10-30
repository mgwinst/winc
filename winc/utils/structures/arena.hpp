#pragma once

#include <type_traits>
#include <ranges>
#include <vector>

#include "utils/allocators/bump.hpp"

class Arena
{
public:
    Arena() = default;

    Arena(const Arena&) = delete;
    Arena& operator=(const Arena&) = delete;
    Arena(Arena&&) = default;
    Arena& operator=(Arena&&) = default;

    ~Arena()
    {
        destroy_all();
    }

    template <typename T, typename... Args>
        requires std::is_constructible_v<T, Args...>
    [[nodiscard]] T* emplace(Args&&... args)
    {
        void* ptr = alloc_.allocate(sizeof(T));
        T* obj_ptr = new (ptr) T{ std::forward<Args>(args)... };

        if constexpr (!std::is_trivially_destructible_v<T>)
            destructor_list_.emplace_back(ptr, [](void* p) { static_cast<T*>(p)->~T(); });

        return obj_ptr;
    }

    void reset()
    {
        destroy_all();
        alloc_.deallocate();
    }

private:
    struct Destructor
    {
        void* ptr;
        void (*func)(void*);
    };

    std::vector<Destructor> destructor_list_;
    BumpAllocator alloc_;

    void destroy_all()
    {
        for (auto& dest : std::views::reverse(destructor_list_)) {
            dest.func(dest.ptr);
        }
    }
};
