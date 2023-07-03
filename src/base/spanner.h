#pragma once

#include <cstdint>
#include <vector>

using std::vector;

namespace xtreaming {

// Indexes a list of variable-length spans, mapping from individual items to which spans they
// belong to and their offsets within those spans.
//
// Equivalent to just having a big array of span ID per item, but much more space-efficient.
class Spanner {
  public:
    // Construct the index given the item offset of each span and the slot size.
    void Init(const vector<int64_t>& span_sizes, int64_t slot_size);

    // Look up an item in the index, returning its span and relative offset within the span.
    void Find(int64_t item_id, int64_t* span_id, int64_t* span_item_id) const;

  private:
    vector<int64_t> span_ends_;  // Cumulative sum of items per span.
    int64_t slot_size_;          // Fixed slot size that spans are broken into.
    vector<int64_t> slot_ends_;  // Span ID of the first sample past each slot.
    int64_t num_items_;          // Total number of items.
};

}  // namespace xtreaming
