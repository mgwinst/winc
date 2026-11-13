#include <cassert>
#include <unordered_map>
#include <utility>

#include "LoweringEngine.hpp"
#include "frontend/ast/ast.hpp"
#include "frontend/sema/helpers.hpp"
#include "middleend/ir/IR.hpp"
#include "utils/casting.hpp"
#include "utils/enums.hpp"
#include "utils/fatal.hpp"

Value* LoweringEngine::materialize(Value* value, ExprUse produced, ExprUse requested)
{
    if (requested == ExprUse::Address) {
        assert(produced == ExprUse::Address);
        return value;
    }

    if (produced == ExprUse::Address)
        return builder_.create<Load>(value);

    return value;
}

Value* LoweringEngine::lower(ASTNode* node, ExprUse use)
{
    if (auto* translation_unit = dyn_cast<TranslationUnitDecl>(node)) {
        for (auto* decl : translation_unit->decls_) {
            lower(decl);
        }

        return nullptr;
    }

    else if (auto* var = dyn_cast<VarDecl>(node)) {
        auto* alloca = builder_.create<Alloca>(var->symbol_->type_, var->symbol_->identifier_);
        alloca_map_[var->symbol_->identifier_] = alloca;

        if (!var->init_)
            return alloca;

        // Initializer lists already perform their stores.
        if (isa<InitListExpr>(var->init_)) {
            lower(var->init_);
            return nullptr;
        }

        return builder_.create<Store>(alloca, lower(var->init_));
    }

    else if (auto* param = dyn_cast<ParamDecl>(node)) {
        auto* arg = builder_.create<Argument>(param->symbol_->type_, param->symbol_->identifier_);
        auto* alloca = builder_.create<Alloca>(param->symbol_->type_, param->symbol_->identifier_ + ".ptr");
        alloca_map_[param->symbol_->identifier_] = alloca;
        builder_.create<Store>(alloca, arg);

        return nullptr;
    }

    else if (auto* func = dyn_cast<FuncDecl>(node)) {
        auto* function = builder_.create<Function>(func->symbol_->type_, func->symbol_->identifier_);
        function_map_[func->symbol_->identifier_] = function;
        set_current_function(function);

        auto* entry = builder_.create<BasicBlock>();
        set_current_block(entry);

        auto* return_type = cast<FunctionType>(func->symbol_->type_)->return_type_;

        current_function()->return_block_ = builder_.create<BasicBlock>();

        if (return_type == translation_unit_.type_table_.builtin_types_["void"]) {
            set_current_block(current_function()->return_block_);
            builder_.create<Return>();
        } else {
            current_function()->return_value_ = builder_.create<Alloca>(return_type, "retval");
            set_current_block(current_function()->return_block_);
            builder_.create<Return>(builder_.create<Load>(return_type, current_function()->return_value_));
        }

        set_current_block(entry);

        for (auto* p : func->params_) {
            lower(p);
        }

        lower(func->body_);

        return function;
    }

    else if (isa<RecordDecl>(node)) {
        return nullptr;
    }

    else if (auto* compound = dyn_cast<CompoundStmt>(node)) {
        for (auto c : compound->children_) {
            lower(c);
        }

        return nullptr;
    }

    else if (auto* return_stmt = dyn_cast<ReturnStmt>(node)) {
        if (return_stmt->value_) {
            auto* value = lower(return_stmt->value_);
            builder_.create<Store>(current_function()->return_value_, value);
        }

        builder_.create<Branch>(current_function()->return_block_);

        return nullptr;
    }

    else if (isa<BreakStmt>(node)) {
        return builder_.create<Branch>(get_loop_context().end_);
    }

    else if (isa<ContinueStmt>(node)) {
        return builder_.create<Branch>(get_loop_context().header_);
    }

    else if (auto* if_stmt = dyn_cast<IfStmt>(node)) {
        auto* header_block = current_block();
        auto* cond = lower(if_stmt->cond_);

        auto* if_then_block = builder_.create<BasicBlock>();
        auto* if_end_block = builder_.create<BasicBlock>();

        set_current_block(if_then_block);
        lower(if_stmt->then_stmt_);
        builder_.create<Branch>(if_end_block);

        BasicBlock* if_else_block = nullptr;
        if (if_stmt->else_stmt_) {
            if_else_block = builder_.create<BasicBlock>();
            set_current_block(if_else_block);
            lower(if_stmt->else_stmt_);
            builder_.create<Branch>(if_end_block);
        }

        set_current_block(header_block);

        if (if_else_block) {
            builder_.create<Branch>(cond, if_then_block, if_else_block);
        } else {
            builder_.create<Branch>(cond, if_then_block, if_end_block);
        }

        set_current_block(if_end_block);

        return nullptr;
    }

    else if (auto* while_stmt = dyn_cast<WhileStmt>(node)) {
        auto* preheader = current_block();
        auto* cond = builder_.create<BasicBlock>(); // loop header
        auto* body = builder_.create<BasicBlock>();
        auto* end = builder_.create<BasicBlock>();

        push_loop_context(preheader, cond, body, end);

        builder_.create<Branch>(cond);

        set_current_block(cond);
        auto* cmp = lower(while_stmt->cond_);
        builder_.create<Branch>(cmp, body, end);

        set_current_block(body);
        lower(while_stmt->body_);
        builder_.create<Branch>(cond);

        set_current_block(end);

        pop_loop_context();

        return nullptr;
    }

    else if (auto* int_literal = dyn_cast<IntegerLiteralExpr>(node)) {
        return materialize(builder_.create<Const>(int_literal->value_), ExprUse::Value, use);
    }

    else if (auto* float_literal = dyn_cast<FloatLiteralExpr>(node)) {
        return materialize(builder_.create<Const>(float_literal->value_), ExprUse::Value, use);
    }

    else if (auto* hex_literal = dyn_cast<HexLiteralExpr>(node)) {
        return materialize(builder_.create<Const>(hex_literal->value_), ExprUse::Value, use);
    }

    else if (auto* char_literal = dyn_cast<CharLiteralExpr>(node)) {
        return materialize(builder_.create<Const>(char_literal->value_), ExprUse::Value, use);
    }

    else if (auto* str_literal = dyn_cast<StringLiteralExpr>(node)) {
        return materialize(builder_.create<Const>(str_literal->value_), ExprUse::Value, use);
    }

    else if (auto* bool_literal = dyn_cast<BooleanLiteralExpr>(node)) {
        auto* result = builder_.create<Const>(static_cast<int64_t>(bool_literal->value_));
        return materialize(result, ExprUse::Value, use);
    }

    else if (auto* unary = dyn_cast<UnaryExpr>(node)) {
        if (unary->op_ == "&") {
            auto* ptr = lower(unary->operand_, ExprUse::Address);
            return materialize(ptr, ExprUse::Value, use);
        }

        if (unary->op_ == "*") {
            auto* ptr = lower(unary->operand_);
            return materialize(ptr, ExprUse::Address, use);
        }

        if (unary->op_ == "!") {
            auto* operand = lower(unary->operand_);
            return materialize(builder_.create<Not>(operand), ExprUse::Value, use);
        }

        fatal_error("unsupported unary op");
    }

    else if (auto* binary = dyn_cast<BinaryExpr>(node)) {
        const BinaryOp op = binary_ops[binary->op_];
        assert(op != BinaryOp::Invalid);

        if (op == BinaryOp::Assign) {
            auto* dst = lower(binary->left_, ExprUse::Address);
            auto* src = lower(binary->right_);
            builder_.create<Store>(dst, src);
            return materialize(src, ExprUse::Value, use);
        }

        auto* left = lower(binary->left_);
        auto* right = lower(binary->right_);
        Value* result = nullptr;

        switch (op) {
            case BinaryOp::Add: result = builder_.create<Add>(left, right); break;
            case BinaryOp::Sub: result = builder_.create<Sub>(left, right); break;
            case BinaryOp::Mul: result = builder_.create<Mul>(left, right); break;
            case BinaryOp::Div: result = builder_.create<Div>(left, right); break;
            case BinaryOp::Mod: result = builder_.create<Mod>(left, right); break;
            case BinaryOp::Eq:  result = builder_.create<Eq>(left, right);  break;
            case BinaryOp::Ne:  result = builder_.create<Ne>(left, right);  break;

            case BinaryOp::lt:
                if (is_unsigned(left->type_))
                    result = builder_.create<Ult>(left, right);
                else
                    result = builder_.create<Slt>(left, right);
                break;

            case BinaryOp::le:
                if (is_unsigned(left->type_))
                    result = builder_.create<Ule>(left, right);
                else
                    result = builder_.create<Sle>(left, right);
                break;

            case BinaryOp::Shl: result = builder_.create<Shl>(left, right); break;

            case BinaryOp::Shr:
                if (is_unsigned(left->type_))
                    result = builder_.create<Lshr>(left, right);
                else
                    result = builder_.create<Ashr>(left, right);
                break;

            case BinaryOp::Xor: result = builder_.create<Xor>(left, right); break;
            case BinaryOp::Or:  result = builder_.create<Or>(left, right);  break;
            case BinaryOp::And: result = builder_.create<And>(left, right); break;
            default: fatal_error("lowering binary op error");
        }

        return materialize(result, ExprUse::Value, use);
    }

    else if (auto* ref = dyn_cast<ReferenceExpr>(node)) {
        return materialize(get_value(ref), ExprUse::Address, use);
    }

    else if (auto* call = dyn_cast<CallExpr>(node)) {
        auto* callee = function_map_[cast<ReferenceExpr>(call->callee_)->name_];

        std::vector<Value*> args;
        for (auto* arg : call->args_)
            args.push_back(lower(arg));

        return materialize(builder_.create<Call>(callee, std::move(args)), ExprUse::Value, use);
    }

    else if (auto* expr = dyn_cast<MemberExpr>(node)) {
        auto* base = cast<ReferenceExpr>(expr->base_);

        auto* type = base->symbol_->type_;

        if (expr->is_arrow_) {
            type = cast<PointerType>(type)->inner_type_;
        }

        auto* record_type = cast<RecordType>(type);

        auto field_index = record_type->field_position(expr->member_);
        auto* index = builder_.create<Const>(static_cast<int64_t>(field_index));

        auto* base_ptr = lower(
            expr->base_,
            expr->is_arrow_ ? ExprUse::Value : ExprUse::Address
        );

        return materialize(builder_.create<PtrAdd>(base_ptr, index), ExprUse::Address, use);
    }

    else if (auto* expr = dyn_cast<ArraySubscriptExpr>(node)) {
        auto* base = cast<ReferenceExpr>(expr->base_);

        auto* base_ptr = lower(
            expr->base_,
            isa<PointerType>(base->symbol_->type_) ? ExprUse::Value : ExprUse::Address
        );
        auto* index = lower(expr->index_);

        return materialize(builder_.create<PtrAdd>(base_ptr, index), ExprUse::Address, use);
    }

    else if (auto* init_list_expr = dyn_cast<InitListExpr>(node)) {
        auto* base_ptr = alloca_map_[cast<VarDecl>(init_list_expr->target_)->name_];

        for (auto i = 0uz; i < init_list_expr->init_values_.size(); ++i) {
            auto* index = builder_.create<Const>(static_cast<int64_t>(i));
            auto* ptr = builder_.create<PtrAdd>(base_ptr, index);

            auto* init_value = lower(init_list_expr->init_values_[i]);
            builder_.create<Store>(ptr, init_value);
        }

        return nullptr;
    }

    else {
        std::println("{}", (int)node->kind_);
        fatal_error("LoweringEngine::lower()");
    }
}













Function* LoweringEngine::current_function() const
{
    return builder_.current_function_;
}

BasicBlock* LoweringEngine::current_block() const
{
    return builder_.current_block_;
}

void LoweringEngine::set_current_function(Function* function)
{
    builder_.current_function_ = function;
}

void LoweringEngine::set_current_block(BasicBlock* block)
{
    builder_.current_block_ = block;
}

Value* LoweringEngine::get_value(ReferenceExpr* ref)
{
    if (auto it = alloca_map_.find(ref->symbol_->identifier_); it != alloca_map_.end())
        return it->second;

    fatal_error("variable not found during lowering");
    return nullptr;
}

void LoweringEngine::push_loop_context(BasicBlock* preheader, BasicBlock* header, BasicBlock* body, BasicBlock* end)
{
    loop_context_stack_.emplace(preheader, header, body, end);
}

void LoweringEngine::pop_loop_context()
{
    loop_context_stack_.pop();
}

LoopContext& LoweringEngine::get_loop_context()
{
    return loop_context_stack_.top();
}
