#include "base/json.h"

#include <cstring>
#include <cstdio>

using namespace xtreaming;

int main() {
    json obj = {
        {"key", "val"}
    };

    string ret;
    string err;
    assert(GetString(obj, "key", "default", &ret, &err));
    assert(ret == "val");
    assert(err.empty());

    assert(GetString(obj, "key", "default", &ret, &err));
    assert(ret == "val");
    assert(err.empty());

    assert(GetString(obj, "dne", "default", &ret, &err));
    assert(ret == "default");
    assert(err.empty());

    obj["key"] = 42;
    assert(!GetString(obj, "key", "default", &ret, &err));
    assert(err == "`key` must be a string.");
}
