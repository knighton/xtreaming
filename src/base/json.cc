#include "json.h"

#include <cstring>

#include "base/string.h"

namespace xtreaming {

bool Contains(const json& obj, const string& key) {
    return obj.contains(key) && !obj[key].is_null();
}

bool GetBool(const json& obj, const string& key, bool* ret, string* err) {
    auto& val = obj[key];
    if (val.is_boolean()) {
        *ret = val;
        return true;
    } else {
        *err = StringPrintf("`%s` must be a bool.", key.c_str());
        return false;
    }
}

bool GetBool(const json& obj, const string& key, bool def, bool* ret, string *err) {
    if (Contains(obj, key)) {
        return GetBool(obj, key, ret, err);
    } else {
        *ret = def;
        return true;
    }
}

bool GetBool(const json& obj, const json& bak, const string& key, bool def, bool* ret,
             string* err) {
    if (Contains(obj, key)) {
        return GetBool(obj, key, ret, err);
    } else if (Contains(bak, key)) {
        return GetBool(bak, key, ret, err);
    } else {
        *ret = def;
        return true;
    }
}

bool GetInt64(const json& obj, const string& key, int64_t* ret, string* err) {
    auto& val = obj[key];
    if (val.is_number_integer()) {
        *ret = val;
        return true;
    } else {
        *err = StringPrintf("`%s` must be an int64.", key.c_str());
        return false;
    }
}

bool GetInt64(const json& obj, const string& key, int64_t def, int64_t* ret, string *err) {
    if (Contains(obj, key)) {
        return GetInt64(obj, key, ret, err);
    } else {
        *ret = def;
        return true;
    }
}

bool GetInt64(const json& obj, const json& bak, const string& key, int64_t def, int64_t* ret,
              string* err) {
    if (Contains(obj, key)) {
        return GetInt64(obj, key, ret, err);
    } else if (Contains(bak, key)) {
        return GetInt64(bak, key, ret, err);
    } else {
        *ret = def;
        return true;
    }
}

bool GetUInt32(const json& obj, const string& key, uint32_t* ret, string* err) {
    auto& val = obj[key];
    if (val.is_number_integer()) {
        int64_t i64 = val;
        if (i64 < 0 || UINT32_MAX < i64) {
            *err = StringPrintf("`%s` is out of range (must fit in uint32_t).", key.c_str());
            return false;
        }
        *ret = (uint32_t)i64;
        return true;
    } else {
        *err = StringPrintf("`%s` must be a uint32.", key.c_str());
        return false;
    }
}

bool GetUInt32(const json& obj, const string& key, uint32_t def, uint32_t* ret, string *err) {
    if (Contains(obj, key)) {
        return GetUInt32(obj, key, ret, err);
    } else {
        *ret = def;
        return true;
    }
}

bool GetUInt32(const json& obj, const json& bak, const string& key, uint32_t def, uint32_t* ret,
               string* err) {
    if (Contains(obj, key)) {
        return GetUInt32(obj, key, ret, err);
    } else if (Contains(bak, key)) {
        return GetUInt32(bak, key, ret, err);
    } else {
        *ret = def;
        return true;
    }
}

bool GetDouble(const json& obj, const string& key, double* ret, string* err) {
    auto& val = obj[key];
    if (val.is_number()) {
        *ret = val;
        return true;
    } else {
        *err = StringPrintf("`%s` must be a double.", key.c_str());
        return false;
    }
}

bool GetDouble(const json& obj, const string& key, double def, double* ret, string *err) {
    if (Contains(obj, key)) {
        return GetDouble(obj, key, ret, err);
    } else {
        *ret = def;
        return true;
    }
}

bool GetDouble(const json& obj, const json& bak, const string& key, double def, double* ret,
               string* err) {
    if (Contains(obj, key)) {
        return GetDouble(obj, key, ret, err);
    } else if (Contains(bak, key)) {
        return GetDouble(bak, key, ret, err);
    } else {
        *ret = def;
        return true;
    }
}

bool GetString(const json& obj, const string& key, string* ret, string* err) {
    auto& val = obj[key];
    if (val.is_string()) {
        *ret = val;
        return true;
    } else {
        *err = StringPrintf("`%s` must be a string.", key.c_str());
        return false;
    }
}

bool GetString(const json& obj, const string& key, const string& def, string* ret, string* err) {
    if (Contains(obj, key)) {
        return GetString(obj, key, ret, err);
    } else {
        *ret = def;
        return true;
    }
}

bool GetString(const json& obj, const json& bak, const string& key, const string& def, string* ret,
               string* err) {
    if (Contains(obj, key)) {
        return GetString(obj, key, ret, err);
    } else if (Contains(bak, key)) {
        return GetString(bak, key, ret, err);
    } else {
        *ret = def;
        return true;
    }
}

bool GetStrings(const json& obj, const string& key, vector<string>* ret, string* err) {
    auto& val = obj[key];
    if (val.is_array()) {
        ret->clear();
        ret->reserve(val.size());
        for (auto& sub : val) {
            if (sub.is_string()) {
                ret->emplace_back(sub);
            } else {
                *err = StringPrintf("`%s` must be an array of strings.", key.c_str());
                return false;
            }
        }
        return true;
    } else {
        *err = StringPrintf("`%s` must be an array of strings.", key.c_str());
        return false;
    }
}

bool GetStrings(const json& obj, const string& key, const vector<string>& def, vector<string>* ret,
                string* err) {
    if (Contains(obj, key)) {
        return GetStrings(obj, key, ret, err);
    } else {
        *ret = def;
        return true;
    }
}

bool GetStrings(const json& obj, const json& bak, const string& key, const vector<string>& def,
                vector<string>* ret, string* err) {
    if (Contains(obj, key)) {
        return GetStrings(obj, key, ret, err);
    } else if (Contains(bak, key)) {
        return GetStrings(bak, key, ret, err);
    } else {
        *ret = def;
        return true;
    }
}

bool GetObject(const json& obj, const string& key, const json** ret, string* err) {
    auto& val = obj[key];
    if (val.is_object()) {
        *ret = &val;
        return true;
    } else {
        *err = StringPrintf("`%s` must be an object.", key.c_str());
        return false;
    }
}

bool GetObject(const json& obj, const string& key, const json* def, const json** ret,
               string* err) {
    if (Contains(obj, key)) {
        return GetObject(obj, key, ret, err);
    } else {
        *ret = def;
        return true;
    }
}

bool GetObject(const json& obj, const json& bak, const string& key, const json* def,
               const json** ret, string* err) {
    if (Contains(obj, key)) {
        return GetObject(obj, key, ret, err);
    } else if (Contains(bak, key)) {
        return GetObject(bak, key, ret, err);
    } else {
        *ret = def;
        return true;
    }
}

bool GetBytes(const json& obj, const string& key, int64_t* ret, string* err) {
    auto& val = obj[key];
    if (val.is_number_integer()) {
        *ret = val;
        return true;
    } else if (val.is_string()) {
        if (val.empty()) {
            *err = StringPrintf("Bytes string `%s` is empty.", key.c_str());
            return false;
        }
        return ParseBytes(val, ret, err);
    } else {
        *err = StringPrintf("`%s` must be a number of bytes (eg, `7` or `7kb`).", key.c_str());
        return false;
    }
}

bool GetBytes(const json& obj, const string& key, int64_t def, int64_t* ret, string* err) {
    if (Contains(obj, key)) {
        return GetBytes(obj, key, ret, err);
    } else {
        *ret = def;
        return true;
    }
}

bool GetBytes(const json& obj, const json& bak, const string& key, int64_t def, int64_t* ret,
              string* err) {
    if (Contains(obj, key)) {
        return GetBytes(obj, key, ret, err);
    } else if (Contains(bak, key)) {
        return GetBytes(bak, key, ret, err);
    } else {
        *ret = def;
        return true;
    }
}

bool GetCount(const json& obj, const string& key, int64_t* ret, string* err) {
    auto& val = obj[key];
    if (val.is_number_integer()) {
        *ret = val;
        return true;
    } else if (val.is_string()) {
        if (val.empty()) {
            *err = StringPrintf("Count string `%s` is empty.", key.c_str());
            return false;
        }
        return ParseCount(val, ret, err);
    } else {
        *err = StringPrintf("`%s` must be a number of items (eg, `7` or `7k`).", key.c_str());
        return false;
    }
}

bool GetCount(const json& obj, const string& key, int64_t def, int64_t* ret, string* err) {
    if (Contains(obj, key)) {
        return GetCount(obj, key, ret, err);
    } else {
        *ret = def;
        return true;
    }
}

bool GetCount(const json& obj, const json& bak, const string& key, int64_t def, int64_t* ret,
              string* err) {
    if (Contains(obj, key)) {
        return GetCount(obj, key, ret, err);
    } else if (Contains(bak, key)) {
        return GetCount(bak, key, ret, err);
    } else {
        *ret = def;
        return true;
    }
}

bool GetTime(const json& obj, const string& key, double* ret, string* err) {
    auto& val = obj[key];
    if (val.is_number()) {
        *ret = val;
        return true;
    } else if (val.is_string()) {
        if (val.empty()) {
            *err = StringPrintf("Time string `%s` is empty.", key.c_str());
            return false;
        }
        return ParseTime(val, ret, err);
    } else {
        *err = StringPrintf("`%s` must be an interval of time (eg, `7.0` or `7.0s`).", key.c_str());
        return false;
    }
}

bool GetTime(const json& obj, const string& key, double def, double* ret, string* err) {
    if (Contains(obj, key)) {
        return GetTime(obj, key, ret, err);
    } else {
        *ret = def;
        return true;
    }
}

bool GetTime(const json& obj, const json& bak, const string& key, double def, double* ret,
             string* err) {
    if (Contains(obj, key)) {
        return GetTime(obj, key, ret, err);
    } else if (Contains(bak, key)) {
        return GetTime(bak, key, ret, err);
    } else {
        *ret = def;
        return true;
    }
}

}  // namespace xtreaming
