#pragma once

#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

using std::map;
using std::pair;
using std::set;
using std::string;
using std::vector;

namespace xtreaming {

struct FileInfo {
    string path;
    int64_t num_bytes{0};
    map<string, string> hashes;
};

class Shard {
  public:
    int64_t stream_id() const { return stream_id_; }
    int64_t sample_offset() const { return sample_offset_; }

    void set_sample_offset(int64_t sample_offset) { sample_offset_ = sample_offset; }

    const set<string>& hash_algos() const { return hash_algos_; }
    int64_t num_samples() const { return num_samples_; }
    int64_t size_limit() const { return size_limit_; }
    const string& zip_algo() const { return zip_algo_; }

    const vector<pair<FileInfo*, FileInfo*>>& file_pairs() const { return file_pairs_; }

    virtual ~Shard();

    void Init(int64_t stream_id, const set<string>& hash_algos, int64_t num_samples,
              int64_t size_limit, const string& zip_algo);

    void EvictRaw(const string& local, const string& split) const;
    void EvictZip(const string& local, const string& split) const;
    void Evict(const string& local, const string& split) const;

    bool InitLocalDir(const string& local, const string& split, bool keep_zip,
                      set<string>& files) const;

  protected:
    int64_t stream_id_{-1L};      // ID of the stream this shard is from.
    int64_t sample_offset_{-1L};  // Offset of this shard in the global sample ID space.

    set<string> hash_algos_;    // List of hashes applied to each file comprising the shard.
    int64_t num_samples_{-1L};  // Number of samples in this shard.
    int64_t size_limit_{-1L};   // Size limit in bytes when this shard was written.
    string zip_algo_;           // Compression algorithm used, or empty if none.

    vector<pair<FileInfo*, FileInfo*>> file_pairs_;  // Pairs of (raw info, maybe zip info).
};

}  // namespace xtreaming
