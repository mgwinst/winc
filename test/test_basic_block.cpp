#include "test/test_utilities.hpp"

TEST_CASE("BasicBlock::terminator() returns the last instruction in the block and must be branch/ret instruction")
{
    IRBuilder builder{};

    auto* block1 = builder.create<BasicBlock>();
    
    auto* ret = builder.create<Return>();

    block1->insert(ret);

    CHECK(isa<Return>(block1->terminator()));
}

TEST_CASE("unconditional branch instruction adds successor to BasicBlock")
{
    IRBuilder builder{};

    auto* block1 = builder.create<BasicBlock>();
    auto* block2 = builder.create<BasicBlock>();
    
    auto* branch = builder.create<Branch>(block2);

    block1->insert(branch);

    REQUIRE(block1->successors().size() == 1);
    CHECK(block1->successors()[0] == block2);

    REQUIRE(block2->predecessors().size() == 1);
    REQUIRE(block2->predecessors()[0] == block1);
}

TEST_CASE("conditional branch instruction adds all successors to block")
{
    IRBuilder builder{};

    auto* block1 = builder.create<BasicBlock>();
    auto* block2 = builder.create<BasicBlock>();
    auto* block3 = builder.create<BasicBlock>();
    
    auto* branch = builder.create<Branch>(nullptr, block2, block3);

    block1->insert(branch);

    REQUIRE(block1->successors().size() == 2);
    CHECK(block1->successors()[0] == block2);
    CHECK(block1->successors()[1] == block3);

    REQUIRE(block2->predecessors().size() == 1);
    CHECK(block2->predecessors()[0] == block1);

    REQUIRE(block3->predecessors().size() == 1);
    CHECK(block3->predecessors()[0] == block1);
}

TEST_CASE("delete successor block converts conditional branch instruction to unconditional")
{
    IRBuilder builder{};

    auto* block1 = builder.create<BasicBlock>();
    auto* block2 = builder.create<BasicBlock>();
    auto* block3 = builder.create<BasicBlock>();

    auto* branch = builder.create<Branch>(nullptr, block2, block3);

    block1->insert(branch);

    auto* b = cast<Branch>(block1->terminator());

    CHECK(b->branch_kind_ == BranchKind::Conditional);
    CHECK(block1->successors().size() == 2);

    delete block3;

    CHECK(b->branch_kind_ == BranchKind::Unconditional);
    CHECK(block1->successors().size() == 1);
    CHECK(block1->successors()[0] == block2);
}

TEST_CASE("delete successor block deletes branch instruction from predecessor")
{
    IRBuilder builder{};

    auto* block1 = builder.create<BasicBlock>();
    auto* block2 = builder.create<BasicBlock>();

    auto* branch = builder.create<Branch>(block2);

    block1->insert(branch);

    CHECK(block1->instructions_.size() == 1);

    delete block2;

    CHECK(block1->instructions_.size() == 0);
}