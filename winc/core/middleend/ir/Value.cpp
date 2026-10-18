#include "IR.hpp"

#include "utils/casting.hpp"

Value::Value(ValueKind kind, Type* type, std::string_view name, Value* parent) :
    kind_{ kind },
    type_{ type },
    name_{ name },
    users_{ },
    parent_{ parent } {}

void Value::add_use(Value* value)
{
    if (value) {
        users_.push_back(value);
    }
}

void Value::replace_uses_with(Value* value)
{
    if (value && value != this) {
        // when user is a phi the cast will fail (make phi inherit from Instruction)
        for (auto* user : static_cast_view<Instruction>(this->users_)) {
            for (auto& operand : user->operands_) {
                if (operand == this) {
                    operand = value;
                }
            }
            value->users_.push_back(user);
        }
    }

    users_.clear();
}
