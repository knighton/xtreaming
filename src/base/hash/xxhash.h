#pragma once

#include <string>

using std::string;

namespace xtreaming {

// Returns hex digest.
string XXH32(const char* buffer, size_t size);
string XXH64(const char* buffer, size_t size);
string XXH128(const char* buffer, size_t size);
string XXH3_64(const char* buffer, size_t size);
string XXH3_128(const char* buffer, size_t size);

}  // namespace xtreaming
