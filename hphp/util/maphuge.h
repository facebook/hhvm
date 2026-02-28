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

#include <folly/portability/SysMman.h>

namespace HPHP {

inline constexpr bool hugePagesSupported() {
#ifdef MADV_HUGEPAGE
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

// Read the page size of transparent huge pages. On x86_64, this is 2MB.
// On aarch64, the value can be different (e.g., 512MB)
size_t readTHPSize();

inline size_t THPPageSize() {
#ifdef __x86_64__
  return 2 * 1024 * 1024;
#endif
  return readTHPSize();
}

}
