#include "fast.h"

#include "base/string.h"

namespace xtreaming {

bool Fast::Init(const json& obj, string* err) {
    algo_ = "fast";

    return Determiner::Init(obj, err);
}

Fast* Fast::New(const json& obj, string* err) {
    auto ret = new Fast;
    if (!ret->Init(obj, err)) {
        delete ret;
        return nullptr;
    }

    return ret;
}

namespace {

bool AreCodivisible(int64_t can, int64_t phy) {
    if (can < phy) {
        return !(phy % can);
    } else if (phy < can) {
        return !(can % phy);
    } else {
        return true;
    }
}

}  // namespace

bool Fast::Determine(int64_t num_canonical_nodes, int64_t batch_size, int64_t num_physical_nodes,
                     int64_t ranks_per_node, int64_t workers_per_rank, int64_t epoch_size,
                     int64_t sample_offset, xt::xarray<int64_t>* ids, string* err) {
    if (num_canonical_nodes <= 0) {
        *err = StringPrintf("`num_canonical_nodes` must be positive, but got: %ld.",
                            num_canonical_nodes);
        return false;
    }

    if (batch_size <= 0) {
        *err = StringPrintf("`batch_size` must be positive, but got: %ld.", batch_size);
        return false;
    }

    if (num_physical_nodes <= 0) {
        *err = StringPrintf("`num_physical_nodes` must be positive, but got: %ld.",
                            num_physical_nodes);
        return false;
    }

    if (ranks_per_node <= 0) {
        *err = StringPrintf("`ranks_per_node` must be positive, but got: %ld.", ranks_per_node);
        return false;
    }

    if (workers_per_rank <= 0) {
        *err = StringPrintf("`workers_per_rank` must be positive, but got: %ld.",
                            workers_per_rank);
        return false;
    }

    if (epoch_size <= 0) {
        *err = StringPrintf("`epoch_size` must be positive, but got: %ld.", epoch_size);
        return false;
    }

    if (sample_offset < 0) {
        *err = StringPrintf("`sample_offset` must be non-negative, but got: %ld.", sample_offset);
        return false;
    }

    if (!AreCodivisible(num_canonical_nodes, num_physical_nodes)) {
        *err = StringPrintf("Canonical and physical nodes must be an even ratio of each other, "
                            "otherwise striping slices of shards over nodes may cause all nodes "
                            "to download all shards. Got `num_canonical_nodes` = %ld, "
                            "`num_physical_nodes` = %ld.", num_canonical_nodes,
                            num_physical_nodes);
        return false;
    }

    if (epoch_size <= sample_offset) {
        *err = StringPrintf("Attempted to resume further into the epoch than it has samples: "
                            "`epoch_size` = %ld, `sample_offset` = %ld.", epoch_size,
                            sample_offset);
        return false;
    }

    if (sample_offset % num_physical_nodes) {
        sample_offset -= sample_offset % num_physical_nodes;
    }

    // Divide the full dataset sample range into a sample range per canonical node.
    int64_t samples_per_canonical_node = (epoch_size + num_canonical_nodes - 1) /
        num_canonical_nodes;
    int64_t padding = 0;
    int64_t node_ratio = 0;
    if (num_canonical_nodes < num_physical_nodes) {
        node_ratio = num_physical_nodes / num_canonical_nodes;
        int64_t overflow = samples_per_canonical_node % node_ratio;
        if (overflow) {
            padding = node_ratio - overflow;
        }
    }
    int64_t padded_samples_per_canonical_node = samples_per_canonical_node + padding;

    // Create the initial sample ID matrix.
    //
    // ids: (canonical nodes, padded samples per canonical node).
    *ids = xt::arange(num_canonical_nodes * padded_samples_per_canonical_node);
    *ids = ids->reshape({num_canonical_nodes, padded_samples_per_canonical_node});

    // Adjust row offsets to ignore the padding.
    //
    // row_offsets: (canonical nodes, 1).
    xt::xarray<int64_t> row_offsets = xt::arange(num_canonical_nodes) * padding;
    row_offsets = xt::expand_dims(row_offsets, 1);
    *ids -= row_offsets;

    // Reconfigure where each row starts iterating for irregular-sized rows.
    //
    // row_starts: (canonical nodes, 1).
    xt::xarray<int64_t> row_starts = xt::arange(num_canonical_nodes) * epoch_size /
        num_canonical_nodes;
    row_starts = xt::expand_dims(row_starts, 1);
    *ids += row_starts - xt::view(*ids, xt::all(), xt::range(0, 1));

    // For short rows (length not evenly divisible), repeat the last ID to get even length.
    //
    // row_stops: (canonical nodes, 1).
    xt::xarray<int64_t> row_stops = xt::arange(1L, 1 + num_canonical_nodes) * epoch_size /
        num_canonical_nodes;
    row_stops = xt::expand_dims(row_stops, 1);
    xt::xarray<int64_t> are_rows_short = (row_stops - row_starts) < samples_per_canonical_node;
    xt::view(*ids, xt::all(), xt::range(samples_per_canonical_node - 1,
             samples_per_canonical_node)) -= are_rows_short;

    // If padding was needed, repeat samples to populate it.
    if (padding) {
        auto& x = padded_samples_per_canonical_node;
        int64_t y = x - padding - node_ratio + 1;
        xt::view(*ids, xt::all(), xt::range(x - padding, x)) =
            xt::view(*ids, xt::all(), xt::range(y - padding, y));
    }

    // Drop samples that have already been seen.
    //
    // ids: (canonical nodes, samples per node).
    *ids = xt::view(*ids, xt::all(), xt::range(sample_offset / num_canonical_nodes,
                                               padded_samples_per_canonical_node));

    // Map from canonical nodes onto physical nodes.
    //
    // ids: (physical nodes, samples per node).
    *ids = xt::transpose(*ids);
    ids->reshape({-1L, num_physical_nodes});
    *ids = xt::transpose(*ids);

    // Interleave the node sample ranges over each node's ranks, padding by repeating the last
    // sample.
    //
    // ids: (physical nodes, samples per rank, ranks per node).
    {
        int64_t overflow = ids->shape(1) % ranks_per_node;
        if (overflow) {
            int64_t underflow = ranks_per_node - overflow;
            int64_t end = ids->shape(1) - ranks_per_node + 1;
            xt::xarray<int64_t> last = xt::view(*ids, xt::all(), xt::range(end - underflow, end));
            *ids = xt::concatenate(xtuple(*ids, last), 1);
        }
        ids->reshape({num_physical_nodes, -1L, ranks_per_node});
    }

    // Pad with -1 adequately for reshaping across workers.
    //
    // ids: (physical nodes, samples per rank, ranks per node).
    {
        int64_t samples_per_rank = ids->shape(1);
        int64_t rank_batch_size = workers_per_rank * batch_size;
        int64_t ceil_batches_per_rank = (samples_per_rank + rank_batch_size - 1) / rank_batch_size;
        int64_t ceil_samples_per_rank = ceil_batches_per_rank * rank_batch_size;
        int64_t overflow = ceil_samples_per_rank - samples_per_rank;
        if (overflow) {
            xt::xarray<int64_t> pad = -xt::ones<int64_t>({num_physical_nodes, overflow,
                                                          ranks_per_node});
            *ids = xt::concatenate(xtuple(*ids, pad), 1);
        }
    }

    // Interleave each rank's padded samples across its workers.
    //
    // ids: (physical nodes, ranks per node, workers per rank, batches per worker, batch size).
    *ids = xt::transpose(*ids, {0, 2, 1});
    ids->reshape({num_physical_nodes, ranks_per_node, -1L, workers_per_rank, batch_size});
    *ids = xt::transpose(*ids, {0, 1, 3, 2, 4});

    return true;
}

bool Fast::Determine(int64_t num_physical_nodes, int64_t ranks_per_node, int64_t workers_per_rank,
                     int64_t epoch_size, int64_t sample_offset, xt::xarray<int64_t>* ids,
                     string* err) {
    return Determine(num_canonical_nodes_, batch_size_, num_physical_nodes, ranks_per_node,
                     workers_per_rank, epoch_size, sample_offset, ids, err);
}

}  // namespace xtreaming
