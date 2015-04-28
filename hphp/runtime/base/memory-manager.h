/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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
#include <utility>
#include <set>
#include <unordered_map>

#include <folly/Memory.h>

#include "hphp/util/alloc.h" // must be included before USE_JEMALLOC is used
#include "hphp/util/compilation-flags.h"
#include "hphp/util/trace.h"
#include "hphp/util/thread-local.h"

#include "hphp/runtime/base/imarker.h"
#include "hphp/runtime/base/memory-usage-stats.h"
#include "hphp/runtime/base/request-event-handler.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/sweepable.h"
#include "hphp/runtime/base/header-kind.h"
#include "hphp/runtime/base/smart-ptr.h"

// used for mmapping contiguous heap space
// If used, anonymous pages are not cleared when mapped with mmap. It is not
// enabled by default and should be checked before use
#define       MAP_UNINITIALIZED 0x4000000 /* XXX Fragile. */

namespace HPHP {
struct APCLocalArray;
struct MemoryManager;
struct ObjectData;
struct ResourceData;

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

namespace smart {

// STL-style allocator for the smart allocator.  (Unfortunately we
// can't use allocator_traits yet.)
//
// You can also use smart::Allocator as a model of folly's
// SimpleAllocator where appropriate.
//

template <class T>
struct Allocator {
  typedef T              value_type;
  typedef T*             pointer;
  typedef const T*       const_pointer;
  typedef T&             reference;
  typedef const T&       const_reference;
  typedef std::size_t    size_type;
  typedef std::ptrdiff_t difference_type;

  template <class U>
  struct rebind {
    typedef Allocator<U> other;
  };

  pointer address(reference value) const {
    return &value;
  }
  const_pointer address(const_reference value) const {
    return &value;
  }

  Allocator() noexcept {}
  Allocator(const Allocator&) noexcept {}
  template<class U> Allocator(const Allocator<U>&) noexcept {}
  ~Allocator() noexcept {}

  size_type max_size() const {
    return std::numeric_limits<std::size_t>::max() / sizeof(T);
  }

  pointer allocate(size_type num, const void* = 0) {
    pointer ret = (pointer)smart_malloc(num * sizeof(T));
    return ret;
  }

  template<class U, class... Args>
  void construct(U* p, Args&&... args) {
    ::new ((void*)p) U(std::forward<Args>(args)...);
  }

  void destroy(pointer p) {
    p->~T();
  }

  void deallocate(pointer p, size_type num) {
    smart_free(p);
  }

  template<class U> bool operator==(const Allocator<U>&) const {
    return true;
  }

  template<class U> bool operator!=(const Allocator<U>&) const {
    return false;
  }
};

}

//////////////////////////////////////////////////////////////////////

/*
 * Debug mode header.
 *
 * For size-untracked allocations, this sits in front of the user
 * payload for small allocations, and in front of the BigNode in
 * big allocations.  The allocatedMagic aliases the space for the
 * FreeList::Node pointers, but should catch double frees due to
 * kAllocatedMagic.
 *
 * For size-tracked allocations, this always sits in front of
 * whatever header we're using (SmallNode or BigNode).
 *
 * We set requestedSize to kFreedMagic when a block is not
 * allocated.
 */
struct DebugHeader {
  static constexpr uintptr_t kAllocatedMagic = 0xDB6000A110C0A7EDull;
  static constexpr size_t kFreedMagic =        0x5AB07A6ED4110CEEull;

