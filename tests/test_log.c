#define _POSIX_C_SOURCE 200809L
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "../log.h"

/* ---- log_parse_level ------------------------------------------------- */

static void test_parse_level_valid(void) {
    assert(log_parse_level("DEBUG") == LOG_LEVEL_DEBUG);
    assert(log_parse_level("INFO")  == LOG_LEVEL_INFO);
    assert(log_parse_level("WARN")  == LOG_LEVEL_WARN);
    assert(log_parse_level("ERROR") == LOG_LEVEL_ERROR);
}

static void test_parse_level_case_insensitive(void) {
    assert(log_parse_level("debug") == LOG_LEVEL_DEBUG);
    assert(log_parse_level("Info")  == LOG_LEVEL_INFO);
    assert(log_parse_level("wArN")  == LOG_LEVEL_WARN);
    assert(log_parse_level("error") == LOG_LEVEL_ERROR);
}

static void test_parse_level_null(void) {
    assert(log_parse_level(NULL) == LOG_LEVEL_INFO);
}

static void test_parse_level_unknown(void) {
    assert(log_parse_level("TRACE")   == LOG_LEVEL_UNKNOWN);
    assert(log_parse_level("")        == LOG_LEVEL_UNKNOWN);
    assert(log_parse_level("garbage") == LOG_LEVEL_UNKNOWN);
}

/* ---- log_level_color ------------------------------------------------- */

static void test_level_color_valid(void) {
    assert(log_level_color(LOG_LEVEL_DEBUG) != NULL);
    assert(log_level_color(LOG_LEVEL_INFO)  != NULL);
    assert(log_level_color(LOG_LEVEL_WARN)  != NULL);
    assert(log_level_color(LOG_LEVEL_ERROR) != NULL);
    /* Verify distinct colors for levels that should differ */
    assert(log_level_color(LOG_LEVEL_DEBUG) != log_level_color(LOG_LEVEL_ERROR));
    assert(log_level_color(LOG_LEVEL_WARN)  != log_level_color(LOG_LEVEL_ERROR));
}

static void test_level_color_out_of_range(void) {
    assert(log_level_color((LogLevel)-1)              != NULL);
    assert(log_level_color((LogLevel)LOG_LEVEL_COUNT)  != NULL);
    assert(log_level_color((LogLevel)99)               != NULL);
}

/* ---- log_set_pre_write ----------------------------------------------- */

static int pre_write_called = 0;
static FILE* pre_write_stream = NULL;

static void mock_pre_write(FILE* out) {
    pre_write_called = 1;
    pre_write_stream = out;
}

static void test_pre_write_fires(void) {
    pre_write_called = 0;
    pre_write_stream = NULL;
    log_set_pre_write(mock_pre_write);

    /* WARN/ERROR go to stderr */
    log_msg("ERROR", "test");
    assert(pre_write_called == 1);
    assert(pre_write_stream == stderr);

    /* Reset and test stdout path */
    pre_write_called = 0;
    pre_write_stream = NULL;
    log_msg("INFO", "test");
    assert(pre_write_called == 1);
    assert(pre_write_stream == stdout);

    log_set_pre_write(NULL);
}

static void test_pre_write_null_safe(void) {
    pre_write_called = 0;
    log_set_pre_write(NULL);
    log_msg("INFO", "test");
    assert(pre_write_called == 0);
}

/* ---- log_msg_impl edge cases ----------------------------------------- */

static void test_null_message(void) {
    /* Should not crash — prints "(null)" */
    log_msg("INFO", NULL);
}

static void test_null_func(void) {
    /* NULL func should produce "?" in output */
    log_msg_impl("test", "INFO", NULL);
}

/* ---- output routing -------------------------------------------------- */

/* Helper: redirect a stream to a temp file, call fn, read back the output */
static int capture_stream(FILE* stream, void (*fn)(void), char* buf, size_t bufsz) {
    (void)fflush(stream);
    int orig_fd = dup(fileno(stream));
    assert(orig_fd >= 0);

    FILE* tmp = tmpfile();
    assert(tmp != NULL);
    int tmp_fd = fileno(tmp);

    int rc = dup2(tmp_fd, fileno(stream));
    assert(rc >= 0);
    fn();
    (void)fflush(stream);

    /* Read back */
    off_t len = lseek(tmp_fd, 0, SEEK_END);
    lseek(tmp_fd, 0, SEEK_SET);
    if (len <= 0) {
        buf[0] = '\0';
    } else {
        size_t to_read = (size_t)len < bufsz - 1 ? (size_t)len : bufsz - 1;
        ssize_t n = read(tmp_fd, buf, to_read);
        buf[n > 0 ? (size_t)n : 0] = '\0';
    }

    rc = dup2(orig_fd, fileno(stream));
    assert(rc >= 0);
    close(orig_fd);
    fclose(tmp);
    return (int)len;
}

static void emit_info(void)  { log_msg("INFO", "route_test"); }
static void emit_debug(void) { log_msg("DEBUG", "route_test"); }
static void emit_warn(void)  { log_msg("WARN", "route_test"); }
static void emit_error(void) { log_msg("ERROR", "route_test"); }

