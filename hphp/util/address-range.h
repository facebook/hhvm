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

#include <folly/CPortability.h>

#include "hphp/util/alloc-defs.h"

#include "hphp/util/lock-free-ptr-wrapper.h"
#include "hphp/util/ptr.h"

namespace HPHP {

// Address ranges for managed arenas.
//
// We have 5 main arenas: Low, LowSmall, High, HighCold and Local. The number
// in the table is which chunk we allocate memory from first.
//
// Start of address range     Low       LowSmall  High      HighCold  Local
// --------------------------------------------------------------------------
// 0 GB                             Reserved for HHVM binary + TC
// 2 GB                       1         1
// 4 GB - Emergency - Small   3         2
// 4 GB - Emergency           4         3
// 4 GB                       2
// 32 GB                                          1         2
// 256 GB - HighColdCap                                     1
// 256 GB
// 1024 GB                                                            1
// 2048 GB                          Reserved for jemalloc auto arena
// --------------------------------------------------------------------------
//
// Overflow of any arena will cause a crash, so size them large enough to make
// sure we run out of memory before they overflow. These constants are only
// meaningful when we have control over the virtual address space, i.e. when
// USE_JEMALLOC is defined. We make them available for all modes to avoid having
// ifdefs everywhere.

constexpr uintptr_t kLowArenaMinAddr = 2ull << 30;
constexpr uintptr_t kLowArenaMaxAddr = 4ull << 30;
constexpr uintptr_t kMidArenaMaxAddr = 32ull << 30;
constexpr size_t kLowSmallArenaSize = 128 << 20;
constexpr size_t kLowEmergencySize = 128 << 20;

constexpr unsigned kSharedMaxShift = 38;
constexpr uintptr_t kSharedMaxAddr = 1ull << kSharedMaxShift;
constexpr size_t kHighColdCap = 4ull << 30;
constexpr uintptr_t kHighArenaMaxAddr = kSharedMaxAddr - kHighColdCap;
constexpr uintptr_t kHighArenaMinAddr = kMidArenaMaxAddr;

constexpr size_t kHighArenaMaxCap = kHighArenaMaxAddr - kHighArenaMinAddr;

// Arenas for request heap starts at kLocalArenaMinAddr.
constexpr uintptr_t kLocalArenaMinAddr = 1ull << 40;
constexpr size_t kLocalArenaSizeLimit = 64ull << 30;
// Extra pages for Arena 0
constexpr uintptr_t kArena0Base = 2ull << 40;
constexpr uintptr_t kDebugAddr = 3ull << 39;

#ifndef USE_JEMALLOC
// Not performance sensitive without jemalloc. Use the beginning of the bump
// allocated low arena for convenience.
#ifdef FOLLY_SANITIZE_ADDRESS
// ASAN: 0x000000010000xxxx
constexpr uintptr_t kStaticLiteralsMinAddr = kLowArenaMaxAddr;
constexpr uintptr_t kStaticLiteralsMaxAddr = kLowArenaMaxAddr + (1ul << 16);
#else
// No jemalloc: 0x000000008000xxxx
constexpr uintptr_t kStaticLiteralsMinAddr = kLowArenaMinAddr;
constexpr uintptr_t kStaticLiteralsMaxAddr = kLowArenaMinAddr + (1ul << 16);
#endif
constexpr size_t kLowEmergencyStolenByStaticLiterals = 0;
#elif defined(__aarch64__)
// ARM: 0x00000000FFFFxxxx (taken from the LowEmergency range)
// - 1 instruction to load literal into a register (using MOVN)
constexpr uintptr_t kStaticLiteralsMinAddr = kLowArenaMaxAddr - (1ull << 16);
constexpr uintptr_t kStaticLiteralsMaxAddr = kLowArenaMaxAddr;
// The stolen space must be aligned to 2M due to RangeState constraints.
constexpr size_t kLowEmergencyStolenByStaticLiterals = (2ull << 20);
static_assert(kStaticLiteralsMinAddr > kLowArenaMaxAddr - kLowEmergencySize);
static_assert(kLowEmergencyStolenByStaticLiterals >= kStaticLiteralsMaxAddr - kStaticLiteralsMinAddr);
static_assert(kLowEmergencyStolenByStaticLiterals < kLowEmergencySize);
#else
// X64: 0x000000007FFFxxxx (taken from the TC range)
// - 5-6 bytes to load literal into a register (depending on register)
// - 6 bytes single instruction to store it into 32 bit memory slot
// - 7 bytes single instruction to store it into 64 bit memory slot
//   (only possible because it fits into signed int32_t)
constexpr uintptr_t kStaticLiteralsMinAddr = kLowArenaMinAddr - (1ull << 16);
constexpr uintptr_t kStaticLiteralsMaxAddr = kLowArenaMinAddr;
constexpr size_t kLowEmergencyStolenByStaticLiterals = 0;
#endif

inline bool is_low_mem(void* m) {
  return reinterpret_cast<uintptr_t>(m) < kMidArenaMaxAddr;
}

namespace alloc {

// List of address ranges ManagedArena can manage.
enum class AddrRangeClass : uint32_t {
  Low = 0,                         // [.., kLowArenaMaxAddr - kLowEmergencySize - kLowSmallArenaSize)
  LowSmall,                        // [kLowArenaMaxAddr - kLowEmergencySize - kLowSmallArenaSize, kLowArenaMaxAddr - kLowEmergencySize)
  LowEmergency,                    // [kLowArenaMaxAddr - kLowEmergencySize, kLowArenaMaxAddr)
  Mid,                             // [kLowArenaMaxAddr, kMidArenaMaxAddr)
  High,                            // [kMidArenaMaxAddr, kHighArenaMaxAddr)
  HighCold,                        // [kHighArenaMaxAddr, kSharedMaxAddr)
  Global,                          // [kArena0Base, ...)
  NumRangeClasses,
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

  LockFreePtrWrapper<char*>::ScopedLock lock() {
    return low_internal.lock_for_update();
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
                                        std::memory_order_acq_rel)) {
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
                                         std::memory_order_acq_rel)) {
        return reinterpret_cast<void*>(newUse);
      }
    } while (true);
  }

  // Reset low_use, for immediate deallocation after allocation. Return whether
  // the operation was successful.
  bool tryFreeLow(void* ptr, size_t size) {
    auto const p = reinterpret_cast<uintptr_t>(ptr);
    assertx(p < low_use.load(std::memory_order_acquire));
    assertx(p >= low());
    uintptr_t expected = p + size;
    return low_use.compare_exchange_strong(expected, p,
                                           std::memory_order_acq_rel);
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
size_t getMidMapped();

}

}