  uintptr_t allocatedMagic;
  uint8_t pad[3];
  HeaderKind kind;
  size_t requestedSize; // zero for size-untracked allocator
  size_t returnedCap;
};

/*
 * Slabs are consumed via bump allocation.  The individual allocations are
 * quantized into a fixed set of size classes, the sizes of which are an
 * implementation detail documented here to shed light on the algorithms that
 * compute size classes.  Request sizes are rounded up to the nearest size in
 * the relevant SMART_SIZES table; e.g. 17 is rounded up to 32.  There are
 * 2^LG_SMART_SIZES_PER_DOUBLING size classes for each doubling of size
 * (ignoring the alignment-constrained smallest size classes), which limits
 * internal fragmentation to 33% or 20%, for LG_SMART_SIZES_PER_DOUBLING
 * set to 1 or 2, respectively.
 */

#define LG_SMART_SIZES_PER_DOUBLING 2

#if (LG_SMART_SIZES_PER_DOUBLING == 1)
#define SMART_SIZES \
/*        index, delta, size */ \
  SMART_SIZE( 0,    16,   16) \
  SMART_SIZE( 1,    16,   32) \
  SMART_SIZE( 2,    16,   48) \
  SMART_SIZE( 3,    16,   64) \
  SMART_SIZE( 4,    32,   96) \
  SMART_SIZE( 5,    32,  128) \
  SMART_SIZE( 6,    64,  192) \
  SMART_SIZE( 7,    64,  256) \
  SMART_SIZE( 8,   128,  384) \
  SMART_SIZE( 9,   128,  512) \
  SMART_SIZE(10,   256,  768) \
  SMART_SIZE(11,   256, 1024) \
  SMART_SIZE(12,   512, 1536) \
  SMART_SIZE(13,   512, 2048) \
  SMART_SIZE(14,  1024, 3072) \
  SMART_SIZE(15,  1024, 4096) \

#elif (LG_SMART_SIZES_PER_DOUBLING == 2)
#define SMART_SIZES \
/*        index, delta, size */ \
  SMART_SIZE( 0,    16,   16) \
  SMART_SIZE( 1,    16,   32) \
  SMART_SIZE( 2,    16,   48) \
  SMART_SIZE( 3,    16,   64) \
  SMART_SIZE( 4,    16,   80) \
  SMART_SIZE( 5,    16,   96) \
  SMART_SIZE( 6,    16,  112) \
  SMART_SIZE( 7,    16,  128) \
  SMART_SIZE( 8,    32,  160) \
  SMART_SIZE( 9,    32,  192) \
  SMART_SIZE(10,    32,  224) \
  SMART_SIZE(11,    32,  256) \
  SMART_SIZE(12,    64,  320) \
  SMART_SIZE(13,    64,  384) \
  SMART_SIZE(14,    64,  448) \
  SMART_SIZE(15,    64,  512) \
  SMART_SIZE(16,   128,  640) \
  SMART_SIZE(17,   128,  768) \
  SMART_SIZE(18,   128,  896) \
  SMART_SIZE(19,   128, 1024) \
  SMART_SIZE(20,   256, 1280) \
  SMART_SIZE(21,   256, 1536) \
  SMART_SIZE(22,   256, 1792) \
  SMART_SIZE(23,   256, 2048) \
  SMART_SIZE(24,   512, 2560) \
  SMART_SIZE(25,   512, 3072) \
  SMART_SIZE(26,   512, 3584) \
  SMART_SIZE(27,   512, 4096) \

#else
#  error Need SMART_SIZES definition for specified LG_SMART_SIZES_PER_DOUBLING
#endif

__attribute__((__aligned__(64)))
constexpr uint8_t kSmartSize2Index[] = {
#define S2I_16(i)  i,
#define S2I_32(i)  S2I_16(i) S2I_16(i)
#define S2I_64(i)  S2I_32(i) S2I_32(i)
#define S2I_128(i) S2I_64(i) S2I_64(i)
#define S2I_256(i) S2I_128(i) S2I_128(i)
#define S2I_512(i) S2I_256(i) S2I_256(i)
#define S2I_1024(i) S2I_512(i) S2I_512(i)
#define SMART_SIZE(index, delta, size) S2I_##delta(index)
  SMART_SIZES
#undef S2I_16
#undef S2I_32
#undef S2I_64
#undef S2I_128
#undef S2I_256
#undef S2I_512
#undef S2I_1024
#undef SMART_SIZE
};

constexpr uint32_t kMaxSmartSizeLookup = 4096;

constexpr unsigned kLgSlabSize = 21;
constexpr size_t kSlabSize = size_t{1} << kLgSlabSize;
constexpr unsigned kLgSmartSizeQuantum = 4;
constexpr size_t kSmartSizeAlign = 1u << kLgSmartSizeQuantum;
constexpr size_t kSmartSizeAlignMask = kSmartSizeAlign - 1;

constexpr size_t kDebugExtraSize = debug ?
                                   ((sizeof(DebugHeader) + kSmartSizeAlignMask)
                                    & ~kSmartSizeAlignMask) : 0;

constexpr unsigned kLgSizeClassesPerDoubling = LG_SMART_SIZES_PER_DOUBLING;
constexpr unsigned kLgMaxSmartSize = kLgSlabSize;
static_assert(kLgMaxSmartSize > kLgSmartSizeQuantum + 1,
              "Too few size classes");
constexpr size_t kNumSmartSizes = (kLgMaxSmartSize - kLgSmartSizeQuantum
                                  - (kLgSizeClassesPerDoubling - 1))
                                  << kLgSizeClassesPerDoubling;
/*
 * The maximum size where we use our custom allocator for request-local memory.
 *
 * Allocations larger than this size go to the underlying malloc implementation,
 * and certain specialized allocator functions have preconditions about the
 * requested size being above or below this number to avoid checking at runtime.
 */
constexpr size_t kMaxSmartSize = (size_t{1} << kLgMaxSmartSize)
                                 - kDebugExtraSize;

constexpr unsigned kSmartPreallocCountLimit = 8;
constexpr size_t kSmartPreallocBytesLimit = size_t{1} << 9;

/*
 * Constants for the various debug junk-filling of different types of
 * memory in hhvm.
 *
 * jemalloc uses 0x5a to fill freed memory, so we use 0x6a for the
 * request-local allocator so it is easy to tell the difference when
 * debugging.  There's also 0x7a for junk-filling some cases of
 * ex-TypedValue memory (evaluation stack).
 */
constexpr char kSmartFreeFill   = 0x6a;
constexpr char kTVTrashFill     = 0x7a; // used by interpreter
constexpr char kTVTrashFill2    = 0x7b; // used by smart pointer dtors
constexpr char kTVTrashJITStk   = 0x7c; // used by the JIT for stack slots
constexpr char kTVTrashJITFrame = 0x7d; // used by the JIT for stack frames
constexpr char kTVTrashJITHeap  = 0x7e; // used by the JIT for heap
constexpr uintptr_t kSmartFreeWord = 0x6a6a6a6a6a6a6a6aLL;
constexpr uintptr_t kMallocFreeWord = 0x5a5a5a5a5a5a5a5aLL;

//////////////////////////////////////////////////////////////////////

// Header MemoryManager uses for StringDatas that wrap APCHandle
struct StringDataNode {
  StringDataNode* next;
  StringDataNode* prev;
};

// This is the header MemoryManager uses to remember large allocations
// so they can be auto-freed in MemoryManager::reset()
struct BigNode {
  size_t nbytes;
  char pad[3];
  HeaderKind kind;
  uint32_t index;
};

// Header used for small smart_malloc allocations (but not *Size allocs)
struct SmallNode {
  size_t padbytes;
  char pad[3];
  HeaderKind kind;
};

// all FreeList entries are parsed by inspecting this header.
struct FreeNode {
  FreeNode* next;
  union {
    struct {
      char pad[3];
      HeaderKind kind;
      uint32_t size;
    };
    uint64_t kind_size;
  };
};

// header for HNI objects with NativeData payloads. see native-data.h
// for details about memory layout.
struct NativeNode {
  uint32_t sweep_index; // index in MM::m_natives
  uint32_t obj_offset; // byte offset from this to ObjectData*
  char pad[3];
  HeaderKind kind;
};

// header for Resumable objects. See layout comment in resumable.h
struct ResumableNode {
  size_t framesize;
  char pad[3];
  HeaderKind kind; // Resumable
};

// POD type for tracking arbitrary memory ranges
struct MemBlock {
  void* ptr;
  size_t size; // bytes
};

///////////////////////////////////////////////////////////////////////////////

/*
 * Allocator for slabs and big blocks.
 */
struct BigHeap {
  struct iterator;
  BigHeap() {}
  bool empty() const {
    return m_slabs.empty() && m_bigs.empty();
  }

