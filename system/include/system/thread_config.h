#pragma once
#include <chrono>
#include <optional>

namespace jkl::system {

/*!
 * @brief thread configuration
 *
 * need to configure thread with different behaviour
 *
 * @ingroup thread
 */
struct thread_config {
  /// how often call sleep in cycles
  std::optional<size_t> _to_sleep_cycles;
  /// how often call sleep time measurement
  std::optional<std::chrono::microseconds> _to_sleep_time;
  /// how long thread will sleep
  std::optional<std::chrono::microseconds> _sleep;
  /// how often need call logic function
  std::optional<std::chrono::microseconds> _logic_call_time;
  /// how often need call logic function in cycles
  std::optional<size_t> _logic_call_cycles;
  /// flush statistic
  std::optional<std::chrono::microseconds> _flush_statistic;
};

};  // namespace jkl::system
