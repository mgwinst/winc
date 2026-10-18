#pragma once

#include "frontend/sema/types/types.hpp"
#include "frontend/ast/ast.hpp"
#include "middleend/ir/IR.hpp"

std::string type_to_str(Type* type);
std::string node_to_str(ASTNode* node, std::string indent);
std::string ir_value_to_str(Value* value);
void print(AST& ast);
void print(IRProgram& program);
