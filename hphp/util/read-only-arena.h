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

#include <cstdlib>
#include <mutex>
#include <thread>

#include <folly/Range.h>

#include "hphp/util/alloc.h"

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
template<typename Alloc = VMColdAllocator<char>>
struct ReadOnlyArena {
  using Allocator = typename Alloc::template rebind<char>::other;
  /*
   * All pointers returned from ReadOnlyArena will have at least this
   * alignment.
   */
  static constexpr size_t kMinimalAlignment = 8;
  static constexpr size_t size2m = 2ull << 20;

  /*
   * Create a ReadOnlyArena that uses at least `minChunkSize' bytes for each
   * call to the allocator.  `minChunkSize' will be rounded up to the nearest
   * multiple of 2M.
   */
  explicit ReadOnlyArena(size_t minChunkSize)
    : m_minChunkSize((minChunkSize + (size2m - 1)) & ~(size2m - 1)) {
  }

  /*
   * Destroying a ReadOnlyArena will release all the chunks it allocated, but
   * generally ReadOnlyArenas should be used for extremely long-lived data.
   */
  ~ReadOnlyArena();

  ReadOnlyArena(const ReadOnlyArena&) = delete;
  ReadOnlyArena& operator=(const ReadOnlyArena&) = delete;

  /*
   * Returns: the number of bytes we've allocated (from malloc) in this arena.
   */
  size_t capacity() const;

  /*
   * Returns: a pointer to a read only memory region that contains a copy of
   * [data, data + dataLen).
   */
  const void* allocate(const void* data, size_t dataLen);

private:
  void ensureFree(size_t bytes);

private:
  Allocator m_alloc;
  char* m_frontier{nullptr};
  char* m_end{nullptr};
  size_t const m_minChunkSize;
  using AR = typename Alloc::template rebind<folly::Range<char*>>::other;
  std::vector<folly::Range<char*>, AR> m_chunks;
  mutable std::mutex m_mutex;
};

//////////////////////////////////////////////////////////////////////

}
#endif
