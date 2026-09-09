#include <cassert>
#include <cstdlib>
#include <expected>

#include "frontend/parser/parser.hpp"
#include "frontend/ast/ast.hpp"
#include "frontend/lexer/token.hpp"
#include "frontend/translation_unit.hpp"
#include "frontend/error/error.hpp"
#include "utils/string_utils.hpp"
#include "utils/macros.hpp"

using namespace Syntax;

// we use Source{ prev_token } after we call expect(token)

Parser::Parser(TranslationUnit& translation_unit) noexcept :
    tranlation_unit_{ translation_unit },
    lexer_{ translation_unit },
    ast_{ } {}

AST Parser::run()
{
    parse_translation_unit();
    return std::move(ast_);
}

void Parser::eat_token() noexcept
{
    prev_token_ = cur_token_;

    if (next_token_) {
        cur_token_ = *next_token_;
        next_token_.reset();
    } else {
        cur_token_ = lexer_.lex_token();
    }
}

bool Parser::is_cur_token(const TokenType token_type) const noexcept
{
    return cur_token_.type_ == token_type ? true : false;
}

bool Parser::is_next_token(const TokenType token_type) noexcept
{
    next_token_ = lexer_.lex_token();
    return next_token_->type_ == token_type ? true : false;
}

// there is an error with the panic mechanism

void Parser::panic(const std::optional<Error> error) noexcept
{
    if (error) {
        tranlation_unit_.diagnostics_.register_error(error.value());
    }

    while (!is_cur_token(TokenType::KEYWORD_STRUCT) &&
           !is_cur_token(TokenType::KEYWORD_FUNCTION) &&
           !is_cur_token(TokenType::KEYWORD_VAR) &&
           !is_cur_token(TokenType::RBRACE) &&
           !is_cur_token(TokenType::SEMICOLON) &&
           !is_cur_token(TokenType::END_OF_FILE)) {
        eat_token();
    }

    eat_token();
}

std::expected<std::string_view, Error> Parser::match(TokenType token_type) noexcept
{
    if (is_cur_token(token_type)) {
        auto lexeme = cur_token_.lexeme_;
        eat_token();

        return lexeme;
    }

    return std::unexpected{ SyntaxError{ prev_token_ } };
}

void Parser::parse_translation_unit() noexcept
{
    TranslationUnitDecl* translation_unit = ast_.emplace<TranslationUnitDecl>(translation_unit->name_);

    eat_token();

    while (!is_cur_token(TokenType::END_OF_FILE)) {
        auto decl = parse_decl();

        if (!decl)
            panic(decl.error());
        else
            translation_unit->decls_.push_back(*decl);
    }

    ast_.root_ = translation_unit;
}

// var <ident>: parse_type()
std::expected<TypeExpr*, Error> Parser::parse_type() noexcept
{
    bool is_const = false;
    if (is_cur_token(TokenType::KEYWORD_CONST)) {
        is_const = true;
        eat_token();
    }

    if (!is_cur_token(TokenType::IDENTIFIER) && !is_cur_token(TokenType::TYPE))
        return std::unexpected{ SyntaxError{cur_token_, "Expecting built-in or user-defined type"} };

    auto type_name = std::string{ cur_token_.lexeme_ };
    TypeExpr* type = ast_.emplace<NamedTypeExpr>(std::move(type_name));
    eat_token();

    if (is_const)
        type = ast_.emplace<QualifierTypeExpr>(QualifierKind::Const, type);

    switch (cur_token_.type_) {
        case TokenType::LBRACKET: {
            eat_token();

            if (is_cur_token(TokenType::RBRACKET)) {
                eat_token();
                return ast_.emplace<ArrayTypeExpr>(type);
            }

            auto size = parse_expr();
            if (!size) return std::unexpected{ size.error() };

            expect(TokenType::RBRACKET);
            return ast_.emplace<ArrayTypeExpr>(type, *size);
        }

        case TokenType::STAR: {
            eat_token();
            return ast_.emplace<PointerTypeExpr>(type);
        }

        default:
            return type;
    }
}

// fn, struct, var -> all decls possible
std::expected<Decl*, Error> Parser::parse_decl() noexcept
{
    switch (cur_token_.type_) {
        case TokenType::KEYWORD_FUNCTION: {
            eat_token();

            auto func_decl = parse_func_decl();
            if (!func_decl) return std::unexpected{ func_decl.error() };

            return *func_decl;
        }

        case TokenType::KEYWORD_STRUCT: {
            eat_token();

            auto s = parse_struct_def();
            if (!s) return std::unexpected{ s.error() };

            return *s;
        }

        default: {
            return std::unexpected{ SyntaxError{ cur_token_ } };
        }
    }
}

