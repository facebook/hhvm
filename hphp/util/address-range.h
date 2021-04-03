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

#include "hphp/util/alloc-defs.h"

#include "hphp/util/lock-free-ptr-wrapper.h"

namespace HPHP {

// Address ranges for managed arenas.

// Low arenas are in [lowArenaMinAddr(), 4G), and high arena are in
// [4G, kUncountedMaxAddr).
// LOW_PTR builds won't work if low arena overflows. High arena overflow would
// result in a crash, so size it large enough to make sure we run out of memory
// before it overflows. These constants are only meaningful when
// addr_encodes_persistency is true. We make them available for all modes to
// avoid having ifdefs everywhere.
extern uintptr_t lowArenaMinAddr();

constexpr uintptr_t kLowArenaMaxAddr = 1ull << 32;
constexpr unsigned kUncountedMaxShift = 38;
constexpr uintptr_t kUncountedMaxAddr = 1ull << kUncountedMaxShift;
constexpr size_t kHighColdCap = 4ull << 30;
constexpr uintptr_t kHighArenaMaxAddr = kUncountedMaxAddr - kHighColdCap;
constexpr size_t kHighArenaMaxCap = kHighArenaMaxAddr - kLowArenaMaxAddr;

// Areans for request heap starts at kLocalArenaMinAddr.
constexpr uintptr_t kLocalArenaMinAddr = 1ull << 40;
constexpr size_t kLocalArenaSizeLimit = 64ull << 30;
// Extra pages for Arena 0
constexpr uintptr_t kArena0Base = 2ull << 40;

namespace alloc {

// List of address ranges ManagedArena can manage.
enum AddrRangeClass : uint32_t {
  VeryLow = 0,                     // below 2G, 31-bit address
  Low,                             // [2G, 4G), 32-bit address
  Uncounted,                       // [4G, kHighArenaMaxAddr)
  UncountedCold,                   // [kHighArenaMaxAddr, kUncountedMaxAddr)
};

// Direction of the bump allocator.
enum class Direction : uint32_t {
  LowToHigh,
  HighToLow
};

enum class Reserved {};
enum class Mapped {};

struct RangeMapper;

// An address range, supporting bump mapping and allocation from both ends.
struct RangeState {
  // Default constructor that does nothing.
  RangeState() = default;

  // Constructor that accepts an already mapped address range (so there is no
  // need for mappers).
  RangeState(uintptr_t lowAddr, uintptr_t highAddr, Mapped);

  // Constructor that accepts an already reserved address range.
  RangeState(uintptr_t lowAddr, uintptr_t highAddr, Reserved);

  // Constructor that reserves the range and throws if reservation fails.
  RangeState(uintptr_t lowAddr, uintptr_t highAddr);

  RangeState(const RangeState&) = delete;

  RangeState& operator=(const RangeState&) = delete;

  uintptr_t low() const {
    return reinterpret_cast<uintptr_t>(low_internal.get());
  }

  uintptr_t high() const {
    return reinterpret_cast<uintptr_t>(high_internal);
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

  size_t retained() const {             // mapped but not yet used
    size_t ret = 0;
    auto const lu = low_use.load(std::memory_order_acquire);
    auto const lm = low_map.load(std::memory_order_acquire);
    if (lm >= lu) {
      ret += lm - lu;
    }
    auto const hu = high_use.load(std::memory_order_acquire);
    auto const hm = high_map.load(std::memory_order_acquire);
    if (hu >= hm) {
      ret += hu - hm;
    }
    return ret;
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
  bool trivial(size_t size, size_t align, Direction d) const {
    auto const mask = align - 1;
    assertx((align & mask) == 0);
    if (d == Direction::LowToHigh) {
      auto const use = low_use.load(std::memory_order_acquire);
      auto const aligned = (use + mask) & ~mask;
      return aligned + size <= low_map.load(std::memory_order_acquire);
    } else {
      auto const use = high_use.load(std::memory_order_acquire);
      auto const aligned = (use - size) & ~mask;
      return aligned >= high_map.load(std::memory_order_acquire);
    }
  }

  // Whether free space in this range is insufficient for the allocation.
  bool infeasible(size_t size, size_t align, Direction d) const {
    auto const mask = align - 1;
    assertx((align & mask) == 0);
    if (d == Direction::LowToHigh) {
      auto const newUse =
        ((low_use.load(std::memory_order_acquire) + mask) & ~mask) + size;
      return newUse > high_map.load(std::memory_order_acquire);
    } else {
      auto const newUse =
        (high_use.load(std::memory_order_acquire) - size) & ~mask;
      return newUse < low_map.load(std::memory_order_acquire);
    }
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
  void* tryAlloc(size_t size, size_t align, Direction D) {
    if (D == Direction::LowToHigh) return tryAllocLow(size, align);
    return tryAllocHigh(size, align);
  }

  // Atomically move frontier, and return nullptr if more mappings are needed.
  void* tryAllocLow(size_t size, size_t align) {
    auto const mapFrontier = low_map.load(std::memory_order_acquire);
    auto oldUse = low_use.load(std::memory_order_acquire);
    auto const mask = align - 1;
    assertx((align & mask) == 0);
    do {
      auto const aligned = (oldUse + mask) & ~mask;
      auto const newUse = aligned + size;
      // Need to add more mapping.
      if (newUse > mapFrontier) return nullptr;
      if (low_use.compare_exchange_weak(oldUse, newUse,
                                        std::memory_order_release,
                                        std::memory_order_acquire)) {
        return reinterpret_cast<void*>(aligned);
      }
    } while (true);
  }

  void* tryAllocHigh(size_t size, size_t align) {
    auto const mapFrontier = high_map.load(std::memory_order_acquire);
    auto oldUse = high_use.load(std::memory_order_acquire);
    auto const mask = align - 1;
    assertx((align & mask) == 0);
    do {
      auto const newUse = (oldUse - size) & ~mask;
      // Need to add more mapping.
      if (newUse < mapFrontier) return nullptr;
      if (high_use.compare_exchange_weak(oldUse, newUse,
                                         std::memory_order_release,
                                         std::memory_order_acquire)) {
        return reinterpret_cast<void*>(newUse);
      }
    } while (true);
  }

  // Reset low_use, for immediate deallocation after allocation. Return whether
  // the operation was successful.
  bool tryFreeLow(void* ptr, size_t size) {
    auto const p = reinterpret_cast<uintptr_t>(ptr);
    assertx(p < low_use.load(std::memory_order_relaxed));
    assertx(p >= low());
    uintptr_t expected = p + size;
    return low_use.compare_exchange_strong(expected, p,
                                           std::memory_order_relaxed);
  }

  std::atomic<uintptr_t> low_use{0};
  std::atomic<uintptr_t> low_map{0};
  std::atomic<uintptr_t> high_use{0};
  std::atomic<uintptr_t> high_map{0};

  // Use lower bits as a a small lock.  Call lock() before adding new mappings.
  LockFreePtrWrapper<char*> low_internal{nullptr};
  char* high_internal{nullptr};

  RangeMapper* low_mapper{nullptr};
  RangeMapper* high_mapper{nullptr};
};

static_assert(sizeof(RangeState) <= 64, "");

RangeState& getRange(AddrRangeClass rc);

size_t getLowMapped();

}

}
