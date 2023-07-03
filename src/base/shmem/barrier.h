#pragma once

#include <pthread.h>

namespace xtreaming {

// Barrier that can work between processes, by living in shared memory.
class SharedBarrier {
  public:
    // Initialize the barrier.
    void Init(int count);

    // Wait until all participants have reached the barrier.
    void Wait();

  private:
    pthread_barrier_t barrier_;
};

}  // namespace xtreaming
