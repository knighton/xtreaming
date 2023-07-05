#include "shard.h"

#include "serial/mds/shard.h"

#include <cassert>

namespace xtreaming {

Shard* GetShard(int64_t stream_id, const json& obj) {
    const string& format = obj["format"];
    if (format == "mds") {
        MDSShard* shard = new MDSShard;
        shard->InitFromJSON(stream_id, obj);
        return shard;
    } else {
        assert(false);
    }
}

}  // namespace xtreaming
