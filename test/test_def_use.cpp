#include "test/test_utilities.hpp"

TEST_CASE("instruction def-use list")
{
    IRBuilder builder{};

    auto* type = new IntegerType{32, true};
    auto* alloca = builder.create<Alloca>(type, "a");

    auto* num = builder.create<Const>(42);

    auto* store = builder.create<Store>(alloca, num);

    SUBCASE("instruction creation updates def-use chain")
    {
        CHECK(alloca->users_.size() == 1);
        CHECK(alloca->users_[0] == store);

        CHECK(store->users_.empty());
        CHECK(store->operands_.size() == 2);
        CHECK(store->operands_[0] == alloca);
        CHECK(store->operands_[1] == num);
    }

    SUBCASE("instruction destruction updates use-list of operands")
    {
        delete store;
        CHECK(alloca->users_.empty());
    }
}

TEST_CASE("replace_uses_with() modifies graph properly updates all uses with new value")
{
    IRBuilder builder{};

    auto* type1 = new IntegerType{32, true};
    auto* alloca1 = builder.create<Alloca>(type1, "a");

    auto* type2 = new IntegerType{32, true};
    auto* alloca2 = builder.create<Alloca>(type2, "b");

    auto* num = builder.create<Const>(42);

    auto* add = builder.create<Add>(alloca1, alloca2);

    alloca1->replace_uses_with(num);

    SUBCASE("operand is replaced with new value")
    {
        CHECK(add->operands_[0] == num);
    }

    SUBCASE("updates the use list of the value we are replacing with")
    {
        CHECK(std::ranges::contains(num->users_, add));
    }
}