#include <print>
#include <format>
#include <iostream>

#include "print.hpp"
#include "utils/casting.hpp"
#include "utils/fatal.hpp"
#include "frontend/sema/symbol.hpp"


namespace {

std::unordered_map<QualifierKind, std::string> qualkind_to_str {
    {QualifierKind::Const, "const"}
};

std::unordered_map<RecordKind, std::string> record_kind_to_str {
    {RecordKind::Struct,  "struct"},
    {RecordKind::Enum,    "enum"},
    {RecordKind::Union,   "union"},
};

}


// ******************** TYPE PRINTING ********************


std::string type_to_str(Type* type)
{
    if (auto* t = dyn_cast<VoidType>(type)) {
        return "void";
    }

    else if (auto* t = dyn_cast<ByteType>(type)) {
        return "byte";
    }

    else if (auto* t = dyn_cast<CharType>(type)) {
        return "char";
    }

    else if (auto* t = dyn_cast<BoolType>(type)) {
        return "bool";
    }

    else if (auto* t = dyn_cast<IntegerType>(type)) {
        if (t->is_signed_)
            return std::format("int{}", t->bit_width_);
        return std::format("uint{}", t->bit_width_);
    }

    else if (auto* t = dyn_cast<FloatType>(type)) {
        return std::format("float{}", t->bit_width_);
    }

    else if (auto* t = dyn_cast<PointerType>(type)) {
        return std::format("{}*", type_to_str(t->inner_type_));
    }

    else if (auto* t = dyn_cast<ArrayType>(type)) {
        return std::format("{}[{}]", type_to_str(t->inner_type_), t->size_);
    }

    else if (auto* t = dyn_cast<QualifierType>(type)) {
        return std::format("{} {}", qualkind_to_str[t->kind_], type_to_str(t->inner_type_));
    }

    else if (auto* t = dyn_cast<FunctionType>(type)) {
        return std::format("Function '{}'", t->name_);
    }

    else if (auto* t = dyn_cast<RecordType>(type)) {
        return std::format("{} {}", record_kind_to_str[t->kind_], t->name_);
    }

    else {
        fatal_error("type_to_str()");
    }
}


// ******************** AST printing ********************


