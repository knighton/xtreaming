#include "sampler.h"

#include "base/string.h"

namespace xtreaming {

Sampler::~Sampler() {
}

bool Sampler::Init(const json& obj, string* err) {
    if (!GetUInt32(obj, "seed", 1337, &seed_, err)) {
        return false;
    }

    if (!GetCount(obj, "epoch_size", -1L, &epoch_size_, err)) {
        return false;
    }

    string txt;
    if (!GetString(obj, "strategy", "balanced", &txt, err)) {
        if (txt == "balanced") {
            fixed_ = false;
        } else if (txt == "fixed") {
            fixed_ = true;
        } else {
            *err = StringPrintf("Unknown sampling strategy: `%s` (must be `balanced` or `fixed`).",
                                txt.c_str());
            return false;
        }
    }

    return true;
}

}  // namespace xtreaming
