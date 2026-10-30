#pragma once

#include <unordered_map>
#include <string>

enum class RecordKind
{
    Struct,
    Union,
    Enum,
};

enum class QualifierKind
{
    Const
};

enum class BinaryOp
{
    Invalid,
    Assign,
    Add,
    Sub,
    Mul,
    Div,
    Mod,
    Xor,
    Or,
    And,
    Shl,
    Shr,
    Eq,
    Ne,
    lt,
    le,
};

inline std::unordered_map<std::string, BinaryOp> binary_ops = {
    {"=", BinaryOp::Assign},
    {"+", BinaryOp::Add},
    {"-", BinaryOp::Sub},
    {"*", BinaryOp::Mul},
    {"/", BinaryOp::Div},
    {"%", BinaryOp::Mod},
    {"^", BinaryOp::Xor},
    {"|", BinaryOp::Or},
    {"&", BinaryOp::And},
    {"<<", BinaryOp::Shl},
    {">>", BinaryOp::Shr},
    {"==", BinaryOp::Eq},
    {"!=", BinaryOp::Ne},
    {"<", BinaryOp::lt},
    {"<=", BinaryOp::le},
};
