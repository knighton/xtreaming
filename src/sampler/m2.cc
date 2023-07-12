#include "m2.h"

#include <algorithm>
#include <random>

using std::default_random_engine;
using std::random_device;
using std::uniform_int_distribution;

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

void TopOffCounts(int64_t target, default_random_engine* rng, vector<int64_t>* counts) {
    assert(!counts->empty());

    int64_t sum = 0;
    for (auto& value : *counts) {
        sum += value;
    }

    auto shortfall = target - sum;
    if (!shortfall) {
        return;
    }

    assert(shortfall < counts->size());
    uniform_int_distribution<int64_t> choose(0, counts->size() - 1);
    vector<bool> chosen;
    chosen.resize(counts->size());
    for (int64_t i = 0; i < shortfall; ++i) {
        int64_t idx;
        do {
            idx = choose(*rng);
        } while (chosen[idx]);
        chosen[idx] = true;
        ++(*counts)[idx];
    }
}

void SubsampleExtending(int64_t begin, int64_t count, int64_t choose, default_random_engine* rng,
                        vector<int64_t>* values) {
    vector<int64_t> indices;
    indices.resize(count);
    for (int64_t i = 0; i < count; ++i) {
        indices[i] = begin + i;
    }
    shuffle(indices.begin(), indices.end(), *rng);
    for (int64_t i = 0; i < choose; ++i) {
        values->emplace_back(i);
    }
}

}  // namespace

void M2::Sample(const vector<Stream>& streams, const vector<Shard*>& shards, int64_t epoch,
                vector<int64_t>* subshard_sizes, vector<int64_t>* fake_to_real) {
    subshard_sizes->clear();
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
            TopOffCounts(stream.choose(), &rng, &stream_shard_chooses);
        }

        // Iterate over each shard of this stream.
        for (int64_t j = 0; j < stream.num_shards(); ++j) {
            auto shard_id = stream.shard_offset() + j;
            auto& shard = shards[shard_id];
            auto& shard_choose = stream_shard_chooses[j];

            // Calculate shuffle units.
            auto num_full_repeats = shard_choose / shard->num_samples();
            for (int64_t k = 0; k < num_full_repeats; ++k) {
                subshard_sizes->emplace_back(shard->num_samples());
            }
            auto remainder = shard_choose % shard->num_samples();
            if (remainder) {
                subshard_sizes->emplace_back(remainder);
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
                SubsampleExtending(shard->sample_offset(), shard->num_samples(), target, &rng,
                                   fake_to_real);
            }
        }
    }
}

}  // namespace xtreaming
