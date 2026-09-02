#include <doctest/doctest.h>

#include <array>

#include "frontend/lexer/lexer.hpp"

std::array token_table = {
    TokenType::IDENTIFIER,
    TokenType::IDENTIFIER,
    TokenType::INT_LITERAL,
    TokenType::FLOAT_LITERAL,
    TokenType::HEX_LITERAL,
    TokenType::HEX_LITERAL,
    TokenType::STRING_LITERAL,
    TokenType::KEYWORD_FUNCTION,
    TokenType::KEYWORD_STRUCT,
    TokenType::KEYWORD_IF,
    TokenType::KEYWORD_ELSE,
    TokenType::KEYWORD_WHILE,
    TokenType::KEYWORD_FOR,
    TokenType::KEYWORD_RETURN,
    TokenType::KEYWORD_BREAK,
    TokenType::KEYWORD_CONTINUE,
    TokenType::KEYWORD_CONST,
    TokenType::KEYWORD_TRUE,
    TokenType::KEYWORD_FALSE,
    TokenType::KEYWORD_INLINE,
    TokenType::EQUAL,
    TokenType::PLUS,
    TokenType::MINUS,
    TokenType::STAR,
    TokenType::SLASH,
    TokenType::PERCENT,
    TokenType::LESS,
    TokenType::GREATER,
    TokenType::LPAREN,
    TokenType::RPAREN,
    TokenType::LBRACE,
    TokenType::RBRACE,
    TokenType::LBRACKET,
    TokenType::RBRACKET,
    TokenType::AMPERSAND,
    TokenType::SEMICOLON,
    TokenType::COLON,
    TokenType::COMMA,
    TokenType::DOT,
    TokenType::BANG,
    TokenType::PIPE,
    TokenType::CARROT,
    TokenType::EQUAL_EQUAL,
    TokenType::BANG_EQUAL,
    TokenType::PLUS_EQUAL,
    TokenType::MINUS_EQUAL,
    TokenType::STAR_EQUAL,
    TokenType::SLASH_EQUAL,
    TokenType::PERCENT_EQUAL,
    TokenType::LESS_EQUAL,
    TokenType::GREATER_EQUAL,
    TokenType::PLUS_PLUS,
    TokenType::MINUS_MINUS,
    TokenType::LESS_LESS,
    TokenType::GREATER_GREATER,
    TokenType::AMPERSAND_AMPERSAND,
    TokenType::PIPE_PIPE,
    TokenType::ARROW,
    TokenType::SLASH_SLASH,
    TokenType::TYPE,
    TokenType::TYPE,
    TokenType::TYPE,
    TokenType::TYPE,
    TokenType::TYPE,
    TokenType::TYPE,
    TokenType::TYPE,
    TokenType::TYPE,
    TokenType::TYPE,
    TokenType::TYPE,
    TokenType::TYPE,
    TokenType::TYPE,
    TokenType::TYPE,
    TokenType::TYPE,
    TokenType::TYPE,
    TokenType::TYPE,
    TokenType::TYPE,
};

TEST_CASE("tokens") {
    TranslationUnit translation_unit{ "srctest/lexer/tokens.txt" };
    Lexer lexer{ translation_unit };

    int i = 0;
    while (true) {
        Token tok = lexer.get_token();
        if (tok.type_ == TokenType::END_OF_FILE)
            break;

        INFO(std::format("{} != {}", token_type_to_string(tok.type_), token_type_to_string(token_table[i])));

        CHECK(tok.type_ == token_table[i]);

        ++i;
    }
}
