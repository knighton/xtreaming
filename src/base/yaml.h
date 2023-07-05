#pragma once

#include <string>

#include "base/json.h"

using std::string;

namespace xtreaming {

// Parse YAML text into the provided nlohmann::json object, setting `err` on failure.
//
// Beware! Only supports a minimal subset of YAML that we need.
bool ParseYAML(const string& txt, json* obj, string *err);

}  // namespace xtreaming
