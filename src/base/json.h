#pragma once

#include <string>

#include "third_party/json/json.hpp"

using std::string;

namespace xtreaming {

using json = nlohmann::json;

// Safe getters that populate `error` if error.
// GetString appends the data to the provided buffer.
bool GetBool(const json& obj, const string& key, bool def, bool* ret, string* err);
bool GetInt64(const json& obj, const string& key, int64_t def, int64_t* ret, string* err);
bool GetUInt32(const json& obj, const string& key, uint32_t def, uint32_t* ret, string* err);
bool GetNumBytes(const json& obj, const string& key, int64_t def, int64_t* ret, string* err);
bool GetString(const json& obj, const string& key, const string& def, string* buf, char** ret,
               string* err);

}  // namespace xtreaming
