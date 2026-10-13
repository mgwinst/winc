#pragma once

#include <vector>

#include "types.hpp"

inline std::size_t hash_combine(std::size_t seed, std::size_t value)
{
    return seed ^ (value + 0x9e3779b9 + (seed << 6) + (seed >> 2));
}

inline std::size_t hash_combine(std::size_t seed, void* ptr)
{
    std::uintptr_t addr = reinterpret_cast<std::uintptr_t>(ptr);
    return seed ^ (addr + 0x9e3779b9 + (seed << 6) + (seed >> 2));
}

inline std::size_t hash_vector(std::size_t seed, const std::vector<Type*>& vec)
{
    auto h = seed;
    for (auto v : vec)
    {
        h = hash_combine(h, v);
    }

    return h;
}

template <typename T>
struct TypeHasher;

template <>
struct TypeHasher<PointerType>
{
    static constexpr std::size_t magic = 0x2d4f6e8a9b1c3d5fULL;

    static std::size_t hash(Type* inner_type)
    {
        return hash_combine(magic, inner_type);
    }
};

template <>
struct TypeHasher<ArrayType>
{
    static constexpr std::size_t magic = 0x3e6d8f9ab1c2d4e7ULL;

    static std::size_t hash(Type* inner_type, uint64_t size)
    {
        auto h = hash_combine(magic, inner_type);
        return hash_combine(h, size);
    }
};

template <>
struct TypeHasher<QualifierType>
{
    static constexpr std::size_t magic = 0x4f7e9a0bc1d2e3f8ULL;

    static std::size_t hash(QualifierKind q, Type* inner_type)
    {
        auto h = hash_combine(magic, static_cast<std::size_t>(q));
        return hash_combine(h, inner_type);
    }
};

template <>
struct TypeHasher<FunctionType>
{
    static constexpr std::size_t magic = 0x5a8cb9d0e1f20364ULL;

    static std::size_t hash(const std::string& name, const std::vector<Type*>& params, Type* ret)
    {
        auto h = hash_combine(magic, std::hash<std::string>{}(name));
        h = hash_vector(h, params);
        return hash_combine(h, ret);
    }
};

template <>
struct TypeHasher<RecordType>
{
    static constexpr std::size_t magic = 0x6e2fb3d9a3f20581ULL;

    static std::size_t hash(RecordKind kind, const std::string& name)
    {
        auto h = hash_combine(magic, static_cast<std::size_t>(kind));
        return hash_combine(h, std::hash<std::string>{}(name));
    }
};
