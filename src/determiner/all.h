#pragma once

#include <string>

#include "base/json.h"
#include "determiner/determiner.h"

using std::string;

namespace xtreaming {

Determiner* GetDeterminer(const json& obj, string* err);

}  // namespace xtreaming
