#include "middleend/ir/IR.hpp"

inline bool is_pure(auto& value) {
    return value->kind_ >= ValueKind::Add && value->kind_ <= ValueKind::PtrAdd;
}
