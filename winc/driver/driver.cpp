
#include <span>
#include <print>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <filesystem>
#include <vector>
#include <ranges>
#include <utility>
#include <thread>

#include "compiler/compiler.hpp"
#include "driver/driver.hpp"

void print_winc_help()
{
    auto msg = R"(winc: the winlang compiler

    usage: winc [-options] [file1.w file2.w ...]

    options:
      [-ast] -> dump AST
      [-ir]  -> dump IR
      [-opt] -> dump optimized IR
      [-mir] -> dump machine IR
      [-h]   -> help
    )";

    std::println(stderr, "{}", msg);
}

void Driver::parse_command_args(int argc, const char** argv)
{
    auto args = std::span{argv, static_cast<size_t>(argc)}.subspan(1) | std::ranges::to<std::unordered_set<std::string>>();

    if (args.empty())
    {
        std::println(stderr, "winc: error: no input files");
        std::exit(1);
    }

    if (args.contains("-h"))
    {
        print_winc_help();
        std::exit(1);
    }

    std::vector<std::string> input_files;
    uint32_t flags;

    uint32_t errors = 0;
    for (auto arg : args)
    {
        if (arg.ends_with(".w"))
        {
            if (std::filesystem::exists(arg))
            {
                input_files.push_back(arg);
            }
            else
            {
                std::println(stderr, "winc: error: '{}' file not found", arg);
                ++errors;
            }
        }
        else if (auto it = winc_flags.find(arg); it != winc_flags.end())
        {
            flags |= it->second;
        }
        else
        {
            std::println(stderr, "winc: error: unsupported argument '{}'", arg);
            ++errors;
        }
    }

    if (errors > 0)
    {
        std::exit(1);
    }

    if (input_files.empty())
    {
        std::println(stderr, "winc: error: no input file");
        std::exit(1);
    }

    input_files_ = std::move(input_files);
    compile_options_ = CompileOptions{flags, Target{}};
}

void Driver::compile()
{
    std::vector<std::jthread> threads;

    for (auto file : input_files_)
    {
        threads.emplace_back([&] { Compiler{}.run(file, compile_options_); });
    }
}

void Driver::run()
{
    compile();
}
