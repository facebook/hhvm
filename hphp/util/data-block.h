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

#ifndef incl_HPHP_DATA_BLOCK_H
#define incl_HPHP_DATA_BLOCK_H

#include <boost/noncopyable.hpp>
#include <cstdint>
#include <sys/mman.h>

#include "hphp/util/assertions.h"
#include "hphp/util/trace.h"
#include "hphp/util/util.h"

namespace HPHP { namespace Transl {

#define TRACEMOD ::HPHP::Trace::datablock

namespace sz {
  constexpr int nosize = 0;
  constexpr int byte  = 1;
  constexpr int word  = 2;
  constexpr int dword = 4;
  constexpr int qword = 8;
}

typedef uint8_t* Address;
typedef uint8_t* CodeAddress;

/**
 * DataBlock is a simple bump-allocating wrapper around a chunk of memory.
 */
struct DataBlock : private boost::noncopyable {

  DataBlock() : m_base(nullptr), m_frontier(nullptr), m_size(0) {}

  DataBlock(DataBlock&& other)
    : m_base(other.m_base), m_frontier(other.m_frontier), m_size(other.m_size) {
    other.m_base = other.m_frontier = nullptr;
    other.m_size = 0;
  }

  DataBlock& operator=(DataBlock&& other) {
    m_base = other.m_base;
    m_frontier = other.m_frontier;
    m_size = other.m_size;
    other.m_base = other.m_frontier = nullptr;
    other.m_size = 0;
    return *this;
  }

  /**
   * Uses an existing chunk of memory.
   */
  void init(Address start, size_t sz) {
    m_base = m_frontier = start;
    m_size = sz;
  }

  /*
   * alloc --
   *
   *   Simple bump allocator.
   *
   * allocAt --
   *
   *   Some clients need to allocate with an externally maintained frontier.
   *   allocAt supports this.
   */
  void* allocAt(size_t &frontierOff, size_t sz, size_t align = 16) {
    align = Util::roundUpToPowerOfTwo(align);
    uint8_t* frontier = m_base + frontierOff;
    assert(m_base && frontier);
    int slop = uintptr_t(frontier) & (align - 1);
    if (slop) {
      int leftInBlock = (align - slop);
      frontier += leftInBlock;
      frontierOff += leftInBlock;
    }
    assert((uintptr_t(frontier) & (align - 1)) == 0);
    frontierOff += sz;
    assert(frontierOff <= m_size);
    return frontier;
  }

  template<typename T> T* alloc(size_t align = 16, int n = 1) {
    size_t frontierOff = m_frontier - m_base;
    T* retval = (T*)allocAt(frontierOff, sizeof(T) * n, align);
    m_frontier = m_base + frontierOff;
    return retval;
  }

  bool canEmit(size_t nBytes) {
    assert(m_frontier >= m_base);
    assert(m_frontier <= m_base + m_size);
    return m_frontier + nBytes <= m_base + m_size;
  }

  bool isValidAddress(const CodeAddress tca) const {
    return tca >= m_base && tca < (m_base + m_size);
  }

  void byte(const uint8_t byte) {
    always_assert(canEmit(sz::byte));
    TRACE(10, "%p b : %02x\n", m_frontier, byte);
    *m_frontier = byte;
    m_frontier += sz::byte;
  }
  void word(const uint16_t word) {
    always_assert(canEmit(sz::word));
    *(uint16_t*)m_frontier = word;
    TRACE(10, "%p w : %04x\n", m_frontier, word);
    m_frontier += sz::word;
  }
  void dword(const uint32_t dword) {
    always_assert(canEmit(sz::dword));
    TRACE(10, "%p d : %08x\n", m_frontier, dword);
    *(uint32_t*)m_frontier = dword;
    m_frontier += sz::dword;
  }
  void qword(const uint64_t qword) {
    always_assert(canEmit(sz::qword));
    TRACE(10, "%p q : %016" PRIx64 "\n", m_frontier, qword);
    *(uint64_t*)m_frontier = qword;
    m_frontier += sz::qword;
  }

  void bytes(size_t n, const uint8_t *bs) {
    always_assert(canEmit(n));
    TRACE(10, "%p [%ld b] : [%p]\n", m_frontier, n, bs);
    if (n <= 8) {
      // If it is a modest number of bytes, try executing in one machine
      // store. This allows control-flow edges, including nop, to be
      // appear idempotent on other CPUs.
      union {
        uint64_t qword;
        uint8_t bytes[8];
      } u;
      u.qword = *(uint64_t*)m_frontier;
      for (size_t i = 0; i < n; ++i) {
        u.bytes[i] = bs[i];
      }

      // If this address doesn't span cache lines, on x64 this is an
      // atomic store.  We're not using atomic_release_store() because
      // this code path occurs even when it may span cache lines, and
      // that function asserts about this.
      *reinterpret_cast<uint64_t*>(m_frontier) = u.qword;
    } else {
      memcpy(m_frontier, bs, n);
    }
    m_frontier += n;
  }

  void skip(size_t nbytes) {
    alloc<uint8_t>(1, nbytes);
  }

  Address base() const { return m_base; }
  Address frontier() const { return m_frontier; }

  void setFrontier(Address addr) {
    m_frontier = addr;
  }

  size_t capacity() const {
    return m_size;
  }

  size_t used() const {
    return m_frontier - m_base;
  }

  size_t available() const {
    return m_size - (m_frontier - m_base);
  }

  bool contains(CodeAddress addr) const {
    return addr >= m_base && addr < (m_base + m_size);
  }

  bool empty() const {
    return m_base == m_frontier;
  }

  void clear() {
    m_frontier = m_base;
  }

 protected:
  Address m_base;
  Address m_frontier;
  size_t m_size;
};

typedef DataBlock CodeBlock;

#undef TRACEMOD

}}

#endif
