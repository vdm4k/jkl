#pragma once
#include <sched.h>
#include <sys/time.h>
#include <time.h>

#include <cerrno>
#include <chrono>
#include <cstdint>

namespace jkl::system::time {

static inline uint64_t get_time() noexcept {
  union {
    uint64_t _tsc;
    struct {
      uint32_t _lo;
      uint32_t _hi;
    };
  } rdtsc;

  asm volatile("rdtsc" : "=a"(rdtsc._lo), "=d"(rdtsc._hi));
  return rdtsc._tsc;
}

static inline void sleep(std::chrono::microseconds const& time) {
  if (time.count()) {
    std::chrono::seconds const sec{
        std::chrono::duration_cast<std::chrono::seconds>(time)};
    timespec requested{sec.count(), (time - sec).count() * 1000};
    timespec remaining{0, 0};
    while (nanosleep(&requested, &remaining) == -1 && EINTR == errno)
      requested = remaining;
  } else {
    sched_yield();
  }
}

}  // namespace jkl::system::time
