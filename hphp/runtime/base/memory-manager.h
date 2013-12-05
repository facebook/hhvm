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

#include <array>
#include <vector>

#include "folly/Memory.h"

#include "hphp/util/alloc.h" // must be included before USE_JEMALLOC is used
#include "hphp/util/trace.h"
#include "hphp/util/thread-local.h"
#include "hphp/runtime/base/memory-usage-stats.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

/*
 * Request local memory in HHVM is managed by a thread local object
 * called MemoryManager.
 *
 * The object may be accessed with MM(), but higher-level apis are
 * also provided below.
 *
 * The MemoryManager serves the following funcitons in hhvm:
 *
 *   - Managing request-local memory.
 *
 *   - Tracking "sweepable" objects--i.e. objects that must run custom
 *     cleanup code if they are still live at the end of the request.
 *
 *   - Accounting for usage of memory "by this request", whether it
 *     goes through the request-local allocator, or the underlying
 *     malloc implementation.  (This feature is gated on being
 *     compiled with jemalloc.)
 */
struct MemoryManager;
MemoryManager& MM();

//////////////////////////////////////////////////////////////////////

/*
 * smart_malloc api for request-scoped memory
 *
 * This is the most generic entry point to the request local
 * allocator.  If you easily know the size of the allocation at free
 * time, it might be more efficient to use MM() apis directly.
 *
 * These functions behave like C's malloc/free, but get memory from
 * the current thread's MemoryManager instance.  At request-end, any
 * un-freed memory is explicitly freed (and in debug, garbage filled).
 * If any pointers to this memory survive beyond a request, they'll be
 * dangling pointers.
 *
 * These functions only guarantee 8-byte alignment for the returned
 * pointer.
 */
void* smart_malloc(size_t nbytes);
void* smart_calloc(size_t count, size_t bytes);
void* smart_realloc(void* ptr, size_t nbytes);
void  smart_free(void* ptr);

/*
 * Smart (de)allocate for non-POD C++-style stuff.  (Runs constructors
 * and destructors.)
 *
 * Unlike the normal operator delete, smart_delete requires ~T() must
 * be nothrow and that p is not null.
 */
template<class T, class... Args> T* smart_new(Args&&...);
template<class T> void smart_delete(T* p);

/*
 * Allocate an array of objects.  Similar to smart_malloc, but with
 * support for constructors.
 *
 * Note that explicitly calling smart_delete will run the destructors,
 * but if you let the allocator sweep it the destructors will not be
 * called.
 *
 * Unlike the normal operator delete, smart_delete_array requires ~T()
 * must be nothrow.
 */
template<class T> T* smart_new_array(size_t count);
template<class T> void smart_delete_array(T* t, size_t count);

//////////////////////////////////////////////////////////////////////

/*
 * The maximum size where we use our custom allocator for
 * request-local memory.
 *
 * Allocations larger than this size go to the underlying malloc
 * implementation, and certain specialized allocator functions have
 * preconditions about the requested size being above or below this
 * number to avoid checking at runtime.
 */
constexpr size_t kMaxSmartSize = 2048;

/*
 * Constants for the various debug junk-filling of different types of
 * memory in hhvm.
 *
 * jemalloc uses 0x5a to fill freed memory, so we use 0x6a for the
 * request-local allocator so it is easy to tell the difference when
 * debugging.  There's also 0x7a for junk-filling some cases of
 * ex-TypedValue memory (evaluation stack).
 */
constexpr char kSmartFreeFill = 0x6a;
constexpr char kTVTrashFill = 0x7a;
constexpr uintptr_t kSmartFreeWord = 0x6a6a6a6a6a6a6a6aLL;
constexpr uintptr_t kMallocFreeWord = 0x5a5a5a5a5a5a5a5aLL;

//////////////////////////////////////////////////////////////////////

/*
 * This is the header MemoryManager uses for large allocations, and
 * it's also used for StringData's that wrap APCHandle.
 *
 * TODO(#2946560): refactor this not to be shared with StringData.
 */
struct SweepNode {
  SweepNode* next;
  union {
    SweepNode* prev;
    size_t padbytes;
  };
};

//////////////////////////////////////////////////////////////////////

struct MemoryManager {
  /*
   * Lifetime managed with a ThreadLocalSingleton.  Use MM() to access
   * the current thread's MemoryManager.
   */
  typedef ThreadLocalSingleton<MemoryManager> TlsWrapper;
  static void Create(void*);
  static void Delete(MemoryManager*);
  static void OnThreadExit(MemoryManager*);

  /*
   * This is an RAII wrapper to temporarily mask counting allocations
   * from stats tracking in a scoped region.
   *
   * Usage:
   *
   *   MemoryManager::MaskAlloc masker(MM());
   */
  struct MaskAlloc;

  /*
   * Returns true iff a sweep is in progress.  I.e., is the current
   * thread running inside a call to MemoryManager::sweep().
   *
   * It is legal to call this function even when the current thread's
   * MemoryManager may not be set up (i.e. between requests).
   */
  static bool sweeping();

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
   * size class (although stats accounting may undercount in this
   * case).  The size passed to smartFreeSize must be the exact size
   * that was passed to smartMallocSize for that allocation.
   *
   * The returned pointer is guaranteed to be 16-byte aligned.
   *
   * Pre: size > 0 && size <= kMaxSmartSize
   */
  void* smartMallocSize(uint32_t size);
  void smartFreeSize(void* p, uint32_t size);

