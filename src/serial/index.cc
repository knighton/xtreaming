#include "index.h"

#include <cstdio>

#include "base/json.h"
#include "base/string.h"
#include "serial/shard.h"

namespace xtreaming {

void LoadIndex(int64_t stream_id, const Stream* stream, vector<Shard*>* shards, Logger* logger) {
    // Maybe log scope enter/exit.
    string stream_name;
    if (stream->name().empty()) {
        stream_name = StringPrintf("stream_%ld", stream_id);
    } else {
        stream_name = stream->name();
    }
    string base_scope_name = StringPrintf("init/shards/load_indexes/%s", stream_name.c_str());
    auto scope = logger->Scope(base_scope_name);

    // Locate and attempt to open the index file.
    auto local = stream->local().c_str();
    auto split = stream->split().c_str();
    auto basename = stream->index().size() ? stream->index().c_str() : "index.json";
    string filename = StringPrintf("%s/%s/%s", local, split, basename);
    FILE* file = fopen(filename.c_str(), "rb");
    assert(file);

    // Parse the index file.
    json obj;
    try {
        auto scope2 = logger->Scope(base_scope_name + "/read_and_parse");
        obj = json::parse(file);
    } catch (const json::parse_error& e) {
        fprintf(stderr, "%s\n", e.what());
        return;
    }

    // Initialize each shard from the index.
    shards->clear();
    if (obj.contains("shards") && obj["shards"].is_array()) {
        auto scope2 = logger->Scope(base_scope_name + "/get_shards");
        shards->reserve(obj["shards"].size());
        for (auto& info : obj["shards"]) {
            Shard* shard = GetShard(stream_id, info);
            shards->emplace_back(shard);
        }
    } else {
        fprintf(stderr, "Index is missing `shards` array.\n");
        return;
    }
}

}  // namespace xtreaming
