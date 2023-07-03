#include "json.h"

#include <cstring>

#include "base/string.h"

namespace xtreaming {

bool GetBool(const json& obj, const string& key, bool def, bool* ret, string* err) {
    if (!obj.contains(key)) {
        *ret = def;
        return true;
    }

    auto& val = obj[key];
    if (val.is_boolean()) {
        *ret = val;
        return true;
    } else {
        *err = StringPrintf("`%s` must be a boolean.", key.c_str());
        return false;
    }
}

bool GetInt64(const json& obj, const string& key, int64_t def, int64_t* ret, string* err) {
    if (!obj.contains(key)) {
        *ret = def;
        return true;
    }

    auto& val = obj[key];
    if (val.is_number_integer()) {
        *ret = val;
        return true;
    } else {
        *err = StringPrintf("`%s` must be an integer.", key.c_str());
        return false;
    }
}

bool GetUInt32(const json& obj, const string& key, uint32_t def, uint32_t* ret, string* err) {
    int64_t i64;
    if (!GetInt64(obj, "sampling_seed", 1337, &i64, err)) {
        *ret = def;
        return true;
    }

    if (i64 < 0 || UINT32_MAX < i64) {
        *err = StringPrintf("`%s` is out of range (must fit in uint32_t).", key.c_str());
        return false;
    }

    *ret = (uint32_t)i64;
    return true;
}

bool GetNumBytes(const json& obj, const string& key, int64_t def, int64_t* ret, string* err) {
    if (!obj.contains(key)) {
        *ret = def;
        return true;
    }

    auto& val = obj[key];
    if (val.is_number_integer()) {
        *ret = val;
        return true;
    } else if (val.is_string()) {
        if (val.empty()) {
            *err = StringPrintf("Bytes string `%s` is empty.", key.c_str());
            return false;
        }
        return ParseNumBytes(val, ret, err);
    } else {
        *err = StringPrintf("`%s` must be either an integer or a bytes string.", key.c_str());
        return false;
    }
}

bool GetString(const json& obj, const string& key, const string& def, string* ret, string* err) {
    if (!obj.contains(key)) {
        *ret = def;
        return true;
    }
    
    auto& val = obj[key];
    if (val.is_string()) {
        *ret = val;
        return true;
    } else {
        *err = StringPrintf("`%s` must be a string.", key.c_str());
        return false;
    }
}

}  // namespace xtreaming
