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

#ifndef incl_HPHP_MEMORY_MANAGER_H_
#define incl_HPHP_MEMORY_MANAGER_H_

#include <boost/noncopyable.hpp>

#include "folly/Memory.h"

#include "hphp/util/alloc.h" // must be included before USE_JEMALLOC is used
#include "hphp/util/trace.h"
#include "hphp/util/thread-local.h"
#include "hphp/runtime/base/memory-usage-stats.h"

#include <array>
#include <vector>
#include <deque>
#include <queue>
#include <map>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

struct SweepNode {
  SweepNode* next;
  union {
    SweepNode* prev;
    size_t padbytes;
  };
};

// jemalloc uses 0x5a but we use 0x6a so we can tell the difference
// when debugging.  There's also 0x7a for some cases of ex-TypedValue
// memory.
constexpr char kSmartFreeFill = 0x6a;
constexpr char kTVTrashFill = 0x7a;
constexpr uintptr_t kSmartFreeWord = 0x6a6a6a6a6a6a6a6aLL;
constexpr uintptr_t kMallocFreeWord = 0x5a5a5a5a5a5a5a5aLL;

/**
 * A garbage list is a freelist of items that uses the space in the items
 * to store a singly linked list.
 */
class GarbageList {
  struct NakedNode { NakedNode* next; };
  NakedNode* ptr;
public:
  GarbageList() : ptr(nullptr) {
  }

  // Pops an item, or returns NULL
  void* maybePop() {
    auto ret = ptr;
    if (LIKELY(ret != nullptr)) {
      ptr = ret->next;
    }
    return ret;
  }

  // Pushes an item on to the list. The item must be larger than
  // sizeof(void*)
  void push(void* val) {
    auto convval = (NakedNode*)val;
    convval->next = ptr;
    ptr = convval;
  }

  // Number of items on the list.  We calculate this iteratively
  // on the assumption we don't query this often, so iterating is
  // faster than keeping a size field up-to-date.
  uint size() const {
    uint sz = 0;
    for (Iterator it = begin(), e = end(); it != e; ++it, ++sz) {}
    return sz;
  }

  bool empty() const {
    return !ptr;
  }

  // Remove all items from this list
  void clear() {
    ptr = nullptr;
  }

  class Iterator {
  public:
    explicit Iterator(const GarbageList& lst) : curptr(lst.ptr) {}

    Iterator(const Iterator &other) : curptr(other.curptr) {}
    Iterator() : curptr(nullptr) {}

    bool operator==(const Iterator &it) {
      return curptr == it.curptr;
    }

    bool operator!=(const Iterator &it) {
      return !operator==(it);
    }

    Iterator &operator++() {
      assert(curptr);
      curptr = curptr->next;
      return *this;
    }

    Iterator operator++(int) {
      auto ret(*this);
      operator++();
      return ret;
    }

    void* operator*() const {
      return curptr;
    }

  private:
    NakedNode* curptr;
  };

  Iterator begin() const {
    return Iterator(*this);
  }

  Iterator end() const {
    return Iterator();
  }

  typedef Iterator iterator;
};

/**
 * MemoryManager categorizes memory usage into 3 categories and maintain some
 * of them with different strategy:
 *
 *  1. Fixed size objects: de/allocated by SmartAllocators, these objects have
 *     exactly the same size.
 *  2. Interally malloc-ed and variable sized memory held by fixed size
 *     objects, for example, StringData's m_data.
 *  3. Freelance memory, malloced by extensions or STL classes, that are
 *     completely out of MemoryManager's control.
 */
struct MemoryManager : boost::noncopyable {
  typedef ThreadLocalSingleton<MemoryManager> TlsWrapper;
  struct MaskAlloc;

  static constexpr size_t kMaxSmartSize = 2048;

  static void Create(void* storage);
  static void Delete(MemoryManager*);
  static void OnThreadExit(MemoryManager*);
  static MemoryManager* TheMemoryManager() {
    return TlsWrapper::getNoCheck();
  }

