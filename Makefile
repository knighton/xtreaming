CXX = clang++

FLAGS = \
    -std=c++17 \
    -O3 \
    -Isrc/ \
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
    -Wno-weak-vtables

SOURCES = `find src/ -type f -name "*.cc"`

all:
	mkdir -p bin/base/
	$(CXX) $(FLAGS) $(SOURCES) src/base/json_test.cpp -o bin/base/json_test
	$(CXX) $(FLAGS) $(SOURCES) src/base/spanner_test.cpp -o bin/base/spanner_test
	$(CXX) $(FLAGS) $(SOURCES) src/base/string_test.cpp -o bin/base/string_test
	$(CXX) $(FLAGS) $(SOURCES) src/main.cpp -o bin/main

test:
	./bin/base/json_test
	./bin/base/spanner_test
	./bin/base/string_test
