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

#include "hphp/util/managed-arena.h"

#include "hphp/util/address-range.h"
#include "hphp/util/assertions.h"

#if USE_JEMALLOC_EXTENT_HOOKS

namespace HPHP { namespace alloc {

static_assert(sizeof(RangeState) <= 64, "");
static_assert(alignof(RangeState) <= 64, "");
using RangeStateStorage = std::aligned_storage<sizeof(RangeState), 64>::type;

RangeArenaStorage g_lowArena{};
RangeArenaStorage g_highArena{};
RangeStateStorage g_ranges[3];
ArenaArray g_arenas;

NEVER_INLINE RangeState& getRange(AddrRangeClass index) {
  auto result = reinterpret_cast<RangeState*>(g_ranges + index);
  if (!result->low()) {
    static std::atomic_flag lock = ATOMIC_FLAG_INIT;
    while (lock.test_and_set(std::memory_order_acquire)) {
      // Spin while another thread initializes the ranges. We don't really reach
      // here, because the function is called very early during process
      // initialization when there is only one thread. Do it just for extra
      // safety in case someone starts to abuse the code.
    }
    if (!result->high()) {
      new (&(g_ranges[AddrRangeClass::VeryLow]))
        RangeState(kLowArenaMinAddr, 2ull << 30);
      new (&(g_ranges[AddrRangeClass::Low]))
        RangeState(2ull << 30, kLowArenaMaxAddr);
      new (&(g_ranges[AddrRangeClass::Uncounted]))
        RangeState(kLowArenaMaxAddr, kHighArenaMaxAddr);
    }
    lock.clear(std::memory_order_release);
    assertx(result->high());
  }
  return *result;
}

//////////////////////////////////////////////////////////////////////

namespace {
// jemalloc mibs that are frequently used, initialized early when there is only
// one thread.
size_t g_pactive_mib[4];                // "stats.arenas.<i>.pactive"
size_t g_epoch_mib[1];                  // "epoch"
bool g_mib_initialized = false;

void initializeMibs() {
  if (g_mib_initialized) return;
  size_t miblen = 1;
  mallctlnametomib("epoch", g_epoch_mib, &miblen);
  miblen = 4;
  mallctlnametomib("stats.arenas.0.pactive", g_pactive_mib, &miblen);
  g_mib_initialized = true;
}

void mallctl_epoch() {
  if (!g_mib_initialized) return;
  uint64_t epoch = 1;
  mallctlbymib(g_epoch_mib, 1, nullptr, nullptr, &epoch, sizeof(epoch));
}

size_t activeSize(unsigned arenaId) {
  if (!g_mib_initialized) return 0;
  size_t mib[4] =
    {g_pactive_mib[0], g_pactive_mib[1], arenaId, g_pactive_mib[3]};
  size_t pactive = 0;
  size_t sz = sizeof(pactive);
  if (mallctlbymib(mib, 4, &pactive, &sz, nullptr, 0)) return 0;
  auto const activeSize = pactive * s_pageSize;
  return activeSize;
}

}

template<typename ExtentAllocator>
std::string ManagedArena<ExtentAllocator>::reportStats() {
  mallctl_epoch();
  char buffer[128];
  using Traits = extent_allocator_traits<ExtentAllocator>;
  std::snprintf(buffer, sizeof(buffer),
                "Arena %d: capacity %zd, max_capacity %zd, used %zd\n",
                id(),
                ExtentAllocator::allocatedSize(),
                ExtentAllocator::maxCapacity(),
                activeSize(id()));
  return std::string{buffer};
}

template<typename ExtentAllocator>
size_t ManagedArena<ExtentAllocator>::unusedSize() {
  mallctl_epoch();
  auto const active = activeSize(id());
  return ExtentAllocator::allocatedSize() - active;
}

template<typename ExtentAllocator>
void ManagedArena<ExtentAllocator>::init() {
  using Traits = extent_allocator_traits<ExtentAllocator>;

  if (!g_mib_initialized) {
    initializeMibs();
  }
  if (m_arenaId != 0) {
    // Shouldn't call init() multiple times for the same instance.
    not_reached();
    return;
  }
  size_t idSize = sizeof(m_arenaId);
  if (mallctl("arenas.create", &m_arenaId, &idSize, nullptr, 0)) {
    throw std::runtime_error{"arenas.create"};
  }
  char command[32];
  if (extent_hooks_t* hooks_ptr = Traits::get_hooks()) {
    std::snprintf(command, sizeof(command), "arena.%d.extent_hooks", m_arenaId);
    if (mallctl(command, nullptr, nullptr, &hooks_ptr, sizeof(hooks_ptr))) {
      throw std::runtime_error{command};
    }
  }
  ssize_t decay_ms = Traits::get_decay_ms();
  std::snprintf(command, sizeof(command),
                "arena.%d.dirty_decay_ms", m_arenaId);
  if (mallctl(command, nullptr, nullptr, &decay_ms, sizeof(decay_ms))) {
    throw std::runtime_error{command};
  }

  if (Traits::get_hooks() != nullptr) {
    // The only place where we need `GetByArenaId` is in custom extent hooks.
    assertx(GetByArenaId<ManagedArena>(m_arenaId) == nullptr);
    for (auto& i : g_arenas) {
      if (!i.first) {
        i.first = m_arenaId;
        i.second = this;
        return;
      }
    }
    // Should never reached here, as there should be spare entries in g_arenas.
    throw std::out_of_range{
      "too many ManagedArena's, check MAX_MANAGED_ARENA_COUNT"};
  }
}

template void ManagedArena<MultiRangeExtentAllocator>::init();
template size_t ManagedArena<MultiRangeExtentAllocator>::unusedSize();
template std::string ManagedArena<MultiRangeExtentAllocator>::reportStats();

template void ManagedArena<DefaultExtentAllocator>::init();

}}

#endif
