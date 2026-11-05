#include "IR.hpp"

Function* IRProgram::insert(Function* function)
{
    functions_.emplace_back(function);
    return function;
}
