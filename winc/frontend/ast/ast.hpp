#pragma once

#include <cstdint>
#include <vector>
#include <utility>
#include <vector>

#include "utils/structures/arena.hpp"
#include "utils/enums.hpp"
#include "frontend/error/source.hpp"

class Symbol;

enum class ASTNodeKind : uint8_t
{
    TranslationUnitDecl,
    VarDecl,
    ParamDecl,
    FuncDecl,
    RecordDecl,

    CompoundStmt,
    ReturnStmt,
    BreakStmt,
    ContinueStmt,
    IfStmt,
    WhileStmt,
    ForStmt,

    IntegerLiteralExpr,
    FloatLiteralExpr,
    HexLiteralExpr,
    CharLiteralExpr,
    StringLiteralExpr,
    BooleanLiteralExpr,

    UnaryExpr,
    BinaryExpr,
    ReferenceExpr,
    CallExpr,
    MemberExpr,
    ArraySubscriptExpr,
    InitListExpr,

    QualifierTypeExpr,
    PointerTypeExpr,
    ReferenceTypeExpr,
    ArrayTypeExpr,
    NamedTypeExpr,

    Invalid
};

struct ASTNode
{
    ASTNodeKind kind_;
    Source source_;

    explicit ASTNode(ASTNodeKind kind, Source source = {}) :
        kind_{ kind },
        source_{ source } {}
};

struct Decl : ASTNode
{
    std::string name_;
    Symbol* symbol_;

    Decl(ASTNodeKind kind, std::string name, Source source = {}) :
        ASTNode{kind, source},
        name_{ std::move(name) },
        symbol_{ nullptr } {}
};

struct Stmt : ASTNode
{
    using ASTNode::ASTNode;
};

struct Expr : Stmt
{
    using Stmt::Stmt;
};

struct TypeExpr : ASTNode
{
    using ASTNode::ASTNode;
};

struct TranslationUnitDecl : Decl
{
    std::vector<ASTNode*> decls_;

    TranslationUnitDecl(std::string name) :
        Decl{ASTNodeKind::TranslationUnitDecl, std::move(name)},
        decls_{} {}
};

struct VarDecl : Decl
{
    ASTNode* type_expr_; // should be Type* through resolve_type() in parser since we call unconditionally during semantic analyzer pass
    ASTNode* init_; // single expr or init_list_expr

    VarDecl(std::string name, ASTNode* type_expr, ASTNode* init = nullptr, Source source = {}) :
        Decl{ASTNodeKind::VarDecl, std::move(name), source},
        type_expr_{ type_expr },
        init_{ init } {}
};

struct ParamDecl : Decl
{
    ASTNode* type_expr_;

    ParamDecl(std::string name, ASTNode* type_expr, Source source = {}) :
        Decl{ASTNodeKind::ParamDecl, std::move(name), source},
        type_expr_{ type_expr } {}
};

struct FuncDecl : Decl
{
    std::vector<ASTNode*> params_;
    ASTNode* return_type_expr_;
    ASTNode* body_;

    FuncDecl(std::string name, std::vector<ASTNode*> params, ASTNode* return_type_expr, ASTNode* body, Source source = {}) :
        Decl{ASTNodeKind::FuncDecl, std::move(name), source},
        params_{ std::move(params) },
        return_type_expr_{ return_type_expr },
        body_{ body } {}
};

struct RecordDecl : Decl
{
    RecordKind kind_;
    std::vector<ASTNode*> fields_;

    RecordDecl(RecordKind kind, std::string name, std::vector<ASTNode*> fields, Source source = {}) :
        Decl{ASTNodeKind::RecordDecl, std::move(name), source},
        kind_{ kind },
        fields_{ std::move(fields) } {}
};

struct CompoundStmt : Stmt
{
    std::vector<ASTNode*> children_; // exprs/decls/returns

    CompoundStmt(std::vector<ASTNode*> children) :
        Stmt{ ASTNodeKind::CompoundStmt },
        children_{ std::move(children) } {}
};

