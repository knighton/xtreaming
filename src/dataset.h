#pragma once

#include <string>
#include <vector>

#include "base/json.h"
#include "stream.h"

using std::string;
using std::vector;

namespace xtreaming {

class Dataset {
  public:
    bool Init(const json& obj, string* error);

  private:
    vector<Stream> streams_;
};

}  // namespace xtreaming
