#include "stream.h"

#include "base/string.h"

namespace xtreaming {
namespace {

string DeriveLocal(const string& remote, const string& split) {
    return "/tmp/TODO";  // XXX
}

}  // namespace

bool Stream::Init(const json& obj, const json& bak, string* err) {
    // Init paths:

    if (!GetString(obj, "remote", "", &remote_, err)) {
        return false;
    }

    if (!GetString(obj, "local", "", &local_, err)) {
        return false;
    }

    if (!GetString(obj, "split", "", &split_, err)) {
        return false;
    }

    if (remote_.empty() && local_.empty()) {
        *err = "At least one of `remote` and `local` must be provided.";
        return false;
    }

    if (local_.empty()) {
        local_ = DeriveLocal(remote_, split_);
    }

    // Init behavior:

    if (!GetInt64(obj, bak, "download_retry", 3, &download_retry_, err)) {
        return false;
    }
    if (download_retry_ < 0) {
        *err = StringPrintf("`download_retry` must be non-negative (got: %ld).", download_retry_);
        return false;
    }

    if (!GetTime(obj, bak, "download_timeout", 60.0, &download_timeout_, err)) {
        return false;
    }
    if (download_timeout_ <= 0) {
        *err = StringPrintf("`download_timeout` must be positive (got: %.lf).", download_timeout_);
        return false;
    }

    if (!GetStrings(obj, bak, "hash_algos", {}, &hash_algos_, err)) {
        return false;
    }

    if (!GetBool(obj, bak, "non_hashed_ok", true, &non_hashed_ok_, err)) {
        return false;
    }

    if (!GetBool(obj, bak, "keep_zip", false, &keep_zip_, err)) {
        return false;
    }

    safe_keep_zip_ = keep_zip_ || (remote_ == local_);

    // Init weights:

    if (!GetDouble(obj, "proportion", -1, &proportion_, err)) {
        return false;
    }

    if (!GetDouble(obj, "repeat", -1, &repeat_, err)) {
        return false;
    }

    if (!GetInt64(obj, "choose", -1L, &choose_, err)) {
        return false;
    }

    int num_weights = (0 <= proportion_) + (0 <= repeat_) + (0 <= choose_);
    if (1 < num_weights) {
        *err = "At most one type of weight (`proportion`, `repeat`, and `choose`) can be used.";
        return false;
    }

    // Init accounting:

    shard_offset_ = -1L;
    num_shards_ = -1L;
    sample_offset_ = -1L;
    num_samples_ = -1L;

    return true;
}

}  // namespace xtreaming
