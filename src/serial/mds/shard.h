#pragma once

#include <set>
#include <string>
#include <vector>

#include "base/json.h"
#include "serial/base/shard.h"

using std::set;
using std::string;
using std::vector;

namespace xtreaming {

struct MDSColumn {
    string name;
    string type;
    int64_t num_bytes;
};

class MDSShard : public Shard {
  public:
    const FileInfo* raw_data() const { return raw_data_; }
    const FileInfo* zip_data() const { return zip_data_; }
    const vector<MDSColumn>& columns() const { return columns_; }

    virtual ~MDSShard() override;

    void Init(int64_t stream_id, const set<string>& hash_algos, int64_t num_samples,
              int64_t size_limit, const string& zip_algo, FileInfo* raw_data, FileInfo* zip_data,
              const vector<MDSColumn>& columns);

    void InitFromJSON(int64_t stream_id, const json& obj);

  protected:
    FileInfo* raw_data_{nullptr};
    FileInfo* zip_data_{nullptr};
    vector<MDSColumn> columns_;
};

}  // namespace xtreaming
