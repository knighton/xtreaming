#include "all.h"

#include "base/string.h"
#include "shuffler/naive.h"
#include "shuffler/s1b.h"
#include "shuffler/s1br.h"
#include "shuffler/s1n.h"
#include "shuffler/s1s.h"

namespace xtreaming {

Shuffler* GetShuffler(const json& obj, string* err) {
    string algo;
    if (!GetString(obj, "algo", &algo, err)) {
        return nullptr;
    }

    if (algo == "naive") {
        return Naive::New(obj, err);
    } else if (algo == "s1b") {
        return S1B::New(obj, err);
    } else if (algo == "s1br") {
        return S1BR::New(obj, err);
    } else if (algo == "s1n") {
        return S1N::New(obj, err);
    } else if (algo == "s1s") {
        return S1S::New(obj, err);
    } else {
        *err = StringPrintf("Unknown shuffling algorithm: `%s`.", algo.c_str());
        return nullptr;
    }
}

}  // namespace xtreaming
