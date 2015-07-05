/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_BITOPS_H_
#define incl_HPHP_BITOPS_H_

#if !defined(__x86_64__) && !defined(__AARCH64EL__)
#include <folly/Bits.h>
#endif

namespace HPHP {

// GLIBC doesn't provide an fls primitive. Since we're rolling our own
// anyway, fix ffs's wacky offset-by-one historical implementation. These
// guys return success/failure (failure for input of all zeros) and the
// unoffset bit position in their reference param.
template<typename I64>
inline bool ffs64(I64 input, I64 &out) {
  bool retval;
#if defined(__x86_64__)
  asm volatile (
    "bsfq  %2, %1\n\t"   // bit scan forward
    "setnz %0\n\t":      // zero retval if input == 0
    "=r"(retval), "=r"(out):
    "r"(input):
    "cc"
  );
#elif defined(__AARCH64EL__)
  asm volatile (
    "rbit  %2, %2\n\t"  // reverse bits
    "clz   %1, %2\n\t"  // count leading zeros
    "cmp   %1, #64\n\t"
    "cset  %0, NE":     // return (result != 64)
    "=r"(retval), "=r"(out), "+r"(input):
    :
    "cc"
  );
#else
  out = folly::findFirstSet(input);
  retval = input != 0;
#endif
  return retval;
}

template<typename I64>
inline bool fls64(I64 input, I64 &out) {
  bool retval;
#if defined(__x86_64__)
  asm volatile (
    "bsrq  %2, %1\n\t"   // bit scan reverse
    "setnz %0\n\t":      // zero retval if input == 0
    "=r"(retval), "=r"(out):
    "r"(input):
    "cc"
  );
#elif defined(__AARCH64EL__)
  asm volatile (
    "clz   %1, %2\n\t"      // count leading zeros
    "neg   %1, %1\n\t"
    "adds  %1, %1, #63\n\t" // result = 63 - (# of leading zeros)
                            // "s" suffix sets condition flags
    "cset  %0, PL":         // return (result >= 0)
                            //   because result < 0 iff input == 0
    "=r"(retval), "=r"(out):
    "r"(input):
    "cc"
  );
#else
  out = folly::findLastSet(input);
  retval = input != 0;
#endif
  return retval;
}

} // HPHP

#endif
