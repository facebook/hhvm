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
#ifndef incl_HPHP_MAPHUGE_H_
#define incl_HPHP_MAPHUGE_H_

#include <folly/portability/SysMman.h>

namespace HPHP {

inline constexpr bool hugePagesSupported() {
#ifdef MADV_HUGEPAGE
  // Kernels earlier than 3.2.28 may have bugs dealing with MADV_HUGEPAGE.  The
  // bug should've been fixed in production kernels now, so we no longer do
  // run-time kernel version checks.
  return true;
#else
  return false;
#endif
}

inline void hintHuge(void* mem, size_t length) {
#ifdef MADV_HUGEPAGE
  if (hugePagesSupported()) {
    madvise(mem, length, MADV_HUGEPAGE);
  }
#endif
}

}

#endif
