#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "base/json.h"
#include "serial/base/shard.h"

using std::string;
using std::vector;

namespace xtreaming {

class Stream {
  public:
    const string& remote() const { return remote_; }
    const string& local() const { return local_; }
    const string& split() const { return split_; }
    const string& index() const { return index_; }

    int64_t download_retry() const { return download_retry_; }
    double download_timeout() const { return download_timeout_; }
    const vector<string>& hash_algos() const { return hash_algos_; }
    bool non_hashed_ok() const { return non_hashed_ok_; }
    bool keep_zip() const { return keep_zip_; }
    bool safe_keep_zip() const { return safe_keep_zip_; }

    double proportion() const { return proportion_; }
    double repeat() const { return repeat_; }
    int64_t choose() const { return choose_; }

    int64_t shard_offset() const { return shard_offset_; }
    int64_t num_shards() const { return num_shards_; }
    int64_t sample_offset() const { return sample_offset_; }
    int64_t num_samples() const { return num_samples_; }

    void set_shard_offset(int64_t shard_offset) { shard_offset_ = shard_offset; }
    void set_num_shards(int64_t num_shards) { num_shards_ = num_shards; }
    void set_sample_offset(int64_t sample_offset) { sample_offset_ = sample_offset; }
    void set_num_samples(int64_t num_samples) { num_samples_ = num_samples; }

    // Initialize from its config and a backup config shared by all streams.
    bool Init(const json& obj, const json& all, string* err);

    // Verify that all streams are weighted either in absolute terms or relatively.
    static bool CrossCheckWeights(const vector<Stream>& streams, bool* relative, string* err);

    // Drive how the streams are sampled given stream weights and underlying sizes.
    static bool DeriveSampling(vector<Stream>* streams, bool relative, uint32_t seed,
                               int64_t* epoch_size, string* err);

    // Scan my local dir, normalizing files and gathering which shards are present.
    void InitLocalDir(const vector<Shard*> shards, vector<bool>* is_present) const;

  private:
    // Sampling derivations.
    static bool DeriveSamplingRelatively(vector<Stream>* streams, uint32_t seed,
                                         int64_t* epoch_size, string* err);
    static bool DeriveSamplingAbsolutely(vector<Stream>* streams, int64_t* epoch_size,
                                         string* err);

    // Paths.

    string remote_;  // Path to the remote, persistent copy of the dataset.
                     //
                     // If empty, assumes dataset is already completely downloaded to local and
                     // local must exist.

    string local_;  // Local filesystem path where the dataset will be cached on the node.
                    //
                    // If empty, we use a hash of remote and split as the local path and remote
                    // must exist.

    string split_;  // Appends this subdir to `remote` and `local` paths.

    string index_;  // Relative path to the index file.
                    //
                    // If empty, we use `index.json`.

    // Behavior.
    //
    // If these fields are not provided, they take their value from StreamingDataset defaults.

    int64_t download_retry_;  // Number of download re-attempts before giving up.

    double download_timeout_;  // Time of seconds to wait for a shard to download before giving up.

    vector<string> hash_algos_;  // Ranked list of hashes to validate. Checks only the first match
                                 // (i.e., the first hash that is present in both this list and the
                                 // shard metadta). Can be empty.
                                 //
                                 // If not given, takes its value from StreamingDataset.

    bool non_hashed_ok_;  // Whether it is okay to not hash validate a shard due to not match. If
                          // false, asserts. If true, skips.

    bool keep_zip_;  // Whether to keep or drop compressed versions of shards upon download. If
                     // false, drops iff remote is not local. If true, keeps.

    bool safe_keep_zip_;  // Whether to keep or drop compressed versions of shards upon download.
                          // If false, drops. If true, keeps.

    // Weights.
    //
    // Stream weights must be either entirely relatve or entirely absolute. If no weight is
    // specified, this means absolute weighting at 1:1 sampling ratio.

    double proportion_;  // What proportion of the combined dataset each epoch is this stream
                         // (relative weighting).
                         //
                         // If negative, its value is derived from the others.

    double repeat_;  // How many times to repeat each sample of this stream in the combined dataset
                     // each epoch, in expectation (absolute weighting).
                     //
                     // If negative, its value is derived from the others.

    int64_t choose_;  // How many samples to choose from this stream for the combined dataset each
                      // epoch (absolute weighting).
                      //
                      // If negative, its value is derived from the others.

    // Offsets and sizes.
    //
    // These fields are set during the process of loading all shards, not in init.

    int64_t shard_offset_;   // Offset of our first shard over all streams.
    int64_t num_shards_;     // Number of shards this stream contains.
    int64_t sample_offset_;  // Offset of our first sample over all streams.
    int64_t num_samples_;    // Number of samples this stream contains.
};

}  // namespace xtreaming
