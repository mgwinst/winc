#include <algorithm>

#include "analyzer.hpp"
#include "frontend/sema/types/types.hpp"
#include "helpers.hpp"
#include "frontend/ast/ast.hpp"
#include "utils/casting.hpp"
#include "utils/print/print.hpp"

void SemanticAnalyzer::decorate(ASTNode* node)
{
    if (auto* ast_translation_unit = dyn_cast<TranslationUnitDecl>(node)) {

        translation_unit_.symbol_table_.enter_scope();

        for (auto* decl : ast_translation_unit->decls_) {
            decorate(decl);
        }

        translation_unit_.symbol_table_.exit_scope();
    }

    else if (auto* var = dyn_cast<VarDecl>(node)) {
        auto* type = translation_unit_.type_table_.resolve_type(var->type_expr_);

        if (auto existing = translation_unit_.symbol_table_.lookup(var->name_)) {
            register_error(std::format("redefinition of '{}'", var->name_), var->source_);
        }

        var->symbol_ = translation_unit_.symbol_table_.insert(var, type);

        if (var->init_) {
            decorate(var->init_);
        }
    }

    else if (auto* param = dyn_cast<ParamDecl>(node)) {
        auto* type = translation_unit_.type_table_.resolve_type(param->type_expr_);

        if (auto existing = translation_unit_.symbol_table_.lookup(param->name_)) {
            register_error(std::format("redefinition of '{}'", param->name_), param->source_);
        }

        param->symbol_ = translation_unit_.symbol_table_.insert(param, type);
    }

    else if (auto* func = dyn_cast<FuncDecl>(node)) {
        auto* type = cast<FunctionType>(translation_unit_.type_table_.resolve_type(func));

        if (auto existing = translation_unit_.symbol_table_.lookup(func->name_)) {
            for (auto [_, symbol] : *existing) {
                if (auto* existing_type = dyn_cast<FunctionType>(symbol->type_)) {
                    if (type == existing_type) {
                        register_error(std::format("redefinition of '{}'", func->name_), func->source_);
                    } else if (type->params_ == existing_type->params_ && type->return_type_ != existing_type->return_type_) {
                        register_error(std::format("ambiguous redefinition of '{}' | prototype differs only in return type", func->name_), func->source_);
                    }
                } else {
                    register_error(std::format("redefinition of '{}'", func->name_), func->source_);
                }
            }
        }

        func->symbol_ = translation_unit_.symbol_table_.insert(func, type);

        translation_unit_.symbol_table_.enter_scope();

        for (auto* p : func->params_) {
            decorate(p);
        }

        decorate(func->body_);

        translation_unit_.symbol_table_.exit_scope();

        if (type->return_type_ == translation_unit_.type_table_.builtin_types_["void"]) {
            auto* ret = ast_.emplace<ReturnStmt>();
            cast<CompoundStmt>(func->body_)->children_.push_back(ret);
        }
    }

    else if (auto* record = dyn_cast<RecordDecl>(node)) {
        auto* type = translation_unit_.type_table_.resolve_type(record);

        if (translation_unit_.symbol_table_.exists_in_scope(record->name_)) {
            register_error(std::format("redefinition of '{}'", record->name_), record->source_);
        }

        record->symbol_ = translation_unit_.symbol_table_.insert(record, type);

        translation_unit_.symbol_table_.enter_scope();

        for (auto* field : record->fields_) {
            decorate(field);
        }

        translation_unit_.symbol_table_.exit_scope();
    }

    else if (auto* cmpd_stmt = dyn_cast<CompoundStmt>(node)) {
        for (auto* child : cmpd_stmt->children_) {
            decorate(child);
        }
    }

    else if (auto* return_stmt = dyn_cast<ReturnStmt>(node)) {
        if (return_stmt->value_) {
            decorate(return_stmt->value_);
        }
    }

    else if (auto* if_stmt = dyn_cast<IfStmt>(node)) {
        translation_unit_.symbol_table_.enter_scope();

        decorate(if_stmt->cond_);
        decorate(if_stmt->then_stmt_);
        decorate(if_stmt->else_stmt_);

        translation_unit_.symbol_table_.exit_scope();
    }

    else if (auto* while_stmt = dyn_cast<WhileStmt>(node)) {
        translation_unit_.symbol_table_.enter_scope();

        decorate(while_stmt->cond_);
        decorate(while_stmt->body_);

        translation_unit_.symbol_table_.exit_scope();
    }

    else if (auto* for_stmt = dyn_cast<ForStmt>(node)) {
        translation_unit_.symbol_table_.enter_scope();

        decorate(for_stmt->init_);
        decorate(for_stmt->cond_);
        decorate(for_stmt->update_);
        decorate(for_stmt->body_);

        translation_unit_.symbol_table_.exit_scope();
    }

    else if (auto* binary = dyn_cast<BinaryExpr>(node)) {
        decorate(binary->left_);
        decorate(binary->right_);
    }

    // should the function pattern matching logic be contained in this handler?
    else if (auto* ref = dyn_cast<ReferenceExpr>(node)) {
        if (auto existing = translation_unit_.symbol_table_.lookup(ref->name_)) {

            assert(existing->size() == 1);

            auto [_, symbol] = *existing->begin();
            ref->symbol_ = symbol;
        } else {
            register_error(std::format("use of undeclared identifier '{}'", ref->name_), ref->source_);
            // invalid reference state -> invalid symbol (ref->symbol_ == nullptr)
        }
    }

    else if (auto* call = dyn_cast<CallExpr>(node)) {
        for (auto* arg : call->args_) {
            decorate(arg);
        }

        decorate(call->callee_);
    }

    else if (auto* member_expr = dyn_cast<MemberExpr>(node)) {
        decorate(member_expr->base_);
    }

    else if (auto* arr_expr = dyn_cast<ArraySubscriptExpr>(node)) {
        decorate(arr_expr->base_);
        decorate(arr_expr->index_);
    }

    else if (auto* init_list = dyn_cast<InitListExpr>(node)) {
        for (auto* value : init_list->init_values_) {
            decorate(value);
        }
    }

    return;
}



























