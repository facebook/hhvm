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

#ifndef incl_HPHP_UTIL_OVERFLOW_H
#define incl_HPHP_UTIL_OVERFLOW_H

namespace HPHP {

inline bool add_overflow(int64_t a, int64_t b) {
  auto mask = int64_t(1) << 63;
  // same sign, result different sign
  return ((a & mask) == (b & mask)) && ((a & mask) != ((a + b) & mask));
}

inline bool sub_overflow(int64_t a, int64_t b) {
  if (a == 0 && b == std::numeric_limits<int64_t>::min()) {
    return true;
  }
  return add_overflow(a, -b);
}

inline bool mul_overflow(int64_t a, int64_t b) {
  auto max = std::numeric_limits<int64_t>::max();
  auto min = std::numeric_limits<int64_t>::min();

  // Handle bad div cases first.
  if (a == 0 || b == 0) {
    return false;
  }
  if ((a == min && b == -1) || (b == min && a == -1)) {
    return true;
  }

  if (a > 0) {
    if (b > 0) {
      return a > max / b;
    } else {
      return b < min / a;
    }
  } else {
    if (b < 0) {
      return a < max / b;
    } else {
      return a < min / b;
    }
  }
}

}

#endif
