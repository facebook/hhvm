/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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
#include <thread>
#include <mutex>
#include <boost/noncopyable.hpp>

#include "hphp/util/tiny-vector.h"
#include "hphp/util/alloc.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

/*
 * ReadOnlyArena is an arena allocator that can be used for
 * arena-lifetime read only data.  In practice this is used in HPHP
 * for process-lifetime cold runtime data.
 *
 * When allocating from this arena, you have to provide the data
 * that's supposed to go in the block.  The allocator will temporarily
 * make the page writeable and put the data in there, then mprotect it
 * back to read only.
 *
 * One read only arena may safely be concurrently accessed by multiple
 * threads.
 */
struct ReadOnlyArena : private boost::noncopyable {
  /*
   * All pointers returned from ReadOnlyArena will have at least this
   * alignment.
   */
  static constexpr size_t kMinimalAlignment = 8;

  /*
   * Create a ReadOnlyArena that uses chunkSize bytes for each call to
   * mmap.  `chunkSize' must be a multiple of the page size
   * (s_pageSize).
   *
   * Note: s_pageSize is a dynamically initialized static, so do not
   * create global ReadOnlyArenas.
   */
  explicit ReadOnlyArena(size_t chunkSize);

  /*
   * Destroying a ReadOnlyArena will munmap all the chunks it
   * allocated, but generally ReadOnlyArenas should be used for
   * extremely long-lived data.
   */
  ~ReadOnlyArena();

  /*
   * Returns: the number of bytes we've mmaped in this arena.
   */
  size_t capacity() const;

  /*
   * Pre: dataLen is less than or equal to the chunkSize.
   *
   * Returns: a pointer to a read only memory region that contains a
   * copy of [data, data + dataLen).
   *
   * Throws: if we fail to mmap.
   */
  const void* allocate(const void* data, size_t dataLen);

private:
  void grow();

private:
  size_t const m_chunkSize;

  mutable std::mutex m_mutex;
  unsigned char* m_frontier;
  unsigned char* m_end;
  TinyVector<unsigned char*,4> m_chunks;
};

//////////////////////////////////////////////////////////////////////

}


#endif

