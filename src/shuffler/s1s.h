#pragma once

#include "shuffler/s1.h"

namespace xtreaming {

class S1S : public S1 {
  public:
    virtual bool Init(const json& obj, string *err) override;

    static S1S* New(const json& obj, string* err);

    static void Shuffle(const vector<int64_t>& shard_sizes, int64_t num_nodes, uint32_t seed,
                        int64_t epoch, vector<int64_t>* sample_ids, Logger* logger);

    void Shuffle(const vector<int64_t>& shard_sizes, int64_t num_nodes, int64_t epoch,
                 vector<int64_t>* sample_ids, Logger* logger) override;
};

}  // namespace xtreaming
