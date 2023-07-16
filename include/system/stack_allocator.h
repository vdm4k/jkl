#pragma once

#include <cstddef>
namespace bro::system {

/** @defgroup stack_allocator stack_allocator
 *  @{
 */

/**
 * Allocate array of values. On stack if size < preallocated size
 *
 */
template<typename ValueType, size_t Size>
struct stack_allocator {
   /**
   * default ctor
   */
    explicit stack_allocator() noexcept = default;

   /**
   * ctor with size
   */
    explicit stack_allocator(size_t size) : _size(size) {
        if(_size > Size)
            _begin = new ValueType[size];
    }

   /**
   * dtor
   */
    ~stack_allocator() {
        if(_begin != _array)
            delete []_begin;
    }

   /**
   * default copy ctor
   */
    stack_allocator(stack_allocator const &res) = delete;

   /**
   * default move ctor
   */
    stack_allocator(stack_allocator &&res) = delete;

   /**
   * default assign operator
   */
    stack_allocator &operator=(stack_allocator const &res) = delete;

   /**
   * default assign move operator
   */
    stack_allocator &operator=(stack_allocator &&res) = delete;

   /**
   * get size of current array
   * @return _size
   */
    size_t get_size() const noexcept {
        return _size;
    }

    /**
   * get pointer on array
   *
   * @return pointer on array
   */
    ValueType * get_array() const noexcept { return _begin; }

private:
    ValueType _array[Size]{};       ///< default array
    ValueType *_begin = _array;     ///< pointer on array
    size_t _size = Size;            ///< size of array
};


/** @} */ // end of result

} // namespace bro::system