Type* SemanticAnalyzer::check_type(ASTNode* node)
{
    if (auto* translation_unit = dyn_cast<TranslationUnitDecl>(node)) {
        for (auto* decl : translation_unit->decls_) {
            check_type(decl);
        }

        return nullptr;
    }

    else if (auto* var = dyn_cast<VarDecl>(node)) {
        if (var->init_) {
            if (auto* init_list = dyn_cast<InitListExpr>(var->init_)) {
                if (auto* record_type = dyn_cast<RecordType>(var->symbol_->type_)) {
                    if (init_list->init_values_.size() > record_type->fields_.size()) {
                        register_error(std::format("too many initializers for type '{}'", type_to_str(var->symbol_->type_)), init_list->source_);
                    }

                    for (auto [field, init_value] : std::views::zip(record_type->fields_, init_list->init_values_)) {
                        auto* init_value_type = check_type(init_value);
                        if (init_value_type == nullptr) {
                            return nullptr;
                        }

                        if (init_value_type != field.type_) {
                            register_error(std::format("cannot initialize field '{}' of type '{}' with value of type '{}'",
                                field.name_,
                                type_to_str(field.type_),
                                type_to_str(init_value_type)),
                                init_value->source_);
                        }
                    }
                } else if (auto* array_type = dyn_cast<ArrayType>(var->symbol_->type_)) {
                    if (init_list->init_values_.size() > array_type->size_) {
                        register_error(std::format("too many initializers for type '{}'", type_to_str(var->symbol_->type_)), init_list->source_);
                    }

                    for (auto* init_value : init_list->init_values_) {
                        auto* init_value_type = check_type(init_value);
                        if (init_value_type == nullptr) {
                            return nullptr;
                        }

                        if (init_value_type != array_type->inner_type_) {
                            register_error(std::format("error: cannot initialize array element of type '{}' with a value of type '{}'",
                                type_to_str(array_type->inner_type_),
                                type_to_str(init_value_type)),
                                init_value->source_);
                        }
                    }
                } else {
                    register_error(std::format("cannot use initializer list on non-aggregate type '{}'", type_to_str(var->symbol_->type_)), init_list->source_);
                    return nullptr;
                }
            } else {
                auto* init_type = check_type(var->init_);

                if (init_type == nullptr) {
                    return nullptr;
                }

                if (remove_const_qualifier(var->symbol_->type_) != remove_const_qualifier(init_type)) {
                    register_error(std::format("type mismatch ({} and {})", type_to_str(var->symbol_->type_), type_to_str(init_type)), var->source_);
                    return nullptr;
                }

                return init_type;
            }
        }

        return nullptr;
    }

    // make sure return type and return statment type match
    else if (auto* func = dyn_cast<FuncDecl>(node)) {
        auto* return_type = cast<FunctionType>(func->symbol_->type_)->return_type_;

        check_type(func->body_);

        return nullptr;
    }

    else if (auto* record = dyn_cast<RecordDecl>(node)) {
        for (auto* field : record->fields_) {
            check_type(field);
        }

        return nullptr;
    }

    else if (auto* compound = dyn_cast<CompoundStmt>(node)) {
        for (auto* c : compound->children_) {
            check_type(c);
        }

        return nullptr;
    }

    else if (auto* return_stmt = dyn_cast<ReturnStmt>(node)) {
        if (return_stmt->value_) {
            check_type(return_stmt->value_);
        }

        return nullptr;
    }

    else if (auto* if_stmt = dyn_cast<IfStmt>(node)) {
        auto* cond_type = check_type(if_stmt->cond_);
        if (cond_type == nullptr) {
            return nullptr;
        }

        if (!convertible_to_boolean(cond_type)) {
            register_error(std::format("condition expression must be of boolean or scalar type"), if_stmt->cond_->source_);
            return nullptr;
        }

        check_type(if_stmt->then_stmt_);

        if (if_stmt->else_stmt_) {
            check_type(if_stmt->else_stmt_);
        }

        return nullptr;
    }

    else if (auto* while_stmt = dyn_cast<WhileStmt>(node)) {
        auto* cond_type = check_type(while_stmt->cond_);
        if (cond_type == nullptr) {
            return nullptr;
        }

        if (!convertible_to_boolean(cond_type)) {
            register_error(std::format("condition expression must be of boolean or scalar type"), while_stmt->cond_->source_);
            return nullptr;
        }

        return nullptr;
    }

    else if (dyn_cast<IntegerLiteralExpr>(node)) {
        return translation_unit_.type_table_.builtin_types_["int"];
    }

    else if (dyn_cast<FloatLiteralExpr>(node)) {
        return translation_unit_.type_table_.builtin_types_["float"];
    }

    else if (dyn_cast<HexLiteralExpr>(node)) {
        return translation_unit_.type_table_.builtin_types_["int"];
    }

    else if (dyn_cast<CharLiteralExpr>(node)) {
        return translation_unit_.type_table_.builtin_types_["char"];
    }

    else if (dyn_cast<StringLiteralExpr>(node)) {
        auto* char_type = translation_unit_.type_table_.builtin_types_["char"];
        return translation_unit_.type_table_.get_or_create<PointerType>(char_type);
    }

    else if (dyn_cast<BooleanLiteralExpr>(node)) {
        return translation_unit_.type_table_.builtin_types_["bool"];
    }

    else if (auto* unary = dyn_cast<UnaryExpr>(node)) {
        auto* operand_type = check_type(unary->operand_);
        if (operand_type == nullptr) {
            return nullptr;
        }

        if (unary->op_ == "&") {
            return translation_unit_.type_table_.get_or_create<PointerType>(operand_type);
        }

        else if (unary->op_ == "*") {
            if (!isa<PointerType>(operand_type)) {
                register_error(std::format("dereferencing non-pointer type {}", type_to_str(operand_type)), unary->source_);
                return nullptr;
            }
            // dereference -> return underlying type
            return cast<PointerType>(operand_type)->inner_type_;
        }

        else if (unary->op_ == "!") {
            if (!convertible_to_boolean(operand_type)) {
                register_error(std::format("invalid argument type '{}' to unary expression '!'", type_to_str(operand_type)), unary->source_);
                return nullptr;
            }
            return translation_unit_.type_table_.builtin_types_["bool"];
        }

        else if (unary->op_ == "~") {
            if (!isa<IntegerType>(operand_type)) {
                register_error(std::format("non-integral type '{}' to unary expression '~'", type_to_str(operand_type)), unary->source_);
                return nullptr;
            }
        }

        else if (unary->op_ == "++" || unary->op_ == "--") {
            if (!is_scalar(operand_type) || is_const(operand_type)) {
                register_error(std::format("cannot '{}' value of type ({})", unary->op_, type_to_str(operand_type)), unary->source_);
                return nullptr;
            }
        }

        return operand_type;
    }

    else if (auto* binary = dyn_cast<BinaryExpr>(node)) {
        auto* left_type = check_type(binary->left_);
        auto* right_type = check_type(binary->right_);

        if (left_type == nullptr || right_type == nullptr) {
            return nullptr;
        }

        if (binary->op_ == "=") {
            if (!(is_lvalue(binary->left_) || is_pointer_dereference(binary->left_))) {
                register_error(std::format("cannot assign to an rvalue", type_to_str(left_type), type_to_str(right_type)), binary->source_);
                return nullptr;
            }

            if (is_const(left_type)) {
                register_error(std::format("cannot assign to a const-qualified variable", type_to_str(left_type), type_to_str(right_type)), binary->source_);
                return nullptr;
            }
        }

        left_type = remove_const_qualifier(left_type);
        right_type = remove_const_qualifier(right_type);

        if (left_type != right_type) {
            register_error(std::format("type mismatch ({} and {})", type_to_str(left_type), type_to_str(right_type)), binary->source_);
            return nullptr;
        }

        if (is_arithmetic_op(binary->op_)) {
            if (!is_scalar(left_type) || !is_scalar(right_type)) {
                register_error(std::format("invalid operands for '{}' ({} and {})", binary->op_, type_to_str(left_type), type_to_str(right_type)), binary->source_);
                return nullptr;
            }
        }

        else if (is_bitwise_op(binary->op_)) {
            if (!isa<IntegerType>(left_type) || !isa<IntegerType>(right_type)) {
                register_error(std::format("invalid operands for '{}' ({} and {})", binary->op_, type_to_str(left_type), type_to_str(right_type)), binary->source_);
                return nullptr;
            }
        }

        else if (is_logical_op(binary->op_)) {
            if (!convertible_to_boolean(left_type) || !convertible_to_boolean(right_type)) {
                register_error(std::format("invalid operands for '{}' ({} and {})", binary->op_, type_to_str(left_type), type_to_str(right_type)), binary->source_);
                return nullptr;
            }
            return translation_unit_.type_table_.builtin_types_["bool"];
        }

        else if (is_relational_op(binary->op_)) {
            if (!is_scalar(left_type) || !is_scalar(right_type)) {
                register_error(std::format("invalid operands for '{}' ({} and {})", binary->op_, type_to_str(left_type), type_to_str(right_type)), binary->source_);
                return nullptr;
            }
            return translation_unit_.type_table_.builtin_types_["bool"];
        }

        return left_type;
    }

    else if (auto* ref = dyn_cast<ReferenceExpr>(node)) {
        return ref->symbol_->type_;
    }

    else if (auto* call = dyn_cast<CallExpr>(node)) {
        auto* func_type = cast<FunctionType>((cast<ReferenceExpr>(call->callee_)->symbol_->type_));

        if (func_type->params_.size() != call->args_.size()) {
            register_error(std::format("expected '{}' arguments in call expression", func_type->params_.size()), call->source_);
            return nullptr;
        }

        for (auto [param_type, arg] : std::views::zip(func_type->params_, call->args_)) {
            auto* arg_type = check_type(arg);
            if (arg_type == nullptr)
                return nullptr;

            if (param_type != arg_type) {
                register_error(std::format("passing '{}' to parameter of incompatible type '{}'", type_to_str(arg_type), type_to_str(param_type)), call->source_);
                return nullptr;
            }
        }

        return func_type->return_type_;
    }

    // TODO: handle const properly with struct
    // TODO: ordering of pointer + arrow check is wrong when you try to access field of a T* when T is not struct

    else if (auto* expr = dyn_cast<MemberExpr>(node)) {
        auto* base = cast<ReferenceExpr>(expr->base_);

        auto* type = base->symbol_->type_;

        if (isa<PointerType>(type) && !expr->is_arrow_) {
            register_error(std::format("cannot access member '{}' with '.' because expression has '{}' type (use '->' instead)", expr->member_, type_to_str(base->symbol_->type_)), expr->source_);
            return nullptr;
        }

        if (!isa<PointerType>(type) && expr->is_arrow_) {
            register_error(std::format("cannot access member '{}' with '->' because expression has '{}' type (use '.' instead)", expr->member_, type_to_str(base->symbol_->type_)), expr->source_);
            return nullptr;
        }

        if (auto* pointer_type = dyn_cast<PointerType>(type)) {
            type = pointer_type->inner_type_;
        }

        if (auto* record_type = dyn_cast<RecordType>(type)) {
            auto* field = record_type->lookup_field(expr->member_);
            if (field == nullptr) {
                register_error(std::format("no member named '{}' in '{}'", expr->member_, type_to_str(record_type)), expr->source_);
                return nullptr;
            } else {
                return field->type_;
            }
        } else {
            register_error(std::format("member reference base type '{}' is not a struct", type_to_str(type)), expr->source_);
            return nullptr;
        }
    }

    else if (auto* expr = dyn_cast<ArraySubscriptExpr>(node)) {
        auto* base = cast<ReferenceExpr>(expr->base_);

        if (auto* array_type = dyn_cast<ArrayType>(base->symbol_->type_)) {
            return array_type->inner_type_;
        } else if (auto* pointer_type = dyn_cast<PointerType>(base->symbol_->type_)) {
            return pointer_type->inner_type_;
        } else {
            register_error(std::format("type '{}' does not support subscript operator", type_to_str(base->symbol_->type_)), expr->source_);
            return nullptr;
        }
    }

    else {
        return nullptr;
    }
}


















