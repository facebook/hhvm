/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#pragma once

#include <stdint.h>
#include <type_traits>

#include "hphp/util/assertions.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

template <class T>
inline void
assert_address_is_atomically_accessible(ATTRIBUTE_UNUSED T* address) {
  static_assert(sizeof(T) == 1 || sizeof(T) == 2 || sizeof(T) == 4 ||
                  sizeof(T) == 8,
                "T must be a 1, 2, 4, or 8 byte object for atomic access");
  static_assert(
    std::is_arithmetic<T>::value || std::is_pointer<T>::value,
    "Atomic operations only supported for built in integer, floating point "
    "and pointer types.");

#if defined(__x86_64__) || defined(_M_X64)
  assert(((uintptr_t(address) + sizeof(T) - 1) & ~63ul) ==
         ( uintptr_t(address)                  & ~63ul) &&
        "Atomically accessed addresses may not span cache lines");
#elif __aarch64__
  // N-byte accesses must be N-byte aligned
  assert((uintptr_t(address) & (sizeof(T) - 1)) == 0);
#else
# error What kind of memory accesses are atomic on this architecture?
#endif
}

/*
 * In some data structures (like Func) we need to copy objects that contain atomics.
 */
template<typename T> class CopyableAtomic : public std::atomic<T> {
  static_assert(std::is_integral<T>::value || std::is_pointer<T>::value);
  static_assert(sizeof(T) == 1 || sizeof(T) == 2 ||
                sizeof(T) == 4 || sizeof(T) == 8);
 public:
  CopyableAtomic() = default;
  explicit CopyableAtomic(T&& v) : std::atomic<T>(std::move(v)) { }
  explicit CopyableAtomic(const CopyableAtomic<T>& other)
    : std::atomic<T>(other.load(std::memory_order_acquire)) {
  }
  CopyableAtomic& operator=(const CopyableAtomic<T>& other) {
    this->store(other.load(std::memory_order_acquire),
                std::memory_order_release);
    return *this;
  }
};

///////////////////////////////////////////////////////////////////////////////
}
