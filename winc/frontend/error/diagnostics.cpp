#include <print>

#include "frontend/error/diagnostics.hpp"

void Diagnostics::register_error(const Error& error)
{
    errors_.push_back(error);

    if (errors_.size() >= 20)
    {
        dump_errors();
        std::println(stderr, "too many errors emitted, stopping now");
        exit(1);
    }
}

void Diagnostics::register_error(std::string msg, Source source)
{
    errors_.emplace_back(msg, source);

    if (errors_.size() >= 20)
    {
        dump_errors();
        std::println(stderr, "too many errors emitted, stopping now");
        exit(1);
    }
}

void Diagnostics::register_warning(const Warning& warning)
{
    warnings_.push_back(warning);
}

void Diagnostics::register_warning(std::string msg, Source source)
{
    warnings_.emplace_back(msg, source);
}

void Diagnostics::dump_errors() const
{
    for (const auto& error : errors_)
    {
        std::println("{}", error.msg_);
    }

    std::println("{} error(s) generated", errors_.size());
}

void Diagnostics::dump_warnings() const
{
    for (const auto& warning : warnings_)
    {
        std::println("{}", warning.msg_);
    }
}

bool Diagnostics::contains_errors() const { return errors_.size() > 0; }
bool Diagnostics::contains_warnings() const { return warnings_.size() > 0; }

void Diagnostics::report()
{
    if (contains_warnings())
    {
        dump_warnings();
    }

    if (contains_errors())
    {
        dump_errors();
        exit(1);
    }

    errors_.clear();
    warnings_.clear();
}
