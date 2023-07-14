#pragma once

#include <string>
#include <vector>

#include "serial/base/shard.h"
#include "stream.h"

using std::string;
using std::vector;

namespace xtreaming {

void LoadIndex(int64_t stream_id, const Stream* stream, vector<Shard*>* shards, Logger* logger);

}  // namespace xtreaming
