#include "shard.h"

#include <filesystem>

namespace fs = std::filesystem;

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

void Shard::EvictRaw(const string& local, const string& split) const {
    for (auto& pair : file_pairs_) {
        string path = local + "/" + split + "/" + pair.first->path;
        if (fs::is_regular_file(path)) {
            fs::remove(path);
        }
    }
}

void Shard::EvictZip(const string& local, const string& split) const {
    for (auto& pair : file_pairs_) {
        if (pair.second) {
            string path = local + "/" + split + "/" + pair.second->path;
            if (fs::is_regular_file(path)) {
                fs::remove(path);
            }
        }
    }
}

void Shard::Evict(const string& local, const string& split) const {
    EvictRaw(local, split);
    EvictZip(local, split);
}

bool Shard::InitLocalDir(const string& local, const string& split, bool keep_zip,
                         set<string>& files) const {
    // For raw/zip to be considered present, each raw/zip file must be present.
    int64_t raw_files_present = 0;
    int64_t zip_files_present = 0;
    for (auto& pair : file_pairs_) {
        string path = local + "/" + split + "/" + pair.first->path;
        if (files.find(path) != files.end()) {
            ++raw_files_present;
        }
        if (pair.second) {
            path = local + "/" + split + "/" + pair.second->path;
            if (files.find(path) != files.end()) {
                ++zip_files_present;
            }
        }
    }

    // If the shard raw files are partially present, garbage collect the present ones and mark
    // the shard raw as not present, in order to achieve consistency.
    bool has_raw;
    if (!raw_files_present) {
        has_raw = false;
    } else if (raw_files_present < file_pairs_.size()) {
        has_raw = false;
        EvictRaw(local, split);
    } else {
        has_raw = true;
    }

    //  Same as the above, but for shard zip files.
    bool has_zip;
    if (!zip_files_present) {
        has_zip = false;
    } else if (zip_files_present < file_pairs_.size()) {
        has_zip = false;
        EvictZip(local, split);
    } else {
        has_zip = true;
    }

    // Do we keep_zip?
    if (keep_zip) {
        // If we can keep_zip, and we do, and have either raw or zip, we must have the other one
        // too, because they are downloaded and decompressed together.
        if (!zip_algo_.empty() && (has_zip != has_raw)) {
            if (has_raw) {
                has_raw = false;
                EvictRaw(local, split);
            } else if (has_zip) {
                has_zip = false;
                EvictZip(local, split);
            }
        }
    } else {
        // If we don't keep_zip, drop any zip files.
        if (has_zip) {
            has_zip = false;
            EvictZip(local, split);
        }
    }

    // Now, the shard is either entirely or not at all present given keep_zip.
    return has_raw;
}

}  // namespace xtreaming
