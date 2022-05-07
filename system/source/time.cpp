#include <sys/time.h>
#include <system/time.h>

#include <cerrno>
#include <chrono>
#include <cstdint>
#include <mutex>
#include <thread>

namespace jkl::system::time {

static uint64_t g_tsc_resolution_in_ms{0};
static uint64_t const g_usec_per_sec{1'000'000};
static uint64_t g_start_timeofday{0};
static uint64_t g_start_tsc{0};

void init_timestamp() {
  static std::once_flag flag;
  std::call_once(flag, []() {
    size_t const avg = 10;
    std::chrono::microseconds const synchro_sleep{std::chrono::milliseconds(1)};
    for (size_t i = 0; i < avg; ++i) {
      uint64_t before_sleep = read_tsc();
      sleep(synchro_sleep);
      uint64_t after_sleep = read_tsc();
      g_tsc_resolution_in_ms += after_sleep - before_sleep;
    }

    g_tsc_resolution_in_ms /= avg;
    struct timeval tv {};
    gettimeofday(&tv, nullptr);

    g_start_timeofday = tv.tv_sec * g_usec_per_sec + tv.tv_usec;
    g_start_tsc = read_tsc();
  });
}

std::chrono::microseconds get_timestamp() noexcept {
  uint64_t delta_tsc = read_tsc() - g_start_tsc;
  uint64_t delta_usec = uint64_t(static_cast<long double>(delta_tsc) *
                                 g_usec_per_sec / g_tsc_resolution_in_ms);
  return std::chrono::microseconds(g_start_timeofday + delta_usec);
}

uint64_t read_tsc() noexcept {
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

void sleep(std::chrono::microseconds const& time) noexcept {
  if (time.count()) {
    std::this_thread::sleep_for(time);
  } else {
    std::this_thread::yield();
  }
}

}  // namespace jkl::system::time
