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

#include <folly/Memory.h>

#include "hphp/util/alloc.h" // must be included before USE_JEMALLOC is used
#include "hphp/util/compilation-flags.h"
#include "hphp/util/trace.h"
#include "hphp/util/thread-local.h"

#include "hphp/runtime/base/memory-usage-stats.h"
#include "hphp/runtime/base/request-event-handler.h"

namespace HPHP {
struct APCLocalArray;
struct MemoryManager;

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

enum class HeaderKind : uint8_t {
  // ArrayKind aliases
  Packed, Mixed, StrMap, IntMap, VPacked, Empty, Apc, Globals, Proxy,
  // Other ordinary refcounted heap objects
  String, Object, Resource, Ref,
  Native, // a NativeData header preceding an HNI ObjectData
  Sweepable, // a Sweepable header preceding an ObjectData ResourceData
  SmallMalloc, // small smart_malloc'd block
  BigMalloc, // big smart_malloc'd block
  BigObj, // big size-tracked object (valid header follows BigNode)
  Free, // small block in a FreeList
  Hole, // wasted space not in any freelist
  Debug // a DebugHeader
};

const size_t HeaderKindOffset = 11;
const unsigned NumHeaderKinds = (uint8_t)HeaderKind::Debug+1;

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
constexpr char kSmartFreeFill = 0x6a;
constexpr char kTVTrashFill = 0x7a;
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
  uint32_t obj_offset;
  char pad[3];
  HeaderKind kind;
};

// POD type for tracking arbitrary memory ranges
struct MemBlock {
  void* ptr;
  size_t size; // bytes
};

// allocator for slabs and big blocks
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

 private:
  void enlist(BigNode*, HeaderKind kind, size_t size);

 private:
  std::vector<MemBlock> m_slabs;
  std::vector<BigNode*> m_bigs;
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
   * An RAII wrapper to suppress OOM checking in a region.
   */
  struct SuppressOOM;

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
   * Return a lower bound estimate of the capacity that will be returned
   * for the requested size.
   */
  static uint32_t estimateSmartCap(uint32_t requested) {
    return requested <= kMaxSmartSize ? smartSizeClass(requested) : requested;
  }

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

  /*
   * During session shutdown, before resetAllocator(), this phase runs
   * through the sweep lists running cleanup for anything that needs
   * to run custom tear down logic before we throw away the
   * request-local memory.
   */
  void sweep();

  /*
   * Returns ptr to head node of m_strings linked list. This used by
   * StringData during a reset, enlist, and delist
   */
  StringDataNode& getStringList() { return m_strings; }

  /*
   * Returns true if there are no allocated slabs
   */
  bool empty() const { return m_heap.empty(); }

  /*
   * Release all the request-local allocations.  Zeros all the free
   * lists and may return some underlying storage to the system
   * allocator. This also resets all internally-stored memory usage stats.
   *
   * This is called after sweep in the end-of-request path.
   */
  void resetAllocator();

  /*
   * Prepare for being idle for a while by releasing or madvising
   * as much as possible.
   */
  void flush();

  /*
   * Reset all stats that are synchronzied externally from the memory manager.
   * Used between sessions and to signal that external sync is now safe to
   * begin (after shared structure initialization that should not be counted is
   * complete.)
   */
  void resetExternalStats();

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

  /*
   * Open and close respectively a stats-tracking interval. Return whether or
   * not the tracking state was changed as a result of the call.
   */
  bool startStatsInterval();
  bool stopStatsInterval();

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

  /*
   * Methods for maintaining dedicated sweep lists of sweepable NativeData
   * objects, and APCLocalArray instances.
   */
  void addNativeObject(NativeNode*);
  void removeNativeObject(NativeNode*);
  void addApcArray(APCLocalArray*);
  void removeApcArray(APCLocalArray*);

  /*
   * Object tracking keeps instances of object data's by using track/untrack.
   * It is then possible to iterate them by iterating the memory manager.
   * This entire feature is enabled/disabled by using setObjectTracking(bool).
   */
  void setObjectTracking(bool val);
  bool getObjectTracking();

  /*
   * Iterating the memory manager tracked objects.
   */
  template<class Fn> void forEachObject(Fn);

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

  /*
   * Request-local heap profiling context.
   */
  struct ReqProfContext {
    bool flag{false};
    bool prof_active{false};
    bool thread_prof_active{false};
    std::string filename;
  };

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

private:
  TRACE_SET_MOD(smartalloc);

  void* m_front;
  void* m_limit;
  std::array<FreeList,kNumSmartSizes> m_freelists;
  StringDataNode m_strings; // in-place node is head of circular list
  std::vector<APCLocalArray*> m_apc_arrays;
  MemoryUsageStats m_stats;
  BigHeap m_heap;
  std::vector<NativeNode*> m_natives;

  bool m_sweeping;
  bool m_trackingInstances;
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
