#pragma once

#include <random>
#include <utility>

#include "shuffler/shuffler.h"

using std::default_random_engine;
using std::pair;

namespace xtreaming {

class S1 : public Shuffler {
  public:
    virtual bool Init(const json& obj, string *err) override;

    static void BreakSpansOverNodes(int64_t num_samples, int64_t num_nodes,
                                    vector<pair<int64_t, int64_t>>* spans,
                                    vector<pair<int64_t, int64_t>>* meta_spans);

    static void ShuffleShards(const vector<int64_t>& shard_sizes, int64_t num_nodes, uint32_t seed,
                              int64_t epoch, int64_t* num_samples,
                              vector<pair<int64_t, int64_t>>* spans,
                              vector<pair<int64_t, int64_t>>* meta_spans,
                              default_random_engine* epoch_rng);

    virtual void Shuffle(const vector<int64_t>& shard_sizes, int64_t num_nodes, int64_t epoch,
                         vector<int64_t>* sample_ids) override = 0;
};

}  // namespace xtreaming
