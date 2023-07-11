#include "determiner.h"

#include "base/string.h"

namespace xtreaming {

Determiner::~Determiner() {
}

bool Determiner::Init(const json& obj, string* err) {
    if (!GetInt64(obj, "canonical_nodes", 1, &num_canonical_nodes_, err)) {
        return false;
    }
    if (num_canonical_nodes_ < 1) {
        *err = StringPrintf("`canonical_nodes` must be a positive integer, but got: %ld.",
                            num_canonical_nodes_);
        return false;
    }

    if (!GetInt64(obj, "batch_size", 0, &batch_size_, err)) {
        return false;
    }
    if (batch_size_ < 1) {
        *err = "`batch_size` must be a positive integer.";
        return false;
    }

    return true;
}

}  // namespace xtreaming
