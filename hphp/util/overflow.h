/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

template<typename T>
typename std::enable_if<std::is_integral<T>::value, bool>::type
add_overflow(T a, T b) {
  // Cast to unsigned so that ua + ub isn't undefined.
  auto const ua = static_cast<typename std::make_unsigned<T>::type>(a);
  auto const ub = static_cast<typename std::make_unsigned<T>::type>(b);

  // Overflow if the inputs have the same sign, and the result of addition has
  // the opposite sign.
  return (~(ua ^ ub) & (ua ^ (ua + ub))) >> std::numeric_limits<T>::digits;
}

template<typename T>
typename std::enable_if<std::is_integral<T>::value, bool>::type
sub_overflow(T a, T b) {
  if (b == std::numeric_limits<T>::min()) {
    // a - (INT_MIN)  -->  a - (-huge)  -->  a + huge
    return a >= 0;
  }
  return add_overflow(a, -b);
}

template<typename T>
typename std::enable_if<std::is_integral<T>::value, bool>::type
mul_overflow(T a, T b) {
  auto max = std::numeric_limits<T>::max();
  auto min = std::numeric_limits<T>::min();

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
