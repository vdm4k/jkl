#pragma once
#include <chrono>
#include <optional>

namespace bro::system::thread {

/*!
 * @brief thread configuration
 *
 * need to configure thread with different behaviour
 *
 * @ingroup thread
 */
struct config {
  /// sleep every n loop
  std::optional<size_t> _call_sleep_on_n_loop;
  /// sleep every n empty loop in a row ( empty loop = main function return 0 or false )
  std::optional<size_t> _call_sleep_on_n_empty_loop_in_a_row;
  /// how often call sleep time measurement
  std::optional<std::chrono::microseconds> _call_sleep;
  /// how long thread will sleep
  std::optional<std::chrono::microseconds> _sleep;
  /// how often need call logic function
  std::optional<std::chrono::microseconds> _call_logic_fun;
  /// how often need call logic function in cycles
  std::optional<size_t> _call_logic_on_n_loop;
  /// flush statistic
  std::optional<std::chrono::microseconds> _flush_statistic;
};

}; // namespace bro::system::thread
