CXX = clang++

FLAGS = \
    -std=c++17 \
    -O3 \
    -Isrc/ \
    -Isrc/third_party/xtensor/ \
    -Isrc/third_party/zstd/ \
    -Wpedantic \
    -Wall \
    -Weverything \
    -Wextra \
    -Wno-poison-system-directories \
    -Wno-c++11-extensions \
    -Wno-c++98-compat \
    -Wno-c++98-compat-pedantic \
    -Wno-sign-conversion \
    -Wno-sign-compare \
    -Wno-padded \
    -Wno-format-nonliteral \
    -Wno-old-style-cast \
    -Wno-covered-switch-default \
    -Wno-weak-vtables \
    -Wno-extra-semi-stmt \
    \
    -Wno-unused-macros \
    -Wno-used-but-marked-unused \
    -Wno-documentation-unknown-command \
    -Wno-zero-as-null-pointer-constant \
    -Wno-implicit-fallthrough \
    -Wno-disabled-macro-expansion \
    -Wno-unreachable-code-break \
    -Wno-comma \
    -Wno-deprecated \
    -Wno-extra-semi-stmt \
    -Wno-macro-redefined \
    -Wno-reserved-macro-identifier \
    -Wno-reserved-identifier \
    -Wno-missing-variable-declarations \
    -Wno-unreachable-code \
    -Wno-unreachable-code-return \

SOURCES = `find src/ -type f -name "*.c"` `find src/ -type f -name "*.cc"`

all:
	mkdir -p bin/
	mkdir -p bin/base/
	mkdir -p bin/shuffler/
	#$(CXX) $(FLAGS) $(SOURCES) src/base/json_test.cpp -o bin/base/json_test
	#$(CXX) $(FLAGS) $(SOURCES) src/base/spanner_test.cpp -o bin/base/spanner_test
	#$(CXX) $(FLAGS) $(SOURCES) src/base/string_test.cpp -o bin/base/string_test
	#$(CXX) $(FLAGS) $(SOURCES) src/base/world_test.cpp -o bin/base/world_test
	#$(CXX) $(FLAGS) $(SOURCES) src/shuffler/bench.cpp -o bin/shuffler/bench
	$(CXX) $(FLAGS) $(SOURCES) src/main.cpp -o bin/main

test:
	./bin/base/json_test
	./bin/base/spanner_test
	./bin/base/string_test
	./bin/base/world_test
