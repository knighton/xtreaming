#include "mutex.h"

namespace xtreaming {

void SharedMutex::Init() {
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(&mux_, &attr);
}

void SharedMutex::Lock() {
    pthread_mutex_lock(&mux_);
}

void SharedMutex::Unlock() {
    pthread_mutex_unlock(&mux_);
}

}  // namespace xtreaming
