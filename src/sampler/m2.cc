#include "m2.h"

#include <algorithm>
#include <random>

using std::default_random_engine;
using std::random_device;
using std::shuffle;

namespace xtreaming {

bool M2::Init(const json& obj, string* err) {
    algo_ = "m2";

    return Sampler::Init(obj, err);
}

M2* M2::New(const json& obj, string* err) {
    auto ret = new M2;
    if (!ret->Init(obj, err)) {
        delete ret;
        return nullptr;
    }

    return ret;
}

namespace {

void FixShortfall(int64_t target, default_random_engine* rng, vector<int64_t>* values) {
    int64_t sum = 0;
    for (auto& value : *values) {
        sum += value;
    }

    auto shortfall = target - sum;
    if (!shortfall) {
        return;
    }

    vector<int64_t> indices;
    indices.reserve(values->size());
    for (int64_t i = 0; i < values->size(); ++i) {
        indices.emplace_back(i);
    }
    shuffle(indices.begin(), indices.end(), *rng);
    for (int64_t i = 0; i < shortfall; ++i) {
        auto& index = indices[i];
        ++(*values)[index];
    }
}

}  // namespace

void M2::Sample(const vector<Stream>& streams, const vector<Shard*>& shards, int64_t epoch,
                vector<int64_t>* shuffle_units, vector<int64_t>* fake_to_real) {
    shuffle_units->clear();
    fake_to_real->clear();

    // Get seed (different each epoch if balanced, same each epoch if fixed sampling). 
    uint32_t seed = seed_;
    if (!fixed_) {
        seed += (uint32_t)epoch;
    }

    // Init PRNG.
    random_device random;
    default_random_engine rng(random());
    rng.seed(seed);

    // Iterate over each stream.
    for (int64_t i = 0; i < streams.size(); ++i) {
        auto& stream_id = i;
        auto& stream = streams[stream_id];

        // Gather samples per stream shard.
        vector<int64_t> stream_shard_num_samples;
        stream_shard_num_samples.reserve(stream.num_shards());
        for (int64_t j = 0; j < stream.num_shards(); ++j) {
            auto shard_id = stream.shard_offset() + j;
            auto& shard = shards[shard_id];
            stream_shard_num_samples.emplace_back(shard->num_samples());
        }

        // Calculate choose per stream shard.
        vector<int64_t> stream_shard_chooses = stream_shard_num_samples;
        if (stream.choose() != stream.num_samples()) {
            for (auto& choose : stream_shard_chooses) {
                choose *= stream.choose();
                choose /= stream.num_samples();
            }
            FixShortfall(stream.choose(), &rng, &stream_shard_chooses);
        }

        // Iterate over each shard of this stream.
        for (int64_t j = 0; j < stream.num_shards(); ++j) {
            auto shard_id = stream.shard_offset() + j;
            auto& shard = shards[shard_id];
            auto& shard_choose = stream_shard_chooses[j];

            // Calculate shuffle units.
            auto num_full_repeats = shard_choose / shard->num_samples();
            for (int64_t k = 0; k < num_full_repeats; ++k) {
                shuffle_units->emplace_back(shard->num_samples());
            }
            auto remainder = shard_choose % shard->num_samples();
            if (remainder) {
                shuffle_units->emplace_back(remainder);
            }

            // Calculate sample IDs of any full repeats.
            for (int64_t k = 0; k < num_full_repeats; ++k) {
                for (int64_t m = 0; m < shard->num_samples(); ++m) {
                    fake_to_real->emplace_back(shard->sample_offset() + m);
                }
            }

            // Calculate sample IDs of a possible partial repeat.
            auto target = shard_choose % shard->num_samples();
            if (target) {
                vector<int64_t> counts;
                counts.resize(shard->num_samples());
                FixShortfall(target, &rng, &counts);
                for (int64_t k = 0; k < shard->num_samples(); ++k) {
                    if (counts[k]) {
                        fake_to_real->emplace_back(shard->sample_offset() + k);
                    }
                }
            }
        }
    }
}

}  // namespace xtreaming
