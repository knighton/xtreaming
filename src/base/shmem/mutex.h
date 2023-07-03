#pragma once

#include <pthread.h>

namespace xtreaming {

// Mutex that can work between processes, by living in shared memory.
class SharedMutex {
  public:
    // Initialize the lock.
    void Init();

    // Acquire the lock.
    void Lock();

    // Release the lock.
    void Unlock();

  private:
    pthread_mutex_t mux_;
};

}  // namespace xtreaming
