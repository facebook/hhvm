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

#include "hphp/util/alloc.h"
#include "hphp/util/assertions.h"
#include "hphp/util/jemalloc-util.h"
#include "hphp/util/ptr.h"
#include "hphp/util/roar.h"

#include <cinttypes>
#include <folly/portability/SysMman.h>

namespace HPHP {

uintptr_t lowArenaMinAddr() {
  const char* str = getenv("HHVM_LOW_ARENA_START");
  if (str == nullptr) {
    // 2GB if we are not low-mem constrained.
    if (use_packedptr || !use_lowptr) return 2ull << 30;

#if defined(INSTRUMENTED_BUILD)
    // 2GB if instrumented build due to large text size.
    return 2ull << 30;
#endif

#if !defined(NDEBUG) || defined(DEBUG)
    // 2GB if debug build due to larger text size
    return 2ull << 30;
#endif

    // 1GB for pure low-mem, steal extra 128MB for roar.
    if (use_roar) return (1ull << 30) + (128ull << 20); // 1GB + 128MB
    return 1ull << 30; // 1GB
  }

  uintptr_t start = 0;
  if (sscanf(str, "0x%lx", &start) == 1) return start;
  if (sscanf(str, "%lu", &start) == 1) return start;
  fprintf(stderr, "Bad environment variable HHVM_LOW_ARENA_START: %s\n", str);
  abort();
}

namespace alloc {

void RangeState::reserve() {
  auto const base = reinterpret_cast<void*>(low());
  auto const size = capacity();
  if (size == 0) return;
  auto ret = mmap(base, size, PROT_NONE,
                  MAP_FIXED_NOREPLACE | MAP_ANONYMOUS | MAP_PRIVATE | MAP_NORESERVE, -1, 0);
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

size_t getLowMapped() {
  size_t low_mapped = 0;
#if USE_JEMALLOC
  // The low range [1G, 4G) is divided into two ranges, and shared by 3
  // arenas.
  low_mapped += alloc::getRange(alloc::AddrRangeClass::Low).used();
#ifdef USE_PACKEDPTR
  low_mapped += alloc::getRange(alloc::AddrRangeClass::LowSmall).used();
#endif
  low_mapped += alloc::getRange(alloc::AddrRangeClass::LowEmergency).used();
#endif
  return low_mapped;
}

size_t getMidMapped() {
#if USE_JEMALLOC
  // The mid range [4G, 32G)
  return alloc::getRange(alloc::AddrRangeClass::Mid).used();
#else
  return 0;
#endif
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

RangeState::RangeState(uintptr_t lowAddr, uintptr_t highAddr, Mapped)
  : low_use(lowAddr)
  , low_map(highAddr)                   // already mapped the whole range
  , high_use(highAddr)
  , high_map(highAddr)
  , low_internal(reinterpret_cast<char*>(lowAddr))
  , high_internal(reinterpret_cast<char*>(highAddr)) {
}

}}