// x: int;
// x: int = 0;

// x: Point;
// x: Point = {};
// x: Point = {1, 2}

// x: int[4];
// x: int[4] = {}
// x: int[4] = {1, 2, 3, 4}

std::expected<Decl*, Error> Parser::parse_var_decl() noexcept
{
    auto [name] = expect(TokenType::IDENTIFIER);
    if (!name) return std::unexpected{ name.error() };

    Source source{ prev_token_ };

    EXPECT_COLON();

    auto type = parse_type();
    if (!type) return std::unexpected{ type.error() };

    if (is_cur_token(TokenType::EQUAL)) {
        eat_token();

        if (is_cur_token(TokenType::LBRACE)) {
            eat_token();

            auto expr = parse_init_list_expr();
            if (!expr) return std::unexpected{ expr.error() };

            EXPECT_SEMICOLON();

            return ast_.emplace<VarDecl>(std::string{ *name }, *type, *expr, source);
        }

        auto expr = parse_expr();
        if (!expr) return std::unexpected{ expr.error() };

        EXPECT_SEMICOLON();

        return ast_.emplace<VarDecl>(std::string{ *name }, *type, *expr, source);
    } else {
        EXPECT_SEMICOLON();

        return ast_.emplace<VarDecl>(std::string{ *name }, *type, nullptr, source);
    }
}

std::expected<Decl*, Error> Parser::parse_func_decl() noexcept
{
    auto [name] = expect(TokenType::IDENTIFIER);
    if (!name) return std::unexpected{ SyntaxError{cur_token_, "missing function identifier"} };

    Source source{ prev_token_ };

    EXPECT_LPAREN();

    std::vector<ASTNode*> params;

    while (!is_cur_token(TokenType::RPAREN)) {
        auto param_decl = parse_param_decl();
        if (!param_decl) return std::unexpected{ param_decl.error() };

        params.push_back(*param_decl);

        if (is_cur_token(TokenType::COMMA)) eat_token();
    }

    eat_token();

    auto [arrow] = expect(TokenType::ARROW);
    if (!arrow) return std::unexpected{ SyntaxError{cur_token_, "function missing trailing return type"} };

    auto return_type = parse_type();
    if (!return_type) return std::unexpected{ return_type.error() };

    EXPECT_LBRACE();

    auto body = parse_compound_stmt();
    if (!body) return std::unexpected{ body.error() };

    return ast_.emplace<FuncDecl>(std::string{ *name }, std::move(params), *return_type, *body, source);
}

std::expected<Decl*, Error> Parser::parse_param_decl() noexcept
{
    auto [name] = expect(TokenType::IDENTIFIER);
    if (!name) return std::unexpected{ name.error() };

    Source source{ prev_token_ };

    EXPECT_COLON();

    auto type = parse_type();
    if (!type) return std::unexpected{ type.error() };

    return ast_.emplace<ParamDecl>(std::string{ *name }, *type, source);
}

std::expected<Decl*, Error> Parser::parse_struct_def() noexcept
{
    auto [name] = expect(TokenType::IDENTIFIER);
    if (!name) return std::unexpected{SyntaxError{cur_token_, "missing struct identifier"}};

    Source source{ prev_token_ };

    EXPECT_LBRACE();

    std::vector<ASTNode*> fields;

    while (!is_cur_token(TokenType::RBRACE)) {
        auto field = parse_field();

        if (!field) return std::unexpected{ field.error() };

        fields.push_back(*field);
    }

    EXPECT_RBRACE();

    return ast_.emplace<RecordDecl>(RecordKind::Struct, std::string{ *name }, std::move(fields), source);
}

std::expected<Decl*, Error> Parser::parse_field() noexcept
{
    return parse_var_decl();
}

std::expected<Stmt*, Error> Parser::parse_return_stmt() noexcept
{
    if (is_cur_token(TokenType::SEMICOLON))
        return ast_.emplace<ReturnStmt>();

    auto expr = parse_expr();
    if (!expr) return std::unexpected{ expr.error() };

    EXPECT_SEMICOLON();

    return ast_.emplace<ReturnStmt>(*expr);
}

