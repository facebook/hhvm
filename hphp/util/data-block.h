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

#ifndef incl_HPHP_DATA_BLOCK_H
#define incl_HPHP_DATA_BLOCK_H

#include <map>
#include <set>
#include <cstdint>
#include <cstring>
#include <sys/mman.h>

#include <folly/Bits.h>
#include <folly/Format.h>

#include "hphp/util/assertions.h"

namespace HPHP {

namespace sz {
  constexpr int nosize = 0;
  constexpr int byte  = 1;
  constexpr int word  = 2;
  constexpr int dword = 4;
  constexpr int qword = 8;
}

typedef uint8_t* Address;
typedef uint8_t* CodeAddress;

struct DataBlockFull : std::runtime_error {
  std::string name;

  DataBlockFull(const std::string& blockName, const std::string msg)
      : std::runtime_error(msg)
      , name(blockName)
    {}

  ~DataBlockFull() noexcept {}
};

/**
 * DataBlock is a simple bump-allocating wrapper around a chunk of memory, with
 * basic tracking for unused memory and a simple interface to allocate it.
 *
 * Memory is allocated from the end of the block unless specifically allocated
 * using allocInner.
 *
 * Unused memory can be freed using free(). If the memory is at the end of the
 * block, the frontier will be moved back.
 *
 * Free memory is coalesced and allocation is done by best-fit.
 */
struct DataBlock {

  DataBlock() : m_base(nullptr), m_frontier(nullptr), m_size(0), m_name("") {}

  DataBlock(const DataBlock& other) = delete;
  DataBlock& operator=(const DataBlock& other) = delete;

  DataBlock(DataBlock&& other) noexcept
    : m_base(other.m_base)
    , m_frontier(other.m_frontier)
    , m_size(other.m_size)
    , m_name(other.m_name) {
    other.m_base = other.m_frontier = nullptr;
    other.m_size = 0;
    other.m_name = "";
  }

  DataBlock& operator=(DataBlock&& other) {
    m_base = other.m_base;
    m_frontier = other.m_frontier;
    m_size = other.m_size;
    m_name = other.m_name;
    other.m_base = other.m_frontier = nullptr;
    other.m_size = 0;
    other.m_name = "";
    return *this;
  }

  /**
   * Uses an existing chunk of memory.
   */
  void init(Address start, size_t sz, const char* name) {
    m_base = m_frontier = start;
    m_size = sz;
    m_name = name;
  }

  /*
   * allocRaw
   * alloc
   *
   * Simple bump allocator, supporting power-of-two alignment. alloc<T>() is a
   * simple typed wrapper around allocRaw().
   */
  void* allocRaw(size_t sz, size_t align = 16) {
    // Round frontier up to a multiple of align
    align = folly::nextPowTwo(align) - 1;
    m_frontier = (uint8_t*)(((uintptr_t)m_frontier + align) & ~align);
    auto data = m_frontier;
    m_frontier += sz;
    assertx(m_frontier < m_base + m_size);
    return data;
  }

  template<typename T> T* alloc(size_t align = 16, int n = 1) {
    return (T*)allocRaw(sizeof(T) * n, align);
  }

  bool canEmit(size_t nBytes) {
    assert(m_frontier >= m_base);
    assert(m_frontier <= m_base + m_size);
    return m_frontier + nBytes <= m_base + m_size;
  }

  void assertCanEmit(size_t nBytes) {
    if (!canEmit(nBytes)) reportFull(nBytes);
  }

  [[noreturn]]
  void reportFull(size_t nbytes) const;

  bool isValidAddress(const CodeAddress tca) const {
    return tca >= m_base && tca < (m_base + m_size);
  }

  bool isFrontierAligned(const size_t alignment) const {
    return ((uintptr_t)m_frontier & (alignment - 1)) == 0;
  }

  void byte(const uint8_t byte) {
    assertCanEmit(sz::byte);
    *m_frontier = byte;
    m_frontier += sz::byte;
  }
  void word(const uint16_t word) {
    assertCanEmit(sz::word);
    *(uint16_t*)m_frontier = word;
    m_frontier += sz::word;
  }
  void dword(const uint32_t dword) {
    assertCanEmit(sz::dword);
    *(uint32_t*)m_frontier = dword;
    m_frontier += sz::dword;
  }
  void qword(const uint64_t qword) {
    assertCanEmit(sz::qword);
    *(uint64_t*)m_frontier = qword;
    m_frontier += sz::qword;
  }

  void bytes(size_t n, const uint8_t *bs) {
    assertCanEmit(n);
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

      // If this address spans cache lines, on x64 this is not an atomic store.
      // This being the case, use caution when overwriting code that is
      // reachable by multiple threads: make sure it doesn't span cache lines.
      *reinterpret_cast<uint64_t*>(m_frontier) = u.qword;
    } else {
      memcpy(m_frontier, bs, n);
    }
    m_frontier += n;
  }

  void skip(size_t nbytes) {
    assertCanEmit(nbytes);
    alloc<uint8_t>(1, nbytes);
  }

  Address base() const { return m_base; }
  Address frontier() const { return m_frontier; }
  std::string name() const { return m_name; }

  void setFrontier(Address addr) {
    assertx(m_base <= addr && addr <= (m_base + m_size));
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

  void zero() {
    memset(m_base, 0, m_frontier - m_base);
    clear();
  }

  // Append address range to free list
  void free(void* addr, size_t len);

  // Attempt to allocate a range from within the free list
  void* allocInner(size_t len);

  size_t numFrees()   const { return m_nfree; }
  size_t numAllocs()  const { return m_nalloc; }
  size_t bytesFree()  const { return m_bytesFree; }
  size_t blocksFree() const { return m_freeRanges.size(); }

 protected:
  Address m_base;
  Address m_frontier;
  size_t  m_size;
  std::string m_name;

  using Offset = uint32_t;
  using Size = uint32_t;

  size_t m_nfree{0};
  size_t m_nalloc{0};

  size_t m_bytesFree{0};
  std::unordered_map<Offset, int64_t> m_freeRanges;
  std::map<Size, std::unordered_set<Offset>> m_freeLists;
};

using CodeBlock = DataBlock;

//////////////////////////////////////////////////////////////////////

struct UndoMarker {
  explicit UndoMarker(CodeBlock& cb)
    : m_cb(cb)
    , m_oldFrontier(cb.frontier()) {
  }

  void undo() {
    m_cb.setFrontier(m_oldFrontier);
  }

private:
  CodeBlock& m_cb;
  CodeAddress m_oldFrontier;
};

/*
 * RAII bookmark for scoped rewinding of frontier.
 */
struct CodeCursor : UndoMarker {
  CodeCursor(CodeBlock& cb, CodeAddress newFrontier) : UndoMarker(cb) {
    cb.setFrontier(newFrontier);
  }

  ~CodeCursor() { undo(); }
};
}

#endif
