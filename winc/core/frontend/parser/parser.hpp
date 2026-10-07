#pragma once

#include <expected>
#include <tuple>

#include "frontend/translation_unit.hpp"
#include "frontend/lexer/lexer.hpp"
#include "frontend/ast/ast.hpp"

struct Parser
{
    TranslationUnit& tranlation_unit_;
    Lexer lexer_;
    AST ast_;
    Token prev_token_, cur_token_;
    std::optional<Token> next_token_;

    Parser(TranslationUnit& translation_unit) noexcept;

    Parser(const Parser&) = delete;
    Parser& operator=(const Parser&) = delete;
    Parser(Parser&&) = delete;
    Parser& operator=(Parser&&) = delete;
    ~Parser() = default;

    AST run();
    void eat_token() noexcept;
    bool is_cur_token(const TokenType token_type) const noexcept;
    bool is_next_token(const TokenType token_type) noexcept;
    void panic(const std::optional<Error> error = std::nullopt) noexcept;
    std::expected<std::string_view, Error> match(TokenType token_type) noexcept;

    auto expect(std::same_as<TokenType> auto... token_types) noexcept
    {
        return std::tuple{ match(token_types)... };
    }

    void parse_translation_unit() noexcept;

    std::expected<Decl*, Error> parse_decl() noexcept;
    std::expected<Decl*, Error> parse_var_decl() noexcept;
    std::expected<Decl*, Error> parse_param_decl() noexcept;
    std::expected<Decl*, Error> parse_func_decl() noexcept;
    std::expected<Decl*, Error> parse_struct_def() noexcept;
    std::expected<Decl*, Error> parse_field_decl() noexcept;
    std::expected<Stmt*, Error> parse_return_stmt() noexcept;
    std::expected<Stmt*, Error> parse_compound_stmt() noexcept;
    std::expected<Stmt*, Error> parse_while_loop() noexcept;
    std::expected<Stmt*, Error> parse_for_loop() noexcept;
    std::expected<Stmt*, Error> parse_if_stmt() noexcept;
    std::expected<Expr*, Error> parse_expr(int min_prec = 0) noexcept;
    std::expected<Expr*, Error> parse_init_list_expr() noexcept;
    std::expected<TypeExpr*, Error> parse_type() noexcept;
    std::expected<Expr*, Error> nud(const Token token) noexcept;
    std::expected<Expr*, Error> led(const Token token, Expr* left) noexcept;
};
