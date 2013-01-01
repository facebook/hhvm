/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#ifndef incl_ATOMIC_H_
#define incl_ATOMIC_H_

#include <stdint.h>
#include <boost/type_traits/is_arithmetic.hpp>
#include <boost/type_traits/is_pointer.hpp>

#if (__GNUC__ > 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 2))
#include <ext/atomicity.h>
#else
#include <bits/atomicity.h>
#endif

#include "util/assertions.h"
#include "util/util.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

template<class T>
inline void assert_address_is_atomically_accessible(T* address) {
  static_assert(
    sizeof(T) == 1 || sizeof(T) == 2 || sizeof(T) == 4 || sizeof(T) == 8,
    "T must be a 1, 2, 4, or 8 byte object for atomic access");
  static_assert(
    boost::is_arithmetic<T>::value || boost::is_pointer<T>::value,
    "Atomic operations only supported for built in integer, floating point "
    "and pointer types.");

  ASSERT(((uintptr_t(address) + sizeof(T) - 1) & ~63ul) ==
         ( uintptr_t(address)                  & ~63ul) &&
        "Atomically accessed addresses may not span cache lines");
}

template<class T> inline T atomic_acquire_load(const T* address) {
  assert_address_is_atomically_accessible(address);

  T ret = *address; // acquire barrier on x64
  Util::compiler_membar();
  return ret;
}

template<class T, class U>
inline void atomic_release_store(T* address, U val) {
  assert_address_is_atomically_accessible(address);

  Util::compiler_membar();
  *address = val; // release barrier on x64 (as long as no one is
                  // doing any non-temporal moves or whatnot).
}

template<typename T>
static inline T atomic_inc(T &count) {
  assert_address_is_atomically_accessible(&count);
  return __gnu_cxx::__exchange_and_add(&count, 1) + 1;
}

static inline int atomic_dec(int &count) {
  assert_address_is_atomically_accessible(&count);
  return __gnu_cxx::__exchange_and_add(&count, -1) - 1;
}

template<class T>
static inline T atomic_add(T &mem, T val) {
  assert_address_is_atomically_accessible(&mem);
  T r;
  asm volatile
   (
    "lock\n\t"
    "xadd %1, %0":
    "+m"( mem ), "=r"( r ): // outputs (%0, %1)
    "1"( val ): // inputs (%2 == %1)
    "memory", "cc" // clobbers
    );
  return r;
}

template<class T>
static inline bool atomic_cas(volatile T* mem, T cmpVal, T newVal) {
  assert_address_is_atomically_accessible(mem);
  return __sync_bool_compare_and_swap(mem, cmpVal, newVal);
}

///////////////////////////////////////////////////////////////////////////////
}

#endif // __ATOMIC_H__
