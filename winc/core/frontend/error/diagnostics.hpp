#pragma once

#include <vector>

#include "frontend/error/error.hpp"

struct Diagnostics
{
    std::vector<Error> errors_;
    std::vector<Warning> warnings_;

    void register_error(const Error& error);
    void register_error(std::string msg, Source source);
    void register_warning(const Warning& warning);
    void register_warning(std::string msg, Source source);
    void report();

    void dump_errors() const;
    void dump_warnings() const;
    bool contains_errors() const;
    bool contains_warnings() const;
};
