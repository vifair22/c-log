# C-Logging

Small, header-only logging helper for C and C++ with colored output and timestamps.

## Features
- Timestamp format: `YYYY-MM-DD HH:MM:SS.sss`
- Colorized levels (DEBUG/WARN/ERROR)
- Case-insensitive level parsing
- Windows VT enable for older consoles
- Optional `LOG_NO_COLOR` build flag

## Demo projects (Visual Studio)
The solution contains two console apps:
- `C-Logging` (C demo) uses `demo_c.c`
- `C-Logging-CPP` (C++ demo) uses `demo_cpp.cpp`

Open `C-Logging.sln`, select the project you want to run, and build/debug.

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

## Disable color
Define `LOG_NO_COLOR` at compile time:
- MSVC: `/DLOG_NO_COLOR`
- GCC/Clang: `-DLOG_NO_COLOR`

## Notes
- On Windows, `log_msg_impl` calls `log_enable_vt()` once to enable ANSI colors.
- `std::string_view` overloads copy into `std::string` to ensure null-terminated strings.
