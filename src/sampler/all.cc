#include "all.h"

#include "base/string.h"
#include "sampler/m2.h"

namespace xtreaming {

Sampler* GetSampler(const json& obj, string* err) {
    string algo;
    if (!GetString(obj, "algo", &algo, err)) {
        return nullptr;
    }

    if (algo == "m2") {
        return M2::New(obj, err);
    } else {
        *err = StringPrintf("Unknown sampling algorithm: `%s`.", algo.c_str());
        return nullptr;
    }
}

}  // namespace xtreaming
