#include "time.h"

#include <chrono>

namespace xtreaming {

int64_t NanoTime() {
    auto now = std::chrono::system_clock::now().time_since_epoch();
    return std::chrono::duration_cast<std::chrono::nanoseconds>(now).count();
}

}  // namespace xtreaming
