#pragma once

#include <cstdint>
#include <string>

#include "base/json.h"

using std::string;

namespace xtreaming {

class World {
  public:
    int64_t node() const { return worker_ / ranks_per_node_ / workers_per_rank_; }
    int64_t num_nodes() const { return num_nodes_; }
    bool is_multinode() const { return 1 < num_nodes_; }

    int64_t rank() const { return worker_ / workers_per_rank_; }
    int64_t num_ranks() const { return num_nodes_ * ranks_per_node_; }
    int64_t rank_of_node() const { return (worker_ / workers_per_rank_) % ranks_per_node_; }
    int64_t ranks_per_node() const { return ranks_per_node_; }

    int64_t worker() const { return worker_; }
    int64_t num_workers() const { return num_nodes_ * ranks_per_node_ * workers_per_rank_; }
    int64_t worker_of_node() const { return worker_ % (ranks_per_node_ * workers_per_rank_); }
    int64_t workers_per_node() const { return ranks_per_node_ * workers_per_rank_; }
    int64_t worker_of_rank() const { return worker_ % workers_per_rank_; }
    int64_t workers_per_rank() const { return workers_per_rank_; }
    bool is_leader() const { return !worker_; }
    bool is_local_leader() const { return !(worker_ % (ranks_per_node_ * workers_per_rank_)); }

    bool Init(const json& obj, string* err);

  private:
    int64_t worker_;
    int64_t num_nodes_;
    int64_t ranks_per_node_;
    int64_t workers_per_rank_;
};

}  // namespace xtreaming
