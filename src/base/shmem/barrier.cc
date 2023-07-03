#include "barrier.h"

namespace xtreaming {

void SharedBarrier::Init(int count) {
    pthread_barrierattr_t attr;
    pthread_barrierattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
    pthread_barrier_init(&barrier_, &attr, count);
}

void SharedBarrier::Wait() {
    pthread_barrier_wait(&barrier_);
}

}  // namespace xtreaming
