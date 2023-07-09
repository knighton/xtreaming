#include "s1s.h"

namespace xtreaming {

bool S1S::Init(const json& obj, string* err) {
    return S1::Init(obj, err);
}

S1S* S1S::New(const json& obj, string* err) {
    auto ret = new S1S;
    if (!ret->Init(obj, err)) {
        delete ret;
        return nullptr;
    }

    return ret;
}

void S1S::Shuffle(const vector<int64_t>& shard_sizes, int64_t num_nodes, uint32_t seed,
                  int64_t epoch, vector<int64_t>* sample_ids) {
    // Shuffle the shards.
    int64_t num_samples;
    vector<pair<int64_t, int64_t>> spans;
    vector<pair<int64_t, int64_t>> meta_spans;
    default_random_engine epoch_rng;
    ShuffleShards(shard_sizes, num_nodes, seed, epoch, &num_samples, &spans, &meta_spans,
                  &epoch_rng);

    // Populate the global sample ID mapping, shuffling within each shard.
    sample_ids->clear();
    sample_ids->resize(num_samples);
    int64_t offset = 0;
    for (auto& span : spans) {
        // Populate sample IDs.
        auto span_size = span.second - span.first;
        for (int64_t i = 0; i < span_size; ++i) {
            (*sample_ids)[offset + i] = span.first + i;
        }

        // Shuffle within each span.
        shuffle(&(*sample_ids)[offset], &(*sample_ids)[offset + span_size], epoch_rng);
        offset += span_size;
    }
}

void S1S::Shuffle(const vector<int64_t>& shard_sizes, int64_t num_nodes, int64_t epoch,
                  vector<int64_t>* sample_ids) {
    Shuffle(shard_sizes, num_nodes, seed_, epoch, sample_ids);
}

}  // namespace xtreaming