  MemoryManager();

  /**
   * Mark current allocator's position as ending point of a generation and
   * sweep all memory that has allocated.
   */
  void sweepAll();
  void rollback();

  /**
   * Display any leaked or double-freed memory.
   */
  void checkMemory();

  /**
   * Find out how much memory we have used so far.
   */
  MemoryUsageStats &getStats(bool refresh=false) {
    if (refresh) {
      refreshStats();
    }
    return m_stats;
  }

  /**
   * Get most recent stats data, as one would with getStats(true) without
   * altering the underlying data in m_stats. Used for obtaining debug info.
   */
  void getStatsSafe(MemoryUsageStats &stats) {
    stats = m_stats;
    refreshStats<false>(stats);
  }

  /**
   * Called during session starts to reset all usage stats.
   */
  void resetStats();

  /**
   * Out-of-line version of refresh stats
   */
  void refreshStatsHelper();

  void refreshStats() {
    refreshStats<true>(m_stats);
  }

  /**
   * Refresh stats to reflect directly malloc()ed memory, and determine whether
   * the request memory limit has been exceeded.
   * The stats parameter allows the updates to be applied to either m_stats as
   * in refreshStats() or to a seperate MemoryUsageStats struct as in
   * getStatsSafe().
   * The template variable live controls whether or not MemoryManager member
   * variables are updated and whether or not to call helper methods in
   * response to memory anomolies.
   */
  template<bool live> void refreshStats(MemoryUsageStats &stats) {
#ifdef USE_JEMALLOC
    // Incrementally incorporate the difference between the previous and current
    // deltas into the memory usage statistic.  For reference, the total
    // malloced memory usage could be calculated as such, if delta0 were
    // recorded in resetStats():
    //
    //   int64 musage = delta - delta0;
    //
    // Note however, that SmartAllocator adds to m_stats.jemallocDebt
    // when it calls malloc(), so that this function can avoid
    // double-counting the malloced memory. Thus musage in the example
    // code may well substantially exceed m_stats.usage.
    if (s_statsEnabled) {
      int64_t delta = int64_t(*m_allocated) - int64_t(*m_deallocated);
      int64_t deltaAllocated = int64_t(*m_allocated) - m_prevAllocated;
      stats.usage += delta - m_delta - stats.jemallocDebt;
      stats.jemallocDebt = 0;
      stats.totalAlloc += deltaAllocated;
      if (live) {
        m_delta = delta;
        m_prevAllocated = int64_t(*m_allocated);
      }
    }
#endif
    if (stats.usage > stats.peakUsage) {
      // NOTE: the peak memory usage monotonically increases, so there cannot
      // be a second OOM exception in one request.
      assert(stats.maxBytes > 0);
      if (live && stats.peakUsage <= stats.maxBytes &&
          stats.usage > stats.maxBytes) {
        refreshStatsHelperExceeded();
      }
      // Check whether the process's active memory limit has been exceeded, and
      // if so, stop the server.
      //
      // Only check whether the total memory limit was exceeded if this request
      // is at a new high water mark.  This check could be performed regardless
      // of this request's current memory usage (because other request threads
      // could be to blame for the increased memory usage), but doing so would
      // measurably increase computation for little benefit.
#ifdef USE_JEMALLOC
      // (*m_cactive) consistency is achieved via atomic operations.  The fact
      // that we do not use an atomic operation here means that we could get a
      // stale read, but in practice that poses no problems for how we are
      // using the value.
      if (live && s_statsEnabled && *m_cactive > m_cactiveLimit) {
        refreshStatsHelperStop();
      }
#endif
      stats.peakUsage = stats.usage;
    }
  }

