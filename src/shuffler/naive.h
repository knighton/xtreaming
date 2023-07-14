#pragma once

#include "shuffler/shuffler.h"

namespace xtreaming {

class Naive : public Shuffler {
  public:
    virtual bool Init(const json& obj, string *err) override;

    static Naive* New(const json& obj, string* err);

    static void Shuffle(const vector<int64_t>& shard_sizes, int64_t num_nodes, uint32_t seed,
                        int64_t epoch, vector<int64_t>* sample_ids, Logger* logger);

    void Shuffle(const vector<int64_t>& shard_sizes, int64_t num_nodes, int64_t epoch,
                 vector<int64_t>* sample_ids, Logger* logger) override;
};

}  //  namespace xtreaming
