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

#ifndef incl_HPHP_UTIL_BUMP_MAPPER_H_
#define incl_HPHP_UTIL_BUMP_MAPPER_H_

#include <atomic>
#include <mutex>
#include <stddef.h>

namespace HPHP { namespace alloc {

enum class LockPolicy : uint8_t {
  Blocking,
  FailFast,
};

/*
 * Bump allocation policy within the fixed address range [m_base, m_base +
 * m_maxCapacity).  Lower address will be used first.
 */
struct BumpAllocState {
  BumpAllocState(uintptr_t base, size_t maxCap, LockPolicy p);

  // Number of bytes mapped in memory (not the reserved size)
  size_t mappedSize() const { return m_currCapacity; }
  // Number of bytes given to jemalloc.
  size_t allocatedSize() { return m_size.load(std::memory_order_relaxed); }
  // Maximum number of bytes allowed for the arena.
  size_t maxCapacity() const { return m_maxCapacity; }
  // Lowest mapped address.
  uintptr_t frontier() const { return m_base + m_currCapacity; }

  const uintptr_t m_base;
  const size_t m_maxCapacity;
  const LockPolicy m_lockPolicy;
  std::atomic_size_t m_size{0};
  size_t m_currCapacity{0};
  std::mutex m_mutex;                   // Protects m_currCapacity
};

/*
 * A (NUMA-aware) BumpMapper extends memory mapping for use in a
 * BumpExtentAllocator.
 *
 * Subclasses customize its behavior in addMappingImpl().
 */
struct BumpMapper {
  explicit BumpMapper(uint32_t maxPages, uint32_t numaMask = 0,
                      short nextNode = 0)
    : m_maxNumPages(maxPages)
    , m_interleaveMask(numaMask)
    , m_nextNode(nextNode) {}
  virtual ~BumpMapper() {}

  // Append a fallback mapper to the end of this chain.
  void append(BumpMapper* m);

  static inline bool trivial(const BumpAllocState& state, size_t newSize) {
    return newSize <= state.m_currCapacity;
  }
  static inline bool infeasible(const BumpAllocState& state, size_t newSize) {
    return newSize > state.m_maxCapacity;
  }

  // Add usable mappings (possibly backed by huge pages) to the usable range of
  // memory.  Return false if it failed.
  bool addMapping(BumpAllocState& state, size_t newSize) {
    if (trivial(state, newSize)) return true;
    if (infeasible(state, newSize)) return false;
    if (!m_failed && addMappingImpl(state, newSize)) return true;
    return m_fallback && m_fallback->addMapping(state, newSize);
  }

 protected:
  // Different subclasses will add pages in different ways.  Return whether new
  // mappings have been added.  Note that this doesn't guarantee successful
  // allocation in multi-threaded environment.
  virtual bool addMappingImpl(BumpAllocState& state, size_t newSize) = 0;

  // An upper bound for the number of pages may be specified upon creation.
  // This is primarily used to limit the number of huge pages used in an arena.
  // If the mapper doesn't use huge pages, it can choose not to look at it in
  // `addMappingImpl()`, and that's why we don't check m_currNumPages <
  // m_maxNumPages in `addMapping()`.
  uint32_t m_maxNumPages{0};
  uint32_t m_currNumPages{0};
  // NUMA interleave masks for the newly added memory.  0 indicates no-NUMA.
  uint32_t m_interleaveMask{0};
  // next NUMA node to grab a page from
  short m_nextNode{0};
  // Whether the current mapper still has failed permanently.  If so we go to
  // the fallback mapper without trying.
  bool m_failed{false};
  // Delegate to another mapper when the current one fails.  This forms a linked
  // list, or a 'chain of responsibility'..
  BumpMapper* m_fallback{nullptr};
};

struct Bump1GMapper : public BumpMapper {
  template<typename... Args>
  explicit Bump1GMapper(Args&&... args)
    : BumpMapper(std::forward<Args>(args)...) {}
 protected:
  bool addMappingImpl(BumpAllocState& state, size_t newSize) override;
};

struct Bump4KMapper : public BumpMapper {
  // Only the NUMA mask is relevant.  `m_maxNumPages` is ignored for this
  // mapper.
  explicit Bump4KMapper(uint32_t numaMask = 0)
    : BumpMapper(0, numaMask) {}
 protected:
  bool addMappingImpl(BumpAllocState& state, size_t newSize) override;
};

// Bump4KMapper with transparent huge pages.
struct Bump2MMapper : public Bump4KMapper {
  template<typename... Args>
  explicit Bump2MMapper(uint32_t numaMask = 0)
    : Bump4KMapper(numaMask) {}

  inline void setMaxPages(uint32_t max2MPages) {
    m_maxNumPages = max2MPages;
  }

 protected:
  bool addMappingImpl(BumpAllocState& state, size_t newSize) override;
};

}}

#endif
