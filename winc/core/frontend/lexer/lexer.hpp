#pragma once

#include <string_view>

#include "frontend/translation_unit.hpp"
#include "frontend/lexer/token.hpp"

class Lexer
{
public:
    explicit Lexer(TranslationUnit& translation_unit) noexcept;

    [[nodiscard]] Token lex_token() noexcept;

private:
    TranslationUnit& translation_unit_;
    std::string_view source_;
    std::string_view::iterator cur_, src_ln_beg_, src_ln_end_;
    std::size_t line_num_, col_num_;

    void advance(std::size_t steps = 1) noexcept;

    [[nodiscard]] Token build_token(TokenType type, std::string_view::iterator start, std::string_view::iterator end) noexcept;
    [[nodiscard]] Token build_token(TokenType type, std::string_view sv) noexcept;

    [[nodiscard]] Token lex_number() noexcept;
    [[nodiscard]] Token lex_integer() noexcept;
    [[nodiscard]] Token lex_float() noexcept;
    [[nodiscard]] Token lex_hex() noexcept;
    [[nodiscard]] Token lex_identifier() noexcept;
    [[nodiscard]] Token lex_char() noexcept;
    [[nodiscard]] Token lex_string() noexcept;
    void lex_comment() noexcept;

    void skip_whitespace() noexcept;
    void set_line_bounds(const char* p) noexcept;
};
