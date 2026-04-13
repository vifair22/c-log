CC      := gcc
CXX     := g++

BUILD   := build
BINS    := $(BUILD)/bins
TESTS   := tests
DEMOS   := demos

# ---- Flags ------------------------------------------------------------
WARN_FLAGS := -Wshadow -Wunused -Wunused-function -Wunused-variable \
              -Wunused-parameter -Wunused-result -Wdouble-promotion \
              -Wformat=2 -Wformat-truncation -Wcast-align \
              -Wcast-qual -Wnull-dereference -Wconversion -Wsign-conversion

C_WARN     := $(WARN_FLAGS) -Wmissing-prototypes -Wstrict-prototypes \
              -Wmissing-declarations

CFLAGS     := -std=c11 -Wall -Wextra -Wpedantic $(C_WARN) \
              -fstack-protector-strong -fstack-clash-protection -O2
CXXFLAGS   := -std=c++17 -Wall -Wextra -Wpedantic $(WARN_FLAGS) \
              -fstack-protector-strong -fstack-clash-protection -O2

STACK_LIMIT := 65536

# ---- Targets ----------------------------------------------------------
.PHONY: all clean test demo demo-nocolor analyze lint

all: $(BINS)/test_log $(BINS)/test_log_cpp $(BINS)/demo_c $(BINS)/demo_cpp

$(BINS):
	mkdir -p $(BINS)

# ---- Tests ------------------------------------------------------------
$(BINS)/test_log: $(TESTS)/test_log.c log.h | $(BINS)
	$(CC) $(CFLAGS) $(TESTS)/test_log.c -o $@

$(BINS)/test_log_cpp: $(TESTS)/test_log_cpp.cpp log.h | $(BINS)
	$(CXX) $(CXXFLAGS) $(TESTS)/test_log_cpp.cpp -o $@

test: $(BINS)/test_log $(BINS)/test_log_cpp
	@$(BINS)/test_log
	@$(BINS)/test_log_cpp

# ---- Demos ------------------------------------------------------------
$(BINS)/demo_c: $(DEMOS)/demo_c.c log.h | $(BINS)
	$(CC) $(CFLAGS) $(DEMOS)/demo_c.c -o $@

$(BINS)/demo_cpp: $(DEMOS)/demo_cpp.cpp log.h | $(BINS)
	$(CXX) $(CXXFLAGS) $(DEMOS)/demo_cpp.cpp -o $@

demo: $(BINS)/demo_c $(BINS)/demo_cpp
	@echo "=== C demo ===" && $(BINS)/demo_c
	@echo "=== C++ demo ===" && $(BINS)/demo_cpp

demo-nocolor: $(BINS)/demo_c_nc $(BINS)/demo_cpp_nc
	@echo "=== C no-color ===" && $(BINS)/demo_c_nc
	@echo "=== C++ no-color ===" && $(BINS)/demo_cpp_nc

$(BINS)/demo_c_nc: $(DEMOS)/demo_c.c log.h | $(BINS)
	$(CC) $(CFLAGS) -DLOG_NO_COLOR $(DEMOS)/demo_c.c -o $@

$(BINS)/demo_cpp_nc: $(DEMOS)/demo_cpp.cpp log.h | $(BINS)
	$(CXX) $(CXXFLAGS) -DLOG_NO_COLOR $(DEMOS)/demo_cpp.cpp -o $@

# ---- Static analysis --------------------------------------------------
analyze:
	@echo "=== stack-usage ==="
	@mkdir -p $(BUILD)/analyze
	@$(CC) $(CFLAGS) -fstack-usage -c $(TESTS)/test_log.c -o $(BUILD)/analyze/test_log.o 2>/dev/null
	@fail=0; for su in $(BUILD)/analyze/*.su; do \
	    [ -f "$$su" ] || continue; \
	    awk -v limit=$(STACK_LIMIT) -v file="$$su" \
	        '$$2+0 > limit { printf "STACK OVERFLOW RISK: %s %s (%s bytes, limit %d)\n", file, $$1, $$2, limit; found=1 } \
	         END { exit (found ? 1 : 0) }' "$$su" || fail=1; \
	done; \
	rm -rf $(BUILD)/analyze; \
	if [ $$fail -ne 0 ]; then echo "stack-usage: FAIL"; exit 1; fi
	@echo "stack-usage: OK"
	@echo "=== gcc-fanalyzer (C) ==="
	@$(CC) $(CFLAGS) -fanalyzer -fsyntax-only $(TESTS)/test_log.c
	@echo "gcc-fanalyzer: OK"
	@echo "=== cppcheck ==="
	@cppcheck --enable=warning,performance,portability --error-exitcode=1 \
	    --suppress=missingIncludeSystem \
	    --suppress=uninitvar:log.h \
	    --quiet $(TESTS)/test_log.c
	@echo "cppcheck: OK"

lint:
	@echo "=== clang-tidy ==="
	@clang-tidy $(TESTS)/test_log.c -- -std=c11 2>&1 | \
	    grep -E "warning:|error:" || echo "clang-tidy: OK"

clean:
	rm -rf $(BUILD)
