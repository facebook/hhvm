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

#ifndef incl_BITOPS_H_
#define incl_BITOPS_H_

namespace HPHP {

// GLIBC doesn't provide an fls primitive. Since we're rolling our own
// anyway, fix ffs's wacky offset-by-one historical implementation. These
// guys return success/failure (failure for input of all zeros) and the
// unoffset bit position in their reference param.
template<typename I64>
inline bool
ffs64(I64 input, I64 &out) {
  bool retval;
  asm volatile("bsfq %2, %1;\n\t"
               "setnz %0;\n\t"
               : "=r"(retval), "=r"(out)
               : "r"(input));
  return retval;
}

template<typename I64>
inline bool
fls64(I64 input, I64 &out) {
  bool retval;
  asm volatile("bsrq %2, %1;\n\t"
               "setnz %0;\n\t"
               : "=r"(retval), "=r"(out)
               : "r"(input));
  return retval;
}

} // HPHP

#endif
