#include "dataset.h"

#include <thread>

#include "base/string.h"
#include "sampler/all.h"
#include "serial/index.h"
#include "shuffler/all.h"

using std::thread;

namespace xtreaming {

bool Dataset::Init(const json& obj, string* err) {
    json empty_obj;
    const json* section;

    // Get indexing arguments.
    int64_t shard_index_bucket_size;
    {
        if (!GetObject(obj, "shard_index", &empty_obj, &section, err)) {
            return false;
        }

        if (!GetInt64(*section, "bucket_size", 1024, &shard_index_bucket_size, err)) {
            return false;
        }

        if (shard_index_bucket_size < 1) {
            *err = StringPrintf("`index.bucket_size` must be postive, but got: %ld.",
                                shard_index_bucket_size);
            return false;
        }
    }

    // Init sampling.
    if (!GetObject(obj, "sampler", &empty_obj, &section, err)) {
        return false;
    }
    sampler_ = GetSampler(*section, err);
    if (!sampler_) {
        return false;
    }

    // Init shuffling.
    if (!GetBool(obj, "shuffle", false, &shuffle_, err)) {
        return false;
    }
    if (!GetObject(obj, "shuffler", &empty_obj, &section, err)) {
        return false;
    }
    shuffler_ = GetShuffler(*section, err);
    if (!shuffler_) {
        return false;
    }

    // Get `stream`.
    const json* all;
    if (!GetObject(obj, "stream", &empty_obj, &all, err)) {
        return false;
    }

    // Get `streams`.
    const json* streams;
    if (!GetObject(obj, "streams", &empty_obj, &streams, err)) {
        return false;
    }

    // Init streams from config.
    if (streams->empty()) {
        // `stream` is taken as the single stream, as `streams` is not provided.
        streams_.resize(1);
        auto& stream = streams_[0];
        if (!stream.Init(*all, *all, err)) {
            return false;
        }
    } else {
        // `stream` is taken as defaults to `streams`, as `streams` is non-empty.
        if (Contains(*all, "remote") || Contains(*all, "local") || Contains(*all, "split")) {
            *err = "If providing `streams`, the top-level `stream` object is used as defaults for "
                "`streams`, and its `remote`, `local`, and `split` fields are meaningless.";
            return false;
        }
        streams_.resize(streams->size());
        int64_t i = 0;
        for (auto it : streams->items()) {
            if (!streams_[i].Init(it.value(), *all, err)) {
                return false;
            }
            ++i;
        }
    }

    // Cross-check stream weighting scheme.
    if (!Stream::CrossCheckWeights(streams_, err)) {
        return false;
    }

    // Initialize each stream's shards from JSON in parallel.
    vector<vector<Shard*>> shard_lists;
    {
        shard_lists.resize(streams_.size());
        vector<thread> threads;
        threads.resize(streams_.size());
        for (int64_t i = 0; i < streams_.size(); ++i) {
            threads[i] = thread(LoadIndex, i, &streams_[i], &shard_lists[i]);
        }
        for (auto& th : threads) {
            th.join();
        }
    }

    // Allocate space for the global list of shards.
    {
        shards_.clear();
        int64_t num_shards = 0;
        for (auto& shards : shard_lists) {
            num_shards += shards.size();
        }
        shards_.resize(num_shards);
    }

    // Gather each stream's shards together into one global list.
    {
        int64_t shard_offset = 0;
        for (auto& shards : shard_lists) {
            int64_t num_bytes = shards.size() * sizeof(shards[0]);
            memcpy(&shards_[shard_offset], &shards[0], num_bytes);
            shard_offset += shards.size();
        }
    }

    // Calculate stream shard/sample sizes/offsets.
    {
        int64_t shard_offset = 0;
        int64_t sample_offset = 0;
        for (int64_t i = 0; i < streams_.size(); ++i) {
            auto& stream = streams_[i];
            stream.set_shard_offset(shard_offset);
            stream.set_sample_offset(sample_offset);
            auto& shards = shard_lists[i];
            stream.set_num_shards(shards.size());
            shard_offset += shards.size();
            int64_t num_samples = 0;
            for (auto& shard : shards) {
                num_samples += shard->num_samples();
            }
            stream.set_num_samples(num_samples);
            sample_offset += num_samples;
        }
    }

    // Calculate shard sample offsets and initialize the shard index.
    {
        int64_t sample_offset = 0;
        vector<int64_t> shard_sizes;
        shard_sizes.reserve(shards_.size());
        for (auto& shard : shards_) {
            shard->set_sample_offset(sample_offset);
            auto size = shard->num_samples();
            sample_offset += size;
            shard_sizes.emplace_back(size);
        }
        shard_index_.Init(shard_sizes, shard_index_bucket_size);
    }

    return true;
}

}  // namespace xtreaming
