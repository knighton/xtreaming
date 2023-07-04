#pragma once

#include <string>

#include "base/json.h"

using std::string;

namespace xtreaming {

// Parse YAML text into the provided nlohmann::json object, setting `error` on failure.
//
// Only supports a minimal subset of YAML that we need.
bool ParseYAML(const string& text, json* obj, string *error);

}  // namespace xtreaming
