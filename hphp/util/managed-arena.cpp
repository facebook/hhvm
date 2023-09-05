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
#include "hphp/util/service-data.h"

#if USE_JEMALLOC_EXTENT_HOOKS

namespace HPHP::alloc {

static_assert(sizeof(RangeState) <= 64, "");
static_assert(alignof(RangeState) <= 64, "");
using RangeStateStorage = std::aligned_storage<sizeof(RangeState), 64>::type;

RangeArenaStorage g_lowerArena{};
RangeArenaStorage g_lowArena{};
RangeArenaStorage g_highArena{};
RangeArenaStorage g_coldArena{};
RangeArenaStorage g_lowColdArena{};
RangeStateStorage g_ranges[5];
ArenaArray g_arenas;
PreMappedArena* g_arena0;               // arena 0, if we end up injecting pages
std::vector<PreMappedArena*> g_local_arenas; // keyed by numa node id

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
        RangeState(lowArenaMinAddr(), 2ull << 30);
      new (&(g_ranges[AddrRangeClass::Low]))
        RangeState(2ull << 30, kLowArenaMaxAddr - kLowEmergencySize);
      new (&(g_ranges[AddrRangeClass::LowEmergency]))
        RangeState(kLowArenaMaxAddr - kLowEmergencySize, kLowArenaMaxAddr);
      new (&(g_ranges[AddrRangeClass::Uncounted]))
        RangeState(kLowArenaMaxAddr, kHighArenaMaxAddr);
      new (&(g_ranges[AddrRangeClass::UncountedCold]))
        RangeState(kHighArenaMaxAddr, kUncountedMaxAddr);
    }
    lock.clear(std::memory_order_release);
    assertx(result->high());
  }
  return *result;
}

//////////////////////////////////////////////////////////////////////

template<typename ExtentAllocator>
std::string ManagedArena<ExtentAllocator>::reportStats() const {
  char buffer[128];
  using Traits = extent_allocator_traits<ExtentAllocator>;
  std::snprintf(buffer, sizeof(buffer),
                "Arena %d: capacity %zd, max_capacity %zd, used %zd\n",
                id(),
                ExtentAllocator::allocatedSize(),
                ExtentAllocator::maxCapacity(),
                s_pageSize * mallctl_pactive(id()));
  return std::string{buffer};
}

template<typename ExtentAllocator>
size_t ManagedArena<ExtentAllocator>::unusedSize() {
  auto const active = s_pageSize * mallctl_pactive(id());
  return ExtentAllocator::allocatedSize() - active;
}

template<typename ExtentAllocator>
void ManagedArena<ExtentAllocator>::create() {
  using Traits = extent_allocator_traits<ExtentAllocator>;
  assertx(m_arenaId == kInvalidArena);
  size_t idSize = sizeof(m_arenaId);
  if (mallctl("arenas.create", &m_arenaId, &idSize, nullptr, 0)) {
    throw std::runtime_error{"arenas.create"};
  }
  char command[32];
  ssize_t decay_ms = Traits::get_decay_ms();
  std::snprintf(command, sizeof(command),
                "arena.%d.dirty_decay_ms", m_arenaId);
  if (mallctl(command, nullptr, nullptr, &decay_ms, sizeof(decay_ms))) {
    throw std::runtime_error{command};
  }
}


template<typename ExtentAllocator>
void ManagedArena<ExtentAllocator>::updateHook() {
  using Traits = extent_allocator_traits<ExtentAllocator>;
  if (auto hooks_ptr = Traits::get_hooks()) {
    // We need to do `GetByArenaId()` in custom extent hooks, so register the
    // arena in the global list. It is important that we do this before actually
    // updating the hooks.
    assertx(GetByArenaId<ManagedArena>(m_arenaId) == nullptr);
    bool registered = false;
    for (auto& i : g_arenas) {
      if (!i.second) {
        i.first = m_arenaId;
        i.second = this;
        registered = true;
        break;
      }
    }
    if (!registered) {
      throw std::out_of_range{
        "too many ManagedArena's, check MAX_MANAGED_ARENA_COUNT"};
    }

    char command[32];
    std::snprintf(command, sizeof(command), "arena.%d.extent_hooks", m_arenaId);
    auto fallback = Traits::get_fallback(this);
    size_t oldSize = sizeof(extent_hooks_t*);
    if (mallctl(command, fallback, (fallback ? &oldSize : nullptr),
                &hooks_ptr, sizeof(hooks_ptr))) {
      throw std::runtime_error{command};
    }
#if (JEMALLOC_VERSION_MAJOR > 5) || \
  ((JEMALLOC_VERSION_MAJOR == 5) && (JEMALLOC_VERSION_MINOR >= 1))
    // Avoid asking excessive memory through the hook, in order to make better
    // use of preallocated pages.
    std::snprintf(command, sizeof(command),
                  "arena.%d.retain_grow_limit", m_arenaId);
    size_t limit = 16ull << 20;
    if (mallctl(command, nullptr, nullptr, &limit, sizeof(limit))) {
      throw std::runtime_error{command};
    }
  }
#endif
}

template void ManagedArena<MultiRangeExtentAllocator>::create();
template void ManagedArena<MultiRangeExtentAllocator>::updateHook();
template size_t ManagedArena<MultiRangeExtentAllocator>::unusedSize();
template std::string ManagedArena<MultiRangeExtentAllocator>::reportStats() const;

template void ManagedArena<DefaultExtentAllocator>::create();
template void ManagedArena<DefaultExtentAllocator>::updateHook();
template void ManagedArena<RangeFallbackExtentAllocator>::create();
template void ManagedArena<RangeFallbackExtentAllocator>::updateHook();

namespace {
ServiceData::CounterCallback s_arena_usage([](ServiceData::CounterMap& counters) {
  #define INSERT(which) \
    if (auto a = alloc::which ## Arena()) { \
      counters.emplace("admin." #which "_arena_usage", s_pageSize * mallctl_pactive(a->id())); \
    }

  INSERT(low)
  INSERT(high)
});
}

}

#endif
