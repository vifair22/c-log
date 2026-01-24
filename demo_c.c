#include <stdio.h>
#include "log.h"

static void test(void) {
    log_msg("Debug", "Log from C function");
}

int main(void) {
    printf("C demo start\n");
    log_msg("DEBUG", "Debug message");
    log_msg("INFO", "Informational message");
    log_msg("WARN", "Warning message");
    log_msg("ERROR", "Error message");
    test();
    printf("C demo end\n");
    return 0;
}