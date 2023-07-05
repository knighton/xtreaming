#include "dataset.h"

namespace xtreaming {

bool Dataset::Init(const json& obj, string* err) {
    json empty_obj;

    // Get `stream`.
    const json* all;
    if (!GetObject(obj, "stream", &empty_obj, &all, err)) {
        return false;
    }

    // Get `streams`.
    const json* streams;
    if (!GetObject(obj, "streams", &empty_obj, &streams, err)) {
        return false;
    }

    // Init streams from config.
    if (streams->empty()) {
        // `stream` is taken as the single stream, as `streams` is not provided.
        streams_.resize(1);
        auto& stream = streams_[0];
        if (!stream.Init(*all, *all, err)) {
            return false;
        }
    } else {
        // `stream` is taken as defaults to `streams`, as `streams` is non-empty.
        if (Contains(*all, "remote") || Contains(*all, "local") || Contains(*all, "split")) {
            *err = "If providing `streams`, the top-level `stream` object is used as defaults for "
                "`streams`, and its `remote`, `local`, and `split` fields are meaningless.";
            return false;
        }
        streams_.resize(streams->size());
        int64_t i = 0;
        for (auto it : streams->items()) {
            if (!streams_[i].Init(it.value(), *all, err)) {
                return false;
            }
            ++i;
        }
    }

    // Cross-check stream weighting scheme.
    if (!Stream::CrossCheckWeights(streams_, err)) {
        return false;
    }

    return true;
}

}  // namespace xtreaming
