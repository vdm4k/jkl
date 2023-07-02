#pragma once
#include <stdint.h>

namespace bro::system::thread {

/*!
 * @brief thread statistic
 *
 * working thread statistic
 *
 * @ingroup thread
 */
struct statistic {
  /// how many loops was done by thread
  uint64_t _loops{0};
  /// max time spend in function
  uint64_t _max_main_function_time{0};
  /// max time spend in logic
  uint64_t _max_logic_function_time{0};
  /// how much time we spend in loop
  uint64_t _busy_time{0};
  /// how many empty loops were
  uint64_t _empty_loops{0};
};
}; // namespace bro::system::thread
