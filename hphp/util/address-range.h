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

#ifndef incl_HPHP_UTIL_ADDRESS_RANGE_H_
#define incl_HPHP_UTIL_ADDRESS_RANGE_H_

#include "hphp/util/alloc-defs.h"

#include "hphp/util/lock-free-ptr-wrapper.h"

namespace HPHP {

// Address ranges for managed arenas. Low arena is in [1G, 4G), and high arena
// in [4G, kUncountedMaxAddr) at most. LOW_PTR builds won't work if low arena
// overflows. High arena overflow would result in a crash, so size it large
// enough to make sure we run out of memory before it overflows. These constants
// are only meaningful when addr_encodes_persistency is true. We make them
// available for all modes to avoid having ifdefs everywhere.
constexpr uintptr_t kLowArenaMinAddr = 1ull << 30;
constexpr uintptr_t kLowArenaMaxAddr = 1ull << 32;
constexpr unsigned kUncountedMaxShift = 38;
constexpr uintptr_t kUncountedMaxAddr = 1ull << kUncountedMaxShift;
constexpr uintptr_t kHighArenaMaxAddr = kUncountedMaxAddr;
constexpr size_t kLowArenaMaxCap = kLowArenaMaxAddr - kLowArenaMinAddr;
constexpr size_t kHighArenaMaxCap = kHighArenaMaxAddr - kLowArenaMaxAddr;

#if USE_JEMALLOC_EXTENT_HOOKS

namespace alloc {

// List of address ranges ManagedArena can manage.
enum AddrRangeClass : uint32_t {
  VeryLow = 0,                          // 31-bit address
  Low,                                  // 2G - 4G, 32-bit address
  Uncounted,                            // 4G - kUncountedMaxAddr
};

// Direction of the bump allocator.
enum class Direction : uint32_t {
  LowToHigh,
  HighToLow
};

struct RangeMapper;

// An address range, supporting bump mapping and allocation from both ends.
struct alignas(64) RangeState {
  // Default constructor that does nothing.
  RangeState() = default;

  // Constructor reserves the address range.
  RangeState(uintptr_t lowAddr, uintptr_t highAddr);

  RangeState(const RangeState&) = delete;

  RangeState& operator=(const RangeState&) = delete;

  char* low() const {
    return low_internal.get().get();
  }

  char* high() const {
    return high_internal;
  }

  size_t lowUsed() const {
    return low_use.load(std::memory_order_acquire) - low();
  }

  size_t highUsed() const {
    return high() - high_use.load(std::memory_order_acquire);
  }

  size_t used() const {
    return lowUsed() + highUsed();
  }

  size_t lowMapped() const {
    auto const mapped = low_map.load(std::memory_order_acquire);
    return mapped - low();
  }

  size_t highMapped() const {
    return high() - high_map.load(std::memory_order_acquire);
  }

  size_t mapped() const {
    return lowMapped() + highMapped();
  }

  size_t capacity() const {
    return high() - low();
  }

  void lock() {
    low_internal.lock_for_update();
  }

  void unlock() {
    low_internal.unlock();
  }

  // Whether it is possible (but not guaranteed when multiple threads are
  // running) to allocate without adding new mappings.
  bool trivial(size_t size, Direction d) const {
    size_t available = 0;
    if (d == Direction::LowToHigh) {
      available = low_map.load(std::memory_order_acquire) -
        low_use.load(std::memory_order_acquire);
    } else {
      available = high_use.load(std::memory_order_acquire) -
        high_map.load(std::memory_order_acquire);
    }
    return size <= available;
  }

  // Whether free space in this range is insufficient for the allocation.
  bool infeasible(size_t size) const {
    auto const available = high_use.load(std::memory_order_acquire) -
      low_use.load(std::memory_order_acquire);
    return size > available;
  }

  // Reserve address space, and throw upon failure.
  void reserve();

  void setLowMapper(RangeMapper* mapper) {
    low_mapper = mapper;
  }

  void setHighMapper(RangeMapper* mapper) {
    high_mapper = mapper;
  }

  RangeMapper* getLowMapper() {
    return low_mapper;
  }

  RangeMapper* getHighMapper() {
    return high_mapper;
  }

  // Try to bump allocate without adding new mappings.
  void* tryAlloc(size_t size, Direction D) {
    if (D == Direction::LowToHigh) return tryAllocLow(size);
    return tryAllocHigh(size);
  }

  // Atomically move frontier, and return nullptr if more mappings are needed.
  void* tryAllocLow(size_t size) {
    auto const mapFrontier = low_map.load(std::memory_order_acquire);
    auto oldUse = low_use.load(std::memory_order_acquire);
    do {
      auto const newUse = oldUse + size;
      // Need to add more mapping.
      if (newUse > mapFrontier) return nullptr;
      if (low_use.compare_exchange_weak(oldUse, newUse,
                                        std::memory_order_release,
                                        std::memory_order_acquire)) {
        return oldUse;
      }
    } while (true);
  }

  void* tryAllocHigh(size_t size) {
    auto const mapFrontier = high_map.load(std::memory_order_acquire);
    auto oldUse = high_use.load(std::memory_order_acquire);
    do {
      // Need to add more mapping.
      if (mapFrontier + size > oldUse) return nullptr;
      auto const newUse = oldUse - size;
      if (high_use.compare_exchange_weak(oldUse, newUse,
                                         std::memory_order_release,
                                         std::memory_order_acquire)) {
        return newUse;
      }
    } while (true);
  }

  std::atomic<char*> low_use{nullptr};
  std::atomic<char*> low_map{nullptr};
  std::atomic<char*> high_use{nullptr};
  std::atomic<char*> high_map{nullptr};

  // Use lower bits as a a small lock.  Call lock() before adding new mappings.
  LockFreePtrWrapper<char*> low_internal{nullptr};
  char* high_internal{nullptr};

  RangeMapper* low_mapper{nullptr};
  RangeMapper* high_mapper{nullptr};
};

static_assert(sizeof(RangeState) <= 64, "");

RangeState& getRange(AddrRangeClass rc);

}

#endif // USE_JEMALLOC_EXTENT_HOOKS

}

#endif
