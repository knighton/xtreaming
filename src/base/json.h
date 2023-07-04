#pragma once

#include <string>

#include "third_party/json/json.hpp"

using std::string;

namespace xtreaming {

using json = nlohmann::json;

// Safe JSON field getters with defaults.
//
// Returns whether succeeded. Only sets `ret` on success. Only sets `err` on failure.
bool GetBool(const json& obj, const string& key, bool def, bool* ret, string* err);
bool GetInt64(const json& obj, const string& key, int64_t def, int64_t* ret, string* err);
bool GetUInt32(const json& obj, const string& key, uint32_t def, uint32_t* ret, string* err);
bool GetBytes(const json& obj, const string& key, int64_t def, int64_t* ret, string* err);
bool GetString(const json& obj, const string& key, const string& def, string* ret, string* err);
bool GetObject(const json& obj, const string& key, const json* def, const json** ret, string* err);

}  // namespace xtreaming
