#include "fast.h"

#include "base/string.h"

namespace xtreaming {

bool Fast::Init(const json& obj, string* err) {
    algo_ = "fast";

    return Determiner::Init(obj, err);
}

Fast* Fast::New(const json& obj, string* err) {
    auto ret = new Fast;
    if (!ret->Init(obj, err)) {
        delete ret;
        return nullptr;
    }

    return ret;
}

namespace {

bool AreCodivisible(int64_t can, int64_t phy) {
    if (can < phy) {
        return !(phy % can);
    } else if (phy < can) {
        return !(can % phy);
    } else {
        return true;
    }
}

}  // namespace

bool Fast::Determine(int64_t num_canonical_nodes, int64_t batch_size, int64_t num_physical_nodes,
                     int64_t ranks_per_node, int64_t workers_per_rank, int64_t epoch_size,
                     int64_t sample_offset, Tensor* ids, string* err) {
    if (num_canonical_nodes <= 0) {
        *err = StringPrintf("`num_canonical_nodes` must be positive, but got: %ld.",
                            num_canonical_nodes);
        return false;
    }

    if (batch_size <= 0) {
        *err = StringPrintf("`batch_size` must be positive, but got: %ld.", batch_size);
        return false;
    }

    if (num_physical_nodes <= 0) {
        *err = StringPrintf("`num_physical_nodes` must be positive, but got: %ld.",
                            num_physical_nodes);
        return false;
    }

    if (ranks_per_node <= 0) {
        *err = StringPrintf("`ranks_per_node` must be positive, but got: %ld.", ranks_per_node);
        return false;
    }

    if (workers_per_rank <= 0) {
        *err = StringPrintf("`workers_per_rank` must be positive, but got: %ld.",
                            workers_per_rank);
        return false;
    }

    if (epoch_size <= 0) {
        *err = StringPrintf("`epoch_size` must be positive, but got: %ld.", epoch_size);
        return false;
    }

    if (sample_offset < 0) {
        *err = StringPrintf("`sample_offset` must be non-negative, but got: %ld.", sample_offset);
        return false;
    }

    if (!AreCodivisible(num_canonical_nodes, num_physical_nodes)) {
        *err = StringPrintf("Canonical and physical nodes must be an even ratio of each other, "
                            "otherwise striping slices of shards over nodes may cause all nodes "
                            "to download all shards. Got `num_canonical_nodes` = %ld, "
                            "`num_physical_nodes` = %ld.", num_canonical_nodes,
                            num_physical_nodes);
        return false;
    }

    if (epoch_size <= sample_offset) {
        *err = StringPrintf("Attempted to resume further into the epoch than it has samples: "
                            "`epoch_size` = %ld, `sample_offset` = %ld.", epoch_size,
                            sample_offset);
        return false;
    }

    if (sample_offset % num_physical_nodes) {
        sample_offset -= sample_offset % num_physical_nodes;
    }

    // TODO

    return true;
}

bool Fast::Determine(int64_t num_physical_nodes, int64_t ranks_per_node, int64_t workers_per_rank,
                     int64_t epoch_size, int64_t sample_offset, Tensor* ids, string* err) {
    return Determine(num_canonical_nodes_, batch_size_, num_physical_nodes, ranks_per_node,
                     workers_per_rank, epoch_size, sample_offset, ids, err);
}

}  // namespace xtreaming
