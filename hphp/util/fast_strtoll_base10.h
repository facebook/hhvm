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

#ifndef incl_HPHP_UTIL_FAST_STRTOLL_BASE10_H_
#define incl_HPHP_UTIL_FAST_STRTOLL_BASE10_H_

namespace HPHP {

// This a fast version of strtoll() specialized for base 10. This version
// does not skip leading white space. It does not check for overflow/underflow,
// in which case the return value becomes different from strtoll(3c) that
// sets the result to LLONG_MAX/LLONG_MIN and errno to ERANGE.
// This function never sets errno.  Thus a caller is responsible for making
// sure the value fits in [LLONG_MIN, LLONG_MAX] range.
inline int64_t fast_strtoll_base10(const char* p) {
  int64_t x = 0;
  bool neg = false;

  if (*p == '-') {
    neg = true;
    ++p;
  } else if (*p == '+') {
    ++p;
  }
  while (*p >= '0' && *p <= '9') {
    x = (x * 10) + ('0' - *p);
    ++p;
  }
  if (!neg) {
    x = -x;
  }

  return x;
}

}

#endif
