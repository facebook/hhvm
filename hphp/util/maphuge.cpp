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

#include "hphp/util/maphuge.h"

#include "hphp/util/hugetlb.h"
#include "hphp/util/kernel-version.h"

#include <folly/portability/SysMman.h>
#include <folly/portability/Unistd.h>

namespace HPHP {

void hintHuge(void* mem, size_t length) {
#ifdef MADV_HUGEPAGE
  if (hugePagesSupported()) {
    madvise(mem, length, MADV_HUGEPAGE);
  }
#endif
}

bool hugePagesSupported() {
#ifdef MADV_HUGEPAGE
  static KernelVersion kv;
  // This kernel fixed a panic when using MADV_HUGEPAGE.
  static KernelVersion minKv("3.2.28-72_fbk12");
  return KernelVersion::cmp(kv, minKv);
#else
  return false;
#endif
}

void hintHugeDeleteData(char* mem, size_t length, int prot, bool shared) {
#ifdef __linux__
  if (reinterpret_cast<uintptr_t>(mem) % size2m != 0) return;
  // I assume you have saved your data on these pages.
  size_t total_2m_pages = length / size2m;
  if (total_2m_pages == 0) return;
  size_t succ_pages =
    remap_interleaved_2m_pages(mem, total_2m_pages, prot, shared);
  total_2m_pages -= succ_pages;
  if (total_2m_pages > 0) {
    mem += succ_pages * size2m;
    // We run out of reserved 2M pages.  In case we unmmaped [from, from +
    // size2m) while failed to back it using huge pages, check and make sure
    // there is some memory there.
    unsigned char v = 0;
    if (mincore(mem, 1, &v) == -1 && errno == ENOMEM) {
      mmap(mem, size2m, prot, MAP_ANONYMOUS | MAP_FIXED |
           (shared ? MAP_SHARED : MAP_PRIVATE), -1, 0);
    }

    // Well, good luck getting transparent huge pages.
    hintHuge(mem, total_2m_pages * size2m);
  }
#endif
}

}
