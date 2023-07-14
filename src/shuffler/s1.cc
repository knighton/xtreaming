#include "s1.h"

using std::make_pair;
using std::random_device;
using std::default_random_engine;
using std::shuffle;

namespace xtreaming {

bool S1::Init(const json& obj, string *err) {
    return Shuffler::Init(obj, err);
}

void S1::BreakSpansOverNodes(int64_t num_samples, int64_t num_nodes,
                             vector<pair<int64_t, int64_t>>* spans,
                             vector<pair<int64_t, int64_t>>* meta_spans) {
    int64_t meta_begin = 0;
    int64_t span_index = 0;
    int64_t samples_so_far = 0;

    vector<pair<int64_t, int64_t>> out_spans;
    meta_spans->clear();

    for (int64_t i = 0; i < num_nodes; ++i) {
        auto meta_end = num_samples * (i + 1) / num_nodes;

        while (true) {
            if (span_index == spans->size()) {
                break;
            }

            auto& span = (*spans)[span_index];
            auto samples_this_span = span.second - span.first;
            if (meta_end < samples_so_far + samples_this_span) {
                if (samples_so_far < meta_end) {
                    auto split = meta_end - samples_so_far;
                    auto new_span = make_pair(span.first, span.first + split);
                    out_spans.emplace_back(new_span);
                    (*spans)[span_index].first += split;
                    samples_so_far += split;
                }
                break;
            }

            out_spans.emplace_back(span);
            ++span_index;
            samples_so_far += samples_this_span;
        }

        auto meta_span = make_pair(meta_begin, out_spans.size());
        meta_spans->emplace_back(meta_span);
        meta_begin = out_spans.size();
    }

    spans->swap(out_spans);
}

void S1::ShuffleShards(const vector<int64_t>& shard_sizes, int64_t num_nodes, uint32_t seed,
                       int64_t epoch, int64_t* num_samples, vector<pair<int64_t, int64_t>>* spans,
                       vector<pair<int64_t, int64_t>>* meta_spans,
                       default_random_engine* epoch_rng, Logger* logger) {
    auto scope = logger->Scope("iter/shuffle/get/shuffle_shards");

    // Create each shard's sample ID span (begin, end excl).
    {
        auto scope2 = logger->Scope("iter/shuffle/get/shuffle_shards/create_spans");
        spans->clear();
        spans->reserve(shard_sizes.size());
        *num_samples = 0;
        for (auto& shard_size : shard_sizes) {
            spans->emplace_back(make_pair(*num_samples, *num_samples + shard_size));
            *num_samples += shard_size;
        }
    }

    // Generate the initial ordering of shards, which is fixed over an entire training run.
    {
        auto scope2 = logger->Scope("iter/shuffle/get/shuffle_shards/shuffle_spans");
        random_device random;
        default_random_engine run_rng(random());
        run_rng.seed(seed);
        shuffle(spans->begin(), spans->end(), run_rng);
    }

    // Break the shard spans at node boundaries (modifies spans in-place).
    {
        auto scope2 = logger->Scope("iter/shuffle/get/shuffle_shards/break_spans_over_nodes");
        BreakSpansOverNodes(*num_samples, num_nodes, spans, meta_spans);
    }

    // Shuffle the span ordering within each node, uniquely to this epoch.
    {
        auto scope2 = logger->Scope("iter/shuffle/get/shuffle_shards/shuffle_spans_intra_node");
        epoch_rng->seed(seed + (uint32_t)epoch);
        for (auto& meta_span : *meta_spans) {
            auto& begin = meta_span.first;
            auto& end = meta_span.second;
            shuffle(&(*spans)[begin], &(*spans)[end], *epoch_rng);
        }
    }
}

}  // namespace xtreaming
