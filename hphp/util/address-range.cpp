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

#include "hphp/util/address-range.h"

#include "hphp/util/assertions.h"
#include <cinttypes>
#include <folly/portability/SysMman.h>

#if USE_JEMALLOC_EXTENT_HOOKS

namespace HPHP { namespace alloc {

void RangeState::reserve() {
  auto const base = reinterpret_cast<void*>(low());
  auto const size = capacity();
  auto ret = mmap(base, size, PROT_NONE,
                  MAP_ANONYMOUS | MAP_PRIVATE | MAP_NORESERVE, -1, 0);
  if (ret != base) {
    char msg[128];
    if (ret == MAP_FAILED) {
      std::snprintf(msg, sizeof(msg),
                    "failed to reserve address range [0x%" PRIxPTR
                    ", 0x%" PRIxPTR "), errno = %d",
                    low(), high(), errno);
    } else {
      munmap(ret, capacity());
      std::snprintf(msg, sizeof(msg),
                    "failed to reserve address range [0x%" PRIxPTR
                    ", 0x%" PRIxPTR "), got 0x%" PRIxPTR " instead",
                    low(), high(), reinterpret_cast<uintptr_t>(ret));
    }
    throw std::runtime_error{msg};
  }
}


RangeState::RangeState(uintptr_t lowAddr, uintptr_t highAddr, Reserved)
  : low_use(lowAddr)
  , low_map(lowAddr)
  , high_use(highAddr)
  , high_map(highAddr)
  , low_internal(reinterpret_cast<char*>(lowAddr))
  , high_internal(reinterpret_cast<char*>(highAddr)) {
  constexpr size_t size2m = 2u << 20;
  always_assert((lowAddr <= highAddr) &&
                !(lowAddr % size2m) && !(highAddr % size2m));
}

RangeState::RangeState(uintptr_t lowAddr, uintptr_t highAddr)
  : RangeState(lowAddr, highAddr, Reserved{}) {
  reserve();
}

}}

#endif
