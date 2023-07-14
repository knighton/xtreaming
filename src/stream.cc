#include "stream.h"

#include <algorithm>
#include <filesystem>
#include <random>
#include <set>

#include "base/hash/xxhash.h"
#include "base/string.h"

namespace fs = std::filesystem;
using std::default_random_engine;
using std::random_device;
using std::shuffle;

namespace xtreaming {
namespace {

string DeriveLocal(const string& remote, const string& split) {
    string root = "/tmp/streaming/dataset/";
    string hash = XXH3_64(remote.c_str(), remote.size());
    string local = root + hash + "/";
    if (!split.empty()) {
        local += split + "/";
    }
    return local;
}

}  // namespace

bool Stream::Init(const json& obj, const json& all, string* err) {
    // Init paths:

    if (!GetString(obj, "remote", "", &remote_, err)) {
        return false;
    }

    if (!GetString(obj, "local", "", &local_, err)) {
        return false;
    }

    if (!GetString(obj, "split", "", &split_, err)) {
        return false;
    }

    if (remote_.empty() && local_.empty()) {
        *err = "At least one of `remote` and `local` must be provided.";
        return false;
    }

    if (local_.empty()) {
        local_ = DeriveLocal(remote_, split_);
    }

    if (!GetString(obj, "index", "", &index_, err)) {
        return false;
    }

    // Init behavior:

    if (!GetInt64(obj, all, "download_retry", 3, &download_retry_, err)) {
        return false;
    }
    if (download_retry_ < 0) {
        *err = StringPrintf("`download_retry` must be non-negative (got: %ld).", download_retry_);
        return false;
    }

    if (!GetTime(obj, all, "download_timeout", 60.0, &download_timeout_, err)) {
        return false;
    }
    if (download_timeout_ <= 0) {
        *err = StringPrintf("`download_timeout` must be positive (got: %.lf).", download_timeout_);
        return false;
    }

    if (!GetStrings(obj, all, "hash_algos", {}, &hash_algos_, err)) {
        return false;
    }

    if (!GetBool(obj, all, "non_hashed_ok", true, &non_hashed_ok_, err)) {
        return false;
    }

    if (!GetBool(obj, all, "keep_zip", false, &keep_zip_, err)) {
        return false;
    }

    safe_keep_zip_ = keep_zip_ || (remote_ == local_);

    // Init weights:

    if (!GetDouble(obj, "proportion", -1L, &proportion_, err)) {
        return false;
    }

    if (!GetDouble(obj, "repeat", -1L, &repeat_, err)) {
        return false;
    }

    if (!GetInt64(obj, "choose", -1L, &choose_, err)) {
        return false;
    }

    int num_weights = (0 <= proportion_) + (0 <= repeat_) + (0 <= choose_);
    if (1 < num_weights) {
        *err = "At most one type of weight (`proportion`, `repeat`, and `choose`) can be used.";
        return false;
    }

    // Init accounting:

    shard_offset_ = -1L;
    num_shards_ = -1L;
    sample_offset_ = -1L;
    num_samples_ = -1L;

    return true;
}

bool Stream::CrosscheckWeights(const vector<Stream>& streams, bool* relative, string* err) {
    int num_relative = 0;
    for (auto& stream : streams) {
        num_relative += (0 <= stream.proportion());
    }

    if (!num_relative) {
        *relative = false;
        return true;
    } else if (num_relative == streams.size()) {
        *relative = true;
        return true;
    } else {
        *err = "Attempted to mix absolute (`repeat`, `choose`, or no weight) and relative "
            "(`proportion`) stream weighting schemes. They must be all one or the other.";
        return false;
    }
}

bool Stream::DeriveSamplingRelatively(vector<Stream>* streams, uint32_t seed, int64_t* epoch_size,
                                      string* err) {
    // Global number of samples to choose defaults to the same size as the underlying.
    if (!*epoch_size) {
        for (auto& stream : *streams) {
            *epoch_size += stream.num_samples_;
        }
    }

    // Normalize proportions.
    double prop_sum = 0;
    for (int64_t i = 0; i < streams->size(); ++i) {
        auto& stream = (*streams)[i];
        if (stream.proportion_ < 0) {
            *err = StringPrintf("Negative proportion in stream %ld.", i);
            return false;
        }
        prop_sum += stream.proportion_;
    }
    for (auto& stream : *streams) {
        stream.proportion_ /= prop_sum;
    }

    // Derive choose, and measure the shortfall due to rounding down.
    int64_t choose_sum = 0;
    for (auto& stream : *streams) {
        stream.choose_ = (int64_t)((double)*epoch_size * stream.proportion_);
        choose_sum += stream.choose_;
    }
    int64_t shortfall = *epoch_size - choose_sum;

    // Initialize stream IDs vector.
    vector<int64_t> stream_ids;
    stream_ids.reserve(streams->size());
    for (int64_t i = 0; i < streams->size(); ++i) {
        stream_ids.emplace_back(i);
    }

    // Randomly assign some streams one extra sample choice to rectify the shortfall.
    random_device random;
    default_random_engine engine(random());
    engine.seed(seed);
    shuffle(stream_ids.begin(), stream_ids.end(), engine);
    for (int64_t i = 0; i < shortfall; ++i) {
        auto& stream_id = stream_ids[i];
        (*streams)[stream_id].choose_ += 1;
    }

    // Now derive repeat.
    for (auto& stream : *streams) {
        stream.repeat_ = (double)stream.choose_ / (double)stream.num_samples_;
    }

    return true;
}

bool Stream::DeriveSamplingAbsolutely(vector<Stream>* streams, int64_t* epoch_size, string* err) {
    // StreamingDataset settings `epoch_size` is not needed for absolute weighted streams.
    if (*epoch_size) {
        *err = "Only set `epoch_size` in StreamingDataset settings when weighting streams "
            "relatively, otherwise there is no need to do so.";
        return false;
    }

    // Derive choose.
    for (auto& stream : *streams) {
        if (0 <= stream.repeat_) {
            stream.choose_ = (int64_t)(stream.repeat_ * (double)stream.num_samples_);
        } else if (0 <= stream.choose_) {
            ;
        } else {
            stream.choose_ = stream.num_samples_;
        }
    }

    // Now derive repeat.
    for (auto& stream : *streams) {
        stream.repeat_ = (double)stream.choose_ / (double)stream.num_samples_;
    }

    // Get the global number of samples to choose.
    *epoch_size = 0;
    for (auto& stream : *streams) {
        *epoch_size += stream.choose_;
    }

    // Finally, derive stream proportions.
    for (auto& stream : *streams) {
        stream.proportion_ = (double)stream.choose_ / (double)*epoch_size;
    }

    return true;
}

bool Stream::DeriveSampling(vector<Stream>* streams, bool relative, uint32_t seed,
                            int64_t* epoch_size, string* err) {
    if (relative) {
        return DeriveSamplingRelatively(streams, seed, epoch_size, err);
    } else {
        return DeriveSamplingAbsolutely(streams, epoch_size, err);
    }
}

void Stream::CheckLocalDir(const vector<Shard*> shards, vector<bool>* is_present) const {
    string dir = local_ + "/" + split_;
    set<string> files;
    for (auto& entry : fs::recursive_directory_iterator(dir)) {
        files.insert(entry.path());
    }

    for (int64_t i = shard_offset_; i < shard_offset_ + num_shards_; ++i) {
        auto& shard = shards[i];
        (*is_present)[i] = shard->CheckLocalDir(local_, split_, keep_zip_, files);
    }
}

}  // namespace xtreaming
