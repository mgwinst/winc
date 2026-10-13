#include "test/test_utils.hpp"

TEST_CASE("return statement")
{
    auto ast = parse("srctest/parser/return.w");

    auto* translation_unit = cast<TranslationUnitDecl>(ast.root_);
    auto* func = cast<FuncDecl>(translation_unit->decls_[0]);
    auto* body = cast<CompoundStmt>(func->body_);

    REQUIRE(body->children_.size() == 1);

    auto* return_stmt = cast<ReturnStmt>(body->children_[0]);

    CHECK(isa<ReferenceExpr>(return_stmt->value_));
}

TEST_CASE("if statement")
{
    auto ast = parse("srctest/parser/ifstmt.w");

    auto* translation_unit = cast<TranslationUnitDecl>(ast.root_);
    auto* func = cast<FuncDecl>(translation_unit->decls_[0]);
    auto* body = cast<CompoundStmt>(func->body_);

    REQUIRE(body->children_.size() == 1);

    auto* ifstmt = cast<IfStmt>(body->children_[0]);

    CHECK(isa<BinaryExpr>(ifstmt->cond_));
    CHECK(isa<CompoundStmt>(ifstmt->then_stmt_));
    CHECK(ifstmt->else_stmt_ == nullptr);
}

TEST_CASE("if-else")
{
    auto ast = parse("srctest/parser/ifelse.w");

    auto* translation_unit = cast<TranslationUnitDecl>(ast.root_);
    auto* func = cast<FuncDecl>(translation_unit->decls_[0]);
    auto* body = cast<CompoundStmt>(func->body_);

    REQUIRE(body->children_.size() == 1);

    auto* ifstmt = cast<IfStmt>(body->children_[0]);

    CHECK(isa<BinaryExpr>(ifstmt->cond_));
    CHECK(isa<CompoundStmt>(ifstmt->then_stmt_));
    CHECK(isa<CompoundStmt>(ifstmt->else_stmt_));
}

TEST_CASE("else-if")
{

}

TEST_CASE("while statement")
{
    auto ast = parse("srctest/parser/whilestmt.w");

    auto* translation_unit = cast<TranslationUnitDecl>(ast.root_);
    auto* func = cast<FuncDecl>(translation_unit->decls_[0]);
    auto* body = cast<CompoundStmt>(func->body_);

    REQUIRE(body->children_.size() == 1);

    auto* whilestmt = cast<WhileStmt>(body->children_[0]);

    CHECK(isa<BinaryExpr>(whilestmt->cond_));
    CHECK(isa<CompoundStmt>(whilestmt->body_));
}

TEST_CASE("for statement")
{
    auto ast = parse("srctest/parser/forstmt.w");

    auto* translation_unit = cast<TranslationUnitDecl>(ast.root_);
    auto* func = cast<FuncDecl>(translation_unit->decls_[0]);
    auto* body = cast<CompoundStmt>(func->body_);

    REQUIRE(body->children_.size() == 1);

    auto* forstmt = cast<ForStmt>(body->children_[0]);

    CHECK(isa<VarDecl>(forstmt->init_));
    CHECK(isa<BinaryExpr>(forstmt->cond_));
    CHECK(isa<UnaryExpr>(forstmt->update_));
    CHECK(isa<CompoundStmt>(forstmt->body_));
}


TEST_CASE("break statment")
{
    auto ast = parse("srctest/parser/breakstmt.w");

    auto* translation_unit = cast<TranslationUnitDecl>(ast.root_);
    auto* func = cast<FuncDecl>(translation_unit->decls_[0]);
    auto* body = cast<CompoundStmt>(func->body_);

    REQUIRE(body->children_.size() == 1);

    auto* forstmt = cast<ForStmt>(body->children_[0]);

    auto* for_body = cast<CompoundStmt>(forstmt->body_);

    REQUIRE(for_body->children_.size() == 1);

    CHECK(isa<BreakStmt>(for_body->children_[0]));

}

TEST_CASE("continue statment")
{
    auto ast = parse("srctest/parser/continuestmt.w");

    auto* translation_unit = cast<TranslationUnitDecl>(ast.root_);
    auto* func = cast<FuncDecl>(translation_unit->decls_[0]);
    auto* body = cast<CompoundStmt>(func->body_);

    REQUIRE(body->children_.size() == 1);

    auto* forstmt = cast<ForStmt>(body->children_[0]);

    auto* for_body = cast<CompoundStmt>(forstmt->body_);

    REQUIRE(for_body->children_.size() == 1);

    CHECK(isa<ContinueStmt>(for_body->children_[0]));
}