  // return true if ptr points into one of the slabs
  bool contains(void* ptr) const;

  // allocate a MemBlock of at least size bytes, track in m_slabs.
  MemBlock allocSlab(size_t size);

  // allocation api for big blocks. These get a BigNode header and
  // are tracked in m_bigs
  MemBlock allocBig(size_t size, HeaderKind kind);
  MemBlock callocBig(size_t size);
  MemBlock resizeBig(void* p, size_t size);
  void freeBig(void*);

  // free all slabs and big blocks
  void reset();

  // Release auxiliary structures to prepare to be idle for a while
  void flush();

  // allow whole-heap iteration
  iterator begin();
  iterator end();

 protected:
  void enlist(BigNode*, HeaderKind kind, size_t size);

 protected:
  std::vector<MemBlock> m_slabs;
  std::vector<BigNode*> m_bigs;
};

///////////////////////////////////////////////////////////////////////////////

/*
 * ContiguousHeap handles allocations and provides a contiguous address space
 * for requests.
 *
 * To turn on build with CONTIGUOUS_HEAP = 1.
 */
struct ContiguousHeap : BigHeap {
  bool contains(void* ptr) const;

  MemBlock allocSlab(size_t size);

  MemBlock allocBig(size_t size, HeaderKind kind);
  MemBlock callocBig(size_t size);
  MemBlock resizeBig(void* p, size_t size);
  void freeBig(void*);

