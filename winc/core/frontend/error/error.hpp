#pragma once

#include <string>
#include <format>

#include "frontend/error/source.hpp"

struct Error
{
    std::string msg_;

    Error(std::string_view msg) :
        msg_{ msg } {}

    Error(std::string_view msg, Source source) :
        msg_{ std::format("{}:{}: error: {}\n{}\n",
            source.line_number_,
            source.column_number_,
            msg,
            format_source(source)) } {}
};

struct SyntaxError : Error
{
    SyntaxError(const Token& token) :
        Error{ std::format("{}:{}: syntax error: unexpected symbol detected",
            token.line_number_,
            token.column_number_) } {}

    SyntaxError(const Token& token, std::string_view err_msg) :
        Error{ std::format("{}:{}: syntax error: {}",
            token.line_number_,
            token.column_number_,
            err_msg) } {}
};

struct Warning
{
    std::string msg_;

    Warning(std::string_view msg, Source source) :
        msg_{ std::format("{}:{}: warning: {}\n{}\n",
            source.line_number_,
            source.column_number_,
            msg,
            format_source(source)) } {}
};