  /*
   * Allocate/deallocate smart-allocated memory that is too big for
   * the small size classes.
   *
   * Returns a pointer and the actual size of the allocation, which
   * amay be larger than the requested size.  The returned pointer is
   * guaranteed to be 16-byte aligned.
   *
   * The size passed to smartFreeSize must either be the size that was
   * passed to smartMallocSize, or the value that was returned as the
   * actual allocation size.
   *
   * Pre: size > kMaxSmartSize
   */
  std::pair<void*,size_t> smartMallocSizeBig(size_t size);
  void smartFreeSizeBig(void* vp, size_t size);

  /*
   * Allocate/deallocate objects when the size is not known to be
   * above or below kMaxSmartSize without a runtime check.
   *
   * These functions use the same underlying allocator as
   * smartMallocSize{,Big}, and it is safe to return allocations using
   * one of those apis as long as the appropriate preconditions on the
   * size are met.
   *
   * The size passed to objFree must be the size passed in to
   * objMalloc.
   *
   * Pre: size > 0
   */
  void* objMalloc(size_t size);
  void objFree(void* vp, size_t size);

  /*
   * Versions of the above APIs that log the allocation/deallocation.
   *
   * These should be used for allocations that reflect PHP "objects" (Object,
   * String, Array, RefData, extension objects, etc.) that make sense to log by
   * capturing a PHP stacktrace to which to charge the allocation.
   */
  void* smartMallocSizeLogged(uint32_t size);
  void smartFreeSizeLogged(void* p, uint32_t size);
  std::pair<void*,size_t> smartMallocSizeBigLogged(size_t size);
  void smartFreeSizeBigLogged(void* vp, size_t size);
  void* objMallocLogged(size_t size);
  void objFreeLogged(void* vp, size_t size);

  /*
   * During session shutdown, before resetAllocator(), this phase runs
   * through the sweep lists running cleanup for anything that needs
   * to run custom tear down logic before we throw away the
   * request-local memory.
   */
  void sweep();

  /*
   * Release all the request-local allocations.  Zeros all the free
   * lists and may return some underlying storage to the system
   * allocator.
   *
   * This is called after sweep in the end-of-request path.
   */
  void resetAllocator();

  /*
   * Reset all internally-stored memory usage stats.  Used between
   * sessions.
   */
  void resetStats();

  /*
   * How much memory this thread has allocated or deallocated.
   */
  int64_t getAllocated() const;
  int64_t getDeallocated() const;

  /*
   * Get access to the current memory allocation stats, without
   * refreshing them first.
   */
  MemoryUsageStats& getStatsNoRefresh();

  /*
   * Get most recent stats, updating the tracked stats in the
   * MemoryManager object.
   */
  MemoryUsageStats& getStats();

  /*
   * Get most recent stats data, as one would with getStats(), but
   * without altering the underlying data stored in the MemoryManager.
   *
   * Used for obtaining debug info.
   */
  MemoryUsageStats getStatsCopy();

private:
  friend class StringData; // for enlist/delist access to m_strings
  friend void* smart_malloc(size_t nbytes);
  friend void* smart_calloc(size_t count, size_t bytes);
  friend void* smart_realloc(void* ptr, size_t nbytes);
  friend void  smart_free(void* ptr);

  struct SmallNode;
  struct DebugHeader;

  struct FreeList {
    struct Node;

    void* maybePop();
    void push(void*);

    Node* head = nullptr;
  };

  static constexpr unsigned kLgSizeQuantum = 4; // 16 bytes
  static constexpr unsigned kNumSizes = kMaxSmartSize >> kLgSizeQuantum;
  static constexpr size_t kSmartSizeMask = (1 << kLgSizeQuantum) - 1;
  static void* TlsInitSetup;

private:
  MemoryManager();
  MemoryManager(const MemoryManager&) = delete;
  MemoryManager& operator=(const MemoryManager&) = delete;

private:
  void* slabAlloc(size_t nbytes);
  char* newSlab(size_t nbytes);
  void* smartEnlist(SweepNode*);
  void* smartMallocSlab(size_t padbytes);
  void* smartMallocBig(size_t nbytes);
  void* smartCallocBig(size_t nbytes);
  void  smartFreeBig(SweepNode*);
  void* smartMalloc(size_t nbytes);
  void* smartRealloc(void* ptr, size_t nbytes);
  void  smartFree(void* ptr);
  static void threadStatsInit();
  static void threadStats(uint64_t*&, uint64_t*&, size_t*&, size_t&);
  void refreshStatsHelper();
  void refreshStats();
  template<bool live> void refreshStatsImpl(MemoryUsageStats& stats);
  void refreshStatsHelperExceeded() const;
#ifdef USE_JEMALLOC
  void refreshStatsHelperStop();
  void* smartMallocSizeBigHelper(void*&, size_t&, size_t);
#endif
  bool checkPreFree(DebugHeader*, size_t, size_t) const;
  template<class SizeT> static SizeT debugAddExtra(SizeT);
  template<class SizeT> static SizeT debugRemoveExtra(SizeT);
  void* debugPostAllocate(void*, size_t, size_t);
  void* debugPreFree(void*, size_t, size_t);

  void logAllocation(void*, size_t);
  void logDeallocation(void*);

private:
  TRACE_SET_MOD(smartalloc);

  char* m_front;
  char* m_limit;
  std::array<FreeList,kNumSizes> m_freelists;
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
  mutable size_t m_cactiveLimit;
  static bool s_statsEnabled;
  static size_t s_cactiveLimitCeiling;
#endif

private:
  bool m_sweeping;
};

//////////////////////////////////////////////////////////////////////

}

#include "hphp/runtime/base/memory-manager-inl.h"

#endif
