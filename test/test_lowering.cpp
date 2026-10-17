#include <doctest/doctest.h>

#include "test/test_utilities.hpp"

TEST_CASE("variable declaration -> alloca")
{
    auto program = lower("srctest/lowering/variable.w");

    auto& function = program.functions_.front();
    auto& block = function->blocks_.front();

    auto it = block->instructions_.begin();

    CHECK(isa<Alloca>(*it));
}

TEST_CASE("assignment -> store") 
{
    auto program = lower("srctest/lowering/assign.w");

    auto& function = program.functions_.front();
    auto& block = function->blocks_.front();

    auto it = block->instructions_.begin();

    CHECK(isa<Alloca>(*it++));
    CHECK(isa<Store>(*it));
}

TEST_CASE("integer addition")
{
    auto program = lower("srctest/lowering/add.w");

    auto& function = program.functions_.front();
    auto& block = function->blocks_.front();

    auto it = block->instructions_.begin();

    CHECK(isa<Alloca>(*it++));
    CHECK(isa<Add>(*it++));
    CHECK(isa<Store>(*it));
}

TEST_CASE("variable reference") 
{
    auto program = lower("srctest/lowering/reference.w");

    auto& function = program.functions_.front();
    auto& block = function->blocks_.front();

    auto it = block->instructions_.begin();
    
    CHECK(isa<Alloca>(*it++));
    CHECK(isa<Store>(*it++));
    CHECK(isa<Alloca>(*it++));
    CHECK(isa<Load>(*it++));
    CHECK(isa<Store>(*it));
}

TEST_CASE("== -> eq") 
{
    auto program = lower("srctest/lowering/equal.w");

    auto& function = program.functions_.front();
    auto& block = function->blocks_.front();

    auto it = block->instructions_.begin();
    
    CHECK(isa<Alloca>(*it++));
    CHECK(isa<Store>(*it++));
    CHECK(isa<Alloca>(*it++));
    CHECK(isa<Store>(*it++));
    CHECK(isa<Load>(*it++));
    CHECK(isa<Load>(*it++));
    CHECK(isa<Eq>(*it));
}

TEST_CASE("!= -> ne") 
{
    auto program = lower("srctest/lowering/not_equal.w");

    auto& function = program.functions_.front();
    auto& block = function->blocks_.front();

    auto it = block->instructions_.begin();
    
    CHECK(isa<Alloca>(*it++));
    CHECK(isa<Store>(*it++));
    CHECK(isa<Alloca>(*it++));
    CHECK(isa<Store>(*it++));
    CHECK(isa<Load>(*it++));
    CHECK(isa<Load>(*it++));
    CHECK(isa<Ne>(*it));
}

TEST_CASE("!x -> not x") 
{
    auto program = lower("srctest/lowering/logical_not.w");

    auto& function = program.functions_.front();
    auto& block = function->blocks_.front();

    auto it = block->instructions_.begin();
    
    CHECK(isa<Alloca>(*it++));
    CHECK(isa<Store>(*it++));
    CHECK(isa<Load>(*it++));
    CHECK(isa<Not>(*it));
}

TEST_CASE("(int < int) -> slt") 
{
    auto program = lower("srctest/lowering/int_lt_int.w");

    auto& function = program.functions_.front();
    auto& block = function->blocks_.front();

    auto it = block->instructions_.begin();
    
    CHECK(isa<Alloca>(*it++));
    CHECK(isa<Store>(*it++));
    CHECK(isa<Alloca>(*it++));
    CHECK(isa<Store>(*it++));
    CHECK(isa<Load>(*it++));
    CHECK(isa<Load>(*it++));
    CHECK(isa<Slt>(*it));
}

TEST_CASE("(int <= int) -> sle") 
{
    auto program = lower("srctest/lowering/int_le_int.w");

    auto& function = program.functions_.front();
    auto& block = function->blocks_.front();

    auto it = block->instructions_.begin();
    
    CHECK(isa<Alloca>(*it++));
    CHECK(isa<Store>(*it++));
    CHECK(isa<Alloca>(*it++));
    CHECK(isa<Store>(*it++));
    CHECK(isa<Load>(*it++));
    CHECK(isa<Load>(*it++));
    CHECK(isa<Sle>(*it));
}

TEST_CASE("(uint < uint) -> ult")
{
    auto program = lower("srctest/lowering/uint_lt_uint.w");

    auto& function = program.functions_.front();
    auto& block = function->blocks_.front();

    auto it = block->instructions_.begin();
    
    CHECK(isa<Alloca>(*it++));
    CHECK(isa<Store>(*it++));
    CHECK(isa<Alloca>(*it++));
    CHECK(isa<Store>(*it++));
    CHECK(isa<Load>(*it++));
    CHECK(isa<Load>(*it++));
    CHECK(isa<Ult>(*it));
}

TEST_CASE("(uint <= uint) -> ule")
{
    auto program = lower("srctest/lowering/uint_le_uint.w");

    auto& function = program.functions_.front();
    auto& block = function->blocks_.front();

    auto it = block->instructions_.begin();
    
    CHECK(isa<Alloca>(*it++));
    CHECK(isa<Store>(*it++));
    CHECK(isa<Alloca>(*it++));
    CHECK(isa<Store>(*it++));
    CHECK(isa<Load>(*it++));
    CHECK(isa<Load>(*it++));
    CHECK(isa<Ule>(*it));
}

