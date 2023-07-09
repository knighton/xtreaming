#pragma once

#include <string>

#include "base/json.h"
#include "sampler/sampler.h"

using std::string;

namespace xtreaming {

Sampler* GetSampler(const json& obj, string* err);

}  // namespace xtreaming
