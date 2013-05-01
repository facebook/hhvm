/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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
#include <util/thread_local.h>
#include <runtime/base/memory/memory_usage_stats.h>

#include <vector>
#include <deque>
#include <queue>
#include <map>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class SmartAllocatorImpl;

struct SweepNode {
  SweepNode* next;
  union {
    SweepNode* prev;
    size_t padbytes;
  };
};

// jemalloc uses 0x5a but we use 0x6a so we can tell the difference
// when debugging.
const char kSmartFreeFill = 0x6a;
const uintptr_t kSmartFreeWord = 0x6a6a6a6a6a6a6a6aLL;
const uintptr_t kMallocFreeWord = 0x5a5a5a5a5a5a5a5aLL;

/**
 * A garbage list is a freelist of items that uses the space in the items
 * to store a singly linked list.
 */
class GarbageList {
public:
  GarbageList() : ptr(nullptr) {
  }

  // Pops an item, or returns NULL
  void* maybePop() {
    void** ret = ptr;
    if (LIKELY(ret != nullptr)) {
      ptr = (void**)*ret;
    }
    return ret;
  }

  // Pushes an item on to the list. The item must be larger than
  // sizeof(void*)
  void push(void* val) {
    void** convval = (void**)val;
    *convval = ptr;
    ptr = convval;
  }

  // Number of items on the list.  We calculate this iteratively
  // on the assumption we don't query this often, so iterating is
  // faster than keeping a size field up-to-date.
  int size() const {
    int sz = 0;
    for (Iterator it = begin(), e = end(); it != e; ++it, ++sz) {}
    return sz;
  }

  bool empty() const {
    return ptr == nullptr;
  }

  // Remove all items from this list
  void clear() {
    ptr = nullptr;
  }

  class Iterator {
  public:
    explicit Iterator(const GarbageList& l) : curptr(l.ptr) {}

    Iterator(const Iterator &other) : curptr(other.curptr) {}
    Iterator() : curptr(nullptr) {}

    bool operator==(const Iterator &it) {
      return curptr == it.curptr;
    }

    bool operator!=(const Iterator &it) {
      return !operator==(it);
    }

    Iterator &operator++() {
      if (curptr) {
        curptr = (void**)*curptr;
      }
      return *this;
    }

    Iterator operator++(int) {
      Iterator ret(*this);
      operator++();
      return ret;
    }

    void* operator*() const {
      return curptr;
    }

  private:
    void** curptr;
  };

  Iterator begin() const {
    return Iterator(*this);
  }

  Iterator end() const {
    return Iterator();
  }

  typedef Iterator iterator;

private:
  void** ptr;
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
class MemoryManager : boost::noncopyable {
  static void* TlsInitSetup;
public:
  typedef ThreadLocalSingleton<MemoryManager> TlsWrapper;
  static void Create(void* storage);
  static void Delete(MemoryManager*);
  static void OnThreadExit(MemoryManager*);
  static MemoryManager* TheMemoryManager() {
    return TlsWrapper::getNoCheck();
  }

  MemoryManager();

  // State for iteration over all the smart allocators registered in a
  // memory manager.
  struct AllocIterator {
    explicit AllocIterator(const MemoryManager* mman);

    // Returns null if we're at the end.
    SmartAllocatorImpl* current() const;
    void next();

  private:
    const MemoryManager& m_mman;
    std::vector<SmartAllocatorImpl*>::const_iterator m_it;
  };

  /**
   * Without calling this, everything should work as if there is no memory
   * manager.
   */
  void enable() { m_enabled = true; }
  void disable() { m_enabled = false; }
  bool isEnabled() { return m_enabled; }

  /**
   * Register a smart allocator. Done by SmartAlloctorImpl's constructor.
   */
  void add(SmartAllocatorImpl *allocator);

  /**
   * Mark current allocator's position as ending point of a generation and
   * sweep all memory that has allocated.
   */
  void sweepAll();
  void rollback();

  /**
   * Write stats to ServerStats.
   */
  void logStats();

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

  class MaskAlloc {
    MemoryManager *m_mm;
  public:
    explicit MaskAlloc(MemoryManager *mm) : m_mm(mm) {
      // capture all mallocs prior to construction
      m_mm->refreshStats();
    }
    ~MaskAlloc() {
#ifdef USE_JEMALLOC
      // exclude mallocs and frees since construction
      if (s_statsEnabled) {
        m_mm->m_prevAllocated = int64_t(*m_mm->m_allocated);
        m_mm->m_delta = int64_t(*m_mm->m_allocated) - int64_t(*m_mm->m_deallocated);
      }
#endif
    }
  };

  void* smartMalloc(size_t nbytes);
  void* smartRealloc(void* ptr, size_t nbytes);
  void* smartCallocBig(size_t totalbytes);
  void  smartFree(void* ptr);
  static const size_t kMaxSmartSize = 2048;

  // allocate nbytes from the current slab, aligned to 16-bytes
  void* slabAlloc(size_t nbytes);

private:
  char* newSlab(size_t nbytes);
  void* smartEnlist(SweepNode*);
  void* smartMallocSlab(size_t padbytes);
  void* smartMallocBig(size_t nbytes);
  void  smartFreeBig(SweepNode*);
  void refreshStatsHelperExceeded();
#ifdef USE_JEMALLOC
  void refreshStatsHelperStop();
#endif

private:
  static const unsigned kLgSizeQuantum = 4; // 16 bytes
  static const unsigned kNumSizes = kMaxSmartSize >> kLgSizeQuantum;
  static const size_t kMask = (1 << kLgSizeQuantum) - 1;

private:
  char *m_front, *m_limit;
  GarbageList m_smartfree[kNumSizes];
  SweepNode m_sweep;   // oversize smart_malloc'd blocks
  SweepNode m_strings; // in-place node is head of circular list
  MemoryUsageStats m_stats;
  bool m_enabled;

