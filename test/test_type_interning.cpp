#include "doctest/doctest.h"

#include "frontend/sema/types/type_table.hpp"

#define INITIAL_BUILTIN_COUNT (18)
#define INITIAL_USER_DEFINED_COUNT (0)

TEST_CASE("canonicalize types")
{
    TypeTable table{};

    CHECK(table.builtin_types_.size() == INITIAL_BUILTIN_COUNT);
    CHECK(table.user_defined_types_.size() == INITIAL_USER_DEFINED_COUNT);

    SUBCASE("built-in type")
    {
        auto int_type_expr = std::make_unique<NamedTypeExpr>("int");

        Type* int1 = table.resolve_type(int_type_expr.get());
        Type* int2 = table.resolve_type(int_type_expr.get());

        CHECK(table.builtin_types_.size() == INITIAL_BUILTIN_COUNT);
        CHECK(int1 == table.builtin_types_["int"]);
        CHECK(int1 == int2);
    }

    SUBCASE("user-defined type")
    {
        auto record = std::make_unique<RecordDecl>(RecordKind::Struct, "Foo", std::vector<ASTNode*>{});

        Type* foo1 = table.resolve_type(record.get());
        Type* foo2 = table.resolve_type(record.get());

        CHECK(table.user_defined_types_.size() == 1);
        CHECK(foo1 == table.user_defined_types_["Foo"]);
        CHECK(foo1 == foo2);
    }

    SUBCASE("composite type")
    {

    }
}
