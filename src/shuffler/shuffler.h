#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "base/json.h"
#include "base/logger.h"

using std::string;
using std::vector;

namespace xtreaming {

class Shuffler {
  public:
    virtual ~Shuffler();

    const string& algo() const { return algo_; }
    uint32_t seed() const { return seed_; }

    virtual bool Init(const json& obj, string *err);

    virtual void Shuffle(const vector<int64_t>& shard_sizes, int64_t num_nodes, int64_t epoch,
                         vector<int64_t>* ids, Logger* logger) = 0;

  protected:
    string algo_;
    uint32_t seed_;
};

}  //  namespace xtreaming
