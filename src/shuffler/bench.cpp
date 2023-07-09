#include <cmath>
#include <cstdint>
#include <random>
#include <string>
#include <vector>

#include "base/json.h"
#include "shuffler/all.h"
#include "shuffler/shuffler.h"

using std::default_random_engine;
using std::random_device;
using std::string;
using std::uniform_int_distribution;
using std::vector;
using namespace xtreaming;

namespace {

void GetShardSizes(int64_t dataset_size, int64_t min_shard_size, int64_t max_shard_size,
                   vector<int64_t>* shard_sizes) {
    random_device random;
    default_random_engine engine(random());
    uniform_int_distribution<int64_t> get_shard_size(min_shard_size, max_shard_size);
    while (dataset_size) {
        auto shard_size = get_shard_size(engine);
        if (dataset_size < shard_size) {
            shard_size = dataset_size;
        }
        shard_sizes->emplace_back(shard_size);
        dataset_size -= shard_size;
    }
}

}  // namespace

int main() {
    int64_t num_nodes = 1;
    int64_t epoch = 0;
    int64_t min_shard_size = 5000;
    int64_t max_shard_size = 10000;

    int64_t pow_min = 10;
    int64_t pow_max = 20;
    int64_t pow_step = 4;
    double timeout = 10.0;

    vector<json> objs = {
        {
            {"algo", "naive"},
            {"seed", 1337},
        },
        {
            {"algo", "s1b"},
            {"seed", 1337},
            {"block_size", 1 << 20},
        },
        {
            {"algo", "s1br"},
            {"seed", 1337},
            {"min_block_size", 1 << 19},
            {"max_block_size", 1 << 20},
        },
        {
            {"algo", "s1n"},
            {"seed", 1337},
        },
        {
            {"algo", "s1s"},
            {"seed", 1337},
        },
    };

    vector<Shuffler*> shufflers;
    shufflers.reserve(objs.size());
    string err;
    for (auto& obj : objs) {
        auto shuffler = GetShuffler(obj, &err);
        if (!shuffler) {
            fprintf(stderr, "%s\n", err.c_str());
            return 1;
        }
        shufflers.emplace_back(shuffler);
    }

    for (auto& shuffler : shufflers) {
        for (int64_t i = pow_min * pow_step; i <= pow_max * pow_step; ++i) {
            double exponent = (double)i / (double)pow_step;
            int64_t dataset_size = (int64_t)pow(2, exponent);
            vector<int64_t> shard_sizes;
            GetShardSizes(dataset_size, min_shard_size, max_shard_size, &shard_sizes);
            vector<int64_t> sample_ids;
            shuffler->Shuffle(shard_sizes, num_nodes, epoch, &sample_ids);
        }
    }
}
