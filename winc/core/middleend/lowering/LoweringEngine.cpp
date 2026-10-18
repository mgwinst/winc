#include <unordered_map>
#include <utility>

#include "LoweringEngine.hpp"
#include "frontend/ast/ast.hpp"
#include "frontend/sema/helpers.hpp"
#include "utils/casting.hpp"
#include "utils/enums.hpp"
#include "utils/fatal.hpp"

bool load_required(ASTNode* node)
{
    if (isa<ReferenceExpr>(node)) {
        return true;
    }

    if (auto* unary = dyn_cast<UnaryExpr>(node)) {
        if (unary->op_ == "*") {
            return true;
        }
    }

    return false;
}

Value* LoweringEngine::lower(ASTNode* node)
{
    if (auto* translation_unit = dyn_cast<TranslationUnitDecl>(node))
    {
        for (auto* decl : translation_unit->decls_) {
            lower(decl);
        }

        return nullptr;
    }

    else if (auto* var = dyn_cast<VarDecl>(node)) {
        auto* alloca = builder_.create<Alloca>(var->symbol_->type_, var->symbol_->identifier_);
        alloca_map_[var->symbol_->identifier_] = alloca;

        if (!var->init_) {
            return alloca;
        } else {
            auto* init_value = lower(var->init_);

            if (load_required(var->init_)) {
                init_value = builder_.create<Load>(init_value);
            }

            // no direct store on initializer list expression
            if (isa<InitListExpr>(var->init_)) {
                return nullptr;
            }

            return builder_.create<Store>(alloca, init_value);
        }
    }

    else if (auto* param = dyn_cast<ParamDecl>(node)) {
        auto* arg = builder_.create<Argument>(param->symbol_->type_, param->symbol_->identifier_);
        auto* alloca = builder_.create<Alloca>(param->symbol_->type_, param->symbol_->identifier_ + ".ptr");
        alloca_map_[param->symbol_->identifier_] = alloca;
        auto* store = builder_.create<Store>(alloca, arg);

        return nullptr;
    }

    else if (auto* func = dyn_cast<FuncDecl>(node)) {
        auto* function = builder_.create<Function>(func->symbol_->type_, func->symbol_->identifier_);
        function_map_[func->symbol_->identifier_] = function;
        set_current_function(function);

        auto* entry = builder_.create<BasicBlock>("entry");
        set_current_block(entry);

        auto* return_type = cast<FunctionType>(func->symbol_->type_)->return_type_;

        current_function()->return_block_ = builder_.create<BasicBlock>("exit");

        if (return_type == translation_unit_.type_table_.builtin_types_["void"]) {
            set_current_block(current_function()->return_block_);
            builder_.create<Return>();
        } else {
            current_function()->return_value_ = builder_.create<Alloca>(return_type, "retval"); // technically doesn't need to be in alloca map, we can't anyway because of collision (many retvals in the graph)
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

    else if (auto* record = dyn_cast<RecordDecl>(node)) {
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

            if (load_required(return_stmt->value_))
                value = builder_.create<Load>(value);

            builder_.create<Store>(current_function()->return_value_, value);
        }

        builder_.create<Branch>(current_function()->return_block_);

        return nullptr;
    }

    else if (auto* break_stmt = dyn_cast<BreakStmt>(node)) {
        return builder_.create<Branch>(get_loop_context().end_);
    }

    else if (auto* continue_stmt = dyn_cast<ContinueStmt>(node)) {
        return builder_.create<Branch>(get_loop_context().header_);
    }

    else if (auto* if_stmt = dyn_cast<IfStmt>(node)) {
        auto* header_block = current_block();
        auto* cond = lower(if_stmt->cond_);

        auto* if_then_block = builder_.create<BasicBlock>("if.then");
        auto* if_end_block = builder_.create<BasicBlock>("if.end");

        set_current_block(if_then_block);
        lower(if_stmt->then_stmt_);
        builder_.create<Branch>(if_end_block);

        BasicBlock* if_else_block = nullptr;
        if (if_stmt->else_stmt_) {
            if_else_block = builder_.create<BasicBlock>("if.else");
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
        auto* cond = builder_.create<BasicBlock>("loop.cond"); // loop header
        auto* body = builder_.create<BasicBlock>("loop.body");
        auto* end = builder_.create<BasicBlock>("loop.end");

        push_loop_context(preheader, cond, body, end);

        builder_.create<Branch>(cond);

        set_current_block(cond);
        auto* cmp = lower(while_stmt->cond_); // assert(cmp is ...)
        builder_.create<Branch>(cmp, body, end);

        set_current_block(body);
        lower(while_stmt->body_);
        builder_.create<Branch>(cond);

        set_current_block(end);

        pop_loop_context();

        return nullptr;
    }

    else if (auto* int_literal = dyn_cast<IntegerLiteralExpr>(node)) {
        return builder_.create<Const>(int_literal->value_);
    }

    else if (auto* float_literal = dyn_cast<FloatLiteralExpr>(node)) {
        return builder_.create<Const>(float_literal->value_);
    }

    else if (auto* hex_literal = dyn_cast<HexLiteralExpr>(node)) {
        return builder_.create<Const>(hex_literal->value_);
    }

    else if (auto* char_literal = dyn_cast<CharLiteralExpr>(node)) {
        return builder_.create<Const>(char_literal->value_);
    }

    else if (auto* str_literal = dyn_cast<StringLiteralExpr>(node)) {
        return builder_.create<Const>(str_literal->value_);
    }

    else if (auto* bool_literal = dyn_cast<BooleanLiteralExpr>(node)) {
        return bool_literal->value_ ?
            builder_.create<Const>(static_cast<int64_t>(1)) :
            builder_.create<Const>(static_cast<int64_t>(0));
    }

    else if (auto* unary = dyn_cast<UnaryExpr>(node)) {
        auto* operand = lower(unary->operand_);

        if (unary->op_ == "!") {
            if (load_required(unary->operand_))
                operand = builder_.create<Load>(operand);
            return builder_.create<Not>(operand);
        }

        // address of
        if (unary->op_ == "&")
            return operand;

        // pointer dereference
        if (unary->op_ == "*")
            return builder_.create<Load>(operand);

        return nullptr;
    }

    else if (auto* binary = dyn_cast<BinaryExpr>(node)) {
        const BinaryOp op = binary_ops[binary->op_];
        assert(op != BinaryOp::Invalid);

        auto* left = lower(binary->left_);
        auto* right = lower(binary->right_);

        if (op == BinaryOp::Assign) {
            if (load_required(binary->right_)) {
                right = builder_.create<Load>(right);
            }

            return builder_.create<Store>(left, right);
        }

        if (load_required(binary->left_))
            left = builder_.create<Load>(left);

        if (load_required(binary->right_))
            right = builder_.create<Load>(right);

        switch (op) {
            case BinaryOp::Add:
                return builder_.create<Add>(left, right);

            case BinaryOp::Sub:
                return builder_.create<Sub>(left, right);

            case BinaryOp::Mul:
                return builder_.create<Mul>(left, right);

            case BinaryOp::Div:
                return builder_.create<Div>(left, right);

            case BinaryOp::Mod:
                return builder_.create<Mod>(left, right);

            case BinaryOp::Eq:
                return builder_.create<Eq>(left, right);

            case BinaryOp::Ne:
                return builder_.create<Ne>(left, right);

            case BinaryOp::lt: {
                if (is_unsigned(left->type_))
                    return builder_.create<Ult>(left, right);
                else
                    return builder_.create<Slt>(left, right);
            }

            case BinaryOp::le: {
                if (is_unsigned(left->type_))
                    return builder_.create<Ule>(left, right);
                else
                    return builder_.create<Sle>(left, right);
            }

            case BinaryOp::Shl:
                return builder_.create<Shl>(left, right);

            case BinaryOp::Shr: {
                if (is_unsigned(left->type_))
                    return builder_.create<Lshr>(left, right);
                else
                    return builder_.create<Ashr>(left, right);
            }

            case BinaryOp::Xor:
                return builder_.create<Xor>(left, right);

            case BinaryOp::Or:
                return builder_.create<Or>(left, right);

            case BinaryOp::And:
                return builder_.create<And>(left, right);

            default:
                fatal_error("lowering binary op error");
        }
    }

    else if (auto* ref = dyn_cast<ReferenceExpr>(node)) {
        return get_value(ref);
    }

    else if (auto* call = dyn_cast<CallExpr>(node)) {
        auto* callee = function_map_[cast<ReferenceExpr>(call->callee_)->name_];

        std::vector<Value*> args;
        for (auto* arg : call->args_) {
            auto* a = lower(arg);

            if (load_required(arg))
                a = builder_.create<Load>(a);

            args.push_back(a);
        }

        return builder_.create<Call>(callee, std::move(args));
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

        auto* base_ptr = get_value(base);

        return builder_.create<PtrAdd>(base_ptr, index);
    }

    else if (auto* expr = dyn_cast<ArraySubscriptExpr>(node)) {
        auto* base = cast<ReferenceExpr>(expr->base_);

        // assert type is ptr or record
        auto* base_ptr = get_value(base);

        auto* index = lower(expr->index_);

        return builder_.create<PtrAdd>(base_ptr, index);
    }

    else if (auto* init_list_expr = dyn_cast<InitListExpr>(node)) {
        auto* base_ptr = alloca_map_[cast<VarDecl>(init_list_expr->target_)->name_];

        for (auto i = 0uz; i < init_list_expr->init_values_.size(); ++i) {
            auto* index = builder_.create<Const>(static_cast<int64_t>(i));
            auto* ptr = builder_.create<PtrAdd>(base_ptr, index);
            builder_.create<Store>(ptr, lower(init_list_expr->init_values_[i]));
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

    return nullptr;

    // assert() instead?
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
