#pragma once

#include <unordered_map>
#include <boost/container/small_vector.hpp>

#include "utils/structures/arena.hpp"
#include "frontend/ast/ast.hpp"
#include "type_hash.hpp"
#include "types.hpp"

using boost::container::small_vector;

class TypeTable
{
public:
    std::unordered_map<std::size_t, small_vector<Type*, 8>> buckets_;
    std::unordered_map<std::string, Type*> builtin_types_;
    std::unordered_map<std::string, Type*> user_defined_types_;

    TypeTable();

    TypeTable(const TypeTable&) = delete;
    TypeTable& operator=(const TypeTable&) = delete;
    TypeTable(TypeTable&&) = default;
    TypeTable& operator=(TypeTable&&) = default;
    ~TypeTable() = default;

    Type* resolve_type(ASTNode* type_expr);

    template <DerivedFromType T, typename... Args>
        requires std::is_constructible_v<T, Args...>
    [[nodiscard]] Type* get_or_create(Args&&... args)
    {
        auto& bucket = buckets_[TypeHasher<T>::hash(args...)];

        for (Type* ptr : bucket) {
            if (T* p = dyn_cast<T>(ptr)) {
                if (*p == T{ args... }) {
                    return ptr;
                }
            }
        }

        Type* ptr = arena.emplace<T>(std::forward<Args>(args)...);
        bucket.push_back(ptr);

        return ptr;
    }

private:
    Arena arena;
};
