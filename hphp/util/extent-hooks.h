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

#ifndef incl_HPHP_UTIL_EXTENT_HOOK_H_
#define incl_HPHP_UTIL_EXTENT_HOOK_H_

#include "hphp/util/alloc-defs.h"
#include "hphp/util/bump-mapper.h"

#include <array>
#include <atomic>

#if USE_JEMALLOC_EXTENT_HOOKS

/*
 * Recent versions of jemalloc (specifically, jemalloc 5.x here) allow us to
 * write hooks that specifiy how an arena interacts with the OS.  We use the
 * mechanism to customize the arena for various purposes, for example,
 * limiting the address ranges, managing huge pages, logging, etc.
 *
 * A limitation in the current implementation is that we only do things in the
 * alloc hook, and ignore everything else.  As a result, we are unable to
 * reclaim memory given to the arena, even when the arena is ready to give it
 * back to the system.  Therefore, this should only be used in arenas where the
 * total size of allocated memory doesn't decrease significantly.
 */

namespace HPHP { namespace alloc {

template<typename T> struct extent_allocator_traits {
 public:
  constexpr static extent_hooks_t* get_hooks() {
    return get_hooks_internal<T>(nullptr);
  }
  constexpr static ssize_t get_decay_ms() {
    return get_decay_ms_internal<T>(nullptr);
  }
  constexpr static extent_hooks_t** get_fallback(T* extentAlloc) {
    return get_fallback_internal<T>(extentAlloc, nullptr);
  }

 private:
  template<typename A>
  static constexpr extent_hooks_t* get_hooks_internal(decltype(&(A::s_hooks))) {
    return &(A::s_hooks);
  }
  template<typename A>
  static constexpr extent_hooks_t* get_hooks_internal(...) {
    return nullptr;
  }
  template<typename A>
  static constexpr ssize_t get_decay_ms_internal(decltype(&(A::s_decay_ms))) {
    return A::s_decay_ms;
  }
  template<typename A>
  static constexpr ssize_t get_decay_ms_internal(...) {
    return 60 * 1000;                   // purge every minute by default
  }
  template<typename A>
  static constexpr extent_hooks_t**
  get_fallback_internal(A* ea, decltype(A::m_fallback_hooks)) {
    return &(ea->m_fallback_hooks);
  }
  template<typename A>
  static constexpr extent_hooks_t** get_fallback_internal(...) {
    return nullptr;
  }
};

/**
 * Default extent hooks used by jemalloc.
 */
struct DefaultExtentAllocator {};

/*
 * An extent allocator that gets mappings from a list of RangeMappers, in order
 * of preference. For example, we may prefer huge pages with addresses below 2G,
 * but anything below 4G is also acceptable.
 */
struct MultiRangeExtentAllocator {
 public:
  MultiRangeExtentAllocator() {
    for (auto& p : m_mappers) p = nullptr;
  }

  void appendMapper(RangeMapper* m);

  size_t allocatedSize() const {
    return m_allocatedSize.load(std::memory_order_relaxed);
  }

  size_t maxCapacity() const;

  static void* extent_alloc(extent_hooks_t* extent_hooks, void* addr,
                            size_t size, size_t alignment, bool* zero,
                            bool* commit, unsigned arena_ind);

 public:
  // The hook passed to the underlying arena upon creation.
  static extent_hooks_t s_hooks;

 private:
  static constexpr std::size_t kMaxMapperCount = 7u;
  std::array<RangeMapper*, kMaxMapperCount> m_mappers;
  std::atomic_size_t m_allocatedSize;
};

/*
 * An extent allocator that initially allocates from a fixed range (probably
 * backed by special mappings such as huge pages), and falls back to normal
 * mappings when the initial range runs out. Pages in the range are never
 * returned to the system, so be careful if memory is tight.
 */
struct RangeFallbackExtentAllocator : RangeState {
 public:
  // Only one range allowed, must initialize at the beginning.  Add mappers
  // later.
  template<typename... Args>
  explicit RangeFallbackExtentAllocator(Args&&... args)
    : RangeState(std::forward<Args>(args)...) {}

  RangeFallbackExtentAllocator(const RangeFallbackExtentAllocator&) = delete;
  RangeFallbackExtentAllocator&
  operator=(const RangeFallbackExtentAllocator&) = delete;

  static void*
  extent_alloc(extent_hooks_t* extent_hooks, void* addr, size_t size,
               size_t alignment, bool* zero, bool* commit, unsigned arena_ind);
  static void
  extent_destroy(extent_hooks_t* extent_hooks, void* addr, size_t size,
                 bool committed, unsigned arena_ind);
  static bool
  extent_commit(extent_hooks_t* extent_hooks, void* addr, size_t size,
                size_t offset, size_t length, unsigned arena_ind);
  static bool
  extent_purge_lazy(extent_hooks_t* extent_hooks, void* addr, size_t size,
                    size_t offset, size_t length, unsigned arena_ind);
  static bool
  extent_purge(extent_hooks_t* extent_hooks, void* addr, size_t size,
               size_t offset, size_t length, unsigned arena_ind);

 private:
  bool inRange(void* addr) {
    auto const p = reinterpret_cast<uintptr_t>(addr);
    return p < high() && p >= low();
  }

 public:
  // The hook passed to the underlying arena upon creation.
  static extent_hooks_t s_hooks;
  extent_hooks_t* m_fallback_hooks{nullptr};
};

}}

#endif // USE_JEMALLOC_EXTENT_HOOKS
#endif // incl_HPHP_UTIL_EXTENT_HOOK_H_
