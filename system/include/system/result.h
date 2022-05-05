#pragma once

namespace jkl {
namespace system {

/** @defgroup result
 *  @{
 */

/**
 * holding status of operation.
 *
 * simple wrapper to return errors.
 */
struct result {
  result() = default;
  result(result const& res) = default;
  result(result&& res) = default;
  result& operator=(result const& res) = default;
  result& operator=(result&& res) = default;
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
  char const* _name = nullptr;
};

/**
 * holding an operation result.
 *
 */
template <typename Tp>
struct expected : result {
  using value_type = Tp;
  expected() = default;

  expected(result const& res) : result(res) {}

  value_type _value;
};

/** @} */  // end of result

}  // namespace system
}  // namespace jkl
