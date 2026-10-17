#pragma once

#include <array>
#include <vector>

template <typename T, typename... Ts>
constexpr auto combine(T first, Ts... rest)
{
    return std::array<T, 1 + sizeof...(Ts)>{first, rest...};
}

template <typename T>
auto prepend(const T& first, std::vector<T> rest)
{
    std::vector<T> result;
    result.reserve(1 + rest.size());
    result.push_back(first);
    result.append_range(std::move(rest));
    return result;
}

template <typename T>
inline constexpr bool always_false_v = false;

void swap_pop(std::ranges::contiguous_range auto& container, auto it)
{
    if (it >= container.begin() && it < container.end()) {
        std::swap(*it, container.back());
        container.pop_back();
    }
}

void swap_pop(std::ranges::contiguous_range auto& container, std::size_t index)
{
    if (index >= 0 && index < container.size()) {
        std::swap(container[index], container.back());
        container.pop_back();
    }
}