  void reset();

  void flush();

  ~ContiguousHeap();

 private:
  // Contiguous Heap Pointers
  char* m_base = nullptr;
  char* m_used;
  char* m_end;
  char* m_peak;
  char* m_OOMMarker;
  FreeNode m_freeList;

  // Contiguous Heap Counters
  uint32_t m_requestCount;
  size_t m_heapUsage;
  size_t m_contiguousHeapSize;

 private:
  void* heapAlloc(size_t nbytes, size_t &cap);
  void  createRequestHeap();
};

///////////////////////////////////////////////////////////////////////////////

struct MemoryManager {
  /*
   * Lifetime managed with a ThreadLocalSingleton.  Use MM() to access
   * the current thread's MemoryManager.
   */
  using TlsWrapper = ThreadLocalSingleton<MemoryManager>;

  static void Create(void*);
  static void Delete(MemoryManager*);
  static void OnThreadExit(MemoryManager*);

  /////////////////////////////////////////////////////////////////////////////

  /*
   * Id that is used when registering roots with the memory manager.
   */
  using RootId = size_t;

  /*
   * This is an RAII wrapper to temporarily mask counting allocations from
   * stats tracking in a scoped region.
   *
   * Usage:
   *   MemoryManager::MaskAlloc masker(MM());
   */
  struct MaskAlloc;

  /*
   * An RAII wrapper to suppress OOM checking in a region.
   */
  struct SuppressOOM;

  /////////////////////////////////////////////////////////////////////////////
  // Allocation.

  /*
   * Return the smart size class for a given requested allocation size.
   *
   * The return value is greater than or equal to the parameter, and
   * less than or equal to MaxSmallSize.
   *
   * Pre: requested <= kMaxSmartSize
   */
  static uint32_t smartSizeClass(uint32_t requested);

  /*
   * Return a lower bound estimate of the capacity that will be returned for
   * the requested size.
   */
  static uint32_t estimateSmartCap(uint32_t requested);

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
  template<bool callerSavesActualSize>
  MemBlock smartMallocSizeBig(size_t size);
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
  void* objMallocLogged(size_t size);
  void objFreeLogged(void* vp, size_t size);
  template<bool callerSavesActualSize>
  MemBlock smartMallocSizeBigLogged(size_t size);
  void smartFreeSizeBigLogged(void* vp, size_t size);

