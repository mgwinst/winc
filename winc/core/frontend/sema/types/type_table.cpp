#include "type_table.hpp"
#include "utils/casting.hpp"
#include "utils/fatal.hpp"

namespace
{
    constexpr bool UNSIGNED = false;
    constexpr bool SIGNED = true;
}

TypeTable::TypeTable()
{
    builtin_types_.emplace("void", arena.emplace<VoidType>());
    builtin_types_.emplace("byte", arena.emplace<ByteType>());
    builtin_types_.emplace("char", arena.emplace<CharType>());
    builtin_types_.emplace("bool", arena.emplace<BoolType>());

    builtin_types_.emplace("int8", arena.emplace<IntegerType>(uint16_t{ 8 }, SIGNED));
    builtin_types_.emplace("int16", arena.emplace<IntegerType>(uint16_t{ 16 }, SIGNED));
    builtin_types_.emplace("int32", arena.emplace<IntegerType>(uint16_t{ 32 }, SIGNED));
    builtin_types_.emplace("int", builtin_types_["int32"]);
    builtin_types_.emplace("int64", arena.emplace<IntegerType>(uint16_t{ 64 }, SIGNED));

    builtin_types_.emplace("uint8", arena.emplace<IntegerType>(uint16_t{ 8 }, UNSIGNED));
    builtin_types_.emplace("uint16", arena.emplace<IntegerType>(uint16_t{ 16 }, UNSIGNED));
    builtin_types_.emplace("uint32", arena.emplace<IntegerType>(uint16_t{ 32 }, UNSIGNED));
    builtin_types_.emplace("uint", builtin_types_["uint32"]);
    builtin_types_.emplace("uint64", arena.emplace<IntegerType>(uint16_t{ 64 }, UNSIGNED));

    builtin_types_.emplace("float16", arena.emplace<FloatType>(uint16_t{ 16 }));
    builtin_types_.emplace("float32", arena.emplace<FloatType>(uint16_t{ 32 }));
    builtin_types_.emplace("float", builtin_types_["float32"]);
    builtin_types_.emplace("float64", arena.emplace<FloatType>(uint16_t{ 64 }));
}

Type* TypeTable::resolve_type(ASTNode* type_expr)
{
    if (auto* qual = dyn_cast<QualifierTypeExpr>(type_expr))
    {
        if (qual->kind_ == QualifierKind::Const) {
            return get_or_create<QualifierType>(QualifierKind::Const, resolve_type(qual->inner_));
        }

        fatal_error("qualifier not supported");
    }
    else if (auto* ptr = dyn_cast<PointerTypeExpr>(type_expr))
    {
        return get_or_create<PointerType>(resolve_type(ptr->inner_));
    }
    else if (auto* arr = dyn_cast<ArrayTypeExpr>(type_expr))
    {
        if (!arr->size_)
        {
            fatal_error("array must have a size");
        }

        if (auto* size = dyn_cast<IntegerLiteralExpr>(arr->size_))
        {
            return get_or_create<ArrayType>(resolve_type(arr->inner_), static_cast<uint64_t>(size->value_));
        }
        else
        {
            fatal_error("array size must be Integer constant");
        }
    }
    else if (auto* named = dyn_cast<NamedTypeExpr>(type_expr))
    {
        if (auto it = builtin_types_.find(named->name_); it != builtin_types_.end())
        {
            return it->second;
        }

        if (auto it = user_defined_types_.find(named->name_); it != user_defined_types_.end())
        {
            return it->second;
        }

        // we need access to the diagnostics object so we can report proper error instead of terminate
        fatal_error(std::format("type '{}' does not exist", named->name_));
    }
    else if (auto* func = dyn_cast<FuncDecl>(type_expr))
    {
        std::vector<Type*> param_types;
        for (auto* param : static_cast_view<ParamDecl>(func->params_))
        {
            param_types.push_back(resolve_type(param->type_expr_));
        }

        auto* ret_type = resolve_type(func->return_type_expr_);

        return get_or_create<FunctionType>(func->name_, param_types, ret_type);
    }
    else if (auto* record = dyn_cast<RecordDecl>(type_expr))
    {
        std::vector<Field> fields;
        for (auto* field : static_cast_view<VarDecl>(record->fields_))
        {
            Type* field_type = resolve_type(field->type_expr_);
            fields.emplace_back(field->name_, field_type);
        }

        auto* record_type = get_or_create<RecordType>(record->kind_, record->name_);
        cast<RecordType>(record_type)->fields_ = std::move(fields);

        user_defined_types_.emplace(record->name_, record_type);
        return record_type;
    }

    fatal_error("TypeTable::resolve_type() -> type mismatch during type resolution");
}
