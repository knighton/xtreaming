#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "base/json.h"
#include "serial/base/shard.h"
#include "stream.h"

using std::string;
using std::vector;

namespace xtreaming {

class Sampler {
  public:
    virtual ~Sampler();

    const string& algo() const { return algo_; }
    uint32_t seed() const { return seed_; }
    int64_t epoch_size() const { return epoch_size_; }
    int64_t* mutable_epoch_size() { return &epoch_size_; }
    bool fixed() const { return fixed_; }

    virtual bool Init(const json& obj, string* err);

    virtual void Sample(const vector<Stream>& streams, const vector<Shard*>& shards, int64_t epoch,
                        vector<int64_t>* subshard_sizes, vector<int64_t>* fake_to_real,
                        Logger* logger) = 0;

  protected:
    string algo_;
    uint32_t seed_;
    int64_t epoch_size_;
    bool fixed_;
};

}  //  namespace xtreaming
