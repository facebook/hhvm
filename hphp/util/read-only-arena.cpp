/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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
#include <stdlib.h>

#include <folly/Exception.h>

#include "hphp/util/assertions.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

typedef std::lock_guard<std::mutex> guard;

namespace {

/*
 * Check return codes on mprotect---we're going to hand memory back to malloc
 * that we've fiddled with protection on, so if something fails it could cause
 * issues in completely unrelated parts of the code.
 */
void checked_mprotect(void* addr, size_t size, int prot) {
  auto const rval = mprotect(addr, size, prot);
  always_assert_flog(rval == 0, "mprotect failed in ReadOnlyArena");
}

}

//////////////////////////////////////////////////////////////////////

ReadOnlyArena::ReadOnlyArena(size_t minChunkSize)
  : m_minChunkSize((minChunkSize + (s_pageSize - 1)) & ~(s_pageSize - 1))
  , m_frontier(nullptr)
  , m_end(nullptr)
{
  always_assert(m_minChunkSize % s_pageSize == 0);
}

ReadOnlyArena::~ReadOnlyArena() {
  for (auto& chunk : m_chunks) {
    checked_mprotect(chunk.data(), chunk.size(), PROT_READ|PROT_WRITE);
    std::free(chunk.data());
  }
}

const void* ReadOnlyArena::allocate(const void* data, size_t dataLen) {
  guard g(m_mutex);

  // Round up to the minimal alignment.
  auto alignedLen =
    (dataLen + (kMinimalAlignment - 1)) & ~(kMinimalAlignment - 1);

  ensureFree(alignedLen);
  always_assert(m_frontier + alignedLen <= m_end);

  auto const ret = m_frontier;
  assert((uintptr_t(ret) & (kMinimalAlignment - 1)) == 0);

  m_frontier += alignedLen;

  auto pageAddr = reinterpret_cast<unsigned char*>(
    uintptr_t(ret) & ~(s_pageSize - 1)
  );
  checked_mprotect(pageAddr, m_frontier - pageAddr, PROT_WRITE|PROT_READ);
  auto const ucData = static_cast<const unsigned char*>(data);
  std::copy(ucData, ucData + dataLen, ret);
  checked_mprotect(pageAddr, m_frontier - pageAddr, PROT_READ);

  return ret;
}

// Pre: mutex already held, or no other threads may be able to access this
// (i.e. it's the ctor).
//
// Post: m_end - m_frontier >= bytes
void ReadOnlyArena::ensureFree(size_t bytes) {
  if (m_end - m_frontier >= bytes) return;

  // Allocate a multiple of s_pageSize, and no less than m_minChunkSize.
  if (bytes > m_minChunkSize) {
    bytes = (bytes + (s_pageSize - 1)) & ~(s_pageSize - 1);
  } else {
    bytes = m_minChunkSize;
  }

  void* vp;
  if (auto err = posix_memalign(&vp, s_pageSize, bytes)) {
    folly::throwSystemError(err, "failed to posix_memalign in "
                            "ReadOnlyArena");
  }
  checked_mprotect(vp, bytes, PROT_READ);

  auto uc = static_cast<unsigned char*>(vp);
  m_frontier = uc;
  m_end = uc + bytes;
  m_chunks.emplace_back(m_frontier, m_end);
}

size_t ReadOnlyArena::capacity() const {
  guard g(m_mutex);
  size_t size = 0;
  for (auto& chunk : m_chunks) {
    size += chunk.size();
  }
  return size;
}

//////////////////////////////////////////////////////////////////////

}

