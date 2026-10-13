#pragma once

#include <ranges>
#include <cassert>

#include "frontend/ast/ast.hpp"
#include "frontend/sema/types/types.hpp"

template <AbstractNode T>
bool category(auto* ptr)
{
    if constexpr (std::same_as<T, Decl>)
        return ptr->kind_ >= ASTNodeKind::TranslationUnitDecl && ptr->kind_ <= ASTNodeKind::RecordDecl;
    else if constexpr (std::same_as<T, Stmt>)
        return ptr->kind_ >= ASTNodeKind::CompoundStmt && ptr->kind_ <= ASTNodeKind::ForStmt;
    else
        return ptr->kind_ >= ASTNodeKind::IntegerLiteralExpr && ptr->kind_ <= ASTNodeKind::InitListExpr;
}

template <DerivedFromASTNode Derived>
bool isa(ASTNode* ptr)
{
    if constexpr (AbstractNode<Derived>)
        return category<Derived>(ptr);

    return ptr && ptr->kind_ == ast_node_kind_v<Derived>;
}

template <DerivedFromType Derived>
bool isa(Type* ptr)
{
    return ptr && ptr->kind_ == type_kind_v<Derived>;
}

template <typename T, typename U>
T* cast(U* ptr)
{
    assert(isa<T>(ptr));
    return static_cast<T*>(ptr);
}

template <typename T, typename U>
T* dyn_cast(U* ptr)
{
    return isa<T>(ptr) ? static_cast<T*>(ptr) : nullptr;
}

template <typename T, typename U>
T* dyn_cast(const std::unique_ptr<U>& ptr)
{
    return isa<T>(ptr.get()) ? static_cast<T*>(ptr.get()) : nullptr;
}

// range casting

template <typename T>
auto static_cast_view(auto&& range)
{
    return std::forward<decltype(range)>(range)
        | std::views::filter([](auto* p) { return p != nullptr; })
        | std::views::transform([](auto* p) { return static_cast<T*>(p); });
}
