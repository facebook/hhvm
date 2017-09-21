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

#ifndef incl_HPHP_DATA_BLOCK_H
#define incl_HPHP_DATA_BLOCK_H

#include <cstdint>
#include <cstring>
#include <map>
#include <set>

#include <folly/Bits.h>
#include <folly/Format.h>
#include <folly/portability/SysMman.h>

#include "hphp/util/arch.h"
#include "hphp/util/assertions.h"

namespace HPHP {

namespace sz {
  constexpr int nosize = 0;
  constexpr int byte  = 1;
  constexpr int word  = 2;
  constexpr int dword = 4;
  constexpr int qword = 8;
}

using Address          = uint8_t*;
using CodeAddress      = uint8_t*;
using ConstCodeAddress = const uint8_t*;

struct DataBlockFull : std::runtime_error {
  std::string name;

  DataBlockFull(const std::string& blockName, const std::string msg)
      : std::runtime_error(msg)
      , name(blockName)
    {}

  ~DataBlockFull() noexcept override {}
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
  DataBlock() = default;

  DataBlock(DataBlock&& other) = default;
  DataBlock& operator=(DataBlock&& other) = default;

  /**
   * Uses an existing chunk of memory.
   *
   * Addresses returned by DataBlock will be in the range [start, start + sz),
   * while writes and reads will happen from the range [dest, dest + sz).
   */
  void init(Address start, Address dest, size_t sz, size_t maxGrow,
            const char* name) {
    assertx(dest != start || sz == maxGrow);

    m_base = m_frontier = start;
    m_destBase = dest;
    m_size = sz;
    m_maxGrow = maxGrow;
    m_name = name;
  }

  void init(Address start, Address dest, size_t sz, const char* name) {
    init(start, dest, sz, sz, name);
  }

  void init(Address start, size_t sz, const char* name) {
    init(start, start, sz, sz, name);
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
    auto const nf = (uint8_t*)(((uintptr_t)m_frontier + align) & ~align);
    assertCanEmit(nf - m_frontier + sz);
    setFrontier(nf);
    auto data = m_frontier;
    m_frontier += sz;
    assertx(m_frontier <= m_base + m_size);
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

  bool grow(size_t nBytes) {
    if (m_maxGrow == m_size) return false;
    assertx(m_destBase != m_base);

    auto const need = nBytes - available();
    auto const amt = std::min(std::max(m_size + need, 2 * m_size), m_maxGrow);
    if (amt < m_size + need) return false;
    if (!m_destBuf) {
      m_destBuf.reset((Address)::malloc(amt));
      ::memcpy(m_destBuf.get(), m_destBase, used());
    } else {
      m_destBuf.reset((Address)::realloc(m_destBuf.release(), amt));
    }
    if (!m_destBuf) reportMallocError(amt);
    m_destBase = m_destBuf.get();
    m_size = amt;
    return true;
  }

  void assertCanEmit(size_t nBytes) {
    if (!canEmit(nBytes) && !grow(nBytes)) reportFull(nBytes);
  }

  [[noreturn]]
  void reportFull(size_t nbytes) const;

  [[noreturn]]
  void reportMallocError(size_t nbytes) const;

  bool isValidAddress(const CodeAddress tca) const {
    return tca >= m_base && tca < (m_base + m_size);
  }

  void byte(const uint8_t byte) {
    assertCanEmit(sz::byte);
    *dest() = byte;
    m_frontier += sz::byte;
  }
  void word(const uint16_t word) {
    assertCanEmit(sz::word);
    *(uint16_t*)dest() = word;
    m_frontier += sz::word;
  }
  void dword(const uint32_t dword) {
    assertCanEmit(sz::dword);
    *(uint32_t*)dest() = dword;
    m_frontier += sz::dword;
  }
  void qword(const uint64_t qword) {
    assertCanEmit(sz::qword);
    *(uint64_t*)dest() = qword;
    m_frontier += sz::qword;
  }

  void bytes(size_t n, const uint8_t *bs) {
    assertCanEmit(n);
    if (n <= 8 && m_destBase == m_base) {
      // If it is a modest number of bytes, try executing in one machine
      // store. This allows control-flow edges, including nop, to be
      // appear idempotent on other CPUs. If m_destBase != m_base then the
      // current block is a temporary buffer and this write is neither required
      // nor safe, as we may override an adjacent buffer or write off the end
      // of an allocation.
      union {
        uint64_t qword;
        uint8_t bytes[8];
      } u;
      u.qword = *(uint64_t*)dest();
      for (size_t i = 0; i < n; ++i) {
        u.bytes[i] = bs[i];
      }

      // If this address spans cache lines, on x64 this is not an atomic store.
      // This being the case, use caution when overwriting code that is
      // reachable by multiple threads: make sure it doesn't span cache lines.
      *reinterpret_cast<uint64_t*>(dest()) = u.qword;
    } else {
      memcpy(dest(), bs, n);
    }
    m_frontier += n;
  }

  void skip(size_t nbytes) {
    alloc<uint8_t>(1, nbytes);
  }

  Address base() const { return m_base; }
  Address frontier() const { return m_frontier; }
  std::string name() const { return m_name; }

  /*
   * DataBlock can emit into a range [A, B] while returning addresses in range
   * [A', B']. This function  will map an address in [A', B'] into [A, B], and
   * it must be used before writing or reading from any address returned by
   * DataBlock.
   */
  Address toDestAddress(CodeAddress addr) {
    assertx(m_base <= addr && addr <= (m_base + m_size));
    return Address(m_destBase + (addr - m_base));
  }

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

  bool contains(ConstCodeAddress addr) const {
    return addr >= m_base && addr < (m_base + m_size);
  }

  bool empty() const {
    return m_base == m_frontier;
  }

  void clear() {
    setFrontier(m_base);
  }

  void zero() {
    memset(m_destBase, 0, m_frontier - m_base);
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

  void sync(Address begin = nullptr,  Address end = nullptr) {
    if (!begin) begin = m_base;
    if (!end) end = m_frontier;
    syncDirect(toDestAddress(begin), toDestAddress(end));
  }

  static void syncDirect(Address begin,  Address end) {
    if (arch() == Arch::ARM) {
      __builtin___clear_cache(reinterpret_cast<char*>(begin),
                              reinterpret_cast<char*>(end));

    }
  }

private:

  using Offset = uint32_t;
  using Size = uint32_t;

  // DataBlock can optionally be growable. The initial expansion of DataBlock
  // will allocate a new buffer that is owned by the DataBlock, subsequent
  // expansions will use realloc to expand this block until m_maxGrow has been
  // reached. Only DataBlocks which have a different m_base from m_destBase may
  // be grown, as expansion may move the location of m_destBase.
  struct Deleter final { void operator()(uint8_t* a) const { ::free(a); } };
  std::unique_ptr<uint8_t, Deleter> m_destBuf{nullptr};

  Address dest() const { return m_destBase + (m_frontier - m_base); }

  // DataBlock can track an alternate pseudo-frontier to support clients that
  // wish to emit code to one location while keeping memory references relative
  // to a separate location. The actual writes will be to m_dest.
  Address m_destBase{nullptr};

  Address m_base{nullptr};
  Address m_frontier{nullptr};
  size_t  m_size{0};
  size_t  m_maxGrow{0};
  std::string m_name;

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
