#pragma once

#include <unordered_map>
#include <unordered_set>
#include <string_view>

enum class TokenType
{
    IDENTIFIER = 128,
    INT_LITERAL,
    FLOAT_LITERAL,
    HEX_LITERAL,
    STRING_LITERAL,
    TYPE,
    KEYWORD_FUNCTION,
    KEYWORD_STRUCT,
    KEYWORD_VAR,
    KEYWORD_IF,
    KEYWORD_ELSE,
    KEYWORD_WHILE,
    KEYWORD_FOR,
    KEYWORD_RETURN,
    KEYWORD_BREAK,
    KEYWORD_CONTINUE,
    KEYWORD_CONST,
    KEYWORD_TRUE,
    KEYWORD_FALSE,
    KEYWORD_INLINE,
    KEYWORD_NEW,
    KEYWORD_DELETE,
    LPAREN,
    RPAREN,
    LBRACKET,
    RBRACKET,
    LBRACE,
    RBRACE,
    EQUAL,
    PLUS,
    MINUS,
    STAR,
    SLASH,
    PERCENT,
    LESS,
    GREATER,
    DOUBLE_QUOTE,
    AMPERSAND,
    BANG,
    CARROT,
    SEMICOLON,
    COLON,
    COMMA,
    DOT,
    PIPE,
    ARROW,
    TILDE,
    EQUAL_EQUAL,
    BANG_EQUAL,
    PLUS_EQUAL,
    MINUS_EQUAL,
    STAR_EQUAL,
    SLASH_EQUAL,
    PERCENT_EQUAL,
    LESS_EQUAL,
    GREATER_EQUAL,
    PLUS_PLUS,
    MINUS_MINUS,
    LESS_LESS,
    GREATER_GREATER,
    STAR_STAR,
    AMPERSAND_AMPERSAND,
    PIPE_PIPE,
    END_OF_FILE,
    INVALID,
};

inline constexpr std::string_view token_type_to_string(TokenType type) noexcept
{
    switch (type) {
        case TokenType::IDENTIFIER:            return "IDENTIFIER";
        case TokenType::INT_LITERAL:           return "INT LITERAL";
        case TokenType::FLOAT_LITERAL:         return "FLOAT LITERAL";
        case TokenType::HEX_LITERAL:           return "HEX LITERAL";
        case TokenType::STRING_LITERAL:        return "STRING_LITERAL";
        case TokenType::TYPE:                  return "TYPE";
        case TokenType::KEYWORD_FUNCTION:      return "KEYWORD_FUNCTION";
        case TokenType::KEYWORD_STRUCT:        return "KEYWORD_STRUCT";
        case TokenType::KEYWORD_VAR:           return "KEYWORD_VAR";
        case TokenType::KEYWORD_IF:            return "KEYWORD_IF";
        case TokenType::KEYWORD_ELSE:          return "KEYWORD_ELSE";
        case TokenType::KEYWORD_WHILE:         return "KEYWORD_WHILE";
        case TokenType::KEYWORD_FOR:           return "KEYWORD_FOR";
        case TokenType::KEYWORD_RETURN:        return "KEYWORD_RETURN";
        case TokenType::KEYWORD_BREAK:         return "KEYWORD_BREAK";
        case TokenType::KEYWORD_CONTINUE:      return "KEYWORD_CONTINUE";
        case TokenType::KEYWORD_CONST:         return "KEYWORD_CONST";
        case TokenType::KEYWORD_TRUE:          return "KEYWORD_TRUE";
        case TokenType::KEYWORD_FALSE:         return "KEYWORD_FALSE";
        case TokenType::KEYWORD_INLINE:        return "KEYWORD_INLINE";
        case TokenType::KEYWORD_NEW:           return "KEYWORD_NEW";
        case TokenType::KEYWORD_DELETE:        return "KEYWORD_DELETE";
        case TokenType::LPAREN:                return "LPAREN";
        case TokenType::RPAREN:                return "RPAREN";
        case TokenType::LBRACKET:              return "LBRACKET";
        case TokenType::RBRACKET:              return "RBRACKET";
        case TokenType::LBRACE:                return "LBRACE";
        case TokenType::RBRACE:                return "RBRACE";
        case TokenType::EQUAL:                 return "EQUAL";
        case TokenType::PLUS:                  return "PLUS";
        case TokenType::MINUS:                 return "MINUS";
        case TokenType::STAR:                  return "STAR";
        case TokenType::SLASH:                 return "SLASH";
        case TokenType::PERCENT:               return "PERCENT";
        case TokenType::LESS:                  return "LESS";
        case TokenType::GREATER:               return "GREATER";
        case TokenType::DOUBLE_QUOTE:          return "DOUBLE_QUOTE";
        case TokenType::AMPERSAND:             return "AMPERSAND";
        case TokenType::BANG:                  return "BANG";
        case TokenType::CARROT:                return "CARROT";
        case TokenType::SEMICOLON:             return "SEMICOLON";
        case TokenType::COLON:                 return "COLON";
        case TokenType::COMMA:                 return "COMMA";
        case TokenType::DOT:                   return "DOT";
        case TokenType::PIPE:                  return "PIPE";
        case TokenType::ARROW:                 return "ARROW";
        case TokenType::TILDE:                 return "TILDE";
        case TokenType::EQUAL_EQUAL:           return "EQUAL_EQUAL";
        case TokenType::BANG_EQUAL:            return "BANG_EQUAL";
        case TokenType::PLUS_EQUAL:            return "PLUS_EQUAL";
        case TokenType::MINUS_EQUAL:           return "MINUS_EQUAL";
        case TokenType::STAR_EQUAL:            return "STAR_EQUAL";
        case TokenType::SLASH_EQUAL:           return "SLASH_EQUAL";
        case TokenType::PERCENT_EQUAL:         return "PERCENT_EQUAL";
        case TokenType::LESS_EQUAL:            return "LESS_EQUAL";
        case TokenType::GREATER_EQUAL:         return "GREATER_EQUAL";
        case TokenType::PLUS_PLUS:             return "PLUS_PLUS";
        case TokenType::MINUS_MINUS:           return "MINUS_MINUS";
        case TokenType::LESS_LESS:             return "LESS_LESS";
        case TokenType::GREATER_GREATER:       return "GREATER_GREATER";
        case TokenType::STAR_STAR:             return "STAR_STAR";
        case TokenType::AMPERSAND_AMPERSAND:   return "AMPERSAND_AMPERSAND";
        case TokenType::PIPE_PIPE:             return "PIPE_PIPE";
        case TokenType::END_OF_FILE:           return "END_OF_FILE";
        case TokenType::INVALID:               return "INVALID";
    }

    return "UNKNOWN";
}

