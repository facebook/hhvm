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

#include "hphp/util/address-range.h"
#include "hphp/util/slab-manager.h"
#include "hphp/util/service-data.h"

#include <atomic>
#include <mutex>
#include <folly/Range.h>

namespace HPHP {

//////////////////////////////////////////////////////////////////////

/*
 * ReadOnlyChunk is a header for a piece of allocated memory. It can work as a
 * bump allocator, and can form a list, either inside a ReadOnlyArena, or in a
 * pool shared by multiple ReadOnlyArenas, using the AtomicTaggedSlabPtr
 * facility.
 */
struct ReadOnlyChunk {
  alloc::RangeState state;
  AtomicTaggedSlabPtr next{};
  ReadOnlyChunk(uintptr_t low, size_t high)
    : state(low, high, alloc::Mapped{}) {}

  auto retained() const {
    return state.retained();
  }
  auto tryAlloc(size_t size, size_t align) {
    return state.tryAllocLow(size, align);
  }
  auto tryFree(void* ptr, size_t size) {
    return state.tryFreeLow(ptr, size);
  }
  // Convert a tagged pointer to `next` to a pointer to the ReadOnlyChunk.
  static ReadOnlyChunk* fromTPtr(const TaggedSlabPtr& tagged) {
    auto constexpr kOffset = offsetof(ReadOnlyChunk, next);
    auto const chunk = reinterpret_cast<char*>(tagged.ptr()) - kOffset;
    return reinterpret_cast<ReadOnlyChunk*>(chunk);
  }
};

/*
 * ReadOnlyArena is a bump allocator for process-lifetime constant data. It is
 * backed by a list of ReadOnlyChunk's. Deallocation is not supported, but an
 * instance is able to return partially allocated ReadOnlyChunks to a global
 * pool (a TaggedSlabList), if it is specified during construction.
 *
 * If `Local` is true, no concurrent allocation is supported for an instance
 * (but allocating new chunks, or returning chunks to the global pool can still
 * happen concurrently among multiple instances).
 */
template<typename Alloc, bool Local = false, size_t Alignment = 16>
struct ReadOnlyArena : TaggedSlabList {
  static_assert((Alignment & (Alignment - 1)) == 0, "");

  static constexpr size_t kMinChunkSize = 128u << 10;
  static constexpr size_t kChunkSizeMask = kMinChunkSize - 1;
  static_assert((kMinChunkSize & kChunkSizeMask) == 0, "");
  static constexpr unsigned kFragFactor = 8u;

  using Allocator = typename Alloc::template rebind<char>::other;

  /*
   * Create a ReadOnlyArena that uses at least `minChunkSize' bytes for each
   * call to the allocator, and returns partially used chunks to `pool` when
   * destructed.
   */
  explicit ReadOnlyArena(
    size_t minChunkSize,
    ServiceData::ExportedCounter* cap_counter,
    TaggedSlabList* pool = nullptr)
    : m_pool(pool),
      m_cap_counter(cap_counter),
      m_minChunkSize((minChunkSize + kChunkSizeMask) & ~kChunkSizeMask) {}
  ReadOnlyArena(const ReadOnlyArena&) = delete;
  ReadOnlyArena& operator=(const ReadOnlyArena&) = delete;

  /*
   * Destroying a ReadOnlyArena will not release the chunks it allocated.
   * Instead, partial chunks may be returned to a global pool.
   */
  ~ReadOnlyArena() {
    if (!m_pool) return;
    // No one should be allocating from here at this moment, so no need to hold
    // the lock.
    while (auto const head = try_shared_pop()) {
      auto const chunk = ReadOnlyChunk::fromTPtr(head);
      // Return the partially used chunk to the global pool.
      if (chunk->retained() * kFragFactor >= kMinChunkSize) {
        m_pool->push_front(head.ptr(), head.tag());
      } // otherwise leak it
    }
  }

