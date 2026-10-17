#pragma once

#include <unordered_set>
#include <unordered_map>
#include <string_view>
#include <cstdint>
#include <concepts>
#include <algorithm>
#include <memory>
#include <vector>
#include <list>
#include <string>
#include <ranges>
#include <utility>
#include <cassert>
#include <format>

#include "boost/container/small_vector.hpp"

#include "frontend/sema/types/types.hpp"

using boost::container::small_vector;

enum class ValueKind
{
    Invalid,
    Function,
    BasicBlock,
    Argument,
    Alloca,
    Store,
    Load,
    Add,
    Sub,
    Mul,
    Div,
    Mod,
    Eq,
    Ne,
    Not,
    Xor,
    Or,
    And,
    Slt,
    Sle,
    Ult,
    Ule,
    Shl,
    Ashr,
    Lshr,
    Call,
    PtrAdd,
    Phi,
    Return,
    Branch,
    Const,
};

struct Instruction;

struct Value
{
    ValueKind kind_;
    Type* type_;
    std::string name_;
    small_vector<Value*, 4> users_;
    Value* parent_;

    Value(ValueKind kind, Type* type = nullptr, std::string_view name = "", Value* parent = nullptr);

    virtual ~Value() = default;

    void add_use(Value* value);
    void replace_uses_with(Value* value);
};

template <typename T>
concept DerivedFromValue = std::derived_from<T, Value>;

struct Instruction : Value
{
    std::vector<Value*> operands_;

    Instruction(ValueKind kind, std::vector<Value*> operands, Type* type = nullptr, std::string_view name = "");

    ~Instruction() override;
};

template <typename T>
concept DerivedFromInstruction = std::derived_from<T, Instruction>;

struct Alloca : Instruction
{
    Alloca(Type* type, std::string_view name);
};

struct Load : Instruction
{
    Load(Type* type, Value* ptr);
    Load(Value* ptr);
};

struct Store : Instruction
{
    Store(Value* dst, Value* src);
};

struct Add : Instruction
{
    Add(Value* src1, Value* src2);
};

struct Sub : Instruction
{
    Sub(Value* src1, Value* src2);
};

struct Mul : Instruction
{
    Mul(Value* src1, Value* src2);
};

struct Div : Instruction
{
    Div(Value* src1, Value* src2);
};

struct Mod : Instruction
{
    Mod(Value* src1, Value* src2);
};

struct Eq : Instruction
{
    Eq(Value* src1, Value* src2);
};

struct Ne : Instruction
{
    Ne(Value* src1, Value* src2);
};

struct Not : Instruction
{
    Not(Value* src);
};

struct Xor : Instruction
{
    Xor(Value* src1, Value* src2);
};

struct Or : Instruction
{
    Or(Value* src1, Value* src2);
};

struct And : Instruction
{
    And(Value* src1, Value* src2);
};

struct Slt : Instruction
{
    Slt(Value* src1, Value* src2);
};

struct Sle : Instruction
{
    Sle(Value* src1, Value* src2);
};

struct Ult : Instruction
{
    Ult(Value* src1, Value* src2);
};

struct Ule : Instruction
{
    Ule(Value* src1, Value* src2);
};

struct Shl : Instruction
{
    Shl(Value* src1, Value* src2);
};

struct Ashr : Instruction
{
    Ashr(Value* src1, Value* src2);
};

struct Lshr : Instruction
{
    Lshr(Value* src1, Value* src2);
};

struct Function;

struct Call : Instruction
{
    Call(Function* callee, std::vector<Value*> args);
};

struct Return : Instruction
{
    Return();
    Return(Value* value);
};

struct PtrAdd : Instruction
{
    PtrAdd(Value* ptr, Value* offset);
};

struct BasicBlock;

enum class BranchKind
{
    Unconditional,
    Conditional
};

struct Branch : Instruction
{
    BranchKind branch_kind_;

    Branch(BasicBlock* target);
    Branch(Value* cond, BasicBlock* bb_true, BasicBlock* bb_false);

    bool is_conditional() const;
    Value* condition();
    std::span<Value*> targets();
};

struct Phi : Value
{
    Alloca* alloca_;
    std::vector<std::pair<Value*, BasicBlock*>> operands_;

    Phi(Alloca* alloca) :
        Value(ValueKind::Phi),
        alloca_{ alloca },
        operands_{ } {}
};

struct Const : Value
{
    std::variant<int64_t, double, std::string> data_;

    Const(auto data) :
        Value{ValueKind::Const, nullptr, std::format("{}", data), nullptr},
        data_{ data } {}
};

struct BasicBlock : Value
{
    std::list<std::unique_ptr<Value>> instructions_;

    BasicBlock(std::string_view name = "");

    ~BasicBlock() override;

    template <DerivedFromValue T>
    T* insert(T* value)
    {
        value->parent_ = this;
        instructions_.push_back(std::unique_ptr<Value>{ value });
        return value;
    }

    bool empty();
    Value* terminator();
    small_vector<BasicBlock*, 2> successors();
    auto predecessors() {
        return users_ | std::views::transform([](Value* value) { return static_cast<BasicBlock*>(value->parent_); });
    };
};

