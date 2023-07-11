#include "world.h"

#include "base/json.h"
#include "base/string.h"

namespace xtreaming {

bool World::Init(const json& obj, string* err) {
    if (!GetInt64(obj, "num_nodes", &num_nodes_, err)) {
        return false;
    }
    if (num_nodes_ <= 0) {
        *err = StringPrintf("`num_nodes` must be a positive integer, but got: %ld.", num_nodes_);
        return false;
    }

    if (!GetInt64(obj, "ranks_per_node", &ranks_per_node_, err)) {
        return false;
    }
    if (ranks_per_node_ <= 0) {
        *err = StringPrintf("`ranks_per_node` must be a positive integer, but got: %ld.",
                            ranks_per_node_);
        return false;
    }

    if (!GetInt64(obj, "workers_per_rank", &workers_per_rank_, err)) {
        return false;
    }
    if (workers_per_rank_ <= 0) {
        *err = StringPrintf("`workers_per_rank` must be a positive integer, but got: %ld.",
                            workers_per_rank_);
        return false;
    }

    if (!GetInt64(obj, "worker", &worker_, err)) {
        return false;
    }
    int64_t num_workers = num_nodes_ * ranks_per_node_ * workers_per_rank_;
    if (worker_ < 0 || num_workers <= worker_) {
        *err = StringPrintf("`worker` must be from 0 to %ld inclusive, but got: %ld. `worker` is "
                            "the global worker ID over all nodes and ranks.", num_workers - 1,
                            worker_);
        return false;
    }

    return true;
}

}  // namespace xtreaming
