// SPDX-License-Identifier: BSD-3-Clause
#pragma once

namespace jkl {
namespace system {

/** @defgroup result result of function execution
 *  @{
 */

/**
 * holding status of operation.
 *
 * simple wrapper to return errors.
 */
struct result {
  /**
  * default ctor
  */
  result() = default;
  /**
  * default copy ctor
  */
  result(result const& res) = default;
  /**
  * default move ctor
  */
  result(result&& res) = default;
  /**
  * default assign operator
  */
  result& operator=(result const& res) = default;
  /**
  * default assign move operator
  */
  result& operator=(result&& res) = default;

  /**
  * ctor from error name
  * 
  * @param name error name. Not own this string
  */
  explicit result(char const* name) : _name(name){};

  /**
   * is result good
   *
   * @return true if have no error
   */
  bool is_ok() const noexcept { return _name == nullptr; }

  /**
   * explicit cast to bool
   *
   * @return true if have no error
   */
  explicit operator bool() const noexcept { return is_ok(); }

  /**
   * get pointer on error description or nullptr
   *
   * @return error
   */
  char const* get_name() const noexcept { return _name; }

 private:
  char const* _name = nullptr;     ///< error name - we not own this string
};

/**
 * holding an operation result. *
 */
template <typename Tp>
struct expected : result {

  using value_type = Tp;  ///< value type

  /**
  * default ctor
  */
  expected() = default;

  /**
  * ctor from result
  * 
  * @param res operation result 
  */
  expected(result const& res) : result(res) {}

  value_type _value;   ///< value
};

/** @} */  // end of result

}  // namespace system
}  // namespace jkl
