#pragma once

#include <print>
#include <string_view>

[[noreturn]] inline void fatal_error(std::string_view msg)
{
    std::println(stderr, "{}", msg);
    std::exit(EXIT_FAILURE);
}
