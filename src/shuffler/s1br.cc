#include "s1br.h"

#include <algorithm>

#include "base/string.h"

using std::min;
using std::uniform_int_distribution;

namespace xtreaming {

bool S1BR::Init(const json& obj, string* err) {
    algo_ = "s1br";

    if (!S1::Init(obj, err)) {
        return false;
    }

    if (!GetCount(obj, "min_block_size", 1L << 19, &min_block_size_, err)) {
        return false;
    }

    if (!GetCount(obj, "max_block_size", 1L << 20, &max_block_size_, err)) {
        return false;
    }

    if (min_block_size_ <= 0) {
        *err = StringPrintf("`min_block_size` must be positive (got: %ld).", min_block_size_);
        return false;
    }

    if (max_block_size_ < min_block_size_) {
        *err = StringPrintf("`min_block_size` (got: %ld) must be less than or equal to "
                            "`max_block_size` (got: %ld).", min_block_size_, max_block_size_);
        return false;
    }

    return true;
}

S1BR* S1BR::New(const json& obj, string* err) {
    auto ret = new S1BR;
    if (!ret->Init(obj, err)) {
        delete ret;
        return nullptr;
    }

    return ret;
}

void S1BR::Shuffle(const vector<int64_t>& shard_sizes, int64_t num_nodes, uint32_t seed,
                   int64_t epoch, int64_t min_block_size, int64_t max_block_size,
                   vector<int64_t>* sample_ids) {
    // Shuffle the shards.
    int64_t num_samples;
    vector<pair<int64_t, int64_t>> spans;
    vector<pair<int64_t, int64_t>> meta_spans;
    default_random_engine epoch_rng;
    ShuffleShards(shard_sizes, num_nodes, seed, epoch, &num_samples, &spans, &meta_spans,
                  &epoch_rng);

    // Populate the global sample ID mapping, shuffling within each block within each node span.
    sample_ids->clear();
    sample_ids->resize(num_samples);
    int64_t end = 0;
    uniform_int_distribution<int64_t> get_block_size(min_block_size, max_block_size);
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

        // Shuffle within each block (which are variably sized within a range).
        int64_t block_begin = node_begin;
        while (block_begin < end) {
            auto block_size = get_block_size(epoch_rng);
            auto block_end = min(block_begin + block_size, end);
            shuffle(&(*sample_ids)[block_begin], &(*sample_ids)[block_end], epoch_rng);
            block_begin += block_size;
        }
    }
}

void S1BR::Shuffle(const vector<int64_t>& shard_sizes, int64_t num_nodes, int64_t epoch,
                  vector<int64_t>* sample_ids) {
    Shuffle(shard_sizes, num_nodes, seed_, epoch, min_block_size_, max_block_size_, sample_ids);
}

}  // namespace xtreaming