struct ReturnStmt : Stmt
{
    ASTNode* value_;

    ReturnStmt(ASTNode* value = nullptr) :
        Stmt{ ASTNodeKind::ReturnStmt },
        value_{ value } {}
};

struct BreakStmt : Stmt
{
    BreakStmt() :
        Stmt{ ASTNodeKind::BreakStmt } {}
};

struct ContinueStmt : Stmt
{
    ContinueStmt() :
        Stmt{ ASTNodeKind::ContinueStmt } {}
};

// nest IfStmt nodes for elif chains
struct IfStmt : Stmt {
    ASTNode* cond_;
    ASTNode* then_stmt_;
    ASTNode* else_stmt_; // could point to another IfStmt node for elif chaining

    IfStmt(ASTNode* cond, ASTNode* then_stmt, ASTNode* else_stmt = nullptr) :
        Stmt{ ASTNodeKind::IfStmt },
        cond_{ cond },
        then_stmt_{ then_stmt },
        else_stmt_{ else_stmt } {}
};

struct WhileStmt : Stmt
{
    ASTNode* cond_;
    ASTNode* body_;

    WhileStmt(ASTNode* cond, ASTNode* body) :
        Stmt{ ASTNodeKind::WhileStmt },
        cond_{ cond },
        body_{ body } {}
};

struct ForStmt : Stmt
{
    ASTNode* init_;
    ASTNode* cond_;
    ASTNode* update_;
    ASTNode* body_;

    ForStmt(ASTNode* init, ASTNode* cond, ASTNode* update, ASTNode* body) :
        Stmt{ ASTNodeKind::ForStmt },
        init_{ init },
        cond_{ cond },
        update_{ update },
        body_{ body } {}
};

struct IntegerLiteralExpr : Expr
{
    int64_t value_;

    IntegerLiteralExpr(int64_t value, Source source = {}) :
        Expr{ ASTNodeKind::IntegerLiteralExpr, source},
        value_{ value } {}
};

struct FloatLiteralExpr : Expr
{
    double value_;

    FloatLiteralExpr(double value, Source source = {}) :
        Expr{ASTNodeKind::FloatLiteralExpr, source},
        value_{ value } {}
};

struct HexLiteralExpr : Expr
{
    int64_t value_;

    HexLiteralExpr(int64_t value, Source source = {}) :
        Expr{ASTNodeKind::HexLiteralExpr, source},
        value_{ value } {}
};

struct CharLiteralExpr : Expr
{
    char value_;

    CharLiteralExpr(char value, Source source = {}) :
        Expr{ASTNodeKind::CharLiteralExpr, source},
        value_{ value } {}
};

struct StringLiteralExpr : Expr
{
    std::string value_;

    StringLiteralExpr(std::string value, Source source = {}) :
        Expr{ASTNodeKind::StringLiteralExpr, source},
        value_{ std::move(value) } {}
};

struct BooleanLiteralExpr : Expr
{
    bool value_;

    BooleanLiteralExpr(bool value, Source source = {}) :
        Expr{ASTNodeKind::BooleanLiteralExpr, source},
        value_{ value } {}
};

struct UnaryExpr : Expr
{
    std::string op_;
    ASTNode* operand_;
    bool is_postfix_ = false;

    UnaryExpr(std::string op, ASTNode* operand, bool is_postfix, Source source = {}) :
        Expr{ASTNodeKind::UnaryExpr, source},
        op_{ std::move(op) },
        operand_{ operand },
        is_postfix_{ is_postfix } {}
};

struct BinaryExpr : Expr
{
    std::string op_;
    ASTNode *left_, *right_;

    BinaryExpr(std::string op, ASTNode* left, ASTNode* right, Source source = {}) :
        Expr{ASTNodeKind::BinaryExpr, source},
        op_{ std::move(op) },
        left_{ left },
        right_{ right } {}
};

