#include "base/json.h"

#include <cstring>
#include <cstdio>

using namespace xtreaming;

int main() {
    json obj;
    string buf;
    char* ret;
    string err;

    obj["key"] = "val";

    assert(GetString(obj, "key", "default", &buf, &ret, &err));
    assert(!strcmp(buf.data(), "val"));
    assert(!strcmp(ret, "val"));
    assert(err.empty());

    assert(GetString(obj, "key", "default", &buf, &ret, &err));
    assert(buf.size() == 8);
    assert(!strcmp(buf.data(), "val\0val"));
    assert(!strcmp(ret, "val"));
    assert(err.empty());

    assert(GetString(obj, "dne", "default", &buf, &ret, &err));
    assert(buf.size() == 16);
    assert(!strcmp(buf.data(), "val\0val\0default"));
    assert(!strcmp(ret, "default"));
    assert(err.empty());
}
