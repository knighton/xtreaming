#include "index.h"

#include <cstdio>

#include "base/json.h"
#include "base/string.h"
#include "serial/shard.h"

namespace xtreaming {

void LoadIndex(int64_t stream_id, const Stream* stream, vector<Shard*>* shards) {
    auto local = stream->local().c_str();
    auto split = stream->split().c_str();
    auto basename = stream->index().size() ? stream->index().c_str() : "index.json";
    string filename = StringPrintf("%s/%s/%s", local, split, basename);

    FILE* file = fopen(filename.c_str(), "rb");
    assert(file);

    json obj;
    try {
        obj = json::parse(file);
    } catch (const json::parse_error& e) {
        printf("%s\n", e.what());
        return;
    }

    shards->clear();
    shards->reserve(obj["shards"].size());
    for (auto& info : obj["shards"]) {
        Shard* shard = GetShard(stream_id, info);
        shards->emplace_back(shard);
    }
}

}  // namespace xtreaming