// if / while / for statements can only occur in compound statements
std::expected<Stmt*, Error> Parser::parse_compound_stmt() noexcept
{
    std::vector<ASTNode*> children;

    while (!is_cur_token(TokenType::RBRACE)) {
        switch (cur_token_.type_) {
            case TokenType::IDENTIFIER: {
                if (is_next_token(TokenType::COLON)) {
                    auto decl = parse_var_decl();
                    if (!decl) return std::unexpected{ decl.error() };

                    children.push_back(*decl);
                    break;
                } else {
                    auto expr = parse_expr();
                    if (!expr) return std::unexpected{ expr.error() };

                    EXPECT_SEMICOLON();
                    children.push_back(*expr);
                    break;
                }
            }

            case TokenType::KEYWORD_FOR: {
                eat_token();

                auto for_loop = parse_for_loop();
                if (!for_loop) return std::unexpected { for_loop.error() };

                children.push_back(*for_loop);
                break;
            }

            case TokenType::KEYWORD_WHILE: {
                eat_token();

                auto while_loop = parse_while_loop();
                if (!while_loop) return std::unexpected { while_loop.error() };

                children.push_back(*while_loop);
                break;
            }

            case TokenType::KEYWORD_IF: {
                eat_token();

                auto if_stmt = parse_if_stmt();
                if (!if_stmt) return std::unexpected { if_stmt.error() };

                children.push_back(*if_stmt);
                break;
            }

            case TokenType::KEYWORD_RETURN: {
                eat_token();

                auto ret = parse_return_stmt();
                if (!ret) return std::unexpected{ ret.error() };

                children.push_back(*ret);
                break;
            }

            case TokenType::KEYWORD_BREAK: {
                eat_token();

                auto b = ast_.emplace<BreakStmt>();
                children.push_back(b);

                EXPECT_SEMICOLON();

                break;
            }

            case TokenType::KEYWORD_CONTINUE: {
                eat_token();

                auto b = ast_.emplace<ContinueStmt>();
                children.push_back(b);

                EXPECT_SEMICOLON();

                break;
            }

            default: {
                auto expr = parse_expr();
                if (!expr) return std::unexpected{ expr.error() };

                EXPECT_SEMICOLON();
                children.push_back(*expr);
                break;
            }
        }
    }


    EXPECT_RBRACE();

    return ast_.emplace<CompoundStmt>(std::move(children));
}

std::expected<Stmt*, Error> Parser::parse_while_loop() noexcept
{
    EXPECT_LPAREN();

    auto expr = parse_expr();
    if (!expr) return std::unexpected{ expr.error() };

    EXPECT_RPAREN();

    EXPECT_LBRACE();

    auto compound_stmt = parse_compound_stmt();
    if (!compound_stmt) return std::unexpected{ compound_stmt.error() };

    return ast_.emplace<WhileStmt>(*expr, *compound_stmt);
}

std::expected<Stmt*, Error> Parser::parse_for_loop() noexcept
{
    EXPECT_LPAREN();

    auto init = parse_var_decl();
    if (!init) return std::unexpected{ init.error() };

    auto cond = parse_expr();
    if (!cond) return std::unexpected{ cond.error() };

    EXPECT_SEMICOLON();

    auto update = parse_expr();
    if (!update) return std::unexpected{ update.error() };

    EXPECT_RPAREN();

    EXPECT_LBRACE();

    auto body = parse_compound_stmt();
    if (!body) return std::unexpected{ body.error() };

    return ast_.emplace<ForStmt>(*init, *cond, *update, *body);
}

std::expected<Stmt*, Error> Parser::parse_if_stmt() noexcept
{
    EXPECT_LPAREN();

    auto cond = parse_expr();
    if (!cond) return std::unexpected{ cond.error() };

    EXPECT_RPAREN();

    EXPECT_LBRACE();
    auto then_branch_body = parse_compound_stmt();
    if (!then_branch_body) return std::unexpected{ then_branch_body.error() };

    if (is_cur_token(TokenType::KEYWORD_ELSE)) {
        eat_token();

        EXPECT_LBRACE();

        auto else_branch_body = parse_compound_stmt();
        if (!else_branch_body) return std::unexpected{ else_branch_body.error() };

        return ast_.emplace<IfStmt>(*cond, *then_branch_body, *else_branch_body);
    }

    return ast_.emplace<IfStmt>(*cond, *then_branch_body);
}

