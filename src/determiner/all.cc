#include "all.h"

#include "base/string.h"
#include "determiner/fast.h"

namespace xtreaming {

Determiner* GetDeterminer(const json& obj, string* err) {
    string algo;
    if (!GetString(obj, "algo", &algo, err)) {
        return nullptr;
    }

    if (algo == "fast") {
        return Fast::New(obj, err);
    } else {
        *err = StringPrintf("Unknown determinism algorithm: `%s`.", algo.c_str());
        return nullptr;
    }
}

}  // namespace xtreaming
