#pragma once

#include <cstddef>
#include <utility>
#include <vector>
#include <cassert>

#include "utils/enums.hpp"

enum class TypeKind
{
    Void,
    Byte,
    Char,
    Bool,
    Integer,
    Float,
    Pointer,
    Array,
    Qualifier,
    Function,
    Record,
    Invalid
};

struct Type
{
    const TypeKind kind_;

    explicit Type(const TypeKind kind) :
        kind_{ kind } {}

    bool operator==(const Type&) const = default;
};

template <typename T>
concept DerivedFromType = std::derived_from<T, Type>;

struct VoidType : Type
{
    VoidType() :
        Type{ TypeKind::Void } {}

    bool operator==(const VoidType&) const { return true; }
};

struct ByteType : Type
{
    ByteType() :
        Type{ TypeKind::Byte } {}

    bool operator==(const ByteType&) const { return true; }
};

struct CharType : Type
{
    CharType() :
        Type{ TypeKind::Char } {}

    bool operator==(const CharType&) const { return true; }
};

struct BoolType : Type
{
    BoolType() :
        Type{ TypeKind::Bool } {}

    bool operator==(const BoolType&) const { return true; }
};

struct IntegerType : Type
{
    uint16_t bit_width_;
    bool is_signed_;

    IntegerType(uint16_t bit_width, bool is_signed) :
        Type{ TypeKind::Integer },
        bit_width_{ bit_width },
        is_signed_{ is_signed } {}

    bool operator==(const IntegerType& other) const = default;
};

struct FloatType : Type
{
    uint16_t bit_width_;

    FloatType(uint16_t bit_width) :
        Type{ TypeKind::Float },
        bit_width_{ bit_width } {}

    bool operator==(const FloatType& other) const = default;
};

struct PointerType : Type
{
    Type* inner_type_;

    PointerType(Type* inner_type) :
        Type{ TypeKind::Pointer },
        inner_type_{ inner_type } {}

    bool operator==(const PointerType& other) const = default;
};

struct ArrayType : Type
{
    Type* inner_type_;
    uint64_t size_;

    ArrayType(Type* inner_type, uint64_t size) :
        Type{ TypeKind::Array },
        inner_type_{ inner_type },
        size_{ size } {}

    bool operator==(const ArrayType& other) const = default;
};

struct QualifierType : Type
{
    QualifierKind kind_;
    Type* inner_type_;

    QualifierType(QualifierKind kind, Type* inner_type) :
        Type{ TypeKind::Qualifier },
        kind_{ kind },
        inner_type_{ inner_type } {}

    bool operator==(const QualifierType& other) const = default;
};

/*
struct Parameter
{

}
*/

struct FunctionType : Type
{
    std::string name_;
    std::vector<Type*> params_;
    Type* return_type_;

    FunctionType(std::string name, std::vector<Type*> params, Type* return_type) :
        Type{ TypeKind::Function },
        name_{ std::move(name) },
        params_{ std::move(params) },
        return_type_{ return_type } {}

    bool operator==(const FunctionType& other) const = default;
};

// fields must know if they are being accessed through a const reference

struct Field
{
    std::string name_;
    Type* type_;
    // uint16_t offset

    bool operator==(const Field& other) const = default;
};

struct RecordType : Type
{
    RecordKind kind_;
    std::string name_;
    std::vector<Field> fields_; // should be a hashmap instead
    // uint32_t size_;
    // uint16_t alignment

    RecordType(RecordKind kind, std::string name) :
        Type{ TypeKind::Record },
        kind_{ kind },
        name_{ std::move(name) } {}

    bool operator==(const RecordType& other) const = default;

    auto* lookup_field(std::string_view name)
    {
        auto it = std::find_if(fields_.begin(), fields_.end(), [&](const Field& f) { return f.name_ == name; });
        return (it != fields_.end()) ? &(*it) : nullptr;
    }

    uint32_t field_position(std::string_view name) const
    {
        auto field = std::find_if(fields_.begin(), fields_.end(), [&](const Field& f) { return f.name_ == name; });
        assert(field != fields_.end());
        return std::distance(fields_.begin(), field);
    }
};

template <typename T>
inline constexpr TypeKind type_kind_v = TypeKind::Invalid;

template <> inline constexpr TypeKind type_kind_v<VoidType>      = TypeKind::Void;
template <> inline constexpr TypeKind type_kind_v<ByteType>      = TypeKind::Byte;
template <> inline constexpr TypeKind type_kind_v<CharType>      = TypeKind::Char;
template <> inline constexpr TypeKind type_kind_v<BoolType>      = TypeKind::Bool;
template <> inline constexpr TypeKind type_kind_v<IntegerType>   = TypeKind::Integer;
template <> inline constexpr TypeKind type_kind_v<FloatType>     = TypeKind::Float;
template <> inline constexpr TypeKind type_kind_v<PointerType>   = TypeKind::Pointer;
template <> inline constexpr TypeKind type_kind_v<ArrayType>     = TypeKind::Array;
template <> inline constexpr TypeKind type_kind_v<QualifierType> = TypeKind::Qualifier;
template <> inline constexpr TypeKind type_kind_v<FunctionType>  = TypeKind::Function;
template <> inline constexpr TypeKind type_kind_v<RecordType>    = TypeKind::Record;
