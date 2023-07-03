#include <cassert>

#include "base/string.h"

using namespace xtreaming;

namespace {

void TestParseNumBytes() {
    string s;
    int64_t i2;
    string e;
    for (int64_t i = 0; i < 1000; ++i) {
        s = StringPrintf("%ld", i);
        assert(ParseNumBytes(s, &i2, &e));
        assert(i == i2);
        assert(e.empty());

        s += "b";
        assert(ParseNumBytes(s, &i2, &e));
        assert(i == i2);
        assert(e.empty());

        s += "b";
        s[s.size() - 2] = 'k';
        assert(ParseNumBytes(s, &i2, &e));
        assert(i << 10 == i2);
        assert(e.empty());

        s[s.size() - 2] = 'm';
        assert(ParseNumBytes(s, &i2, &e));
        assert(i << 20 == i2);
        assert(e.empty());

        s[s.size() - 2] = 'g';
        assert(ParseNumBytes(s, &i2, &e));
        assert(i << 30 == i2);
        assert(e.empty());

        s[s.size() - 2] = 't';
        assert(ParseNumBytes(s, &i2, &e));
        assert(i << 40 == i2);
        assert(e.empty());

        s[s.size() - 2] = 'p';
        assert(ParseNumBytes(s, &i2, &e));
        assert(i << 50 == i2);
        assert(e.empty());

        s[s.size() - 2] = 'e';
        assert(ParseNumBytes(s, &i2, &e));
        assert(i << 60 == i2);
        assert(e.empty());
    }
}

void TestSplitStringByChar() {
    string s;
    char c;
    vector<string> v_want;
    vector<string> v_got;

    s = "";
    c = 'a';
    v_want = {""};
    SplitStringByChar(s, c, &v_got);
    assert(v_want == v_got);

    s = "a";
    c = ' ';
    v_want = {"a"};
    SplitStringByChar(s, c, &v_got);
    assert(v_want == v_got);

    s = "a";
    c = 'a';
    v_want = {"", ""};
    SplitStringByChar(s, c, &v_got);
    assert(v_want == v_got);

    s = "one two three";
    c = ' ';
    v_want = {"one", "two", "three"};
    SplitStringByChar(s, c, &v_got);
    assert(v_want == v_got);
}

void TestSplitStringByWhitespace() {
    string s;
    vector<string> v_want;
    vector<string> v_got;

    s = "";
    v_want = {};
    SplitStringByWhitespace(s, &v_got);
    assert(v_want == v_got);

    s = " one two three  ";
    v_want = {"one", "two", "three"};
    SplitStringByWhitespace(s, &v_got);
    assert(v_want == v_got);
}

}  // namespace

int main() {
    TestParseNumBytes();
    TestSplitStringByChar();
    TestSplitStringByWhitespace();
}