namespace prec
{
    constexpr int32_t assignment  = 10;  // = += -= *= /= %=
    constexpr int32_t logical_or  = 20;  // ||
    constexpr int32_t logical_and = 30;  // &&
    constexpr int32_t bitwise_or  = 40;  // |
    constexpr int32_t bitwise_xor = 50;  // ^
    constexpr int32_t bitwise_and = 60;  // &
    constexpr int32_t equality    = 70;  // == !=
    constexpr int32_t cond        = 80;  // < > <= >=
    constexpr int32_t shift       = 90;  // << >>
    constexpr int32_t add         = 100; // + -
    constexpr int32_t mult        = 110; // * / % ** (might keep ** power to)
    constexpr int32_t unary       = 120; // + - ! ~ & * ++ -- (pre)
    constexpr int32_t postfix     = 130; // () [] . -> ++ -- (post)
}

auto infix_lbp(Token token) noexcept {
    switch (token.type_) {
        case TokenType::EQUAL:
        case TokenType::PLUS_EQUAL:
        case TokenType::MINUS_EQUAL:
        case TokenType::STAR_EQUAL:
        case TokenType::SLASH_EQUAL:
        case TokenType::PERCENT_EQUAL:
            return prec::assignment;

        case TokenType::PIPE_PIPE:
            return prec::logical_or;

        case TokenType::AMPERSAND_AMPERSAND:
            return prec::logical_and;

        case TokenType::PIPE:
            return prec::bitwise_or;

        case TokenType::CARROT:
            return prec::bitwise_xor;

        case TokenType::AMPERSAND:
            return prec::bitwise_and;

        case TokenType::EQUAL_EQUAL:
        case TokenType::BANG_EQUAL:
            return prec::equality;

        case TokenType::LESS:
        case TokenType::GREATER:
        case TokenType::LESS_EQUAL:
        case TokenType::GREATER_EQUAL:
            return prec::cond;

        case TokenType::LESS_LESS:
        case TokenType::GREATER_GREATER:
            return prec::shift;

        case TokenType::PLUS:
        case TokenType::MINUS:
            return prec::add;

        case TokenType::STAR:
        case TokenType::SLASH:
        case TokenType::PERCENT:
        case TokenType::STAR_STAR:
            return prec::mult;

        case TokenType::LPAREN:      // call
        case TokenType::LBRACKET:    // index
        case TokenType::DOT:         // member access
        case TokenType::ARROW:       // member access
        case TokenType::PLUS_PLUS:
        case TokenType::MINUS_MINUS:
            return prec::postfix;

        default:
            return 0;
    }
}

std::expected<Expr*, Error> Parser::nud(const Token token) noexcept
{
    eat_token();

    switch (token.type_) {
        case TokenType::STAR:
        case TokenType::STAR_STAR:
        case TokenType::AMPERSAND:
        case TokenType::PLUS:
        case TokenType::PLUS_PLUS:
        case TokenType::MINUS:
        case TokenType::MINUS_MINUS:
        case TokenType::BANG:
        case TokenType::CARROT:
        case TokenType::TILDE: {
            auto operand = parse_expr(prec::unary);
            if (!operand) return std::unexpected{ operand.error() };

            return ast_.emplace<UnaryExpr>(std::string{ token.lexeme_ }, *operand, false, Source{ prev_token_ }); // postfix = false (default value)
        }

        case TokenType::IDENTIFIER: {
            return ast_.emplace<ReferenceExpr>(std::string{ token.lexeme_ }, Source{ prev_token_ });
        }

        case TokenType::INT_LITERAL: {
            auto value = sv_to_numeric<int64_t>(token.lexeme_);
            if (!value) return std::unexpected{ SyntaxError{cur_token_} };

            return ast_.emplace<IntegerLiteralExpr>(*value);
        }

        case TokenType::FLOAT_LITERAL: {
            auto value = sv_to_numeric<double>(token.lexeme_);
            if (!value) return std::unexpected{ SyntaxError{cur_token_} };

            return ast_.emplace<FloatLiteralExpr>(*value);
        }

        case TokenType::HEX_LITERAL: {
            auto value = sv_to_numeric<int64_t>(token.lexeme_);
            if (!value) return std::unexpected{ SyntaxError{cur_token_} };

            return ast_.emplace<IntegerLiteralExpr>(*value);
        }

        case TokenType::STRING_LITERAL: {
            return ast_.emplace<StringLiteralExpr>(std::string{ token.lexeme_ });
        }

        case TokenType::KEYWORD_TRUE: {
            return ast_.emplace<BooleanLiteralExpr>(true);
        }

        case TokenType::KEYWORD_FALSE: {
            return ast_.emplace<BooleanLiteralExpr>(false);
        }

        case TokenType::LPAREN: {
            auto expr = parse_expr();
            if (!expr) return std::unexpected{ expr.error() };

            EXPECT_RPAREN();
            return expr;
        }

        default:
            return std::unexpected{ SyntaxError{token, "unexpected symbol in expression"} };
    }
}

