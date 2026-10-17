#include "IR.hpp"

Function* IRProgram::insert(Function* function)
{
    functions_.push_back(std::unique_ptr<Function>(function));
    return function;
}
