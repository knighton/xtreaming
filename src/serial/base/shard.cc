#include "shard.h"

namespace xtreaming {

Shard::~Shard() {
    for (auto& pair : file_pairs_) {
        if (pair.first) {
            delete pair.first;
            pair.first = nullptr;
        }
        if (pair.second) {
            delete pair.second;
            pair.second = nullptr;
        }
    }
}

void Shard::Init(int64_t stream_id, const set<string>& hash_algos, int64_t num_samples,
                 int64_t size_limit, const string& zip_algo) {
    stream_id_ = stream_id;
    hash_algos_ = hash_algos;
    num_samples_ = num_samples;
    size_limit_ = size_limit;
    zip_algo_ = zip_algo;
}

bool Shard::InitLocalDir(const string& local, const string& split, set<string>& files) const {
    return false;  // TODO
}

}  // namespace xtreaming
