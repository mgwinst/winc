#pragma once

#include "frontend/sema/types/types.hpp"
#include "frontend/ast/ast.hpp"

std::string type_to_str(Type* type);
std::string node_to_str(ASTNode* node, std::string indent);
void print(AST& ast);