struct ReferenceExpr : Expr
{
    std::string name_;
    Symbol* symbol_;

    ReferenceExpr(std::string name, Source source = {}) :
        Expr{ASTNodeKind::ReferenceExpr, source},
        name_{ std::move(name) } {}
};

struct CallExpr : Expr
{
    ASTNode* callee_; // callee is a reference expression
    std::vector<ASTNode*> args_;

    CallExpr(ASTNode* callee, std::vector<ASTNode*> args, Source source = {}) :
        Expr{ASTNodeKind::CallExpr, source},
        callee_{ callee },
        args_{ std::move(args) } {}
};

struct MemberExpr : Expr
{
    ASTNode* base_;
    std::string member_;
    bool is_arrow_;

    MemberExpr(ASTNode* base, std::string member, bool is_arrow = false, Source source = {}) :
        Expr{ASTNodeKind::MemberExpr, source},
        base_{ base },
        member_{ std::move(member) },
        is_arrow_{ is_arrow } {}
};

struct ArraySubscriptExpr : Expr
{
    ASTNode* base_;
    ASTNode* index_;

    ArraySubscriptExpr(ASTNode* base, ASTNode* index, Source source = {}) :
        Expr{ASTNodeKind::ArraySubscriptExpr, source},
        base_{ base },
        index_{ index } {}
};

struct InitListExpr : Expr
{
    std::vector<ASTNode*> init_values_;
    ASTNode* target_;

    InitListExpr(std::vector<ASTNode*> init_values, Source source = {}) :
        Expr{ASTNodeKind::InitListExpr, source},
        init_values_{ std::move(init_values) },
        target_{ nullptr } {}
};

struct QualifierTypeExpr : TypeExpr
{
    QualifierKind kind_;
    ASTNode* inner_;

    QualifierTypeExpr(QualifierKind kind, ASTNode* inner) :
        TypeExpr{ ASTNodeKind::QualifierTypeExpr },
        kind_{ kind },
        inner_{ inner } {}
};

struct PointerTypeExpr : TypeExpr
{
    ASTNode* inner_;

    PointerTypeExpr(ASTNode* inner) :
        TypeExpr{ ASTNodeKind::PointerTypeExpr },
        inner_{ inner } {}
};

struct ArrayTypeExpr : TypeExpr
{
    ASTNode* inner_;
    ASTNode* size_;

    ArrayTypeExpr(ASTNode* inner, ASTNode* size = nullptr) :
        TypeExpr{ ASTNodeKind::ArrayTypeExpr },
        inner_{ inner },
        size_{ size } {}
};

struct NamedTypeExpr : TypeExpr
{
    std::string name_;

    NamedTypeExpr(std::string name) :
        TypeExpr{ ASTNodeKind::NamedTypeExpr },
        name_{ std::move(name) } {}
};

template <typename T>
inline constexpr ASTNodeKind ast_node_kind_v = ASTNodeKind::Invalid;

