#include <cctype>

#include "frontend/lexer/lexer.hpp"
#include "frontend/lexer/token.hpp"

Lexer::Lexer(TranslationUnit& translation_unit) noexcept :
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

[[nodiscard]] Token Lexer::build_token(TokenType type, std::string_view::iterator start, std::string_view::iterator end) noexcept
{
    Token token{type, std::string_view{start, end}, std::string_view{src_ln_beg_, src_ln_end_}, line_num_, col_num_ - static_cast<std::size_t>(end - start), static_cast<std::size_t>(end - start)};
    return token;
}

[[nodiscard]] Token Lexer::build_token(TokenType type, std::string_view sv) noexcept
{
    Token token{type, sv, std::string_view{src_ln_beg_, src_ln_end_}, line_num_, col_num_ - sv.size(), sv.size()};
    return token;
}

[[nodiscard]] Token Lexer::get_token() noexcept
{
    while (cur_ != source_.end()) {
        if (std::isspace(*cur_)) {
            skip_whitespace();
        } else if (std::isdigit(*cur_)) {
            return get_number();
        } else if (std::isalpha(*cur_)) {
            return get_identifier();
        } else if (*cur_ == '"') {
            return get_string();
        } else if (auto double_symbol = double_symbol_map.find(std::string_view{cur_, 2}); double_symbol != double_symbol_map.end()) {
            auto token = build_token(double_symbol->second, cur_, cur_ + 2);
            advance(2);
            return token;
        } else if (auto symbol = single_symbol_map.find(*cur_); symbol != single_symbol_map.end()) {
            auto token = build_token(symbol->second, cur_, cur_ + 1);
            advance();
            return token;
        } else {
            auto token = build_token(TokenType::INVALID, cur_, cur_ + 1);
            advance();
            return token;
        }
    }

    return Token{TokenType::END_OF_FILE, {}, {}, line_num_, col_num_, 1};
}

Token Lexer::get_number() noexcept
{
    auto start = cur_;

    advance();

    TokenType token_type = TokenType::INVALID;
    if (std::isdigit(*cur_)) {
        token_type = TokenType::INT_LITERAL;
    } else if (*cur_ == '.') {
        token_type = TokenType::FLOAT_LITERAL;
    } else if (std::tolower(*cur_) == 'x') {
        token_type = TokenType::HEX_LITERAL;
    }

    advance();

    auto f = token_type == TokenType::HEX_LITERAL ?
        [](char c) { return std::isalnum(c); } :
        [](char c) { return std::isdigit(c); };

    while (f(*cur_)) {
        advance();
    }

    auto end = cur_;

    return build_token(token_type, std::string_view{start, end});
}

[[nodiscard]] Token Lexer::get_identifier() noexcept
{
    auto start = cur_;

    while (std::isalnum(*cur_) || *cur_ == '_') {
        advance();
    }
    auto end = cur_;

    std::string_view value{start, end};

    if (auto kw = keywords.find(value); kw != keywords.end()) {
        return build_token(kw->second, std::string_view{start, end});
    } else if (auto t = built_in_types.find(value); t != built_in_types.end()) {
        return build_token(TokenType::TYPE, std::string_view{start, end});
    } else {
        return build_token(TokenType::IDENTIFIER, std::string_view{start, end});
    }
}

[[nodiscard]] Token Lexer::get_string() noexcept
{
    auto start = cur_;
    auto start_col_num = col_num_;

    advance();

    while (*cur_ != '"') {
        advance();
    }

    advance();

    auto end = cur_;

    return Token{TokenType::STRING_LITERAL, std::string_view{start, end}};
}

void Lexer::skip_whitespace() noexcept
{
    if (*cur_ == '\n') {
        ++line_num_;
        col_num_ = 1;
        set_line_bounds(cur_);
    } else {
        ++col_num_;
    }
    ++cur_;
}


void Lexer::set_line_bounds(const char* p) noexcept
{
    src_ln_beg_ = ++p;

    while (*p != '\n' && *p != '\0') {
        ++p;
    }

    src_ln_end_ = p;
}