struct Argument : Value
{
    Argument(Type* type, std::string_view name = "");
};

struct Function : Value
{
    std::list<std::unique_ptr<Argument>> args_;
    std::list<std::unique_ptr<BasicBlock>> blocks_; // unordered

    // memory leak
    Value* return_value_ = nullptr;
    BasicBlock* return_block_ = nullptr;

    Function(Type* type, std::string_view name = "");

    ~Function() override;

    BasicBlock* get_entry_block();
    BasicBlock* insert(BasicBlock* block);
    Argument* insert(Argument* arg);
};

template <typename T>
inline constexpr ValueKind value_kind_v = ValueKind::Invalid;

template <> inline constexpr ValueKind value_kind_v<Function>   = ValueKind::Function;
template <> inline constexpr ValueKind value_kind_v<BasicBlock> = ValueKind::BasicBlock;
template <> inline constexpr ValueKind value_kind_v<Argument>   = ValueKind::Argument;
template <> inline constexpr ValueKind value_kind_v<Alloca>     = ValueKind::Alloca; // inst begin
template <> inline constexpr ValueKind value_kind_v<Load>       = ValueKind::Load;
template <> inline constexpr ValueKind value_kind_v<Store>      = ValueKind::Store;
template <> inline constexpr ValueKind value_kind_v<Add>        = ValueKind::Add;
template <> inline constexpr ValueKind value_kind_v<Sub>        = ValueKind::Sub;
template <> inline constexpr ValueKind value_kind_v<Mul>        = ValueKind::Mul;
template <> inline constexpr ValueKind value_kind_v<Div>        = ValueKind::Div;
template <> inline constexpr ValueKind value_kind_v<Mod>        = ValueKind::Mod;
template <> inline constexpr ValueKind value_kind_v<Eq>         = ValueKind::Eq;
template <> inline constexpr ValueKind value_kind_v<Ne>         = ValueKind::Ne;
template <> inline constexpr ValueKind value_kind_v<Not>        = ValueKind::Not;
template <> inline constexpr ValueKind value_kind_v<Xor>        = ValueKind::Xor;
template <> inline constexpr ValueKind value_kind_v<Or>         = ValueKind::Or;
template <> inline constexpr ValueKind value_kind_v<And>        = ValueKind::And;
template <> inline constexpr ValueKind value_kind_v<Slt>        = ValueKind::Slt;
template <> inline constexpr ValueKind value_kind_v<Sle>        = ValueKind::Sle;
template <> inline constexpr ValueKind value_kind_v<Ult>        = ValueKind::Ult;
template <> inline constexpr ValueKind value_kind_v<Ule>        = ValueKind::Ule;
template <> inline constexpr ValueKind value_kind_v<Shl>        = ValueKind::Shl;
template <> inline constexpr ValueKind value_kind_v<Ashr>       = ValueKind::Ashr;
template <> inline constexpr ValueKind value_kind_v<Lshr>       = ValueKind::Lshr;
template <> inline constexpr ValueKind value_kind_v<Call>       = ValueKind::Call;
template <> inline constexpr ValueKind value_kind_v<Return>     = ValueKind::Return;
template <> inline constexpr ValueKind value_kind_v<Branch>     = ValueKind::Branch;
template <> inline constexpr ValueKind value_kind_v<PtrAdd>     = ValueKind::PtrAdd;
template <> inline constexpr ValueKind value_kind_v<Phi>        = ValueKind::Phi; // inst end
template <> inline constexpr ValueKind value_kind_v<Const>      = ValueKind::Const;

class ConstantPool
{
public:
    template <typename T>
    auto try_insert(T literal)
    {
        return pool_.try_emplace(literal, std::make_unique<Const>(literal));
    }

private:
    using const_variant = std::variant<int64_t, double, char, std::string>;

    std::unordered_map<const_variant, std::unique_ptr<Const>> pool_;
};

struct IRProgram
{
    ConstantPool constant_pool_;
    std::list<std::unique_ptr<Function>> functions_;

    IRProgram() = default;
    IRProgram(IRProgram&&) noexcept = default;
    IRProgram& operator=(IRProgram&&) noexcept = default;
    IRProgram(const IRProgram&) = delete;
    IRProgram& operator=(const IRProgram&) = delete;
    ~IRProgram() = default;

    Function* insert(Function* function);
};

inline std::vector<BasicBlock*> post_order(Function* function, bool reverse = false)
{
    if (function->blocks_.empty()) {
        return { };
    }

    BasicBlock* entry = function->get_entry_block();

    std::unordered_set<BasicBlock*> visited;
    std::vector<BasicBlock*> post_order;

    auto dfs = [&](this auto& self, BasicBlock* block) {
        if (visited.contains(block)) {
            return;
        }

        visited.insert(block);
        for (auto* succ : block->successors()) {
            self(succ);
        }

        post_order.push_back(block);
    };

    dfs(entry);

    if (reverse) {
        std::ranges::reverse(post_order);
    }

    return post_order;
}