ASTNode* for_to_while(AST& ast, ForStmt* for_stmt)
{
    cast<CompoundStmt>(for_stmt->body_)->children_.push_back(for_stmt->update_);

    auto* while_stmt = ast.emplace<WhileStmt>(for_stmt->cond_, for_stmt->body_);

    std::vector<ASTNode*> children;
    children.push_back(for_stmt->init_);
    children.push_back(while_stmt);

    return ast.emplace<CompoundStmt>(std::move(children));
}

bool is_compound_assign(std::string_view op)
{
    static const std::unordered_set<std::string_view> compound_ops { "+=", "-=", "*=", "/=", "%=" };
    return compound_ops.contains(op);
}

ASTNode* expand_compound_assignment(AST& ast, BinaryExpr* expr)
{
    auto op = expr->op_[0];
    auto* right = ast.emplace<BinaryExpr>(std::string{ op }, expr->left_, expr->right_);
    return ast.emplace<BinaryExpr>("=", expr->left_, right);
}

ASTNode* arr_idx_to_ptr_arithmetic(AST& tree, ArraySubscriptExpr* expr)
{
    auto* bin_op = tree.emplace<BinaryExpr>("+", expr->base_, expr->index_);
    return tree.emplace<UnaryExpr>("*", bin_op, false);
}

