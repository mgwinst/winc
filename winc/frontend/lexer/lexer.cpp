#include <cctype>
#include <cstdio>

#include "frontend/lexer/lexer.hpp"
#include "frontend/lexer/token.hpp"

Lexer::Lexer(TranslationUnit& translation_unit) noexcept :
    translation_unit_{ translation_unit },
    source_{ translation_unit.source_file_.data() },
    cur_{ source_.begin() },
    line_num_{ 1 },
    col_num_{ 1 }
{
    set_line_bounds(cur_ - 1);
}

void Lexer::advance(std::size_t steps) noexcept
{
    cur_ += steps;
    col_num_ += steps;
}

[[nodiscard]] Token Lexer::build_token(TokenType type, std::string_view sv) noexcept
{
    return Token{type, sv, std::string_view{src_ln_beg_, src_ln_end_}, line_num_, col_num_ - sv.size(), sv.size()};
}

// caller will check if token returned is invalid, if so it has already been registered as an error in diagnostics object
[[nodiscard]] Token Lexer::lex_token() noexcept
{
    while (cur_ != source_.end())
    {
        if (std::isspace(*cur_))
        {
            skip_whitespace();
        }
        else if (std::isdigit(*cur_))
        {
            return lex_number();
        }
        else if (std::isalpha(*cur_) || *cur_ == '_')
        {
            return lex_identifier();
        }
        else if (*cur_ == '\'')
        {
            return lex_char();
        }
        else if (*cur_ == '"')
        {
            return lex_string();
        }
        else if (*cur_ == '/' && *(cur_ + 1) == '/')
        {
            advance(2);
            lex_comment();
        }
        else if (auto double_symbol = double_symbol_map.find(std::string_view{cur_, 2}); double_symbol != double_symbol_map.end())
        {
            auto token = build_token(double_symbol->second, std::string_view{cur_, 2});
            advance(2);
            return token;
        }
        else if (auto symbol = single_symbol_map.find(*cur_); symbol != single_symbol_map.end())
        {
            auto token = build_token(symbol->second, std::string_view{cur_, 1});
            advance();
            return token;
        }
        else
        {
            auto token = build_token(TokenType::INVALID, std::string_view{cur_, 1});
            advance();
            return token;
        }
    }

    return build_token(TokenType::END_OF_FILE, {});
}

Token Lexer::lex_number() noexcept
{
    auto start = cur_;

    advance();

    TokenType token_type;
    int (*f)(char) = nullptr;

    if (*cur_ == '.')
    {
        advance();
        token_type = TokenType::FLOAT_LITERAL;
        f = [](char ch) { return std::isdigit(ch); };
    }
    else if (std::tolower(*cur_) == 'x')
    {
        advance();
        token_type = TokenType::HEX_LITERAL;
        f = [](char ch) { return std::isxdigit(ch); };
    }
    else
    {
        token_type = TokenType::INT_LITERAL;
        f = [](char ch) { return std::isdigit(ch); };
    }

    while (f(*cur_))
    {
        advance();
    }

    auto end = cur_;

    return build_token(token_type, std::string_view{start, end});
}

[[nodiscard]] Token Lexer::lex_identifier() noexcept
{
    auto start = cur_;

    while (std::isalnum(*cur_) || *cur_ == '_')
    {
        advance();
    }

    auto end = cur_;

    std::string_view value{start, end};

    if (auto kw = keywords.find(value); kw != keywords.end())
    {
        return build_token(kw->second, std::string_view{start, end});
    }
    else if (built_in_types.contains(value))
    {
        return build_token(TokenType::TYPE, std::string_view{start, end});
    }
    else
    {
        return build_token(TokenType::IDENTIFIER, std::string_view{start, end});
    }
}

[[nodiscard]] Token Lexer::lex_char() noexcept
{
    auto start = cur_;

    advance();

    if (*cur_ == '\0' || *cur_ == '\n')
    {
        auto token = build_token(TokenType::INVALID, std::string_view{start, 1});
        translation_unit_.diagnostics_.register_error(std::format("character literal missing terminating '\'' character"), Source{ token });
        return token;
    }

    advance();

    if (*cur_ != '\'')
    {
        auto token = build_token(TokenType::INVALID, std::string_view{start, cur_});
        translation_unit_.diagnostics_.register_error(std::format("character literal must contain exactly one character"), Source{ token });
        return token;
    }

    advance();

    auto end = cur_;

    return build_token(TokenType::CHAR_LITERAL, std::string_view{start, end});
}

[[nodiscard]] Token Lexer::lex_string() noexcept
{
    auto start = cur_;

    advance();

    if (*cur_ == '"')
    {
        auto token = build_token(TokenType::INVALID, std::string_view{start, 2});
        translation_unit_.diagnostics_.register_error(std::format("string literal cannot be empty"), Source{ token });
        return token;
    }

    while (*cur_ != '\0')
    {
        if (*cur_ == '"')
        {
            advance();
            break;
        }

        advance();
    }

    // safe because EOF is always on next line
    if (*cur_ == '\0')
    {
        auto token = build_token(TokenType::INVALID, std::string_view{start, 1});
        translation_unit_.diagnostics_.register_error(std::format("string literal missing terminating '\"' character"), Source{ token });
        return token;
    }

    auto end = cur_;

    return build_token(TokenType::STRING_LITERAL, std::string_view{start, end});
}

void Lexer::lex_comment() noexcept
{
    while (*cur_ != '\n' && *cur_ != '\0')
    {
        advance();
    }
}

void Lexer::skip_whitespace() noexcept
{
    if (*cur_ == '\n')
    {
        ++line_num_;
        col_num_ = 1;
        set_line_bounds(cur_);
    }
    else
    {
        ++col_num_;
    }

    ++cur_;
}


void Lexer::set_line_bounds(const char* p) noexcept
{
    src_ln_beg_ = ++p;

    while (*p != '\n' && *p != '\0')
    {
        ++p;
    }

    src_ln_end_ = p;
}