  /////////////////////////////////////////////////////////////////////////////
  // Cleanup.

  /*
   * Prepare for being idle for a while by releasing or madvising as much as
   * possible.
   */
  void flush();

  /*
   * Release all the request-local allocations.
   *
   * Zeros all the free lists and may return some underlying storage to the
   * system allocator.  This also resets all internally-stored memory usage
   * stats.
   *
   * This is called after sweep in the end-of-request path.
   */
  void resetAllocator();

  /*
   * Reset all runtime options for MemoryManager.
   */
  void resetRuntimeOptions();

  /////////////////////////////////////////////////////////////////////////////
  // Heap introspection.

  /*
   * Return true if there are no allocated slabs.
   */
  bool empty() const;

  /*
   * Whether `p' points into memory owned by `m_heap'.  checkContains() will
   * assert that it does.
   */
  bool contains(void* p) const;
  bool checkContains(void* p) const;

  /////////////////////////////////////////////////////////////////////////////
  // Stats.

  /*
   * Get access to the current memory allocation stats, without refreshing them
   * first.
   */
  MemoryUsageStats& getStatsNoRefresh();

  /*
   * Get most recent stats, updating the tracked stats in the MemoryManager
   * object.
   */
  MemoryUsageStats& getStats();

  /*
   * Get most recent stats data, as one would with getStats(), but without
   * altering the underlying data stored in the MemoryManager.
   *
   * Used for obtaining debug info.
   */
  MemoryUsageStats getStatsCopy();

  /*
   * Open and close respectively a stats-tracking interval.
   *
   * Return whether or not the tracking state was changed as a result of the
   * call.
   */
  bool startStatsInterval();
  bool stopStatsInterval();

  /*
   * How much memory this thread has allocated or deallocated.
   */
  int64_t getAllocated() const;
  int64_t getDeallocated() const;

  /*
   * Reset all stats that are synchronzied externally from the memory manager.
   *
   * Used between sessions and to signal that external sync is now safe to
   * begin (after shared structure initialization that should not be counted is
   * complete.)
   */
  void resetExternalStats();

  /////////////////////////////////////////////////////////////////////////////
  // OOMs.

  /*
   * Whether an allocation of `size' would run the request out of memory.
   *
   * This behaves just like the OOM check in refreshStatsImpl().  If the
   * m_couldOOM flag is already unset, we return false, but if otherwise we
   * would exceed the limit, we unset the flag and register an OOM fatal
   * (though we do not modify the MM's stats).
   */
  bool preAllocOOM(int64_t size);

  /*
   * Unconditionally register an OOM fatal. Still respects the m_couldOOM flag.
   */
  void forceOOM();

  /*
   * Reset whether or not we should raise an OOM fatal if we exceed the memory
   * limit for the request.
   *
   * After an OOM fatal, the memory manager refuses to raise another OOM error
   * until this flag has been reset, to try to avoid getting OOMs during the
   * initial OOM processing.
   */
  void resetCouldOOM(bool state = true);

  /////////////////////////////////////////////////////////////////////////////
  // Sweeping.

  /*
   * Returns true iff a sweep is in progress---i.e., is the current thread
   * running inside a call to MemoryManager::sweep()?
   *
   * It is legal to call this function even when the current thread's
   * MemoryManager may not be set up (i.e. between requests).
   */
  static bool sweeping();

  /*
   * During session shutdown, before resetAllocator(), this phase runs through
   * the sweep lists, running cleanup for anything that needs to run custom
   * tear down logic before we throw away the request-local memory.
   */
  void sweep();

