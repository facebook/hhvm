/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_ATOMIC_H_
#define incl_HPHP_ATOMIC_H_

#include <stdint.h>
#include <boost/type_traits/is_arithmetic.hpp>
#include <boost/type_traits/is_pointer.hpp>

#include "hphp/util/assertions.h"
#include "hphp/util/util.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

inline void compiler_membar( ) {
  asm volatile("" : : :"memory");
}

template<class T>
inline void assert_address_is_atomically_accessible(T* address) {
  static_assert(
    sizeof(T) == 1 || sizeof(T) == 2 || sizeof(T) == 4 || sizeof(T) == 8,
    "T must be a 1, 2, 4, or 8 byte object for atomic access");
  static_assert(
    boost::is_arithmetic<T>::value || boost::is_pointer<T>::value,
    "Atomic operations only supported for built in integer, floating point "
    "and pointer types.");

#ifdef __x86_64__
  assert(((uintptr_t(address) + sizeof(T) - 1) & ~63ul) ==
         ( uintptr_t(address)                  & ~63ul) &&
        "Atomically accessed addresses may not span cache lines");
#elif __AARCH64EL__
  // N-byte accesses must be N-byte aligned
  assert((uintptr_t(address) & (sizeof(T) - 1)) == 0);
#else
# error What kind of memory accesses are atomic on this architecture?
#endif
}


/**
 * Use of the functions below is DISCOURAGED.
 * Prefer the std::atomic library in C++11.
 *
 * Not only does it relieve you of worrying about architecture and compiler
 * portability issues, but it also encourages you to really reason through the
 * concurrency you're aiming to manage, by treating atomically-accessed data as
 * a distinct type.
 */

template<class T> inline T atomic_acquire_load(const T* address) {
  assert_address_is_atomically_accessible(address);

  T ret = *address; // acquire barrier on x64
  compiler_membar();
  return ret;
}

template<class T, class U>
inline void atomic_release_store(T* address, U val) {
  assert_address_is_atomically_accessible(address);

  compiler_membar();
  *address = val; // release barrier on x64 (as long as no one is
                  // doing any non-temporal moves or whatnot).
}

///////////////////////////////////////////////////////////////////////////////
}

#endif // __ATOMIC_H__
