#include "m2.h"

#include <algorithm>
#include <random>

using std::default_random_engine;
using std::random_device;
using std::uniform_int_distribution;
using std::uniform_real_distribution;

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

void AdjustCounts(int64_t have, int64_t want, int64_t size, default_random_engine* rng,
                  int64_t* counts) {
    if (have == want) {
        return;
    }
    int64_t step = (have < want) ? 1 : -1L;
    int64_t num_steps = step * (have - want);
    assert(num_steps < size);
    uniform_int_distribution<int64_t> choose(0, size - 1);
    vector<bool>chosen;
    chosen.resize(size);
    for (int64_t i = 0; i < num_steps; ++i) {
        int64_t idx;
        do {
            idx = choose(*rng);
        } while (chosen[idx] || (step == -1 && !counts[idx]));
        chosen[idx] = true;
        counts[idx] += step;
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

    // Get random number generator from seed.
    random_device random;
    default_random_engine rng(random());
    rng.seed(seed);

    // Initialize shard chooses as the number of underlying samples.
    vector<int64_t> shard_choose;
    shard_choose.resize(shards.size());
    for (int64_t i = 0; i < shards.size(); ++i) {
        auto& shard = shards[i];
        shard_choose[i] = shard->num_samples();
    }

    // Calculate exact choose per shard this epoch.
    uniform_real_distribution<double> random_frac(0, 1);
    int64_t epoch_size = 0;
    for (int64_t i = 0; i < streams.size(); ++i) {
        // If stream choose matches stream num samples, we're done.
        auto& stream = streams[i];
        epoch_size += stream.choose();
        if (stream.choose() == stream.num_samples()) {
            continue;
        }

        // Scale shard choose according to the ratio of stream choose to stream sample count.
        double scale = (double)stream.choose() / (double)stream.num_samples();
        auto stream_begin = stream.shard_offset();
        auto stream_end = stream_begin + stream.num_shards();
        int64_t got_stream_choose = 0;
        for (int64_t j = stream_begin; j < stream_end; ++j) {
            auto& int_choose = shard_choose[j];
            double exact_choose = (double)int_choose * scale;
            double frac = exact_choose - (double)(int64_t)exact_choose;
            bool one_more = frac < random_frac(rng);
            int_choose = (int64_t)exact_choose + one_more;
            got_stream_choose += int_choose;
        }

        // If our observed chooses don't add up to the target, adjust them up or down.
        if (got_stream_choose != stream.choose()) {
            AdjustCounts(got_stream_choose, stream.choose(), stream.num_shards(), &rng,
                         &shard_choose[stream_begin]);
        }
    }

    // Use that to calculate (a) subshard sizes and (b) fake to real sample ID mapping.
    fake_to_real->reserve(epoch_size);
    for (int64_t i = 0; i < shards.size(); ++i) {
        auto& shard = shards[i];

        // Handle any full repeats.
        int64_t num_full_repeats = shard_choose[i] / shard->num_samples();
        for (int64_t j = 0; j < num_full_repeats; ++j) {
            subshard_sizes->emplace_back(shard->num_samples());
            for (int64_t k = 0; k < shard->num_samples(); ++k) {
                fake_to_real->emplace_back(shard->sample_offset() + k);
            }
        }

        // Handle any partial repeat.
        int64_t partial_repeat = shard_choose[i] % shard->num_samples();
        if (partial_repeat) {
            subshard_sizes->emplace_back(partial_repeat);
            SubsampleExtending(shard->sample_offset(), shard->num_samples(), partial_repeat, &rng,
                               fake_to_real);
        }
    }
}

}  // namespace xtreaming
