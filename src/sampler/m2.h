#pragma once

#include "sampler/sampler.h"

namespace xtreaming {

class M2 : public Sampler {
  public:
    virtual bool Init(const json& obj, string* err) override;

    virtual void Sample(const vector<Stream>& streams, const vector<Shard*>& shards, int64_t epoch,
                        vector<int64_t>* shuffle_units, vector<int64_t>* fake_to_real) override;
};

}  //  namespace xtreaming
