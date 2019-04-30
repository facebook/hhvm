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
#include "hphp/util/read-only-arena.h"

#include <folly/portability/SysMman.h>

#include "hphp/util/assertions.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

typedef std::lock_guard<std::mutex> guard;

template<typename Alloc>
ReadOnlyArena<Alloc>::~ReadOnlyArena() {
  for (auto& chunk : m_chunks) {
    m_alloc.deallocate(chunk.data(), chunk.size());
  }
}

template<typename Alloc>
const void* ReadOnlyArena<Alloc>::allocate(const void* data, size_t dataLen) {
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

// Pre: mutex already held, or no other threads may be able to access this
// (i.e. it's the ctor).
//
// Post: m_end - m_frontier >= bytes
template<typename Alloc>
void ReadOnlyArena<Alloc>::ensureFree(size_t bytes) {
  if (m_end - m_frontier >= bytes) return;

  if (bytes > m_minChunkSize) {
    bytes = (bytes + (size2m - 1)) & ~(size2m - 1);
  } else {
    bytes = m_minChunkSize;
  }

  m_frontier = m_alloc.allocate(bytes);
  m_end = m_frontier + bytes;
  m_chunks.emplace_back(m_frontier, m_end);
}

template<typename Alloc>
size_t ReadOnlyArena<Alloc>::capacity() const {
  guard g(m_mutex);
  size_t size = 0;
  for (auto& chunk : m_chunks) {
    size += chunk.size();
  }
  return size;
}

//////////////////////////////////////////////////////////////////////

template ReadOnlyArena<VMColdAllocator<char>>::~ReadOnlyArena();
template void ReadOnlyArena<VMColdAllocator<char>>::ensureFree(size_t);
template const void*
ReadOnlyArena<VMColdAllocator<char>>::allocate(const void*, size_t);
template size_t ReadOnlyArena<VMColdAllocator<char>>::capacity() const;

}
