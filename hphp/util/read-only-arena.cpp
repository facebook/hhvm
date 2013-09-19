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
#include "hphp/util/read-only-arena.h"

#include <sys/mman.h>

#include "folly/Exception.h"

#include "hphp/util/assertions.h"

#ifndef MAP_ANONYMOUS
#define MAP_ANONYMOUS MAP_ANON
#endif

namespace HPHP {

//////////////////////////////////////////////////////////////////////

typedef std::lock_guard<std::mutex> guard;

//////////////////////////////////////////////////////////////////////

ReadOnlyArena::ReadOnlyArena(size_t chunkSize)
  : m_chunkSize(chunkSize)
  , m_frontier(nullptr)
  , m_end(nullptr)
{
  always_assert(m_chunkSize % Util::s_pageSize == 0);
  grow();
}

ReadOnlyArena::~ReadOnlyArena() {
  for (auto& chunk : m_chunks) {
    munmap(chunk, m_chunkSize);
  }
}

const void* ReadOnlyArena::allocate(const void* data, size_t dataLen) {
  always_assert(dataLen <= m_chunkSize);
  guard g(m_mutex);

  // Round up to the minimal alignment.
  auto alignedLen =
    (dataLen + (kMinimalAlignment - 1)) & ~(kMinimalAlignment - 1);

  if (m_frontier + alignedLen > m_end) {
    grow();
  }
  always_assert(m_frontier + alignedLen <= m_end);

  auto const ret = m_frontier;
  assert((uintptr_t(ret) & (kMinimalAlignment - 1)) == 0);

  m_frontier += alignedLen;

  auto pageAddr = reinterpret_cast<unsigned char*>(
    uintptr_t(ret) & ~(Util::s_pageSize - 1)
  );
  mprotect(pageAddr, m_frontier - pageAddr, PROT_WRITE|PROT_READ);
  auto const ucData = static_cast<const unsigned char*>(data);
  std::copy(ucData, ucData + dataLen, ret);
  mprotect(pageAddr, m_frontier - pageAddr, PROT_READ);

  return ret;
}

// Pre: mutex already held, or no other threads may be able to access
// this (i.e. it's the ctor).
void ReadOnlyArena::grow() {
  void* vp = mmap(nullptr, m_chunkSize, PROT_READ,
    MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (vp == MAP_FAILED) {
    folly::throwSystemError("failed to mmap in ReadOnlyArena");
  }

  auto uc = static_cast<unsigned char*>(vp);
  m_chunks.push_back(uc);
  m_frontier = uc;
  m_end = uc + m_chunkSize;
}

size_t ReadOnlyArena::capacity() const {
  guard g(m_mutex);
  return m_chunks.size() * m_chunkSize;
}

//////////////////////////////////////////////////////////////////////

}

