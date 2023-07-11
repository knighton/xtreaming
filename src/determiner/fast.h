#pragma once

#include "determiner/determiner.h"

namespace xtreaming {

class Fast : public Determiner {
  public:
    bool Init(const json& obj, string* err) override;

    static Fast* New(const json& obj, string* err);

    static bool Determine(int64_t num_canonical_nodes, int64_t batch_size,
                          int64_t num_physical_nodes, int64_t ranks_per_node,
                          int64_t workers_per_rank, int64_t epoch_size, int64_t sample_offset,
                          Tensor* ids, string* err);

    bool Determine(int64_t num_physical_nodes, int64_t ranks_per_node, int64_t workers_per_rank,
                   int64_t epoch_size, int64_t sample_offset, Tensor* ids, string* err) override;
};

}  // namespace xtreaming