  /*
   * Methods for maintaining dedicated sweep lists of sweepable NativeData
   * objects, APCLocalArray instances, and Sweepables.
   */
  void addNativeObject(NativeNode*);
  void removeNativeObject(NativeNode*);
  void addApcArray(APCLocalArray*);
  void removeApcArray(APCLocalArray*);
  void addSweepable(Sweepable*);

  /////////////////////////////////////////////////////////////////////////////
  // Request profiling.

  /*
   * Trigger heap profiling in the next request.
   *
   * Allocate the s_trigger atomic so that the next request can consume it.  If
   * an unconsumed trigger exists, do nothing and return false; else return
   * true.
   */
  static bool triggerProfiling(const std::string& filename);

  /*
   * Do per-request initialization.
   *
   * Attempt to consume the profiling trigger, and copy it to m_profctx if we
   * are successful.  Also enable jemalloc heap profiling.
   */
  static void requestInit();

  /*
   * Do per-request shutdown.
   *
   * Dump a jemalloc heap profiling, then reset the profiler.
   */
  static void requestShutdown();

  /////////////////////////////////////////////////////////////////////////////

  /*
   * Returns ptr to head node of m_strings linked list. This used by
   * StringData during a reset, enlist, and delist
   */
  StringDataNode& getStringList();

  /*
   * Methods for maintaining maps of root objects keyed by RootIds.
   *
   * The id/object associations are only valid for a single request.  This
   * interface is useful for extensions that cannot physically hold on to a
   * SmartPtr, etc. or other handle class.
   */
  template <typename T> RootId addRoot(SmartPtr<T>&& ptr);
  template <typename T> RootId addRoot(const SmartPtr<T>& ptr);
  template <typename T> SmartPtr<T> lookupRoot(RootId tok) const;
  template <typename T> bool removeRoot(const SmartPtr<T>& ptr);
  template <typename T> SmartPtr<T> removeRoot(RootId token);
  template <typename F> void scanRootMaps(F& m) const;

  /*
   * Heap iterator methods.
   */
  template<class Fn> void forEachObject(Fn);
  template<class Fn> void forEachHeader(Fn);

  /*
   * Run the experimental heap-tracer.
   *
   * Has no effect other than possibly asserting.
   */
  void traceHeap();

  /////////////////////////////////////////////////////////////////////////////

private:
  friend void* smart_malloc(size_t nbytes);
  friend void* smart_calloc(size_t count, size_t bytes);
  friend void* smart_realloc(void* ptr, size_t nbytes);
  friend void  smart_free(void* ptr);

  struct FreeList {
    void* maybePop();
    void push(void*, size_t size);
    FreeNode* head = nullptr;
  };

  struct SweepableList : Sweepable {
    SweepableList() : Sweepable(Init{}) {}
    void sweep() {}
  };

  template <typename T>
  using RootMap =
    std::unordered_map<
      RootId,
      SmartPtr<T>,
      std::hash<RootId>,
      std::equal_to<RootId>,
      smart::Allocator<std::pair<const RootId,SmartPtr<T>>>
    >;

  /*
   * Request-local heap profiling context.
   */
  struct ReqProfContext {
    bool flag{false};
    bool prof_active{false};
    bool thread_prof_active{false};
    std::string filename;
  };

  /////////////////////////////////////////////////////////////////////////////

private:
  MemoryManager();
  MemoryManager(const MemoryManager&) = delete;
  MemoryManager& operator=(const MemoryManager&) = delete;

private:
  void* slabAlloc(uint32_t bytes, unsigned index);
  void* newSlab(size_t nbytes);
  void  updateBigStats();
  void* smartMallocBig(size_t nbytes);
  void* smartCallocBig(size_t nbytes);
  void* smartMalloc(size_t nbytes);
  void* smartRealloc(void* ptr, size_t nbytes);
  void  smartFree(void* ptr);

  static uint32_t bsr(uint32_t x);
  static uint8_t smartSize2IndexCompute(uint32_t size);
  static uint8_t smartSize2IndexLookup(uint32_t size);
  static uint8_t smartSize2Index(uint32_t size);

