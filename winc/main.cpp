#include "driver/driver.hpp"
#include "middleend/ir/IRBuilder.hpp"

int main(int argc, const char** argv)
{
    Driver driver;
    driver.parse_command_args(argc, argv);
    driver.run();
}
