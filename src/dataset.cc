#include "dataset.h"

#include <thread>

#include "base/string.h"
#include "base/xtensor.h"
#include "determiner/all.h"
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

    // Init sampling.
    if (!GetObject(obj, "sampler", &empty_obj, &section, err)) {
        return false;
    }
    sampler_ = GetSampler(*section, err);
    if (!sampler_) {
        return false;
    }

    // Init determinism.
    if (!GetObject(obj, "determiner", &empty_obj, &section, err)) {
        return false;
    }
    determiner_ = GetDeterminer(*section, err);
    if (!determiner_) {
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

    // Init streams and their shards.
    const json* all;
    if (!GetObject(obj, "stream", &empty_obj, &all, err)) {
        return false;
    }
    const json* streams;
    if (!GetObject(obj, "streams", &empty_obj, &streams, err)) {
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

    // Cross-check stream weighting scheme.
    bool relative_weights;
    if (!Stream::CrossCheckWeights(streams_, &relative_weights, err)) {
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

    // Now that we have both the stream weights and the underlying sample counts, derive how they
    // are sampled.
    if (!Stream::DeriveSampling(&streams_, relative_weights, sampler_->seed(),
                                sampler_->mutable_epoch_size(), err)) {
        return false;
    }

    // Scan local directories, normalizing and gathering which shards are currently present.
    vector<bool> is_shard_present;
    is_shard_present.resize(shards_.size());
    for (auto& stream : streams_) {
        stream.CheckLocalDir(shards_, &is_shard_present);
    }

    return true;
}

bool Dataset::Bench() {
    printf("start...\n");

    printf("sampler...\n");
    // Sample each shard of each stream according to its weight.
    //
    // This gives us:
    // * Size of each subshard.
    // * Mapping from resampled fake sample ID to underlying physical sample ID.
    //
    // When heavily upweighting shards, you want to break them up into multiple ephemeral parts so
    // as to not perserverate and tank the model. These are called subshards.
    int64_t epoch = 0;
    vector<int64_t> subshard_sizes;
    vector<int64_t> to_physical;
    sampler_->Sample(streams_, shards_, epoch, &subshard_sizes, &to_physical);

    printf("determiner...\n");
    // Order the global sample space across all nodes, ranks, and workers such that we have an
    // elastically deterministic sample ordering.
    //
    // This gives us:
    // * Tensor of shape (num physical nodes, ranks per node, workers per rank, batches per worker,
    //   samples per batch).
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

    printf("shuffler...\n");
    // If we need to shuffle, shuffle in a node-aware and *underlying* shard-aware way.
    if (shuffle_) {
        vector<int64_t> to_shuffled;
        shuffler_->Shuffle(subshard_sizes, determiner_->num_canonical_nodes(), epoch, &to_shuffled);
        for (int64_t i = 0; i < sample_ids.size(); ++i) {
            auto& sample_id = sample_ids.data()[i];
            if (sample_id != -1L) {
                sample_id = to_shuffled[sample_id];
            }
        }
    }

    printf("mapping...\n");
    // Now that twe have partitioned and shuffled with fake resampled sample IDs, we don't need
    // them anymore, and now convert back to their underlying physical sample IDs.
    for (int64_t i = 0; i < sample_ids.size(); ++i) {
        auto& sample_id = sample_ids.data()[i];
        if (sample_id != -1L) {
            sample_id = to_physical[sample_id];
        }
    }

    printf("done.\n");
    return true;
}

}  // namespace xtreaming
