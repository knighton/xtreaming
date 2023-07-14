#include "dataset.h"

#include <functional>
#include <thread>

#include "base/string.h"
#include "base/time.h"
#include "base/xtensor.h"
#include "determiner/all.h"
#include "sampler/all.h"
#include "serial/index.h"
#include "shuffler/all.h"

using std::function;

namespace xtreaming {

bool Dataset::InitShardIndexArgs(const json& obj, int64_t* bucket_size, string* err) {
    json empty;
    const json* section;
    if (!GetObject(obj, "shard_index", &empty, &section, err)) {
        return false;
    }

    if (!GetInt64(*section, "bucket_size", 1024, bucket_size, err)) {
        return false;
    }

    if (*bucket_size < 1) {
        *err = StringPrintf("`index.bucket_size` must be postive, but got: %ld.", *bucket_size);
        return false;
    }

    return true;
}

bool Dataset::InitSampler(const json& obj, string* err) {
    json empty;
    const json* section;
    if (!GetObject(obj, "sampler", &empty, &section, err)) {
        return false;
    }

    sampler_ = GetSampler(*section, err);
    if (!sampler_) {
        return false;
    }

    return true;
}

bool Dataset::InitDeterminer(const json& obj, string* err) {
    json empty;
    const json* section;
    if (!GetObject(obj, "determiner", &empty, &section, err)) {
        return false;
    }

    determiner_ = GetDeterminer(*section, err);
    if (!determiner_) {
        return false;
    }

    return true;
}

bool Dataset::InitShuffler(const json& obj, string* err) {
    json empty;
    const json* section;
    if (!GetBool(obj, "shuffle", false, &shuffle_, err)) {
        return false;
    }

    if (!GetObject(obj, "shuffler", &empty, &section, err)) {
        return false;
    }

    shuffler_ = GetShuffler(*section, err);
    if (!shuffler_) {
        return false;
    }

    return true;
}

bool Dataset::InitStreams(const json& obj, string* err) {
    json empty;
    const json* all;
    if (!GetObject(obj, "stream", &empty, &all, err)) {
        return false;
    }

    const json* streams;
    if (!GetObject(obj, "streams", &empty, &streams, err)) {
        return false;
    }

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

    return true;
}

bool Dataset::InitShards(string* err) {
    // Initialize each stream's shards from JSON in parallel.
    vector<vector<Shard*>> shard_lists;
    {
        shard_lists.resize(streams_.size());
        vector<std::thread> threads;
        threads.resize(streams_.size());
        for (int64_t i = 0; i < streams_.size(); ++i) {
            threads[i] = std::thread(LoadIndex, i, &streams_[i], &shard_lists[i]);
        }
        for (auto& thread : threads) {
            thread.join();
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

    // Calculate shard sample offsets.
    {
        int64_t sample_offset = 0;
        for (auto& shard : shards_) {
            shard->set_sample_offset(sample_offset);
            auto size = shard->num_samples();
            sample_offset += size;
        }
    }

    return true;
}

bool Dataset::InitShardIndex(int64_t bucket_size, string* err) {
    vector<int64_t> shard_sizes;
    shard_sizes.reserve(shards_.size());
    for (auto& shard : shards_) {
        shard_sizes.emplace_back(shard->num_samples());
    }
    shard_index_.Init(shard_sizes, bucket_size);
    return true;
}

bool Dataset::InitCaches(string* err) {
    vector<bool> is_shard_present;
    is_shard_present.resize(shards_.size());
    for (auto& stream : streams_) {
        stream.CheckLocalDir(shards_, &is_shard_present);
    }
    return true;
}

bool Dataset::Init(const json& obj, string* err) {
    int64_t bucket_size;
    bool relative_weights;
    vector<function<bool()>> stages = {
        [&]{ return InitShardIndexArgs(obj, &bucket_size, err); },
        [&]{ return InitSampler(obj, err); },
        [&]{ return InitDeterminer(obj, err); },
        [&]{ return InitShuffler(obj, err); },
        [&]{ return InitStreams(obj, err); },
        [&]{ return Stream::CrosscheckWeights(streams_, &relative_weights, err); },
        [&]{ return InitShards(err); },
        [&]{ return InitShardIndex(bucket_size, err); },
        [&]{ return Stream::DeriveSampling(&streams_, relative_weights, sampler_->seed(),
                                           sampler_->mutable_epoch_size(), err); },
        [&]{ return InitCaches(err); },
    };

    for (auto& stage : stages) {
        if (!stage()) {
            return false;
        }
    }

    return true;
}

void Dataset::SampleThread(int64_t epoch, vector<int64_t>* subshard_sizes,
                           vector<int64_t>* fake_to_real, int64_t* t0, int64_t* t1) {
    *t0 = NanoTime();
    sampler_->Sample(streams_, shards_, epoch, subshard_sizes, fake_to_real);
    *t1 = NanoTime();
}

namespace {

void Map(const vector<int64_t>& mapping, xt::xarray<int64_t>* ids) {
    for (int64_t i = 0; i < ids->size(); ++i) {
        auto& id = ids->data()[i];
        if (id != -1L) {
            id = mapping[id];
        }
    }
}

}  // namespace

bool Dataset::Bench() {
    // Sample each shard of each stream according to its weight.
    //
    // This gives us:
    // * Size of each subshard.
    // * Mapping from resampled fake sample ID to underlying physical sample ID.
    //
    // When heavily upweighting shards, you want to break them up into multiple ephemeral parts so
    // as to not perserverate and tank the model. These are called subshards.
    //
    // Do this work in parallel. Its results aren't immediately needed.
    int64_t epoch = 0;
    vector<int64_t> subshard_sizes;
    vector<int64_t> fake_to_real;
    int64_t t0;
    int64_t t1;
    auto sampling_thread = std::thread(&Dataset::SampleThread, this, epoch, &subshard_sizes,
                                       &fake_to_real, &t0, &t1);

    // Order the global sample space across all nodes, ranks, and workers such that we have an
    // elastically deterministic sample ordering.
    //
    // This gives us:
    // * Tensor of shape (num physical nodes, ranks per node, workers per rank, batches per worker,
    //   samples per batch).
    t0 = NanoTime();
    int64_t num_physical_nodes = 16;
    int64_t ranks_per_node = 5;
    int64_t workers_per_rank = 7;
    int64_t sample_offset = 256;
    xt::xarray<int64_t> sample_ids;
    string err;
    if (!determiner_->Determine(num_physical_nodes, ranks_per_node, workers_per_rank,
                                sampler_->epoch_size(), sample_offset, &sample_ids, &err)) {
        fprintf(stderr, "%s\n", err.c_str());
        return false;
    }
    auto t = NanoTime() - t0;
    printf("%10.6f Determinism\n", t / 1e9);

    // Join the sampling thread.
    sampling_thread.join();
    t = t1 - t0;
    printf("%10.6f Sampling\n", t / 1e9);

    // If we need to shuffle, shuffle in a node-aware and *underlying* shard-aware way.
    t0 = NanoTime();
    if (shuffle_) {
        vector<int64_t> shuffle;
        shuffler_->Shuffle(subshard_sizes, determiner_->num_canonical_nodes(), epoch, &shuffle);
        Map(shuffle, &sample_ids);
    }
    t = NanoTime() - t0;
    printf("%10.6f Shuffling\n", t / 1e9);

    // Now that twe have partitioned and shuffled with fake resampled sample IDs, we don't need
    // them anymore, and now convert back to their underlying physical sample IDs.
    t0 = NanoTime();
    Map(fake_to_real, &sample_ids);
    t = NanoTime() - t0;
    printf("%10.6f Mapping\n", t / 1e9);

    return true;
}

}  // namespace xtreaming