inline const std::unordered_set<std::string_view> built_in_types {
    "int", "int8", "int16", "int32", "int64",
    "uint", "uint8", "uint16", "uint32", "uint64",
    "float", "float16", "float32", "float64",
    "str", "byte", "bool", "void"
};

inline const std::unordered_map<std::string_view, TokenType> keywords {
    {"fn",       TokenType::KEYWORD_FUNCTION},
    {"struct",   TokenType::KEYWORD_STRUCT},
    {"var",      TokenType::KEYWORD_VAR},
    {"if",       TokenType::KEYWORD_IF},
    {"else",     TokenType::KEYWORD_ELSE},
    {"while",    TokenType::KEYWORD_WHILE},
    {"for",      TokenType::KEYWORD_FOR},
    {"return",   TokenType::KEYWORD_RETURN},
    {"break",    TokenType::KEYWORD_BREAK},
    {"continue", TokenType::KEYWORD_CONTINUE},
    {"const",    TokenType::KEYWORD_CONST},
    {"true",     TokenType::KEYWORD_TRUE},
    {"false",    TokenType::KEYWORD_FALSE},
    {"inline",   TokenType::KEYWORD_INLINE},
    {"new",      TokenType::KEYWORD_NEW},
    {"delete",   TokenType::KEYWORD_DELETE},
};

inline const std::unordered_map<char, TokenType> single_symbol_map {
    {'=',  TokenType::EQUAL},
    {'+',  TokenType::PLUS},
    {'-',  TokenType::MINUS},
    {'*',  TokenType::STAR},
    {'/',  TokenType::SLASH},
    {'%',  TokenType::PERCENT},
    {'<',  TokenType::LESS},
    {'>',  TokenType::GREATER},
    {'(',  TokenType::LPAREN},
    {')',  TokenType::RPAREN},
    {'{',  TokenType::LBRACE},
    {'}',  TokenType::RBRACE},
    {'[',  TokenType::LBRACKET},
    {']',  TokenType::RBRACKET},
    {'&',  TokenType::AMPERSAND},
    {';',  TokenType::SEMICOLON},
    {':',  TokenType::COLON},
    {',',  TokenType::COMMA},
    {'.',  TokenType::DOT},
    {'!',  TokenType::BANG},
    {'|',  TokenType::PIPE},
    {'^',  TokenType::CARROT},
    {'~',  TokenType::TILDE}
};

inline const std::unordered_map<std::string_view, TokenType> double_symbol_map {
    {"==", TokenType::EQUAL_EQUAL},
    {"!=", TokenType::BANG_EQUAL},
    {"+=", TokenType::PLUS_EQUAL},
    {"-=", TokenType::MINUS_EQUAL},
    {"*=", TokenType::STAR_EQUAL},
    {"/=", TokenType::SLASH_EQUAL},
    {"%=", TokenType::PERCENT_EQUAL},
    {"<=", TokenType::LESS_EQUAL},
    {">=", TokenType::GREATER_EQUAL},
    {"++", TokenType::PLUS_PLUS},
    {"--", TokenType::MINUS_MINUS},
    {"<<", TokenType::LESS_LESS},
    {">>", TokenType::GREATER_GREATER},
    {"**", TokenType::STAR_STAR},
    {"&&", TokenType::AMPERSAND_AMPERSAND},
    {"||", TokenType::PIPE_PIPE},
    {"->", TokenType::ARROW},
};

struct Token {
    TokenType type_;
    std::string_view lexeme_, source_line_;
    std::size_t line_number_, column_number_, length_;

    Token(TokenType type, std::string_view lexeme, std::string_view source_line, std::size_t line_number, std::size_t column_number, std::size_t length) noexcept;
    Token(TokenType type, std::string_view lexeme) noexcept;
    Token() = default;
    ~Token() = default;

    std::string to_string() const noexcept;
};
