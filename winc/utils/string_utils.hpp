#pragma once

#include <optional>
#include <string_view>
#include <charconv>

template <typename T>
concept Numeric = std::is_arithmetic_v<T>;

template <std::integral T>
std::optional<T> sv_to_numeric(std::string_view str)
{
    if (str.empty()) {
        return std::nullopt;
    }

    T value = 0;

    auto offset = 0;
    auto base = 10;

    if (str.size() > 2 && std::tolower(str[1]) == 'x') {
        offset = 2;
        base = 16;
    }

    auto [ptr, ec] = std::from_chars(str.begin() + offset, str.end(), value, base);
    if (ec == std::errc{} && ptr == str.end()) {
        return value;
    }

    return std::nullopt;
}

template <std::floating_point T>
std::optional<T> sv_to_numeric(std::string_view str)
{
    if (str.empty()) {
        return std::nullopt;
    }

    T value = 0;

    auto [ptr, ec] = std::from_chars(str.begin(), str.end(), value);
    if (ec == std::errc{} && ptr == str.end()) {
        return value;
    }

    return std::nullopt;
}
