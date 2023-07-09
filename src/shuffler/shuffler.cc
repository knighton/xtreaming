#include "shuffler.h"

namespace xtreaming {

Shuffler::~Shuffler() {
}

bool Shuffler::Init(const json& obj, string* err) {
    return GetUInt32(obj, "seed", 1337, &seed_, err);
}

}  // namespace xtreaming