template <> inline constexpr ASTNodeKind ast_node_kind_v<TranslationUnitDecl> = ASTNodeKind::TranslationUnitDecl;
template <> inline constexpr ASTNodeKind ast_node_kind_v<VarDecl>             = ASTNodeKind::VarDecl;
template <> inline constexpr ASTNodeKind ast_node_kind_v<ParamDecl>           = ASTNodeKind::ParamDecl;
template <> inline constexpr ASTNodeKind ast_node_kind_v<FuncDecl>            = ASTNodeKind::FuncDecl;
template <> inline constexpr ASTNodeKind ast_node_kind_v<RecordDecl>          = ASTNodeKind::RecordDecl;
template <> inline constexpr ASTNodeKind ast_node_kind_v<CompoundStmt>        = ASTNodeKind::CompoundStmt;
template <> inline constexpr ASTNodeKind ast_node_kind_v<ReturnStmt>          = ASTNodeKind::ReturnStmt;
template <> inline constexpr ASTNodeKind ast_node_kind_v<BreakStmt>           = ASTNodeKind::BreakStmt;
template <> inline constexpr ASTNodeKind ast_node_kind_v<ContinueStmt>        = ASTNodeKind::ContinueStmt;
template <> inline constexpr ASTNodeKind ast_node_kind_v<IfStmt>              = ASTNodeKind::IfStmt;
template <> inline constexpr ASTNodeKind ast_node_kind_v<WhileStmt>           = ASTNodeKind::WhileStmt;
template <> inline constexpr ASTNodeKind ast_node_kind_v<ForStmt>             = ASTNodeKind::ForStmt;
template <> inline constexpr ASTNodeKind ast_node_kind_v<IntegerLiteralExpr>  = ASTNodeKind::IntegerLiteralExpr;
template <> inline constexpr ASTNodeKind ast_node_kind_v<FloatLiteralExpr>    = ASTNodeKind::FloatLiteralExpr;
template <> inline constexpr ASTNodeKind ast_node_kind_v<HexLiteralExpr>      = ASTNodeKind::HexLiteralExpr;
template <> inline constexpr ASTNodeKind ast_node_kind_v<CharLiteralExpr>     = ASTNodeKind::CharLiteralExpr;
template <> inline constexpr ASTNodeKind ast_node_kind_v<StringLiteralExpr>   = ASTNodeKind::StringLiteralExpr;
template <> inline constexpr ASTNodeKind ast_node_kind_v<BooleanLiteralExpr>  = ASTNodeKind::BooleanLiteralExpr;
template <> inline constexpr ASTNodeKind ast_node_kind_v<UnaryExpr>           = ASTNodeKind::UnaryExpr;
template <> inline constexpr ASTNodeKind ast_node_kind_v<BinaryExpr>          = ASTNodeKind::BinaryExpr;
template <> inline constexpr ASTNodeKind ast_node_kind_v<ReferenceExpr>       = ASTNodeKind::ReferenceExpr;
template <> inline constexpr ASTNodeKind ast_node_kind_v<CallExpr>            = ASTNodeKind::CallExpr;
template <> inline constexpr ASTNodeKind ast_node_kind_v<MemberExpr>          = ASTNodeKind::MemberExpr;
template <> inline constexpr ASTNodeKind ast_node_kind_v<ArraySubscriptExpr>  = ASTNodeKind::ArraySubscriptExpr;
template <> inline constexpr ASTNodeKind ast_node_kind_v<InitListExpr>        = ASTNodeKind::InitListExpr;
template <> inline constexpr ASTNodeKind ast_node_kind_v<QualifierTypeExpr>   = ASTNodeKind::QualifierTypeExpr;
template <> inline constexpr ASTNodeKind ast_node_kind_v<PointerTypeExpr>     = ASTNodeKind::PointerTypeExpr;
template <> inline constexpr ASTNodeKind ast_node_kind_v<ArrayTypeExpr>       = ASTNodeKind::ArrayTypeExpr;
template <> inline constexpr ASTNodeKind ast_node_kind_v<NamedTypeExpr>       = ASTNodeKind::NamedTypeExpr;

template <typename T>
concept DerivedFromASTNode = std::derived_from<T, ASTNode>;

template <typename T>
concept AbstractNode =
    std::same_as<T, Decl> ||
    std::same_as<T, Stmt> ||
    std::same_as<T, Expr>;

class AST
{
public:
    ASTNode* root_ = nullptr;

    AST() = default;
    AST(const AST&) = delete;
    AST& operator=(const AST&) = delete;
    AST(AST&&) = default;
    AST& operator=(AST&&) = default;
    ~AST() = default;

    template <DerivedFromASTNode T, typename... Args>
        requires std::is_constructible_v<T, Args...>
    [[nodiscard]] T* emplace(Args&&... args)
    {
        return arena.emplace<T>(std::forward<Args>(args)...);
    }

private:
    Arena arena;
};
