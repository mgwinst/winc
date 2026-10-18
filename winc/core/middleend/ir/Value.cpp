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
    if (!value || value == this) {
        return;
    }

    for (auto* user : users_) {
        if (auto* phi = dyn_cast<Phi>(user)) {
            for (auto& operand : phi->operands_) {
                if (operand.first == this) {
                    operand.first = value;
                }
            }
        } else {
            auto* inst = static_cast<Instruction*>(user);
            for (auto& operand : inst->operands_) {
                if (operand == this) {
                    operand = value;
                }
            }
        }

        value->users_.push_back(user);
    }

    users_.clear();
}