  std::vector<SmartAllocatorImpl*> m_smartAllocators;
  std::vector<char*> m_slabs;

#ifdef USE_JEMALLOC
  uint64_t* m_allocated;
  uint64_t* m_deallocated;
  int64_t  m_delta;
  int64_t  m_prevAllocated;
  size_t* m_cactive;
  size_t m_cactiveLimit;

public:
  static bool s_statsEnabled;
  static size_t s_cactiveLimitCeiling;
#endif

  friend class StringData; // for enlist/delist access to m_strings
};

//
// smart_malloc api for request-scoped memory
//
// These functions behave like malloc, but get memory from the current
// thread's MemoryManager instance.  At request-end, any un-freed memory
// is explicitly freed and garbage filled.  If any pointers to this memory
// survive beyond a request, they'll be dangling pointers.
//
// Block sizes <= MemoryManager::kMaxSmartSize are region-allocated
// and are only guaranteed to be 8-byte aligned.  Larger blocks are
// directly malloc'd (with a header) and are 16-byte aligned.
//
// Clients must not mix/match calls between smart_malloc and malloc:
//  - these blocks have a header that malloc wouldn't grok
//  - memory is auto-freed at request-end, unlike malloc
//  - all bookeeping is thread local; freeing a smart_malloc block
//    from a different thread than it was malloc'd from, even while
//    the original request is still running, will just crash and burn.
//
void* smart_malloc(size_t nbytes);
void* smart_calloc(size_t count, size_t bytes);
void* smart_realloc(void* ptr, size_t nbytes);
void  smart_free(void* ptr);

namespace smart {
namespace do_not_use_directly {
/*
 * We are deriving from the std::collection classes to get
 * smart::collection classes that use smart allocation.
 * To avoid the various issues involved with deriving
 * from value types, we want to make sure that there are
 * no references to std::collection<...,SmartStlAlloc<>>
 * other than the ones below. That way we know that
 * a pointer to a smart::collection can never decay
 * to a pointer to a std::collection.
 *
 * The namespace do_not_use_directly should remind us
 * of that.
 *
 */
template <class T>
class SmartStlAlloc {
 public:
  typedef T              value_type;
  typedef T*             pointer;
  typedef const T*       const_pointer;
  typedef T&             reference;
  typedef const T&       const_reference;
  typedef std::size_t    size_type;
  typedef std::ptrdiff_t difference_type;

  template <class U>
  struct rebind {
    typedef SmartStlAlloc<U> other;
  };

  pointer address (reference value) const {
    return &value;
  }
  const_pointer address (const_reference value) const {
    return &value;
  }

  SmartStlAlloc() throw() {}
  SmartStlAlloc(const SmartStlAlloc&) {}
  template <class U>
  SmartStlAlloc (const SmartStlAlloc<U>&) {}
  ~SmartStlAlloc() {}

  size_type max_size () const {
    return std::numeric_limits<std::size_t>::max() / sizeof(T);
  }

  pointer allocate (size_type num, const void* = 0) {
    pointer ret = (pointer)smart_malloc(num * sizeof(T));
    return ret;
  }

  void construct (pointer p, const T& value) {
    new ((void*)p) T(value);
  }

  void construct (pointer p) {
    new ((void*)p) T();
  }

  void destroy (pointer p) {
    p->~T();
  }

  void deallocate (pointer p, size_type num) {
    smart_free(p);
  }
};

template <class T1, class T2>
bool operator== (const SmartStlAlloc<T1>&,
                 const SmartStlAlloc<T2>&) {
  return true;
}
template <class T1, class T2>
bool operator!= (const SmartStlAlloc<T1>&,
                 const SmartStlAlloc<T2>&) {
  return false;
}

}
/*
 * Derivation from value types is generally bad.
 * Here, we derive from classes that do not exist anywhere
 * else in the code base (see comments above).
 *
 * We also add no functionality to the derived class. Your
 * code will not get past code review if you try to do so.
 */
template <class Key, class T, class Compare = std::less<Key> >
class map : public std::map<
  Key, T, Compare,
  do_not_use_directly::SmartStlAlloc<std::pair<const Key, T> > > {};

template <class T>
class deque : public std::deque<T, do_not_use_directly::SmartStlAlloc<T> > {};

template <class T>
class vector : public std::vector<T, do_not_use_directly::SmartStlAlloc<T> > {
  typedef std::vector<T, do_not_use_directly::SmartStlAlloc<T> > Base_;
 public:
  template <typename... A>
  explicit vector(A &&... args) : Base_(std::forward<A>(args)...) {}
};

template <class T>
class list : public std::list<T, do_not_use_directly::SmartStlAlloc<T> > {};

template <class T>
class queue : public std::queue<T, deque<T> > {};

template <class _T, class _U,
          class _V = hphp_hash<_T>,class _W = std::equal_to<_T> >
struct hash_map : std::tr1::unordered_map<
  _T, _U, _V, _W, do_not_use_directly::SmartStlAlloc<std::pair<_T, _U> > > {
  hash_map() : std::tr1::unordered_map<
    _T, _U, _V, _W, do_not_use_directly::SmartStlAlloc<std::pair<_T, _U> >
    >(0) {}
};

template <class _T,
          class _V = hphp_hash<_T>,class _W = std::equal_to<_T> >
struct hash_set : std::tr1::unordered_set<
  _T, _V, _W, do_not_use_directly::SmartStlAlloc<_T> > {
  hash_set() : std::tr1::unordered_set<
    _T, _V, _W, do_not_use_directly::SmartStlAlloc<_T> >(0) {}
};

}
///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_MEMORY_MANAGER_H_
