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
#ifndef incl_HPHP_READ_ONLY_ARENA_H_
#define incl_HPHP_READ_ONLY_ARENA_H_

#include <mutex>
#include <folly/Range.h>

namespace HPHP {

//////////////////////////////////////////////////////////////////////

/*
 * ReadOnlyArena is an arena allocator that can be used for arena-lifetime read
 * only data.  In practice this is used in HPHP for process-lifetime cold
 * runtime data.
 *
 * When allocating from this arena, you have to provide the data that's
 * supposed to go in the block.
 *
 * One read only arena may safely be concurrently accessed by multiple threads.
 */
template<typename Alloc>
struct ReadOnlyArena {
  using Allocator = typename Alloc::template rebind<char>::other;
  /*
   * All pointers returned from ReadOnlyArena will have at least this
   * alignment.
   */
  static constexpr size_t kMinimalAlignment = 8;
  static constexpr size_t size4m = 4ull << 20;

  /*
   * Create a ReadOnlyArena that uses at least `minChunkSize' bytes for each
   * call to the allocator. `minChunkSize' will be rounded up to the nearest
   * multiple of 4M.
   */
  explicit ReadOnlyArena(size_t minChunkSize)
    : m_minChunkSize((minChunkSize + (size4m - 1)) & ~(size4m - 1)) {
  }
  ReadOnlyArena(const ReadOnlyArena&) = delete;
  ReadOnlyArena& operator=(const ReadOnlyArena&) = delete;

  /*
   * Destroying a ReadOnlyArena will release all the chunks it allocated, but
   * generally ReadOnlyArenas should be used for extremely long-lived data.
   */
  ~ReadOnlyArena() {
    for (auto& chunk : m_chunks) {
      m_alloc.deallocate(chunk.data(), chunk.size());
    }
  }

  size_t capacity() const {
    return m_cap;
  }

  /*
   * Returns: a pointer to a read only memory region that contains a copy of
   * [data, data + dataLen).
   */
  const void* allocate(const void* data, size_t dataLen) {
    void* ret;
    {
      // Round up to the minimal alignment.
      auto alignedLen =
        (dataLen + (kMinimalAlignment - 1)) & ~(kMinimalAlignment - 1);
      guard g(m_mutex);
      ensureFree(alignedLen);
      assert(((uintptr_t)m_frontier & (kMinimalAlignment - 1)) == 0);
      assert(m_frontier + alignedLen <= m_end);
      ret = m_frontier;
      m_frontier += alignedLen;
    }
    memcpy(ret, data, dataLen);
    return ret;
  }

 private:
  // Pre: mutex already held, or no other threads may be able to access this.
  // Post: m_end - m_frontier >= bytes
  void ensureFree(size_t bytes) {
    if (m_end - m_frontier >= bytes) return;

    if (bytes > m_minChunkSize) {
      bytes = (bytes + (size4m - 1)) & ~(size4m - 1);
    } else {
      bytes = m_minChunkSize;
    }

    m_frontier = m_alloc.allocate(bytes);
    m_end = m_frontier + bytes;
    m_chunks.emplace_back(m_frontier, m_end);
    m_cap += bytes;
  }

private:
  Allocator m_alloc;
  char* m_frontier{nullptr};
  char* m_end{nullptr};
  size_t const m_minChunkSize;
  size_t m_cap{0};
  using AR = typename Alloc::template rebind<folly::Range<char*>>::other;
  std::vector<folly::Range<char*>, AR> m_chunks;
  mutable std::mutex m_mutex;
  using guard = std::lock_guard<std::mutex>;
};

//////////////////////////////////////////////////////////////////////

}
#endif
