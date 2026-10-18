#pragma once

#include "middleend/ir/IR.hpp"

std::unordered_set<Alloca*> non_escaping_allocas(Function* function);
