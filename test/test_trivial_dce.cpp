#include "doctest/doctest.h"

#include "middleend/transforms/TransformPassManager.hpp"
#include "test_utils.hpp"

TEST_CASE("value with no users is deleted")
{
    auto program = lower("srctest/transforms/trivial_dce.w");

    auto& function = program.functions_.front();
    auto& block = function->blocks_.front();

    // block contains an alloca and branch
    CHECK(block->instructions_.size() == 2);
    CHECK(isa<Alloca>(*std::next(block->instructions_.begin(), 0)));
    CHECK(isa<Branch>(*std::next(block->instructions_.begin(), 1)));

    auto* variable = cast<Alloca>(block->instructions_.front().get());
    CHECK(variable->users_.size() == 0);

    TransformPassManager{{Transforms::TRIVIAL_DCE}}.run(program);

    // block now contains only branch
    CHECK(block->instructions_.size() == 1);
    CHECK(isa<Branch>(block->instructions_.front()));
}
