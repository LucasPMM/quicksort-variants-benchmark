CC ?= cc
COVERAGE_CC ?= gcc
ANALYZER_CC ?= gcc
GCOV ?= gcov
CLANG_FORMAT ?= clang-format-18
CPPFLAGS ?=
CFLAGS ?= -O2
LDFLAGS ?=
LDLIBS ?=
ASAN_OPTIONS ?= detect_leaks=0

STANDARD_FLAGS := -std=c17 -D_POSIX_C_SOURCE=200809L
WARNING_FLAGS := -Wall -Wextra -Wpedantic -Wconversion -Wshadow \
	-Wsign-conversion -Werror
DEPENDENCY_FLAGS := -MMD -MP
SANITIZER_FLAGS := -g -O1 -fno-omit-frame-pointer -fsanitize=address,undefined
COVERAGE_FLAGS := -g -O0 --coverage
ANALYZER_FLAGS := -g -O0 -fanalyzer

TARGET := sort
SOURCES := src/main.c src/application.c src/cli.c src/input.c src/quicksort.c \
	src/partition.c src/stack.c src/benchmark.c src/report.c
HEADERS := $(wildcard include/*.h)
RELEASE_DIR := build/release
SANITIZER_DIR := build/sanitize
COVERAGE_DIR := build/coverage
RELEASE_OBJECTS := $(patsubst src/%.c,$(RELEASE_DIR)/%.o,$(SOURCES))
SANITIZER_OBJECTS := $(patsubst src/%.c,$(SANITIZER_DIR)/%.o,$(SOURCES))
COVERAGE_OBJECTS := $(patsubst src/%.c,$(COVERAGE_DIR)/%.o,$(SOURCES))
SORT_TEST_OBJECTS := $(RELEASE_DIR)/quicksort.o $(RELEASE_DIR)/partition.o \
	$(RELEASE_DIR)/stack.o
SANITIZER_SORT_TEST_OBJECTS := $(SANITIZER_DIR)/quicksort.o \
	$(SANITIZER_DIR)/partition.o $(SANITIZER_DIR)/stack.o
COVERAGE_SORT_TEST_OBJECTS := $(COVERAGE_DIR)/quicksort.o \
	$(COVERAGE_DIR)/partition.o $(COVERAGE_DIR)/stack.o
SORT_TEST := build/tests/sort-tests
SANITIZER_SORT_TEST := $(SANITIZER_DIR)/sort-tests
COVERAGE_SORT_TEST := $(COVERAGE_DIR)/sort-tests
BENCHMARK_TEST := build/tests/benchmark-tests
SANITIZER_BENCHMARK_TEST := $(SANITIZER_DIR)/benchmark-tests
COVERAGE_BENCHMARK_TEST := $(COVERAGE_DIR)/benchmark-tests
FAILURE_TEST := build/tests/failure-tests
SANITIZER_FAILURE_TEST := $(SANITIZER_DIR)/failure-tests
COVERAGE_FAILURE_TEST := $(COVERAGE_DIR)/failure-tests
WRAP_FLAGS := -Wl,--wrap=malloc -Wl,--wrap=calloc -Wl,--wrap=clock_gettime
BENCHMARK_TEST_OBJECTS := $(RELEASE_DIR)/benchmark.o $(RELEASE_DIR)/input.o \
	$(SORT_TEST_OBJECTS)
SANITIZER_BENCHMARK_TEST_OBJECTS := $(SANITIZER_DIR)/benchmark.o \
	$(SANITIZER_DIR)/input.o $(SANITIZER_SORT_TEST_OBJECTS)
COVERAGE_BENCHMARK_TEST_OBJECTS := $(COVERAGE_DIR)/benchmark.o \
	$(COVERAGE_DIR)/input.o $(COVERAGE_SORT_TEST_OBJECTS)
ANALYZER_TARGET := build/analyzer/sort
FORMAT_SOURCES := $(SOURCES) $(HEADERS) tests/test_sort.c \
	tests/test_benchmark.c tests/test_failures.c
BENCHMARK_OUTPUT ?= build/benchmark-matrix.csv
BENCHMARK_SEED ?= 42
FIGURES_DIR ?= docs/figures

.PHONY: all test sanitize coverage analyze format check-format benchmark-matrix figures clean

all: $(TARGET)

$(RELEASE_DIR)/%.o: src/%.c
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) -Iinclude $(STANDARD_FLAGS) $(WARNING_FLAGS) \
		$(CFLAGS) $(DEPENDENCY_FLAGS) -c $< -o $@

$(SANITIZER_DIR)/%.o: src/%.c
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) -Iinclude $(STANDARD_FLAGS) $(WARNING_FLAGS) \
		$(SANITIZER_FLAGS) $(DEPENDENCY_FLAGS) -c $< -o $@

$(COVERAGE_DIR)/%.o: src/%.c
	@mkdir -p $(@D)
	$(COVERAGE_CC) $(CPPFLAGS) -Iinclude $(STANDARD_FLAGS) $(WARNING_FLAGS) \
		$(COVERAGE_FLAGS) $(DEPENDENCY_FLAGS) -c $< -o $@

$(TARGET): $(RELEASE_OBJECTS)
	$(CC) $(RELEASE_OBJECTS) $(LDFLAGS) $(LDLIBS) -o $@

$(SANITIZER_DIR)/$(TARGET): $(SANITIZER_OBJECTS)
	$(CC) $(SANITIZER_FLAGS) $(SANITIZER_OBJECTS) $(LDFLAGS) $(LDLIBS) -o $@

$(COVERAGE_DIR)/$(TARGET): $(COVERAGE_OBJECTS)
	$(COVERAGE_CC) $(COVERAGE_FLAGS) $(COVERAGE_OBJECTS) $(LDFLAGS) $(LDLIBS) -o $@

$(SORT_TEST): tests/test_sort.c $(SORT_TEST_OBJECTS)
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) -Iinclude $(STANDARD_FLAGS) $(WARNING_FLAGS) \
		$(CFLAGS) $^ $(LDFLAGS) $(LDLIBS) -o $@

$(SANITIZER_SORT_TEST): tests/test_sort.c $(SANITIZER_SORT_TEST_OBJECTS)
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) -Iinclude $(STANDARD_FLAGS) $(WARNING_FLAGS) \
		$(SANITIZER_FLAGS) $^ $(LDFLAGS) $(LDLIBS) -o $@

$(COVERAGE_SORT_TEST): tests/test_sort.c $(COVERAGE_SORT_TEST_OBJECTS)
	@mkdir -p $(@D)
	$(COVERAGE_CC) $(CPPFLAGS) -Iinclude $(STANDARD_FLAGS) $(WARNING_FLAGS) \
		$(COVERAGE_FLAGS) $^ $(LDFLAGS) $(LDLIBS) -o $@

$(BENCHMARK_TEST): tests/test_benchmark.c $(BENCHMARK_TEST_OBJECTS)
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) -Iinclude $(STANDARD_FLAGS) $(WARNING_FLAGS) \
		$(CFLAGS) $^ $(LDFLAGS) $(LDLIBS) -o $@

$(SANITIZER_BENCHMARK_TEST): tests/test_benchmark.c \
	$(SANITIZER_BENCHMARK_TEST_OBJECTS)
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) -Iinclude $(STANDARD_FLAGS) $(WARNING_FLAGS) \
		$(SANITIZER_FLAGS) $^ $(LDFLAGS) $(LDLIBS) -o $@

$(COVERAGE_BENCHMARK_TEST): tests/test_benchmark.c \
	$(COVERAGE_BENCHMARK_TEST_OBJECTS)
	@mkdir -p $(@D)
	$(COVERAGE_CC) $(CPPFLAGS) -Iinclude $(STANDARD_FLAGS) $(WARNING_FLAGS) \
		$(COVERAGE_FLAGS) $^ $(LDFLAGS) $(LDLIBS) -o $@

$(FAILURE_TEST): tests/test_failures.c $(BENCHMARK_TEST_OBJECTS)
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) -Iinclude $(STANDARD_FLAGS) $(WARNING_FLAGS) \
		$(CFLAGS) $^ $(WRAP_FLAGS) $(LDFLAGS) $(LDLIBS) -o $@

$(SANITIZER_FAILURE_TEST): tests/test_failures.c \
	$(SANITIZER_BENCHMARK_TEST_OBJECTS)
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) -Iinclude $(STANDARD_FLAGS) $(WARNING_FLAGS) \
		$(SANITIZER_FLAGS) $^ $(WRAP_FLAGS) $(LDFLAGS) $(LDLIBS) -o $@

$(COVERAGE_FAILURE_TEST): tests/test_failures.c \
	$(COVERAGE_BENCHMARK_TEST_OBJECTS)
	@mkdir -p $(@D)
	$(COVERAGE_CC) $(CPPFLAGS) -Iinclude $(STANDARD_FLAGS) $(WARNING_FLAGS) \
		$(COVERAGE_FLAGS) $^ $(WRAP_FLAGS) $(LDFLAGS) $(LDLIBS) -o $@

$(ANALYZER_TARGET): $(SOURCES) $(HEADERS)
	@mkdir -p $(@D)
	$(ANALYZER_CC) $(CPPFLAGS) -Iinclude $(STANDARD_FLAGS) $(WARNING_FLAGS) \
		$(ANALYZER_FLAGS) $(SOURCES) $(LDFLAGS) $(LDLIBS) -o $@

test: $(TARGET) $(SORT_TEST) $(BENCHMARK_TEST) $(FAILURE_TEST)
	$(SORT_TEST)
	$(BENCHMARK_TEST)
	$(FAILURE_TEST)
	sh tests/run_smoke.sh ./$(TARGET)
	python3 tests/test_benchmark_csv.py
	python3 tests/test_evidence.py

sanitize: $(SANITIZER_DIR)/$(TARGET) $(SANITIZER_SORT_TEST) \
	$(SANITIZER_BENCHMARK_TEST) $(SANITIZER_FAILURE_TEST)
	ASAN_OPTIONS=$(ASAN_OPTIONS) $(SANITIZER_SORT_TEST)
	ASAN_OPTIONS=$(ASAN_OPTIONS) $(SANITIZER_BENCHMARK_TEST)
	ASAN_OPTIONS=$(ASAN_OPTIONS) $(SANITIZER_FAILURE_TEST)
	ASAN_OPTIONS=$(ASAN_OPTIONS) sh tests/run_smoke.sh ./$(SANITIZER_DIR)/$(TARGET)

coverage: $(COVERAGE_DIR)/$(TARGET) $(COVERAGE_SORT_TEST) \
	$(COVERAGE_BENCHMARK_TEST) $(COVERAGE_FAILURE_TEST)
	$(COVERAGE_SORT_TEST)
	$(COVERAGE_BENCHMARK_TEST)
	$(COVERAGE_FAILURE_TEST)
	sh tests/run_smoke.sh ./$(COVERAGE_DIR)/$(TARGET)
	python3 tests/test_benchmark_csv.py
	python3 tests/test_evidence.py
	@mkdir -p $(COVERAGE_DIR)/reports
	$(GCOV) -b -c $(COVERAGE_DIR)/*.gcda > $(COVERAGE_DIR)/summary.txt
	@mv ./*.gcov $(COVERAGE_DIR)/reports/
	@tail -n 1 $(COVERAGE_DIR)/summary.txt

analyze: $(ANALYZER_TARGET)

format:
	$(CLANG_FORMAT) -i $(FORMAT_SOURCES)

check-format:
	$(CLANG_FORMAT) --dry-run --Werror $(FORMAT_SOURCES)

benchmark-matrix: $(TARGET)
	sh scripts/run_benchmark_matrix.sh ./$(TARGET) "$(BENCHMARK_OUTPUT)" "$(BENCHMARK_SEED)"

figures:
	python3 scripts/plot_results.py --output-dir "$(FIGURES_DIR)"

clean:
	rm -rf build $(TARGET)

-include $(RELEASE_OBJECTS:.o=.d) $(SANITIZER_OBJECTS:.o=.d) \
	$(COVERAGE_OBJECTS:.o=.d)
