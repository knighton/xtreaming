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
	$(CXX) $(FLAGS) $(SOURCES) src/base/string_test.cpp -o bin/base/string_test

test: all
	./bin/base/string_test
