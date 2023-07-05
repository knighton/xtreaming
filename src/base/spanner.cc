#include "spanner.h"

#include <cassert>

namespace xtreaming {

void Spanner::Init(const vector<int64_t>& span_sizes, int64_t bucket_size) {
    assert(0 < bucket_size);
    span_ends_.clear();
    span_ends_.reserve(span_sizes.size());
    int64_t offset = 0;
    for (auto& size : span_sizes) {
        assert(0 < size);
        offset += size;
        span_ends_.emplace_back(offset);
    }
    bucket_size_ = bucket_size;
    num_items_ = span_ends_[span_ends_.size() - 1];
    int64_t num_buckets = (num_items_ + bucket_size_ - 1) / bucket_size;
    bucket_ends_.resize(num_buckets);
    int64_t span_id = 0;
    for (int64_t bucket_id = 0; bucket_id < num_buckets; ++bucket_id) {
        int64_t item_id = (bucket_id + 1) * bucket_size_;
        while (span_id < span_ends_.size() && span_ends_[span_id] < item_id) {
            ++span_id;
        }
        bucket_ends_[bucket_id] = span_id;
    }
}

void Spanner::Find(int64_t item_id, int64_t* span_id, int64_t* span_item_id) const {
    assert(0 <= item_id);
    assert(item_id < num_items_);
    int64_t bucket_id = item_id / bucket_size_;
    int64_t begin_span_id = bucket_id ? bucket_ends_[bucket_id - 1] : 0;
    int64_t end_span_id = bucket_ends_[bucket_id];
    for (*span_id = begin_span_id; *span_id <= end_span_id; ++(*span_id)) {
        if (item_id < span_ends_[*span_id]) {
            *span_item_id = item_id - (*span_id ? span_ends_[*span_id - 1] : 0);
            return;
        }
    }
    assert(false);
}

}  // namespace xtreaming