std::string node_to_str(ASTNode* node, std::string indent = "")
{
    if (auto* translation_unit = dyn_cast<TranslationUnitDecl>(node)) {
        std::string decls_str{};

        for (auto* decl : translation_unit->decls_) {
            decls_str += node_to_str(decl, indent + "  ");
            if (decl != translation_unit->decls_.back()) {
                decls_str += "\n\n";
            }
        }

        return indent + std::format("TranslationUnitDecl ({})\n{}", translation_unit->name_, decls_str);
    }

    else if (auto* var = dyn_cast<VarDecl>(node)) {
        if (var->init_) {
            return indent + std::format("VarDecl ['{}', {}]\n{}", var->symbol_->identifier_, type_to_str(var->symbol_->type_), node_to_str(var->init_, indent + "  "));
        } else {
            return indent + std::format("VarDecl ['{}', {}]", var->symbol_->identifier_, type_to_str(var->symbol_->type_));
        }
    }

    else if (auto* param = dyn_cast<ParamDecl>(node)) {
        return indent + std::format("ParamDecl ['{}', {}]", param->symbol_->identifier_, type_to_str(param->symbol_->type_));
    }

    else if (auto* func = dyn_cast<FuncDecl>(node)) {
        auto* return_type = cast<FunctionType>(func->symbol_->type_)->return_type_;

        std::string param_type_list_str{};
        std::string param_decls_str{};

        if (!func->params_.empty()) {
            for (auto* param : static_cast_view<ParamDecl>(func->params_)) {
                param_type_list_str += type_to_str(param->symbol_->type_);
                param_decls_str += node_to_str(param, indent + "  ");

                if (param != func->params_.back()) {
                    param_type_list_str += ", ";
                    param_decls_str += '\n';
                }
            }

            return indent + std::format("FuncDecl '{}' ({}) -> ({})\n{}\n{}",
                func->symbol_->identifier_,
                param_type_list_str,
                type_to_str(return_type),
                param_decls_str,
                node_to_str(func->body_, indent + "  "));
        }

        return indent + std::format("FuncDecl '{}' () -> ({})\n{}",
            func->symbol_->identifier_,
            type_to_str(return_type),
            node_to_str(func->body_, indent + "  "));
    }

    else if (auto* record = dyn_cast<RecordDecl>(node)) {
        std::string field_list_str{};
        for (auto* field : record->fields_) {
            field_list_str += node_to_str(field, indent + "  ");
            if (field != record->fields_.back()) {
                field_list_str += '\n';
            }
        }

        return indent + std::format("RecordDecl ['{}']\n{}", type_to_str(record->symbol_->type_), field_list_str);
    }

    else if (auto* cmpd_stmt = dyn_cast<CompoundStmt>(node)) {
        if (cmpd_stmt->children_.empty()) {
            return indent + std::format("CompoundStmt");
        }

        std::string expr_strs{};
        for (auto* child : cmpd_stmt->children_) {
            expr_strs += node_to_str(child, indent + "  ");
            if (child != cmpd_stmt->children_.back()) {
                expr_strs += '\n';
            }
        }

        return indent + std::format("CompoundStmt\n{}", expr_strs);
    }

    else if (auto* return_stmt = dyn_cast<ReturnStmt>(node)) {
        if (return_stmt->value_) {
            return indent + std::format("ReturnStmt\n{}", node_to_str(return_stmt->value_, indent + "  "));
        }

        return indent + std::format("ReturnStmt\n");
    }

    else if (dyn_cast<BreakStmt>(node)) {
        return indent + std::format("BreakStmt");
    }

    else if (dyn_cast<ContinueStmt>(node)) {
        return indent + std::format("ContinueStmt");
    }

    else if (auto* if_stmt = dyn_cast<IfStmt>(node)) {
        if (if_stmt->else_stmt_) {
            return indent + std::format("IfStmt\n{}\n{}\n{}",
                node_to_str(if_stmt->cond_, indent + "  "),
                node_to_str(if_stmt->then_stmt_, indent + "  "),
                node_to_str(if_stmt->else_stmt_, indent + "  "));
        }

        return indent + std::format("IfStmt\n{}\n{}",
            node_to_str(if_stmt->cond_, indent + "  "),
            node_to_str(if_stmt->then_stmt_, indent + "  "));
    }

    else if (auto* while_stmt = dyn_cast<WhileStmt>(node)) {
        return indent + std::format("WhileStmt\n{}\n{}",
            node_to_str(while_stmt->cond_, indent + "  "),
            node_to_str(while_stmt->body_, indent + "  "));
    }

    else if (auto* for_stmt = dyn_cast<ForStmt>(node)) {
        return indent + std::format("ForStmt\n{}\n{}\n{}\n{}",
            node_to_str(for_stmt->init_, indent + "  "),
            node_to_str(for_stmt->cond_, indent + "  "),
            node_to_str(for_stmt->update_, indent + "  "),
            node_to_str(for_stmt->body_, indent + "  "));
    }

    else if (auto* i = dyn_cast<IntegerLiteralExpr>(node)) {
        return indent + std::format("IntLiteral ['{}']", i->value_);
    }

    else if (auto* f = dyn_cast<FloatLiteralExpr>(node)) {
        return indent + std::format("FloatLiteral ['{}']", f->value_);
    }

    else if (auto* f = dyn_cast<HexLiteralExpr>(node)) {
        return indent + std::format("HexLiteral ['{}']", f->value_);
    }

    else if (auto* c = dyn_cast<CharLiteralExpr>(node)) {
        return indent + std::format("CharLiteral ['{}']", c->value_);
    }

    else if (auto* s = dyn_cast<StringLiteralExpr>(node)) {
        return indent + std::format("StringLiteral ['{}']", s->value_);
    }

    else if (auto* b = dyn_cast<BooleanLiteralExpr>(node)) {
        return indent + std::format("BoolLiteral ['{}']", b->value_);
    }

    else if (auto* unary = dyn_cast<UnaryExpr>(node)) {
        auto op_name = unary->is_postfix_ ? "PostFixUnaryOp" : "PrefixUnaryOp";

        return indent + std::format("{} ['{}']\n{}", op_name, unary->op_, node_to_str(unary->operand_, indent + "  "));
    }

    else if (auto* binary = dyn_cast<BinaryExpr>(node)) {
        return indent + std::format("BinOp ['{}']\n{}\n{}",
            binary->op_,
            node_to_str(binary->left_, indent + "  "),
            node_to_str(binary->right_, indent + "  "));
    }

    else if (auto* ref = dyn_cast<ReferenceExpr>(node)) {
        return indent + std::format("RefExpr ['{}', {}]", ref->symbol_->identifier_, type_to_str(ref->symbol_->type_));
    }

    else if (auto* call = dyn_cast<CallExpr>(node)) {
        std::string args_str{};

        for (auto* arg : call->args_) {
            args_str += node_to_str(arg, indent + "  ");
            if (arg != call->args_.back()) {
                args_str += '\n';
            }
        }

        return indent + std::format("CallExpr\n{}\n{}", node_to_str(call->callee_, indent + "  "), args_str);
    }

    else if (auto* expr = dyn_cast<MemberExpr>(node)) {
        std::string_view access_str = expr->is_arrow_ ? "->" : ".";

        return indent + std::format("MemberExpr ['{}{}']\n{}",
            access_str,
            expr->member_,
            node_to_str(expr->base_, indent + "  "));
    }

    else if (auto* arr_expr = dyn_cast<ArraySubscriptExpr>(node)) {
        return indent + std::format("ArraySubscriptExpr\n{}\n{}",
            node_to_str(arr_expr->base_, indent + "  "),
            node_to_str(arr_expr->index_, indent + "  "));
    }

    else if (auto* init_list = dyn_cast<InitListExpr>(node)) {
        std::string init_list_str{};
        for (auto* value : init_list->init_values_) {
            init_list_str += node_to_str(value, indent + "  ");
            if (value != init_list->init_values_.back()) {
                init_list_str += "\n";
            }
        }

        return indent + std::format("InitListExpr\n{}", init_list_str);
    }

    else {
        return "parse error";
    }
}