static void test_info_goes_to_stdout(void) {
    char buf[512];
    int len = capture_stream(stdout, emit_info, buf, sizeof(buf));
    assert(len > 0);
    assert(strstr(buf, "route_test") != NULL);
}

static void test_debug_goes_to_stdout(void) {
    char buf[512];
    int len = capture_stream(stdout, emit_debug, buf, sizeof(buf));
    assert(len > 0);
    assert(strstr(buf, "route_test") != NULL);
}

static void test_warn_goes_to_stderr(void) {
    char buf[512];
    int len = capture_stream(stderr, emit_warn, buf, sizeof(buf));
    assert(len > 0);
    assert(strstr(buf, "route_test") != NULL);
}

static void test_error_goes_to_stderr(void) {
    char buf[512];
    int len = capture_stream(stderr, emit_error, buf, sizeof(buf));
    assert(len > 0);
    assert(strstr(buf, "route_test") != NULL);
}

/* ---- color mode ------------------------------------------------------ */

static void test_color_mode_overrides(void) {
    log_set_color_mode(LOG_COLOR_MODE_ALWAYS);
    assert(log_stream_wants_color(stdout) == 1);
    log_set_color_mode(LOG_COLOR_MODE_NEVER);
    assert(log_stream_wants_color(stdout) == 0);
    log_set_color_mode(LOG_COLOR_MODE_AUTO);
}

static void test_color_auto_plain_when_not_a_terminal(void) {
    FILE* tmp = tmpfile();
    assert(tmp != NULL);
    assert(unsetenv("NO_COLOR") == 0);
    assert(log_stream_wants_color(tmp) == 0);
    fclose(tmp);
}

static void test_color_auto_honours_no_color(void) {
    log_set_color_mode(LOG_COLOR_MODE_AUTO);
    assert(setenv("NO_COLOR", "1", 1) == 0);
    assert(log_stream_wants_color(stdout) == 0);
    /* An empty NO_COLOR does not count as set. */
    assert(setenv("NO_COLOR", "", 1) == 0);
    assert(log_stream_wants_color(stdout) == (isatty(fileno(stdout)) ? 1 : 0));
    assert(unsetenv("NO_COLOR") == 0);
}

static void test_captured_output_has_no_escapes(void) {
    char buf[512];
    log_set_color_mode(LOG_COLOR_MODE_AUTO);
    capture_stream(stdout, emit_info, buf, sizeof(buf));
    assert(strchr(buf, '\x1b') == NULL);
    assert(strstr(buf, "route_test") != NULL);
}

static void test_forced_color_has_escapes(void) {
    char buf[512];
    log_set_color_mode(LOG_COLOR_MODE_ALWAYS);
    capture_stream(stderr, emit_error, buf, sizeof(buf));
    log_set_color_mode(LOG_COLOR_MODE_AUTO);
    assert(strstr(buf, LOG_COLOR_RED) != NULL);
    assert(strstr(buf, LOG_COLOR_RESET) != NULL);
}

/* ---- timestamp format ------------------------------------------------ */

static void test_timestamp_format(void) {
    char buf[512];
    capture_stream(stdout, emit_info, buf, sizeof(buf));
    /* Expect YYYY-MM-DD HH:MM:SS.sss somewhere in the output */
    /* Match pattern: 4 digits, dash, 2, dash, 2, space, 2, colon, 2, colon, 2, dot, 3 */
    size_t slen = strlen(buf);
    const char* p = buf;
    int found = 0;
    while (*p && (size_t)(p - buf) + 20 <= slen) {
        if (p[0] >= '0' && p[0] <= '9' &&
            p[4] == '-' && p[7] == '-' && p[10] == ' ' &&
            p[13] == ':' && p[16] == ':' && p[19] == '.') {
            found = 1;
            break;
        }
        p++;
    }
    assert(found);
}

/* ---- enable_vt (no-op on Linux, but should return 1) ----------------- */

static void test_enable_vt(void) {
    assert(log_enable_vt() == 1);
    assert(log_enable_vt_handle(NULL) == 1);
}

/* ---- runner ---------------------------------------------------------- */

int main(void) {
    int pass = 0;

    #define RUN(fn) do { \
        printf("  %-40s", #fn); \
        fn(); \
        printf("PASS\n"); \
        pass++; \
    } while (0)

    printf("=== log.h tests ===\n");
    RUN(test_parse_level_valid);
    RUN(test_parse_level_case_insensitive);
    RUN(test_parse_level_null);
    RUN(test_parse_level_unknown);
    RUN(test_level_color_valid);
    RUN(test_level_color_out_of_range);
    RUN(test_pre_write_fires);
    RUN(test_pre_write_null_safe);
    RUN(test_null_message);
    RUN(test_null_func);
    RUN(test_info_goes_to_stdout);
    RUN(test_debug_goes_to_stdout);
    RUN(test_warn_goes_to_stderr);
    RUN(test_error_goes_to_stderr);
    RUN(test_color_mode_overrides);
    RUN(test_color_auto_plain_when_not_a_terminal);
    RUN(test_color_auto_honours_no_color);
    RUN(test_captured_output_has_no_escapes);
    RUN(test_forced_color_has_escapes);
    RUN(test_timestamp_format);
    RUN(test_enable_vt);

    printf("---\n%d passed\n", pass);
    return 0;
}
