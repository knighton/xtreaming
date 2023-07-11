#include "xxhash.h"

#include <cstdint>

#include "base/string.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wused-but-marked-unused"
#pragma clang diagnostic ignored "-Wzero-as-null-pointer-constant"
#pragma clang diagnostic ignored "-Wdisabled-macro-expansion"
#pragma clang diagnostic ignored "-Wused-but-marked-unused"
#pragma clang diagnostic ignored "-Wunused-macros"
#define XXH_STATIC_LINKING_ONLY
#define XXH_IMPLEMENTATION
#include "third_party/xxhash/xxhash.h"
#pragma clang diagnostic pop

namespace xtreaming {

string XXH32(const char* buffer, size_t size) {
    uint32_t hash = ::XXH32(buffer, size, 0);
    return StringPrintf("%08x", hash);
}

string XXH64(const char* buffer, size_t size) {
    uint64_t hash = ::XXH64(buffer, size, 0);
    return StringPrintf("%016lx", hash);
}

string XXH128(const char* buffer, size_t size) {
    ::XXH128_hash_t hash = ::XXH128(buffer, size, 0);
    return StringPrintf("%016lx%016lx", ((uint64_t*)&hash)[1], ((uint64_t*)&hash)[0]);
}

string XXH3_64(const char* buffer, size_t size) {
    uint64_t hash = ::XXH3_64bits_withSeed(buffer, size, 0);
    return StringPrintf("%016lx", hash);
}

string XXH3_128(const char* buffer, size_t size) {
    ::XXH128_hash_t hash = ::XXH128(buffer, size, 0);
    return StringPrintf("%016lx%016lx", ((uint64_t*)&hash)[1], ((uint64_t*)&hash)[0]);
}

}  // namespace xtreaming
