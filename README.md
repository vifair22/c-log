# C-Logging

Small, header-only logging helper for C and C++ with colored output and timestamps.

## Features
- Timestamp format: `YYYY-MM-DD HH:MM:SS.sss`
- Colorized levels (DEBUG/WARN/ERROR)
- Case-insensitive level parsing
- Windows VT enable for older consoles
- Plain output when the stream is not a terminal, or when `NO_COLOR` is set
- Optional `LOG_NO_COLOR` build flag

## Build & test
```
make test          # run unit tests
make demo          # run C + C++ demos
make demo-nocolor  # demos with LOG_NO_COLOR
make analyze       # gcc -fanalyzer + cppcheck
```

## C usage
```c
#include "log.h"

int main(void) {
    log_msg("INFO", "Hello from C");
    log_msg("ERROR", "Something failed");
    return 0;
}
```

## C++ usage
```cpp
#include "log.h"
#include <string>
#include <string_view>

int main() {
    log_msg("INFO", "Hello from C++");
    log_msg(std::string("WARN"), std::string("Be careful"));
    log_msg(std::string_view("DEBUG"), std::string_view("Details"));
    return 0;
}
```

## Requirements
Full functionality:
- C: C11 preferred (uses `timespec_get` for millisecond timestamps when available).
- C++: C++17 (enables `std::string_view` overloads).

Minimums (core logging):
- C: C99 (uses `__func__`).
- C++: C++17 (the header includes `<string_view>`).

## Color
Colors are written only when the stream is a terminal and `NO_COLOR` is unset
or empty, so output captured by journald, a pipe or a file stays plain.
Call `log_set_color_mode(LOG_COLOR_MODE_ALWAYS)` or `LOG_COLOR_MODE_NEVER` to
override the check. The setting applies to the translation unit that makes the
call.

To compile colors out entirely, define `LOG_NO_COLOR`:
- MSVC: `/DLOG_NO_COLOR`
- GCC/Clang: `-DLOG_NO_COLOR`

## Notes
- On Windows, `log_msg_impl` calls `log_enable_vt()` once to enable ANSI colors.
- `std::string_view` overloads copy into `std::string` to ensure null-terminated strings.
