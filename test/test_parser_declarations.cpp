#include "test_utils.hpp"

// functions

TEST_CASE("function definition")
{
    auto ast = parse("srctest/parser/func_no_param.w");

    auto* translation_unit = cast<TranslationUnitDecl>(ast.root_);
    REQUIRE(translation_unit->decls_.size() == 1);

    auto* func = cast<FuncDecl>(translation_unit->decls_[0]);
    CHECK(func->name_ == "func");
    CHECK(func->params_.size() == 0);
    CHECK(cast<NamedTypeExpr>(func->return_type_)->name_ == "void");
}

TEST_CASE("single parameter")
{
    auto ast = parse("srctest/parser/func_single_param.w");

    auto* translation_unit = cast<TranslationUnitDecl>(ast.root_);
    auto* func = cast<FuncDecl>(translation_unit->decls_[0]);

    CHECK(func->params_.size() == 1);

    auto* param = cast<ParamDecl>(func->params_[0]);

    CHECK(param->name_ == "x");

    CHECK(cast<NamedTypeExpr>(param->type_expr_)->name_ == "int");
}

TEST_CASE("multiple parameters")
{
    auto ast = parse("srctest/parser/func_multi_param.w");

    auto* translation_unit = cast<TranslationUnitDecl>(ast.root_);
    auto* func = cast<FuncDecl>(translation_unit->decls_[0]);

    CHECK(func->params_.size() == 2);

    auto* param1 = cast<ParamDecl>(func->params_[0]);
    auto* param2 = cast<ParamDecl>(func->params_[1]);

    CHECK(param1->name_ == "x");
    CHECK(param2->name_ == "y");

    CHECK(cast<NamedTypeExpr>(param1->type_expr_)->name_ == "int");
    CHECK(cast<NamedTypeExpr>(param2->type_expr_)->name_ == "int");
}

TEST_CASE("missing 'fn' keyword")
{

}

TEST_CASE("missing return type")
{
    TranslationUnit translation_unit{ "srctest/parser/func_missing_ret_type.w" };
    auto ast = Parser{translation_unit}.run();

    REQUIRE(translation_unit.diagnostics_.contains_errors());
    CHECK(translation_unit.diagnostics_.errors_[0].msg_.contains("syntax error: function missing trailing return type"));
}

// variables

TEST_CASE("variable declaration")
{
    auto ast = parse("srctest/parser/var_decl.w");

    auto* translation_unit = cast<TranslationUnitDecl>(ast.root_);
    REQUIRE(translation_unit->decls_.empty() == false);
    auto* func = cast<FuncDecl>(translation_unit->decls_[0]);
    auto* body = cast<CompoundStmt>(func->body_);

    REQUIRE(body->children_.size() == 1);

    auto* var = cast<VarDecl>(body->children_[0]);

    CHECK(var->name_ == "x");
    CHECK(cast<NamedTypeExpr>(var->type_expr_)->name_ == "int");
    CHECK(var->init_ == nullptr);
}

TEST_CASE("variable initialization")
{
    auto ast = parse("srctest/parser/var_init.w");

    auto* translation_unit = cast<TranslationUnitDecl>(ast.root_);
    auto* func = cast<FuncDecl>(translation_unit->decls_[0]);
    auto* body = cast<CompoundStmt>(func->body_);

    REQUIRE(body->children_.size() == 2);

    auto* var1 = cast<VarDecl>(body->children_[0]);
    auto* var2 = cast<VarDecl>(body->children_[1]);

    CHECK(var1->name_ == "x");
    CHECK(var2->name_ == "y");

    CHECK(cast<ReferenceExpr>(var2->init_)->name_ == "x");
}

// arrays

TEST_CASE("array")
{
    auto ast = parse("srctest/parser/array.w");

    auto* translation_unit = cast<TranslationUnitDecl>(ast.root_);
    auto* func = cast<FuncDecl>(translation_unit->decls_[0]);
    auto* body = cast<CompoundStmt>(func->body_);

    REQUIRE(body->children_.size() == 1);

    auto* arr = cast<VarDecl>(body->children_[0]);

    CHECK(arr->name_ == "arr");
    CHECK(arr->init_ == nullptr);

    auto* type = cast<ArrayTypeExpr>(arr->type_expr_);

    CHECK(cast<IntegerLiteralExpr>(type->size_)->value_ == 10);
    CHECK(cast<NamedTypeExpr>(type->inner_)->name_ == "int");
}

TEST_CASE("array initializer list")
{

}

// structs

TEST_CASE("struct definition")
{
    auto ast = parse("srctest/parser/struct_def.w");

    auto* translation_unit = cast<TranslationUnitDecl>(ast.root_);

    REQUIRE(translation_unit->decls_.size() == 1);

    auto* s = cast<RecordDecl>(translation_unit->decls_[0]);

    CHECK(s->kind_ == RecordKind::Struct);
    CHECK(s->name_ == "Foo");
    CHECK(s->fields_.empty());
}

TEST_CASE("struct definition with fields")
{
    auto ast = parse("srctest/parser/struct_with_fields.w");

    auto* translation_unit = cast<TranslationUnitDecl>(ast.root_);

    REQUIRE(translation_unit->decls_.size() == 1);

    auto* s = cast<RecordDecl>(translation_unit->decls_[0]);

    CHECK(s->kind_ == RecordKind::Struct);
    CHECK(s->name_ == "Foo");
    REQUIRE(s->fields_.size() == 2);

    auto* field1 = cast<VarDecl>(s->fields_[0]);
    auto* field2 = cast<VarDecl>(s->fields_[1]);

    CHECK(field1->name_ == "x");
    CHECK(field2->name_ == "y");

    CHECK(cast<NamedTypeExpr>(field1->type_expr_)->name_ == "int");
    CHECK(cast<NamedTypeExpr>(field2->type_expr_)->name_ == "int");

    CHECK(field1->init_ == nullptr);
    CHECK(field2->init_ == nullptr);
}
