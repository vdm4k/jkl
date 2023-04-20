#pragma once
#include <chrono>
#include <cstdint>

namespace bro::system::time {

/** @defgroup time time
 *  @{
 */

/**
 * init timestamp
 *
 * this function must be called before we start to work with get_time function
 * Need call only once
 */
void init_timestamp();

/**
 * get current timestamp
 *
 * Need call init_timestamp before first call get_timestamp
 */
std::chrono::microseconds get_timestamp() noexcept;

/**
 * read time stamp counter
 *
 */
uint64_t read_tsc() noexcept;

/**
 * call sleep for calling thread
 *
 * same as std sleep_for except call yield if time.count() == 0
 *
 * @param time sleep time
 */
void sleep(std::chrono::microseconds const &time);

/** @} */ // end of time

} // namespace bro::system::time
