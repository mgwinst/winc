#include <format>

#include "frontend/lexer/token.hpp"

Token::Token(TokenType type, std::string_view lexeme, std::string_view source_line, std::size_t line_number, std::size_t column_number, std::size_t length) noexcept:
    type_{ type },
    lexeme_{ lexeme },
    source_line_{ source_line },
    line_number_{ line_number },
    column_number_{ column_number },
    length_{ length } {}

Token::Token(TokenType type, std::string_view lexeme) noexcept:
    type_{ type },
    lexeme_{ lexeme },
    line_number_{ 0 },
    column_number_{ 0 },
    length_{ 0 } {}

std::string Token::to_string() const noexcept
{
    return std::format("[{}] {}:{} {} ", token_type_to_string(type_), line_number_, column_number_, lexeme_);
}
