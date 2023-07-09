#include "naive.h"

#include <algorithm>
#include <random>

#include "base/lint.h"

using std::default_random_engine;
using std::random_device;
using std::shuffle;

namespace xtreaming {

bool Naive::Init(const json& obj, string* err) {
    algo_ = "naive";
    return Shuffler::Init(obj, err);
}

Naive* Naive::New(const json& obj, string* err) {
    auto ret = new Naive;
    if (!ret->Init(obj, err)) {
        delete ret;
        return nullptr;
    }

    return ret;
}

void Naive::Shuffle(const vector<int64_t>& shard_sizes, int64_t num_nodes, uint32_t seed,
                    int64_t epoch, vector<int64_t>* sample_ids) {
    UNUSED(num_nodes);

    sample_ids->clear();

    int64_t total = 0;
    for (auto& shard_size : shard_sizes) {
        total += shard_size;
    }

    sample_ids->reserve(total);
    for (int64_t i = 0; i < total; ++i) {
        sample_ids->emplace_back(i);
    }

    random_device random;
    default_random_engine engine(random());
    engine.seed(seed + (uint32_t)epoch);
    shuffle(sample_ids->begin(), sample_ids->end(), engine);
}

void Naive::Shuffle(const vector<int64_t>& shard_sizes, int64_t num_nodes, int64_t epoch,
                    vector<int64_t>* sample_ids) {
    Shuffle(shard_sizes, num_nodes, seed_, epoch, sample_ids);
}

}  //  namespace xtreaming