  size_t capacity() const {
    return m_cap;
  }

  void* tryAlloc(size_t size) {
    if (auto head = unsafe_peek<Local>()) {
      auto chunk = ReadOnlyChunk::fromTPtr(head);
      // Even if it isn't the most up-to-date head now, this still works.
      auto ret = chunk->tryAlloc(size, Alignment);
      if (Local && ret) recordLast(ret, size);
      return ret;
    }
    return nullptr;
  }

  void* allocate(size_t size)   {
    if (auto p = tryAlloc(size)) return p;
    guard g(m_mutex);
    // Maybe someone else added a chunk already.
    if (auto p = tryAlloc(size)) return p;
    return addChunk(size);
  }

  void deallocate(void* ptr, size_t size) {
    if (!Local) return;
    if (ptr != m_lastAlloc) return;
    // In theory we shouldn't need to get the lock in Local mode, but let's try
    // to be slightly safer in case a bug is introduced.
    guard g(m_mutex);
    if (ptr != m_lastAlloc) return;
    if (auto head = unsafe_peek<Local>()) {
      auto chunk = ReadOnlyChunk::fromTPtr(head);
      DEBUG_ONLY auto const succ = chunk->tryFree(ptr, size);
      assertx(succ);
    }
  }

 private:
  template<size_t align, typename T>
  static T ru(T n) {
    static_assert((align & (align - 1)) == 0, "");
    return (T)(((uintptr_t)n + align - 1) & ~(align - 1));
  }

  // Need to hold the lock before calling this.
  void* addChunk(size_t size) {
    ReadOnlyChunk* chunk = nullptr;
    if (m_pool) {
      if (auto const tPtr = m_pool->try_shared_pop()) {
        chunk = ReadOnlyChunk::fromTPtr(tPtr);
        if (auto ret = chunk->tryAlloc(size, Alignment)) {
          // Add the partial chunk to local list.
          push_front(tPtr.ptr(), tPtr.tag());
          m_lastAlloc = ret;
          return ret;
        }
        // The remaining space in the chunk is too small, return it to the pool.
        m_pool->push_front(tPtr.ptr(), tPtr.tag());
        chunk = nullptr;
      }
    }
    assertx(chunk == nullptr);
    auto allocSize =
      ru<kMinChunkSize>(size + ru<Alignment>(sizeof(ReadOnlyChunk)));
    constexpr size_t kHugeThreshold = 4096 * 1024;
    if (kMinChunkSize < kHugeThreshold && allocSize > kHugeThreshold) {
      allocSize = ru<kHugeThreshold>(allocSize);
    }
    auto const mem = static_cast<char*>(m_alloc.allocate(allocSize));
    auto const low = ru<Alignment>(mem + sizeof(ReadOnlyChunk));
    auto const high = reinterpret_cast<uintptr_t>(mem + allocSize);
    chunk = new (mem) ReadOnlyChunk((uintptr_t)low, (uintptr_t)high);
    m_cap += allocSize;
    m_cap_counter->addValue(allocSize);
    auto ret = chunk->tryAlloc(size, Alignment);
    assertx(ret);
    if (Local) recordLast(ret, size);
    push_front(&(chunk->next), 0);      // do this after we get the memory.
    return ret;
  }

  void recordLast(void* ptr, size_t size) {
    if (!Local) return;
    m_lastAlloc = ptr;
    m_lastSize = size;
  }

private:
  TaggedSlabList* m_pool{nullptr};
  ServiceData::ExportedCounter* m_cap_counter{nullptr};
  // Result of last allocation, used to support immediate deallocation after
  // allocation.
  void* m_lastAlloc{nullptr};
  size_t m_lastSize{0};
  Allocator m_alloc;
  size_t const m_minChunkSize;
  size_t m_cap{0};
  mutable std::mutex m_mutex;
  using guard = std::lock_guard<std::mutex>;
};

//////////////////////////////////////////////////////////////////////

}
