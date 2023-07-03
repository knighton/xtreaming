#include "spanner.h"

#include <cassert>

namespace xtreaming {

void Spanner::Init(const vector<int64_t>& span_sizes, int64_t slot_size) {
    span_ends_.clear();
    span_ends_.reserve(span_sizes.size());
    int64_t offset = 0;
    for (auto& size : span_sizes) {
        offset += size;
        span_ends_.emplace_back(offset);
    }
    slot_size_ = slot_size;
    num_items_ = span_ends_[span_ends_.size() - 1];
    int64_t num_slots = (num_items_ + slot_size_ - 1) / slot_size;
    slot_ends_.resize(num_slots);
    int64_t span_id = 0;
    for (int64_t slot_id = 0; slot_id < num_slots; ++slot_id) {
        int64_t item_id = (slot_id + 1) * slot_size_;
        while (span_id < span_ends_.size() && span_ends_[span_id] < item_id) {
            ++span_id;
        }
        slot_ends_[slot_id] = span_id;
    }
}

void Spanner::Find(int64_t item_id, int64_t* span_id, int64_t* span_item_id) const {
    assert(0 <= item_id);
    assert(item_id < num_items_);
    int64_t slot_id = item_id / slot_size_;
    int64_t begin_span_id = slot_id ? slot_ends_[slot_id - 1] : 0;
    int64_t end_span_id = slot_ends_[slot_id];
    for (*span_id = begin_span_id; *span_id <= end_span_id; ++(*span_id)) {
        if (item_id < span_ends_[*span_id]) {
            *span_item_id = item_id - (*span_id ? span_ends_[*span_id - 1] : 0);
            return;
        }
    }
    assert(false);
}

}  // namespace xtreaming
