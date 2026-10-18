#include "doctest/doctest.h"

#include "middleend/transforms/TransformPassManager.hpp"
#include "test_utils.hpp"

TEST_CASE("dse pass removes all dead stores")
{
    auto program = lower("srctest/transforms/dse.w");

    auto& function = program.functions_.front();
    auto& block = function->blocks_.front();
    auto it = block->instructions_.begin();

    CHECK(block->instructions_.size() == 5);
    CHECK(isa<Alloca>(*it++));
    CHECK(isa<Store>(*it++));
    CHECK(isa<Store>(*it++));
    CHECK(isa<Store>(*it++));
    CHECK(isa<Branch>(*it));

    TransformPassManager{{Transforms::DSE}}.run(program);

    it = block->instructions_.begin();

    CHECK(block->instructions_.size() == 3);
    CHECK(isa<Alloca>(*it++));
    CHECK(isa<Store>(*it++));
    CHECK(isa<Branch>(*it));
}