void SemanticAnalyzer::desugar(ASTNode*& node)
{
    if (auto* translation_unit = dyn_cast<TranslationUnitDecl>(node)) {
        for (auto*& decl : translation_unit->decls_) {
            desugar(decl);
        }
    }

    else if (auto* func = dyn_cast<FuncDecl>(node)) {
        desugar(func->body_);
    }

    else if (auto* cmpd_stmt = dyn_cast<CompoundStmt>(node)) {
        for (auto*& c : cmpd_stmt->children_) {
            desugar(c);
        }
    }

    else if (auto* return_stmt = dyn_cast<ReturnStmt>(node)) {
        if (return_stmt->value_) {
            desugar(return_stmt->value_);
        }
    }

    else if (auto* if_stmt = dyn_cast<IfStmt>(node)) {
        desugar(if_stmt->cond_);
        desugar(if_stmt->then_stmt_);
        if (if_stmt->else_stmt_) {
            desugar(if_stmt->else_stmt_);
        }
    }

    else if (auto* while_stmt = dyn_cast<WhileStmt>(node)) {
        desugar(while_stmt->cond_);
        desugar(while_stmt->body_);
    }

    // for -> while
    else if (auto* for_stmt = dyn_cast<ForStmt>(node)) {
        desugar(for_stmt->body_);
        node = for_to_while(ast_, for_stmt);
    }

    // ++a -> a = a + 1
    else if (auto* unary = dyn_cast<UnaryExpr>(node)) {
        if (unary->op_ == "++" || unary->op_ == "--") {
            auto op = unary->op_[0];

            auto* one = ast_.emplace<IntegerLiteralExpr>(1);
            auto* right = ast_.emplace<BinaryExpr>(std::string{ op }, unary->operand_, one);
            auto* expanded = ast_.emplace<BinaryExpr>("=", unary->operand_, right);

            node = expanded;
        }
    }

    // a += b -> a = a + b
    // a > b -> b < a
    else if (auto* binary = dyn_cast<BinaryExpr>(node)) {
        desugar(binary->left_);
        desugar(binary->right_);

        if (is_compound_assign(binary->op_)) {
            node = expand_compound_assignment(ast_, binary);
        } else if (binary->op_ == ">") {
            binary->op_ = "<";
            std::swap(binary->left_, binary->right_);
        }
    }

    else if (auto* init_list = dyn_cast<InitListExpr>(node)) {
        for (auto*& v : init_list->init_values_) {
            desugar(v);
        }
    }
}