TEST_CASE("<< -> shl") 
{
    auto program = lower("srctest/lowering/shift_left.w");

    auto& function = program.functions_.front();
    auto& block = function->blocks_.front();

    auto it = block->instructions_.begin();
    
    CHECK(isa<Alloca>(*it++));
    CHECK(isa<Store>(*it++));
    CHECK(isa<Load>(*it++));
    CHECK(isa<Shl>(*it));
}

TEST_CASE("signed >> -> ashr") 
{
    auto program = lower("srctest/lowering/arith_shift_right.w");

    auto& function = program.functions_.front();
    auto& block = function->blocks_.front();

    auto it = block->instructions_.begin();
    
    CHECK(isa<Alloca>(*it++));
    CHECK(isa<Store>(*it++));
    CHECK(isa<Load>(*it++));
    CHECK(isa<Ashr>(*it));
}

TEST_CASE("unsigned >> -> lshr") 
{
    auto program = lower("srctest/lowering/logical_shift_right.w");

    auto& function = program.functions_.front();
    auto& block = function->blocks_.front();

    auto it = block->instructions_.begin();
    
    CHECK(isa<Alloca>(*it++));
    CHECK(isa<Store>(*it++));
    CHECK(isa<Load>(*it++));
    CHECK(isa<Lshr>(*it));
}

TEST_CASE("^ -> xor") 
{
    auto program = lower("srctest/lowering/xor.w");

    auto& function = program.functions_.front();
    auto& block = function->blocks_.front();

    auto it = block->instructions_.begin();
    
    CHECK(isa<Alloca>(*it++));
    CHECK(isa<Store>(*it++));
    CHECK(isa<Load>(*it++));
    CHECK(isa<Xor>(*it));
}

TEST_CASE("| -> or") 
{
    auto program = lower("srctest/lowering/or.w");

    auto& function = program.functions_.front();
    auto& block = function->blocks_.front();

    auto it = block->instructions_.begin();
    
    CHECK(isa<Alloca>(*it++));
    CHECK(isa<Store>(*it++));
    CHECK(isa<Load>(*it++));
    CHECK(isa<Or>(*it));
}

TEST_CASE("& -> and") 
{
    auto program = lower("srctest/lowering/and.w");

    auto& function = program.functions_.front();
    auto& block = function->blocks_.front();

    auto it = block->instructions_.begin();
    
    CHECK(isa<Alloca>(*it++));
    CHECK(isa<Store>(*it++));
    CHECK(isa<Load>(*it++));
    CHECK(isa<And>(*it));
}

TEST_CASE("array indexing -> ptradd") 
{
    auto program = lower("srctest/lowering/array_index.w");

    auto& function = program.functions_.front();
    auto& block = function->blocks_.front();

    auto it = block->instructions_.begin();
    
    CHECK(isa<Alloca>(*it++));
    CHECK(isa<PtrAdd>(*it));

    auto* ptradd = cast<PtrAdd>(it->get());
    
    CHECK(isa<Alloca>(ptradd->operands_[0]));
    CHECK(isa<Const>(ptradd->operands_[1]));
}

TEST_CASE("struct member access -> ptradd") 
{
    auto program = lower("srctest/lowering/struct_member_access.w");

    auto& function = program.functions_.front();
    auto& block = function->blocks_.front();

    auto it = block->instructions_.begin();
    
    CHECK(isa<Alloca>(*it++));
    CHECK(isa<PtrAdd>(*it));

    auto* ptradd = cast<PtrAdd>(it->get());
    
    CHECK(isa<Alloca>(ptradd->operands_[0]));
    CHECK(isa<Const>(ptradd->operands_[1]));
}

TEST_CASE("function arguments spill to stack slot") 
{
    auto program = lower("srctest/lowering/arg_spill.w");

    auto& function = program.functions_.front();
    auto& block = function->blocks_.front();

    auto it = block->instructions_.begin();
    
    CHECK(isa<Alloca>(*it++));
    CHECK(isa<Store>(*it++));
    CHECK(isa<Alloca>(*it++));
    CHECK(isa<Store>(*it++));
    CHECK(isa<Alloca>(*it++));
    CHECK(isa<Store>(*it));
}

TEST_CASE("lvalue pointer dereference -> load")
{
    auto program = lower("srctest/lowering/lvalue_ptr_deref.w");

    auto& function = program.functions_.front();
    auto& block = function->blocks_.front();

    auto it = block->instructions_.begin();
    
    CHECK(isa<Alloca>(*it++));
    CHECK(isa<Load>(*it));
}

TEST_CASE("rvalue pointer dereference -> double load")
{
    auto program = lower("srctest/lowering/rvalue_ptr_deref.w");

    auto& function = program.functions_.front();
    auto& block = function->blocks_.front();

    auto it = block->instructions_.begin();
    
    CHECK(isa<Alloca>(*it++));
    CHECK(isa<Alloca>(*it++));
    CHECK(isa<Load>(*it++));
    CHECK(isa<Load>(*it));
}

/*
TEST_CASE("if (x) -> ne x, 0") {}
TEST_CASE("if statement -> br label if.then, label if.end") {}
TEST_CASE("if-else statement -> br label if.then, label if.else") {}
TEST_CASE("&&") {}
*/