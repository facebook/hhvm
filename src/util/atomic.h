/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

#ifndef __ATOMIC_H__
#define __ATOMIC_H__

#if (__GNUC__ > 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 2))
#include <ext/atomicity.h>
#else
#include <bits/atomicity.h>
#endif

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

inline int atomic_inc(int &count) {
  return __gnu_cxx::__exchange_and_add(&count, 1) + 1;
}

inline int atomic_dec(int &count) {
  return __gnu_cxx::__exchange_and_add(&count, -1) - 1;
}

template<class T>
inline T atomic_add(T &mem, T val) {
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

///////////////////////////////////////////////////////////////////////////////
}

#endif // __ATOMIC_H__
