// SPDX-License-Identifier: BSD-3-Clause
#pragma once
#include <stdint.h>

namespace jkl::system {

/*!
 * @brief thread statistic
 *
 * statistic about working thread
 *
 * @ingroup thread
 */
struct thread_statistic {
  /// how many cycles thread did
  uint64_t _cycles{0};
  /// max time spend in function
  uint64_t _max_main_function_time{0};
  /// max time spend in logic
  uint64_t _max_logic_function_time{0};
  /// how much time we spend in loop
  uint64_t _busy_time{0};
};
};  // namespace jkl::system