  struct MaskAlloc {
    explicit MaskAlloc(MemoryManager* mm) : m_mm(mm) {
      // capture all mallocs prior to construction
      m_mm->refreshStats();
    }
    ~MaskAlloc() {
  #ifdef USE_JEMALLOC
      // exclude mallocs and frees since construction
      if (s_statsEnabled) {
        m_mm->m_prevAllocated = int64_t(*m_mm->m_allocated);
        m_mm->m_delta = int64_t(*m_mm->m_allocated) -
          int64_t(*m_mm->m_deallocated);
      }
  #endif
    }

    MaskAlloc(const MaskAlloc&) = delete;
    MaskAlloc& operator=(const MaskAlloc&) = delete;

  private:
    MemoryManager* const m_mm;
  };

  /*
   * Return the smart size class for a given requested allocation
   * size.
   *
   * The return value is greater than or equal to the parameter, and
   * less than or equal to MaxSmallSize.
   *
   * Pre: requested <= kMaxSmartSize
   */
  static uint32_t smartSizeClass(uint32_t requested);

  /*
   * Allocate/deallocate a smart-allocated memory block in a given
   * small size class.  You must be able to tell the deallocation
   * function how big the allocation was.
   *
   * The size passed to smartMallocSize does not need to be an exact
   * size class.  The size passed to smartFreeSize must be the exact
   * size that was passed to smartMallocSize for that allocation.
   *
   * The returned pointer is guaranteed to be 16-byte aligned.
   *
   * Pre: size > 0 && size <= kMaxSmartSize
   */
  void* smartMallocSize(uint32_t size);
  void smartFreeSize(void* p, uint32_t size);

  /*
   * Helper for allocating objects---uses the small size classes if
   * size is small enough, and otherwise smartMallocSizeBig.
   */
  ALWAYS_INLINE void* objMalloc(size_t size) {
    if (LIKELY(size <= kMaxSmartSize)) {
      return smartMallocSize(size);
    }
    return smartMallocSizeBig(size).first;
  }

  ALWAYS_INLINE void objFree(void* vp, size_t size) {
    if (LIKELY(size <= kMaxSmartSize)) {
      return smartFreeSize(vp, size);
    }
    return smartFreeSizeBig(vp, size);
  }

  /*
   * Allocate/deallocate smart-allocated memory that is too big for
   * the small size classes.
   *
   * Returns a pointer and the actual size of the allocation, which
   * may be larger than the requested size.  The returned pointer is
   * guaranteed to be 16-byte aligned.
   */
  std::pair<void*,size_t> smartMallocSizeBig(size_t size);
  void smartFreeSizeBig(void* vp, size_t size);

  // allocate nbytes from the current slab, aligned to 16-bytes
  void* slabAlloc(size_t nbytes);

private:
  friend void* smart_malloc(size_t nbytes);
  friend void* smart_calloc(size_t count, size_t bytes);
  friend void* smart_realloc(void* ptr, size_t nbytes);
  friend void  smart_free(void* ptr);

  struct SmallNode {
    size_t padbytes;  // <= kMaxSmartSize means small block
  };

  /*
   * Debug mode header.
   *
   * This sits in front of the user payload for small allocations, and
   * in front of the SweepNode in big allocations.  The allocatedMagic
   * aliases the space for the GarbageList pointers, but should catch
   * double frees due to kAllocatedMagic.
   *
   * We set requestedSize to kFreedMagic when a block is not
   * allocated.
   */
  struct DebugHeader {
    static constexpr uintptr_t kAllocatedMagic =
                               (static_cast<size_t>(1) << 63) - 0xfac3;
    static constexpr size_t kFreedMagic = static_cast<size_t>(-1);

    uintptr_t allocatedMagic;
    size_t requestedSize;
    size_t returnedCap;
    size_t padding;
  };

