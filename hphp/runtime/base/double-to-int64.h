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

#include <limits>

#include <folly/CPortability.h>

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

inline int64_t double_to_int64(double v)
  FOLLY_DISABLE_UNDEFINED_BEHAVIOR_SANITIZER("float-cast-overflow") {
  if (v >= 0) {
    return v < (double)std::numeric_limits<uint64_t>::max() ? (uint64_t)v : 0u;
  } else if (v < 0) {
    return (int64_t)v;
  } else {
    // If v >= 0 is false and v < 0 is false, then v is NaN. In PHP5, on Intel,
    // you get 0x800..00, a.k.a. the minimum int64_t.
    // We mimic that on all platforms, though this makes us sad.
    return std::numeric_limits<int64_t>::min();
  }
}

///////////////////////////////////////////////////////////////////////////////

}

