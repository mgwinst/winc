#pragma once

#include <unordered_map>
#include <cstdint>
#include <string>

struct Target
{
    std::string_view arch_ = "rv64idm";
    std::string_view os_ = "linux";
    std::string_view abi_ = "lp64d";
};

// state (files, options/flags, target, toolchain, pipeline)
struct CompileOptions
{
    uint32_t flags_ = 0;
    Target target_;
};

// winc flags
inline constexpr uint32_t DUMP_AST = 1 << 0;
inline constexpr uint32_t DUMP_IR  = 1 << 1;
inline constexpr uint32_t DUMP_OPT = 1 << 2;
inline constexpr uint32_t DUMP_MIR = 1 << 3;

inline std::unordered_map<std::string, uint32_t> winc_flags {
    {"-ast", DUMP_AST},
    {"-ir",  DUMP_IR},
    {"-opt", DUMP_OPT},
    {"-mir", DUMP_MIR}
};
