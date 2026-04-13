#include <iostream>
#include <string>
#include <string_view>
#include "../log.h"

static void test_cpp() {
    std::string level = "Debug";
    std::string message = "Log from C++ function";
    log_msg(level, message);
}

int main() {
    std::cout << "C++ demo start\n";
    log_msg("DEBUG", "Debug message");
    log_msg(std::string("INFO"), std::string("Informational message"));
    log_msg(std::string_view("WARN"), std::string_view("Warning message"));
    log_msg("ERROR", "Error message");
    test_cpp();
    std::cout << "C++ demo end\n";
    return 0;
}