  static constexpr unsigned kLgSizeQuantum = 4; // 16 bytes
  static constexpr unsigned kNumSizes = kMaxSmartSize >> kLgSizeQuantum;
  static constexpr size_t kSmartSizeMask = (1 << kLgSizeQuantum) - 1;

private:
  char* newSlab(size_t nbytes);
  void* smartEnlist(SweepNode*);
  void* smartMallocSlab(size_t padbytes);
  void* smartMallocBig(size_t nbytes);
  void* smartCallocBig(size_t nbytes);
  void  smartFreeBig(SweepNode*);
  void* smartMalloc(size_t nbytes);
  void* smartRealloc(void* ptr, size_t nbytes);
  void  smartFree(void* ptr);
  void refreshStatsHelperExceeded();
#ifdef USE_JEMALLOC
  void refreshStatsHelperStop();
  void* smartMallocSizeBigHelper(void*&, size_t&, size_t);
#endif
  bool checkPreFree(DebugHeader*, size_t, size_t);
  template<class SizeT> static SizeT debugAddExtra(SizeT);
  template<class SizeT> static SizeT debugRemoveExtra(SizeT);
  void* debugPostAllocate(void*, size_t, size_t);
  void* debugPreFree(void*, size_t, size_t);

private:
  TRACE_SET_MOD(smartalloc);

  static void* TlsInitSetup;

  char* m_front;
  char* m_limit;
  std::array<GarbageList,kNumSizes> m_sizeTrackedFree;
  std::array<GarbageList,kNumSizes> m_sizeUntrackedFree;
  SweepNode m_sweep;   // oversize smart_malloc'd blocks
  SweepNode m_strings; // in-place node is head of circular list
  MemoryUsageStats m_stats;

  std::vector<char*> m_slabs;

#ifdef USE_JEMALLOC
  uint64_t* m_allocated;
  uint64_t* m_deallocated;
  int64_t m_delta;
  int64_t m_prevAllocated;
  size_t* m_cactive;
  size_t m_cactiveLimit;

public:
  static bool s_statsEnabled;
  static size_t s_cactiveLimitCeiling;
#endif

  friend class StringData; // for enlist/delist access to m_strings
};

/*
 * smart_malloc api for request-scoped memory
 *
 * This is the most generic entry point to the SmartAllocator.  If you
 * easily know the size of the allocation at free time, it might be
 * more efficient to use MM() apis directory.
 *
 * These functions behave like C's malloc/free, but get memory from
 * the current thread's MemoryManager instance.  At request-end, any
 * un-freed memory is explicitly freed (and in debug, garbage filled).
 * If any pointers to this memory survive beyond a request, they'll be
 * dangling pointers.
 *
 * Block sizes <= MemoryManager::kMaxSmartSize are region-allocated
 * and are only guaranteed to be 8-byte aligned.  Larger blocks are
 * directly malloc'd (with a header) and are 16-byte aligned.
 *
 */
void* smart_malloc(size_t nbytes);
void* smart_calloc(size_t count, size_t bytes);
void* smart_realloc(void* ptr, size_t nbytes);
void  smart_free(void* ptr);

/*
 * Similar to smart_malloc, but with support for constructors.  Note
 * that explicitly calling smart_delete will run the destructors, but
 * if you let the allocator sweep it the destructors will not be
 * called.
 */
template<class T> T* smart_new_array(size_t count) {
  T* ret = static_cast<T*>(smart_malloc(count * sizeof(T)));
  size_t i = 0;
  try {
    for (; i < count; ++i) {
      new (&ret[i]) T();
    }
  } catch (...) {
    size_t j = i;
    while (j-- > 0) {
      ret[j].~T();
    }
    smart_free(ret);
    throw;
  }
  return ret;
}

// Unlike the normal operator delete, this requires ~T() must be
// nothrow.
template<class T>
void smart_delete_array(T* t, size_t count) {
  size_t i = count;
  while (i-- > 0) {
    t[i].~T();
  }
  smart_free(t);
}

inline MemoryManager& MM() {
  return *MemoryManager::TheMemoryManager();
}

///////////////////////////////////////////////////////////////////////////////

}

#include "hphp/runtime/base/memory-manager-inl.h"

#endif