std::expected<Expr*, Error> Parser::led(const Token token, Expr* left) noexcept
{
    Source source{ cur_token_ }; // -> func(x, y, z) makes 'func' the offender

    eat_token();

    switch (token.type_) {
        case TokenType::EQUAL:
        case TokenType::PLUS_EQUAL:
        case TokenType::MINUS_EQUAL:
        case TokenType::STAR_EQUAL:
        case TokenType::SLASH_EQUAL:
        case TokenType::PERCENT_EQUAL:
        case TokenType::PLUS:
        case TokenType::MINUS:
        case TokenType::STAR:
        case TokenType::SLASH:
        case TokenType::PERCENT:
        case TokenType::LESS:
        case TokenType::LESS_EQUAL:
        case TokenType::GREATER:
        case TokenType::GREATER_EQUAL:
        case TokenType::EQUAL_EQUAL:
        case TokenType::BANG_EQUAL:
        case TokenType::AMPERSAND:
        case TokenType::PIPE:
        case TokenType::CARROT:
        case TokenType::LESS_LESS:
        case TokenType::GREATER_GREATER:
        case TokenType::AMPERSAND_AMPERSAND:
        case TokenType::PIPE_PIPE:
        case TokenType::STAR_STAR: {
            auto right = parse_expr(infix_lbp(token));
            if (!right) return std::unexpected{ right.error() };

            return ast_.emplace<BinaryExpr>(std::string{ token.lexeme_ }, left, *right, source);
        }

        case TokenType::PLUS_PLUS:
        case TokenType::MINUS_MINUS: {
            return ast_.emplace<UnaryExpr>(std::string{token.lexeme_}, left, true, source);  // postfix = true, since this is only time true, bool seems okay for now, move to clear enum later
        }

        // func(x)
        case TokenType::LPAREN: {
            std::vector<ASTNode*> args;

            while (!is_cur_token(TokenType::RPAREN)) {
                auto arg = parse_expr();
                if (!arg) return std::unexpected{ arg.error() };

                args.push_back(*arg);

                if (is_cur_token(TokenType::COMMA)) eat_token();
            }

            EXPECT_RPAREN();
            return ast_.emplace<CallExpr>(left, std::move(args), source);
        }

        // arr[x]
        case TokenType::LBRACKET: {
            auto index = parse_expr();
            if (!index) return std::unexpected{ index.error() };

            EXPECT_RBRACKET();
            return ast_.emplace<ArraySubscriptExpr>(left, *index, source);
        }

        case TokenType::DOT:
        case TokenType::ARROW: {
            auto [member_name] = expect(TokenType::IDENTIFIER);
            if (!member_name) return std::unexpected{ member_name.error() };

            auto is_arrow = token.type_ == TokenType::ARROW ? true : false;

            return ast_.emplace<MemberExpr>(left, std::string{ *member_name }, is_arrow, source);
        }

        default:
            return std::unexpected{ SyntaxError{token} };
    }
}

std::expected<Expr*, Error> Parser::parse_expr(int min_prec) noexcept
{
    auto left = nud(cur_token_);
    if (!left) return std::unexpected{ left.error() };

    while (true) {
        auto lbp = infix_lbp(cur_token_);

        if (lbp <= min_prec) break;

        auto next = led(cur_token_, *left);
        if (!next) return std::unexpected{ next.error() };

        left = *next;
    }

    return left;
}

std::expected<Expr*, Error> Parser::parse_init_list_expr() noexcept
{
    Source source{ cur_token_ };

    eat_token();

    std::vector<ASTNode*> init_values;

    while (!is_cur_token(TokenType::RBRACE)) {
        auto expr = parse_expr();
        if (!expr) return std::unexpected{ expr.error() };

        init_values.push_back(*expr);

        if (is_cur_token(TokenType::COMMA)) eat_token();
    }

    EXPECT_RBRACE();

    return ast_.emplace<InitListExpr>(std::move(init_values), source);
}
