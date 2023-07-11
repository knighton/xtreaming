#include <cassert>
#include <cstdint>
#include <string>

#include "world.h"

using std::string;
using namespace xtreaming;

int main() {
    int64_t num_nodes = 3;
    int64_t ranks_per_node = 5;
    int64_t workers_per_rank = 7;
    World world;
    json obj;
    string err;
    for (int64_t worker = 0; worker < num_nodes * ranks_per_node * workers_per_rank; ++worker) {
        obj = {
            {"num_nodes", num_nodes},
            {"ranks_per_node", ranks_per_node},
            {"workers_per_rank", workers_per_rank},
            {"worker", worker},
        };
        assert(world.Init(obj, &err));

        assert(world.node() == worker / (ranks_per_node * workers_per_rank));
        assert(world.num_nodes() == num_nodes);
        assert(world.is_multinode() == (1 < num_nodes));

        assert(world.rank() == worker / workers_per_rank);
        assert(world.num_ranks() == num_nodes * ranks_per_node);
        assert(world.rank_of_node() == (worker / workers_per_rank) % ranks_per_node);
        assert(world.ranks_per_node() == ranks_per_node);

        assert(world.worker() == worker);
        assert(world.num_workers() == num_nodes * ranks_per_node * workers_per_rank);
        assert(world.worker_of_node() == worker %  (ranks_per_node * workers_per_rank));
        assert(world.workers_per_node() == ranks_per_node * workers_per_rank);
        assert(world.worker_of_rank() == worker % workers_per_rank);
        assert(world.workers_per_rank() == workers_per_rank);
        assert(world.is_leader() == !worker);
        assert(world.is_local_leader() == !(worker % (ranks_per_node * workers_per_rank)));
    }
}
