#pragma once

#include <string>
#include <vector>

#include "base/json.h"
#include "base/spanner.h"
#include "determiner/determiner.h"
#include "serial/base/shard.h"
#include "sampler/sampler.h"
#include "shuffler/shuffler.h"
#include "stream.h"

using std::string;
using std::vector;

namespace xtreaming {

class Dataset {
  public:
    bool Init(const json& obj, string* err);

  private:
    vector<Stream> streams_;
    vector<Shard*> shards_;
    Spanner shard_index_;
    Sampler* sampler_;
    Determiner* determiner_;
    bool shuffle_;
    Shuffler* shuffler_;
};

}  // namespace xtreaming