void print(AST& ast)
{
    std::println("{}", node_to_str(ast.root_));
}


// ******************** IR printing ********************


std::string get_target_list_str(Branch* branch)
{
    std::string target_label_list{};
    for (auto* target : branch->targets()) {
    target_label_list += std::format("label {}", target->name_);
    if (target != branch->targets().back())
        target_label_list += ", ";
    }

    return target_label_list;
}

std::string get_argument_list_str(Call* call)
{
    std::string args_list_str{};
    for (auto* arg : call->operands_ | std::views::drop(1)) {
        args_list_str += std::format("{}", arg->name_);
        if (arg != call->operands_.back())
            args_list_str += ", ";
    }

    return args_list_str;
}

std::string get_phi_operands_str(Phi* phi)
{
    std::string phi_operands_str{};
    for (auto [value, block] : phi->operands_) {
        phi_operands_str += std::format("[{}, {}]", value->name_, block->name_);
        if (value != phi->operands_.back().first) {
            phi_operands_str += ", ";
        }
    }

    return phi_operands_str;
}

std::string ir_value_to_str(Value* value)
{
    switch (value->kind_) {
        case ValueKind::Alloca: {
            auto* inst = cast<Alloca>(value);
            return std::format("{} = alloca", inst->name_); // x = alloca int32
        }

        case ValueKind::Load: {
            auto* inst = cast<Load>(value);
            return std::format("{} = load {}", inst->name_, inst->operands_[0]->name_);
        }

        case ValueKind::Store: {
            auto* inst = cast<Store>(value);
            return std::format("store {}, {}", inst->operands_[0]->name_, inst->operands_[1]->name_);
        }

        case ValueKind::Add: {
            auto* inst = cast<Add>(value);
            return std::format("{} = add {}, {}", inst->name_, inst->operands_[0]->name_, inst->operands_[1]->name_);
        }

        case ValueKind::Sub: {
            auto* inst = cast<Sub>(value);
            return std::format("{} = sub {}, {}", inst->name_, inst->operands_[0]->name_, inst->operands_[1]->name_);
        }

        case ValueKind::Mul: {
            auto* inst = cast<Mul>(value);
            return std::format("{} = mul {}, {}", inst->name_, inst->operands_[0]->name_, inst->operands_[1]->name_);
        }

        case ValueKind::Div: {
            auto* inst = cast<Div>(value);
            return std::format("{} = div {}, {}", inst->name_, inst->operands_[0]->name_, inst->operands_[1]->name_);
        }

        case ValueKind::Mod: {
            auto* inst = cast<Mod>(value);
            return std::format("{} = mod {}, {}", inst->name_, inst->operands_[0]->name_, inst->operands_[1]->name_);
        }

        case ValueKind::Eq: {
            auto* inst = cast<Eq>(value);
            return std::format("{} = eq {}, {}", inst->name_, inst->operands_[0]->name_, inst->operands_[1]->name_);
        }

        case ValueKind::Ne: {
            auto* inst = cast<Ne>(value);
            return std::format("{} = ne {}, {}", inst->name_, inst->operands_[0]->name_, inst->operands_[1]->name_);
        }

        case ValueKind::Not: {
            auto* inst = cast<Not>(value);
            return std::format("{} = not {}", inst->name_, inst->operands_[0]->name_);
        }

        case ValueKind::Slt: {
            auto* inst = cast<Slt>(value);
            return std::format("{} = slt {}, {}", inst->name_, inst->operands_[0]->name_, inst->operands_[1]->name_);
        }

        case ValueKind::Sle: {
            auto* inst = cast<Sle>(value);
            return std::format("{} = sle {}, {}", inst->name_, inst->operands_[0]->name_, inst->operands_[1]->name_);
        }

        case ValueKind::Ult: {
            auto* inst = cast<Ult>(value);
            return std::format("{} = Ult {}, {}", inst->name_, inst->operands_[0]->name_, inst->operands_[1]->name_);
        }

        case ValueKind::Ule: {
            auto* inst = cast<Ule>(value);
            return std::format("{} = Ule {}, {}", inst->name_, inst->operands_[0]->name_, inst->operands_[1]->name_);
        }

        case ValueKind::Shl: {
            auto* inst = cast<Shl>(value);
            return std::format("{} = shl {}, {}", inst->name_, inst->operands_[0]->name_, inst->operands_[1]->name_);
        }

        case ValueKind::Ashr: {
            auto* inst = cast<Ashr>(value);
            return std::format("{} = ashr {}, {}", inst->name_, inst->operands_[0]->name_, inst->operands_[1]->name_);
        }

        case ValueKind::Lshr: {
            auto* inst = cast<Lshr>(value);
            return std::format("{} = lshr {}, {}", inst->name_, inst->operands_[0]->name_, inst->operands_[1]->name_);
        }

        case ValueKind::Call: {
            auto* inst = cast<Call>(value);

            auto* func = cast<Function>(inst->operands_[0]);

            if (func->return_value_)
                return std::format("{} = call @{}({})", inst->name_, func->name_, get_argument_list_str(inst));

            return std::format("call @{}({})", func->name_, get_argument_list_str(inst));
        }

        case ValueKind::Return: {
            auto* inst = cast<Return>(value);

            if (inst->operands_.empty())
                return "ret";

            return std::format("ret {}", inst->operands_[0]->name_);
        }

        case ValueKind::Branch: {
            auto* inst = cast<Branch>(value);

            return inst->is_conditional() ?
                std::format("br {}, label {}, label {}", inst->condition()->name_, inst->targets()[0]->name_, inst->targets()[1]->name_) :
                std::format("br label {}", inst->operands_[0]->name_);
        }

        case ValueKind::Phi: {
            auto* inst = cast<Phi>(value);
            return std::format("{} = φ {}", inst->name_, get_phi_operands_str(inst));
        }

        case ValueKind::PtrAdd: {
            auto* inst = cast<PtrAdd>(value);
            return std::format("{} = ptradd {}, {}", inst->name_, inst->operands_[0]->name_, inst->operands_[1]->name_);
        };

        default: {
            std::println("{}", (int)value->kind_);
            fatal_error("ir_value_to_str()");
        }
    }
}

