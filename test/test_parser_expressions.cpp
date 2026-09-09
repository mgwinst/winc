#include "frontend/ast/ast.hpp"
#include "test/test_utils.hpp"

// Unary Ops

TEST_CASE("address-of")
{
    auto ast = parse("srctest/parser/address_of.w");

    auto* translation_unit = cast<TranslationUnitDecl>(ast.root_);
    auto* func = cast<FuncDecl>(translation_unit->decls_[0]);
    auto* body = cast<CompoundStmt>(func->body_);

    auto* unary = cast<UnaryExpr>(body->children_[0]);

    CHECK(unary->op_ == "&");
    CHECK(isa<ReferenceExpr>(unary->operand_));
    CHECK(unary->is_postfix_ == false);
}

TEST_CASE("pointer dereference")
{
    auto ast = parse("srctest/parser/pointer_dereference.w");

    auto* translation_unit = cast<TranslationUnitDecl>(ast.root_);
    auto* func = cast<FuncDecl>(translation_unit->decls_[0]);
    auto* body = cast<CompoundStmt>(func->body_);

    auto* unary = cast<UnaryExpr>(body->children_[0]);

    CHECK(unary->op_ == "*");
    CHECK(isa<ReferenceExpr>(unary->operand_));
    CHECK(unary->is_postfix_ == false);
}

TEST_CASE("prefix increment")
{
    auto ast = parse("srctest/parser/increment.w");

    auto* translation_unit = cast<TranslationUnitDecl>(ast.root_);
    auto* func = cast<FuncDecl>(translation_unit->decls_[0]);
    auto* body = cast<CompoundStmt>(func->body_);

    auto* unary = cast<UnaryExpr>(body->children_[0]);

    CHECK(unary->op_ == "++");
    CHECK(unary->is_postfix_ == false);
}

TEST_CASE("prefix decrement")
{
        auto ast = parse("srctest/parser/decrement.w");

    auto* translation_unit = cast<TranslationUnitDecl>(ast.root_);
    auto* func = cast<FuncDecl>(translation_unit->decls_[0]);
    auto* body = cast<CompoundStmt>(func->body_);

    auto* unary = cast<UnaryExpr>(body->children_[0]);

    CHECK(unary->op_ == "--");
    CHECK(unary->is_postfix_ == false);
}

// Binary Ops

TEST_CASE("assignment")
{
    auto ast = parse("srctest/parser/assignment.w");

    auto* translation_unit = cast<TranslationUnitDecl>(ast.root_);
    auto* func = cast<FuncDecl>(translation_unit->decls_[0]);
    auto* body = cast<CompoundStmt>(func->body_);

    auto* binary = cast<BinaryExpr>(body->children_[0]);

    CHECK(binary->op_ == "=");
}

TEST_CASE("compound assignment")
{
    auto ast = parse("srctest/parser/compound_assignment.w");

    auto* translation_unit = cast<TranslationUnitDecl>(ast.root_);
    auto* func = cast<FuncDecl>(translation_unit->decls_[0]);
    auto* body = cast<CompoundStmt>(func->body_);

    auto* binary = cast<BinaryExpr>(body->children_[0]);

    CHECK(binary->op_ == "+=");
}

TEST_CASE("a + b")
{
    auto ast = parse("srctest/parser/add.w");

    auto* translation_unit = cast<TranslationUnitDecl>(ast.root_);
    auto* func = cast<FuncDecl>(translation_unit->decls_[0]);
    auto* body = cast<CompoundStmt>(func->body_);

    auto* binary = cast<BinaryExpr>(body->children_[0]);

    CHECK(binary->op_ == "+");
}

TEST_CASE("function call no arguments")
{

}

TEST_CASE("function call with 2 arguments")
{
    auto ast = parse("srctest/parser/call.w");

    auto* translation_unit = cast<TranslationUnitDecl>(ast.root_);
    auto* func = cast<FuncDecl>(translation_unit->decls_[0]);
    auto* body = cast<CompoundStmt>(func->body_);

    auto* call = cast<CallExpr>(body->children_[0]);

    CHECK(isa<ReferenceExpr>(call->callee_));

    REQUIRE(call->args_.size() == 2);

    CHECK(isa<ReferenceExpr>(call->args_[0]));
    CHECK(isa<ReferenceExpr>(call->args_[1]));
}

TEST_CASE("direct member access")
{
    auto ast = parse("srctest/parser/member_direct.w");

    auto* translation_unit = cast<TranslationUnitDecl>(ast.root_);
    auto* func = cast<FuncDecl>(translation_unit->decls_[0]);
    auto* body = cast<CompoundStmt>(func->body_);

    auto* member_expr = cast<MemberExpr>(body->children_[0]);

    CHECK(cast<ReferenceExpr>(member_expr->base_)->name_ == "foo");
    CHECK(member_expr->member_ == "x");
    CHECK(member_expr->is_arrow_ == false);
}

TEST_CASE("indirect member access")
{
    auto ast = parse("srctest/parser/member_indirect.w");

    auto* translation_unit = cast<TranslationUnitDecl>(ast.root_);
    auto* func = cast<FuncDecl>(translation_unit->decls_[0]);
    auto* body = cast<CompoundStmt>(func->body_);

    auto* member_expr = cast<MemberExpr>(body->children_[0]);

    CHECK(cast<ReferenceExpr>(member_expr->base_)->name_ == "foo");
    CHECK(member_expr->member_ == "x");
    CHECK(member_expr->is_arrow_ == true);

}

TEST_CASE("array subscript")
{
    auto ast = parse("srctest/parser/array_subscript.w");

    auto* translation_unit = cast<TranslationUnitDecl>(ast.root_);
    auto* func = cast<FuncDecl>(translation_unit->decls_[0]);
    auto* body = cast<CompoundStmt>(func->body_);

    auto* arr_sub_expr = cast<ArraySubscriptExpr>(body->children_[0]);

    CHECK(isa<ReferenceExpr>(arr_sub_expr->base_));
    CHECK(isa<ReferenceExpr>(arr_sub_expr->index_));
}

TEST_CASE("new")
{
    auto ast = parse("srctest/parser/new.w");

    auto* translation_unit = cast<TranslationUnitDecl>(ast.root_);
    auto* func = cast<FuncDecl>(translation_unit->decls_[0]);
    auto* body = cast<CompoundStmt>(func->body_);

    auto* new_expr = cast<NewExpr>(body->children_[0]);

    CHECK(isa<CallExpr>(new_expr->constructor_));
}

TEST_CASE("delete")
{
    auto ast = parse("srctest/parser/delete.w");

    auto* translation_unit = cast<TranslationUnitDecl>(ast.root_);
    auto* func = cast<FuncDecl>(translation_unit->decls_[0]);
    auto* body = cast<CompoundStmt>(func->body_);

    auto* delete_expr = cast<DeleteExpr>(body->children_[0]);

    CHECK(isa<ReferenceExpr>(delete_expr->pointer_));
}

// literals, int, float, char, string, byte(hex)
// type casting
// comments
