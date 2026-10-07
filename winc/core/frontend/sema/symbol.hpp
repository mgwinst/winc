#pragma once

#include <cstddef>
#include <string>
#include <unordered_map>
#include <vector>
#include <ranges>
#include <optional>

#include "frontend/ast/ast.hpp"
#include "frontend/sema/types/types.hpp"

enum class StorageClass
{
    Auto,
    Static,
};

enum class Linkage
{
    Internal,
    External,
    Weak,
    None
};

struct Symbol
{
    std::string identifier_;
    Type* type_;
    StorageClass storage_;
    Linkage linkage_;

    Symbol(std::string_view identifier, Type* type, StorageClass storage = StorageClass::Auto, Linkage linkage = Linkage::External) :
        identifier_{ identifier },
        type_{ type },
        storage_{ storage },
        linkage_{ linkage } {}
};

struct SymbolTable
{
    std::vector<std::unordered_multimap<std::string, Symbol*>> scopes_;
    Arena arena_;

    SymbolTable() = default;
    SymbolTable(const SymbolTable&) = delete;
    SymbolTable& operator=(const SymbolTable&) = delete;
    SymbolTable(SymbolTable&&) = default;
    SymbolTable& operator=(SymbolTable&&) = default;
    ~SymbolTable() = default;

    auto& cur_scope()
    {
        assert(!scopes_.empty());
        return scopes_.back();
    }

    Symbol* insert(Decl* node, Type* type)
    {
        auto* symbol = arena_.emplace<Symbol>(node->name_, type);
        cur_scope().emplace(node->name_, symbol);
        return symbol;
    }

    using SymbolMap = std::unordered_multimap<std::string, Symbol*>;
    using SymbolRange = std::ranges::subrange<SymbolMap::iterator, SymbolMap::iterator, std::ranges::subrange_kind::sized>;

    std::optional<SymbolRange> lookup(const std::string& identifier) {
        for (auto& scope : std::views::reverse(scopes_)) {
            auto [first, last] = scope.equal_range(identifier);
            if (first != last) {
                return SymbolRange{first, last, static_cast<std::size_t>(std::distance(first, last))};
            }
        }

        return std::nullopt;
    }

    bool exists_in_scope(const std::string& identifier)
    {
        return cur_scope().contains(identifier);
    }

    void enter_scope()
    {
        scopes_.emplace_back();
    }

    void exit_scope()
    {
        assert(!scopes_.empty());
        scopes_.pop_back();
    }
};
