#include "s1n.h"

#include <algorithm>

#include "base/json.h"

using std::min;

namespace xtreaming {

bool S1N::Init(const json& obj, string* err) {
    return S1::Init(obj, err);
}

S1N* S1N::New(const json& obj, string* err) {
    auto ret = new S1N;
    if (!ret->Init(obj, err)) {
        delete ret;
        return nullptr;
    }

    return ret;
}

void S1N::Shuffle(const vector<int64_t>& shard_sizes, int64_t num_nodes, uint32_t seed,
                  int64_t epoch, vector<int64_t>* sample_ids) {
    // Shuffle the shards.
    int64_t num_samples;
    vector<pair<int64_t, int64_t>> spans;
    vector<pair<int64_t, int64_t>> meta_spans;
    default_random_engine epoch_rng;
    ShuffleShards(shard_sizes, num_nodes, seed, epoch, &num_samples, &spans, &meta_spans,
                  &epoch_rng);

    // Populate the global sample ID mapping, shuffling within each node.
    sample_ids->clear();
    sample_ids->resize(num_samples);
    int64_t end = 0;
    for (auto& meta_span : meta_spans) {
        // Populate sample IDs.
        int64_t node_begin = end;
        for (int64_t i = meta_span.first; i < meta_span.second; ++i) {
            auto& span = spans[i];
            auto span_size = span.second - span.first;
            for (int64_t j = 0; j < span_size; ++j) {
                (*sample_ids)[end + j] = span.first + j;
            }
            end += span_size;
        }

        // Shuffle within each node.
        shuffle(&(*sample_ids)[node_begin], &(*sample_ids)[end], epoch_rng);
    }
}

void S1N::Shuffle(const vector<int64_t>& shard_sizes, int64_t num_nodes, int64_t epoch,
                  vector<int64_t>* sample_ids) {
    Shuffle(shard_sizes, num_nodes, seed_, epoch, sample_ids);
}

}  // namespace xtreaming
