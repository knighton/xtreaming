#pragma once

#include <string>

#include "base/json.h"
#include "shuffler/shuffler.h"

using std::string;

namespace xtreaming {

Shuffler* GetShuffler(const json& obj, string* err);

}  // namespace xtreaming
