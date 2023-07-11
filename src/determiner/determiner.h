#pragma once

#include <cstdint>
#include <string>

#include "base/json.h"
#include "base/xtensor.h"

using std::string;

namespace xtreaming {

class Determiner {
  public:
    virtual ~Determiner();

    const string& algo() const { return algo_; }
    int64_t num_canonical_nodes() const { return num_canonical_nodes_; }
    int64_t batch_size() const { return batch_size_; }

    virtual bool Init(const json& obj, string* err);

    virtual bool Determine(int64_t num_physical_nodes, int64_t ranks_per_node,
                           int64_t workers_per_rank, int64_t epoch_size, int64_t sample_offset,
                           xt::xarray<int64_t>* ids, string* err) = 0;

  protected:
    string algo_;
    int64_t num_canonical_nodes_;
    int64_t batch_size_;
};

}  //  namespace xtreaming
