#include "driver/driver.hpp"

int main(int argc, const char** argv)
{
    Driver driver;
    driver.parse_command_args(argc, argv);
    driver.run();
}
