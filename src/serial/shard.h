#pragma once

#include "base/json.h"
#include "serial/base/shard.h"

namespace xtreaming {

Shard* GetShard(int64_t stream_id, const json& obj);

}  // namespace xtreaming
