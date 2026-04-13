#include <cassert>
#include <cstdio>
#include <string>
#include <string_view>
#include "../log.h"

/* ---- C++ overloads --------------------------------------------------- */

static void test_const_char(void) {
    log_msg("INFO", "const char test");
}

static void test_std_string(void) {
    std::string level = "WARN";
    std::string message = "std::string test";
    log_msg(level, message);
}

static void test_string_view(void) {
    std::string_view level = "DEBUG";
    std::string_view message = "string_view test";
    log_msg(level, message);
}

static void test_mixed_types(void) {
    /* string level, const char* message via implicit conversion */
    log_msg(std::string("ERROR"), std::string("mixed test"));
    log_msg(std::string_view("INFO"), std::string_view("mixed test 2"));
}

/* ---- pre_write hook from C++ ----------------------------------------- */

static int cpp_hook_called = 0;

static void cpp_hook(FILE* out) {
    (void)out;
    cpp_hook_called = 1;
}

static void test_pre_write_from_cpp(void) {
    cpp_hook_called = 0;
    log_set_pre_write(cpp_hook);
    log_msg("INFO", "hook test");
    assert(cpp_hook_called == 1);
    log_set_pre_write(nullptr);
}

/* ---- runner ---------------------------------------------------------- */

int main() {
    int pass = 0;

    #define RUN(fn) do { \
        printf("  %-40s", #fn); \
        fn(); \
        printf("PASS\n"); \
        pass++; \
    } while (0)

    printf("=== log.h C++ tests ===\n");
    RUN(test_const_char);
    RUN(test_std_string);
    RUN(test_string_view);
    RUN(test_mixed_types);
    RUN(test_pre_write_from_cpp);

    printf("---\n%d passed\n", pass);
    return 0;
}