std::string get_arg_list_str(const std::unique_ptr<Function>& function)
{
    std::string arg_list;

    for (auto& arg : function->args_) {
        arg_list += arg->name_ + ": " + type_to_str(arg->type_);
        if (arg != function->args_.back()) {
            arg_list += ", ";
        }
    }

    return arg_list;
}

std::string get_pred_list_str(const std::unique_ptr<BasicBlock>& block)
{
    std::string pred_list;

    if (!block->predecessors().empty()) {
        pred_list = "preds: [";
        for (auto pred : block->predecessors()) {
            pred_list += pred->name_;
            if (pred != block->predecessors().back()) {
                pred_list += ", ";
            }
        }
        pred_list += ']';
    }

    return pred_list;
}

void print(IRProgram& program)
{
    for (const auto& function : program.functions_) {
        std::print("define @{}({}) -> ({}) ", function->name_, get_arg_list_str(function), type_to_str(cast<FunctionType>(function->type_)->return_type_));
        std::cout << "{\n";
        for (const auto& block : function->blocks_) {
            std::println("{:<30}{}", block->name_ + ":", get_pred_list_str(block));
            for (const auto& inst : block->instructions_) {
                std::println("  {}", ir_value_to_str(inst.get()));
            }
            if (block != function->blocks_.back()) {
                std::println();
            }
        }
        std::cout << "}\n\n";
    }
}
