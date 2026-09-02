#pragma once

#include <string_view>
#include <cstdint>
#include <format>

#include "frontend/lexer/token.hpp"

struct Source
{
    uint64_t line_number_, column_number_;
    std::string_view item_, source_line_;

    Source() = default;

    Source(const Token& start, const Token& end) :
        line_number_{ start.line_number_ },
        column_number_{ start.column_number_ },
        item_{start.lexeme_.data(), end.lexeme_.data() + end.length_},
        source_line_{ start.source_line_ } {}

    Source(const Token& token) :
        line_number_{ token.line_number_ },
        column_number_{ token.column_number_ },
        item_{ token.lexeme_ },
        source_line_{ token.source_line_ } {}
};

inline std::string format_source(const Source source)
{
    const auto d = static_cast<std::size_t>(std::distance(source.source_line_.data(), source.item_.data()));
    const auto underline = std::string(d, ' ') + std::string(source.item_.length(), '^');
    return std::format("    {}\n    {}", source.source_line_, underline);
}
