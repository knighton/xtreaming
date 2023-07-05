#include "shard.h"

#include <utility>

using std::make_pair;

namespace xtreaming {

MDSShard::~MDSShard() {
}

void MDSShard::Init(int64_t stream_id, const set<string>& hash_algos, int64_t num_samples,
                    int64_t size_limit, const string& zip_algo, FileInfo* raw_data,
                    FileInfo* zip_data, const vector<MDSColumn>& columns) {
    Shard::Init(stream_id, hash_algos, num_samples, size_limit, zip_algo);
    raw_data_ = raw_data;
    zip_data_ = zip_data;
    columns_ = columns;
    file_pairs_.emplace_back(make_pair(raw_data, zip_data));
}

void MDSShard::InitFromJSON(int64_t stream_id, const json& obj) {
    set<string> hash_algos;
    for (auto& algo : obj["hashes"]) {
        hash_algos.insert(algo);
    }

    int64_t num_samples = obj["samples"];

    int64_t size_limit = obj["size_limit"];

    string zip_algo;
    if (obj.contains("compression") && obj["compression"].is_string()) {
        zip_algo = obj["compression"];
    }

    auto& raw_obj = obj["raw_data"];
    FileInfo* raw_data = new FileInfo;
    raw_data->path = raw_obj["basename"];
    raw_data->num_bytes = raw_obj["bytes"];
    for (auto it : raw_obj["hashes"].items()) {
        raw_data->hashes[it.key()] = it.value();
    }

    auto& zip_obj = obj["zip_data"];
    FileInfo* zip_data = nullptr;
    if (!zip_obj.is_null()) {
        zip_data = new FileInfo;
        zip_data->path = zip_obj["basename"];
        zip_data->num_bytes = zip_obj["bytes"];
        for (auto it : zip_obj["hashes"].items()) {
            zip_data->hashes[it.key()] = it.value();
        }
    }

    vector<MDSColumn> columns;
    int64_t num_columns = obj["column_names"].size();
    columns.resize(num_columns);
    for (int i = 0; i < num_columns; ++i) {
        columns[i].name = obj["column_names"][i];
    }
    for (int i = 0; i < num_columns; ++i) {
        columns[i].type = obj["column_encodings"][i];
    }
    for (int i = 0; i < num_columns; ++i) {
        auto& size = obj["column_sizes"][i];
        columns[i].num_bytes = size.is_null() ? -1L : (int64_t)size;
    }

    Init(stream_id, hash_algos, num_samples, size_limit, zip_algo, raw_data, zip_data, columns);
}

}  // namespace xtreaming
