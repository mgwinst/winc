#pragma once

#include <string_view>

#include "frontend/translation_unit.hpp"
#include "frontend/lexer/token.hpp"

struct Lexer
{
    std::string_view source_;
    std::string_view::iterator cur_, src_ln_beg_, src_ln_end_;
    std::size_t line_num_, col_num_;

    explicit Lexer(TranslationUnit& translation_unit) noexcept;

    void advance(std::size_t steps = 1) noexcept;

    [[nodiscard]] Token build_token(TokenType type, std::string_view::iterator start, std::string_view::iterator end) noexcept;
    [[nodiscard]] Token build_token(TokenType type, std::string_view sv) noexcept;

    [[nodiscard]] Token get_token() noexcept;
    [[nodiscard]] Token get_number() noexcept;
    [[nodiscard]] Token get_identifier() noexcept;
    [[nodiscard]] Token get_string() noexcept;

    void skip_whitespace() noexcept;
    void set_line_bounds(const char* p) noexcept;
};