  static void threadStatsInit();
  static void threadStats(uint64_t*&, uint64_t*&, size_t*&, size_t&);
  void refreshStats();
  template<bool live> void refreshStatsImpl(MemoryUsageStats& stats);
  void refreshStatsHelperExceeded();
  void refreshStatsHelperStop();

  void resetStatsImpl(bool isInternalCall);
  bool checkPreFree(DebugHeader*, size_t, size_t) const;
  template<class SizeT> static SizeT debugAddExtra(SizeT);
  template<class SizeT> static SizeT debugRemoveExtra(SizeT);
  void* debugPostAllocate(void*, size_t, size_t);
  void* debugPreFree(void*, size_t, size_t);

  void logAllocation(void*, size_t);
  void logDeallocation(void*);

  void checkHeap();
  void initHole();
  void initFree();
  BigHeap::iterator begin();
  BigHeap::iterator end();

  void dropRootMaps();
  void deleteRootMaps();

  template <typename T>
  typename std::enable_if<
    std::is_base_of<ResourceData,T>::value,
    RootMap<ResourceData>&
  >::type getRootMap() {
    if (UNLIKELY(!m_resourceRoots)) {
      m_resourceRoots = smart_new<RootMap<ResourceData>>();
    }
    return *m_resourceRoots;
  }

  template <typename T>
  typename std::enable_if<
    std::is_base_of<ObjectData,T>::value,
    RootMap<ObjectData>&
  >::type getRootMap() {
    if (UNLIKELY(!m_objectRoots)) {
      m_objectRoots = smart_new<RootMap<ObjectData>>();
    }
    return *m_objectRoots;
  }

  template <typename T>
  typename std::enable_if<
    std::is_base_of<ResourceData,T>::value,
    const RootMap<ResourceData>&
  >::type getRootMap() const {
    if (UNLIKELY(!m_resourceRoots)) {
      m_resourceRoots = smart_new<RootMap<ResourceData>>();
    }
    return *m_resourceRoots;
  }

  template <typename T>
  typename std::enable_if<
    std::is_base_of<ObjectData,T>::value,
    const RootMap<ObjectData>&
  >::type getRootMap() const {
    if (UNLIKELY(!m_objectRoots)) {
      m_objectRoots = smart_new<RootMap<ObjectData>>();
    }
    return *m_objectRoots;
  }

  /////////////////////////////////////////////////////////////////////////////

private:
  TRACE_SET_MOD(smartalloc);

  void* m_front;
  void* m_limit;
  std::array<FreeList,kNumSmartSizes> m_freelists;
  StringDataNode m_strings; // in-place node is head of circular list
  std::vector<APCLocalArray*> m_apc_arrays;
  MemoryUsageStats m_stats;
#if CONTIGUOUS_HEAP
  ContiguousHeap m_heap;
#else
  BigHeap m_heap;
#endif
  std::vector<NativeNode*> m_natives;
  SweepableList m_sweepables;

  mutable RootMap<ResourceData>* m_resourceRoots{nullptr};
  mutable RootMap<ObjectData>* m_objectRoots{nullptr};

  bool m_sweeping;
  bool m_statsIntervalActive;
  bool m_couldOOM{true};
  bool m_bypassSlabAlloc;

  ReqProfContext m_profctx;
  static std::atomic<ReqProfContext*> s_trigger;

  static void* TlsInitSetup;

#ifdef USE_JEMALLOC
  // pointers to jemalloc-maintained allocation counters
  uint64_t* m_allocated;
  uint64_t* m_deallocated;
  uint64_t m_prevAllocated;
  uint64_t m_prevDeallocated;
  size_t* m_cactive;
  mutable size_t m_cactiveLimit;
  static bool s_statsEnabled;
  static size_t s_cactiveLimitCeiling;
  bool m_enableStatsSync;
#endif
};

//////////////////////////////////////////////////////////////////////

}

#include "hphp/runtime/base/memory-manager-inl.h"

#endif
