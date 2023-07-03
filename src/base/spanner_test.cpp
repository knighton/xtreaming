#include <cassert>

#include "spanner.h"

using namespace xtreaming;

int main() {
    Spanner spanner;
    vector<int64_t> span_sizes = {1, 2, 3, 10, 4};
    int64_t slot_size = 3;
    spanner.Init(span_sizes, slot_size);

    int64_t item_id = 0;
    for (int64_t span_id = 0; span_id < span_sizes.size(); ++span_id) {
        auto& span_size = span_sizes[span_id];
        for (int64_t span_item_id = 0; span_item_id < span_size; ++span_item_id) {
            int64_t got_span_id = -1;
            int64_t got_span_item_id = -1;
            spanner.Find(item_id, &got_span_id, &got_span_item_id);
            assert(got_span_id == span_id);
            assert(got_span_item_id == span_item_id);
            ++item_id;
        }
    }
}
