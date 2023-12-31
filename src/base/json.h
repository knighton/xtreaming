#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "third_party/json/json.hpp"

using std::string;
using std::vector;

namespace xtreaming {

using json = nlohmann::json;

// Safe JSON field getters with defaults.
//
// Returns whether succeeded. Only sets `ret` on success. Only sets `err` on failure.

bool Contains(const json& obj, const string& key);

// Get bool.
bool GetBool(const json& obj, const string& key, bool* ret, string* err);
bool GetBool(const json& obj, const string& key, bool def, bool* ret, string* err);
bool GetBool(const json& obj, const json& bak, const string& key, bool def, bool* ret,
             string* err);

// Get int64.
bool GetInt64(const json& obj, const string& key, int64_t* ret, string* err);
bool GetInt64(const json& obj, const string& key, int64_t def, int64_t* ret, string* err);
bool GetInt64(const json& obj, const json& bak, const string& key, int64_t def, int64_t* ret,
              string* err);

// Get uint32.
bool GetUInt32(const json& obj, const string& key, uint32_t* ret, string* err);
bool GetUInt32(const json& obj, const string& key, uint32_t def, uint32_t* ret, string* err);
bool GetUInt32(const json& obj, const json& bak, const string& key, uint32_t def, uint32_t* ret,
               string* err);

// Get double.
bool GetDouble(const json& obj, const string& key, double* ret, string* err);
bool GetDouble(const json& obj, const string& key, double def, double* ret, string* err);
bool GetDouble(const json& obj, const json& bak, const string& key, double def, double* ret,
               string* err);

// Get string.
bool GetString(const json& obj, const string& key, string* ret, string* err);
bool GetString(const json& obj, const string& key, const string& def, string* ret, string* err);
bool GetString(const json& obj, const json& bak, const string& key, const string& def, string* ret,
               string* err);

// Get strings.
bool GetStrings(const json& obj, const string& key, vector<string>* ret, string* err);
bool GetStrings(const json& obj, const string& key, const vector<string>& def, vector<string>* ret,
                string* err);
bool GetStrings(const json& obj, const json& bak, const string& key, const vector<string>& def,
                vector<string>* ret, string* err);

// Get object.
bool GetObject(const json& obj, const string& key, const json** ret, string* err);
bool GetObject(const json& obj, const string& key, const json* def, const json** ret, string* err);
bool GetObject(const json& obj, const json& bak, const string& key, const json* def,
               const json** ret, string* err);

// Get bytes (`7`, `7kb`, `7mb`, `7gb`, `7tb`, etc).
bool GetBytes(const json& obj, const string& key, int64_t* ret, string* err);
bool GetBytes(const json& obj, const string& key, int64_t def, int64_t* ret, string* err);
bool GetBytes(const json& obj, const json& bak, const string& key, int64_t def, int64_t* ret,
              string* err);

// Get count (`7`, `7k`, `7m`, `7b`, `7t`, etc).
bool GetCount(const json& obj, const string& key, int64_t* ret, string* err);
bool GetCount(const json& obj, const string& key, int64_t def, int64_t* ret, string* err);
bool GetCount(const json& obj, const json& bak, const string& key, int64_t def, int64_t* ret,
              string* err);

// Get time (`7`, `7s`, `7m`, `7m7s`, etc).
bool GetTime(const json& obj, const string& key, double* ret, string* err);
bool GetTime(const json& obj, const string& key, double def, double* ret, string* err);
bool GetTime(const json& obj, const json& bak, const string& key, double def, double* ret,
             string* err);

}  // namespace xtreaming
