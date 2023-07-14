#pragma once

#include <string>
#include <vector>

#include "base/json.h"
#include "base/logger.h"
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

    bool Bench();

  private:
    bool InitLogger(const json& obj, string* err);
    bool InitShardIndexArgs(const json& obj, int64_t* bucket_size, string* err);
    bool InitSampler(const json& obj, string* err);
    bool InitDeterminer(const json& obj, string* err);
    bool InitShuffler(const json& obj, string* err);
    bool InitStreams(const json& obj, string* err);
    bool InitShards(string* err);
    bool InitShardIndex(int64_t bucket_size, string* err);
    bool InitCaches(string* err);

    void SampleThread(int64_t epoch, vector<int64_t>* subshard_sizes,
                      vector<int64_t>* fake_to_real, int64_t* t0, int64_t* t1);

    Logger logger_;
    vector<Stream> streams_;
    vector<Shard*> shards_;
    Spanner shard_index_;
    Sampler* sampler_;
    Determiner* determiner_;
    bool shuffle_;
    Shuffler* shuffler_;
};

}  // namespace xtreaming
