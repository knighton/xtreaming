#pragma once

#include "sampler/sampler.h"

namespace xtreaming {

class M2 : public Sampler {
  public:
    virtual bool Init(const json& obj, string* err) override;

    static M2* New(const json& obj, string* err);

    virtual void Sample(const vector<Stream>& streams, const vector<Shard*>& shards, int64_t epoch,
                        vector<int64_t>* subshard_sizes, vector<int64_t>* fake_to_real) override;
};

}  //  namespace xtreaming
