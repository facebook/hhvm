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
#include "hphp/runtime/base/req-ptr.h"

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
 * req::malloc api for request-scoped memory
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

namespace req {

void* malloc(size_t nbytes);
void* calloc(size_t count, size_t bytes);
void* realloc(void* ptr, size_t nbytes);
void  free(void* ptr);

/*
 * request-heap (de)allocators for non-POD C++-style stuff. Runs constructors
 * and destructors.
 *
 * Unlike the normal operator delete, req::destroy_raw() requires ~T() must
 * be nothrow and that p is not null.
 */
template<class T, class... Args> T* make_raw(Args&&...);
template<class T> void destroy_raw(T* p);

/*
 * Allocate an array of objects.  Similar to req::malloc, but with
 * support for constructors.
 *
 * Note that explicitly calling req::destroy_raw will run the destructors,
 * but if you let the allocator sweep it the destructors will not be
 * called.
 *
 * Unlike the normal operator delete, req::destroy_raw_array requires
 * ~T() must be nothrow.
 */
template<class T> T* make_raw_array(size_t count);
template<class T> void destroy_raw_array(T* t, size_t count);

//////////////////////////////////////////////////////////////////////

// STL-style allocator for the request-heap allocator.  (Unfortunately we
// can't use allocator_traits yet.)
//
// You can also use req::Allocator as a model of folly's
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
    pointer ret = (pointer)req::malloc(num * sizeof(T));
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
    req::free(p);
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
 * Slabs are consumed via bump allocation.  The individual allocations are
 * quantized into a fixed set of size classes, the sizes of which are an
 * implementation detail documented here to shed light on the algorithms that
 * compute size classes.  Request sizes are rounded up to the nearest size in
 * the relevant SMALL_SIZES table; e.g. 17 is rounded up to 32.  There are
 * 4 size classes for each doubling of size
 * (ignoring the alignment-constrained smallest size classes), which limits
 * internal fragmentation to 20%.
 *
 * SMALL_SIZES: Complete table of SMALL_SIZE(index, lg_grp, lg_delta, ndelta,
 *              lg_delta_lookup, ncontig) tuples.
 *   index: Size class index.
 *   lg_grp: Lg group base size (no deltas added).
 *   lg_delta: Lg delta to previous size class.
 *   ndelta: Delta multiplier.  size == 1<<lg_grp + ndelta<<lg_delta
 *   lg_delta_lookup: Same as lg_delta if a lookup table size class, 'no'
 *                    otherwise.
 *   ncontig: Number of contiguous regions to batch allocate in the slow path
 *            due to the corresponding free list being empty.  Must be greater
 *            than zero, and small enough that the contiguous regions fit within
 *            one slab.
 */
#define SMALL_SIZES \
/*         index, lg_grp, lg_delta, ndelta, lg_delta_lookup, ncontig */ \
  SMALL_SIZE(  0,      4,        4,      0,  4,              32) \
  SMALL_SIZE(  1,      4,        4,      1,  4,              32) \
  SMALL_SIZE(  2,      4,        4,      2,  4,              32) \
  SMALL_SIZE(  3,      4,        4,      3,  4,              32) \
  \
  SMALL_SIZE(  4,      6,        4,      1,  4,              24) \
  SMALL_SIZE(  5,      6,        4,      2,  4,              24) \
  SMALL_SIZE(  6,      6,        4,      3,  4,              24) \
  SMALL_SIZE(  7,      6,        4,      4,  4,              24) \
  \
  SMALL_SIZE(  8,      7,        5,      1,  5,              16) \
  SMALL_SIZE(  9,      7,        5,      2,  5,              16) \
  SMALL_SIZE( 10,      7,        5,      3,  5,              16) \
  SMALL_SIZE( 11,      7,        5,      4,  5,              16) \
  \
  SMALL_SIZE( 12,      8,        6,      1,  6,              12) \
  SMALL_SIZE( 13,      8,        6,      2,  6,              12) \
  SMALL_SIZE( 14,      8,        6,      3,  6,              12) \
  SMALL_SIZE( 15,      8,        6,      4,  6,              12) \
  \
  SMALL_SIZE( 16,      9,        7,      1,  7,               8) \
  SMALL_SIZE( 17,      9,        7,      2,  7,               8) \
  SMALL_SIZE( 18,      9,        7,      3,  7,               8) \
  SMALL_SIZE( 19,      9,        7,      4,  7,               8) \
  \
  SMALL_SIZE( 20,     10,        8,      1,  8,               6) \
  SMALL_SIZE( 21,     10,        8,      2,  8,               6) \
  SMALL_SIZE( 22,     10,        8,      3,  8,               6) \
  SMALL_SIZE( 23,     10,        8,      4,  8,               6) \
  \
  SMALL_SIZE( 24,     11,        9,      1,  9,               4) \
  SMALL_SIZE( 25,     11,        9,      2,  9,               4) \
  SMALL_SIZE( 26,     11,        9,      3,  9,               4) \
  SMALL_SIZE( 27,     11,        9,      4,  9,               4) \
  \
  SMALL_SIZE( 28,     12,       10,      1, no,               3) \
  SMALL_SIZE( 29,     12,       10,      2, no,               3) \
  SMALL_SIZE( 30,     12,       10,      3, no,               3) \
  SMALL_SIZE( 31,     12,       10,      4, no,               3) \
  \
  SMALL_SIZE( 32,     13,       11,      1, no,               2) \
  SMALL_SIZE( 33,     13,       11,      2, no,               2) \
  SMALL_SIZE( 34,     13,       11,      3, no,               2) \
  SMALL_SIZE( 35,     13,       11,      4, no,               2) \
  \
  SMALL_SIZE( 36,     14,       12,      1, no,               2) \
  SMALL_SIZE( 37,     14,       12,      2, no,               2) \
  SMALL_SIZE( 38,     14,       12,      3, no,               2) \
  SMALL_SIZE( 39,     14,       12,      4, no,               2) \
  \
  SMALL_SIZE( 40,     15,       13,      1, no,               2) \
  SMALL_SIZE( 41,     15,       13,      2, no,               2) \
  SMALL_SIZE( 42,     15,       13,      3, no,               2) \
  SMALL_SIZE( 43,     15,       13,      4, no,               2) \
  \
  SMALL_SIZE( 44,     16,       14,      1, no,               2) \
  SMALL_SIZE( 45,     16,       14,      2, no,               2) \
  SMALL_SIZE( 46,     16,       14,      3, no,               2) \
  SMALL_SIZE( 47,     16,       14,      4, no,               2) \
  \
  SMALL_SIZE( 48,     17,       15,      1, no,               2) \
  SMALL_SIZE( 49,     17,       15,      2, no,               2) \
  SMALL_SIZE( 50,     17,       15,      3, no,               2) \
  SMALL_SIZE( 51,     17,       15,      4, no,               2) \
  \
  SMALL_SIZE( 52,     18,       16,      1, no,               2) \
  SMALL_SIZE( 53,     18,       16,      2, no,               2) \
  SMALL_SIZE( 54,     18,       16,      3, no,               2) \
  SMALL_SIZE( 55,     18,       16,      4, no,               2) \
  \
  SMALL_SIZE( 56,     19,       17,      1, no,               2) \
  SMALL_SIZE( 57,     19,       17,      2, no,               2) \
  SMALL_SIZE( 58,     19,       17,      3, no,               2) \
  SMALL_SIZE( 59,     19,       17,      4, no,               1) \
  \
  SMALL_SIZE( 60,     20,       18,      1, no,               1) \
  SMALL_SIZE( 61,     20,       18,      2, no,               1) \
  SMALL_SIZE( 62,     20,       18,      3, no,               1) \
  SMALL_SIZE( 63,     20,       18,      4, no,               1) \
  \
  SMALL_SIZE( 64,     21,       19,      1, no,               1) \
  SMALL_SIZE( 65,     21,       19,      2, no,               1) \
  SMALL_SIZE( 66,     21,       19,      3, no,               1) \
  SMALL_SIZE( 67,     21,       19,      4, no,               1) \
  \
  SMALL_SIZE( 68,     22,       20,      1, no,               1) \
  SMALL_SIZE( 69,     22,       20,      2, no,               1) \
  SMALL_SIZE( 70,     22,       20,      3, no,               1) \
  SMALL_SIZE( 71,     22,       20,      4, no,               1) \
  \
  SMALL_SIZE( 72,     23,       21,      1, no,               1) \
  SMALL_SIZE( 73,     23,       21,      2, no,               1) \
  SMALL_SIZE( 74,     23,       21,      3, no,               1) \
  SMALL_SIZE( 75,     23,       21,      4, no,               1) \
  \
  SMALL_SIZE( 76,     24,       22,      1, no,               1) \
  SMALL_SIZE( 77,     24,       22,      2, no,               1) \
  SMALL_SIZE( 78,     24,       22,      3, no,               1) \
  SMALL_SIZE( 79,     24,       22,      4, no,               1) \
  \
  SMALL_SIZE( 80,     25,       23,      1, no,               1) \
  SMALL_SIZE( 81,     25,       23,      2, no,               1) \
  SMALL_SIZE( 82,     25,       23,      3, no,               1) \
  SMALL_SIZE( 83,     25,       23,      4, no,               1) \
  \
  SMALL_SIZE( 84,     26,       24,      1, no,               1) \
  SMALL_SIZE( 85,     26,       24,      2, no,               1) \
  SMALL_SIZE( 86,     26,       24,      3, no,               1) \
  SMALL_SIZE( 87,     26,       24,      4, no,               1) \
  \
  SMALL_SIZE( 88,     27,       25,      1, no,               1) \
  SMALL_SIZE( 89,     27,       25,      2, no,               1) \
  SMALL_SIZE( 90,     27,       25,      3, no,               1) \
  SMALL_SIZE( 91,     27,       25,      4, no,               1) \
  \
  SMALL_SIZE( 92,     28,       26,      1, no,               1) \
  SMALL_SIZE( 93,     28,       26,      2, no,               1) \
  SMALL_SIZE( 94,     28,       26,      3, no,               1) \
  SMALL_SIZE( 95,     28,       26,      4, no,               1) \
  \
  SMALL_SIZE( 96,     29,       27,      1, no,               1) \
  SMALL_SIZE( 97,     29,       27,      2, no,               1) \
  SMALL_SIZE( 98,     29,       27,      3, no,               1) \
  SMALL_SIZE( 99,     29,       27,      4, no,               1) \
  \
  SMALL_SIZE(100,     30,       28,      1, no,               1) \
  SMALL_SIZE(101,     30,       28,      2, no,               1) \
  SMALL_SIZE(102,     30,       28,      3, no,               1) \
  SMALL_SIZE(103,     30,       28,      4, no,               1) \
  \
  SMALL_SIZE(104,     31,       29,      1, no,               1) \
  SMALL_SIZE(105,     31,       29,      2, no,               1) \
  SMALL_SIZE(106,     31,       29,      3, no,               1) \

__attribute__((__aligned__(64)))
constexpr uint8_t kSmallSize2Index[] = {
#define S2I_4(i)  i,
#define S2I_5(i)  S2I_4(i) S2I_4(i)
#define S2I_6(i)  S2I_5(i) S2I_5(i)
#define S2I_7(i)  S2I_6(i) S2I_6(i)
#define S2I_8(i)  S2I_7(i) S2I_7(i)
#define S2I_9(i)  S2I_8(i) S2I_8(i)
#define S2I_no(i)
#define SMALL_SIZE(index, lg_grp, lg_delta, ndelta, lg_delta_lookup, ncontig) \
  S2I_##lg_delta_lookup(index)
  SMALL_SIZES
#undef S2I_4
#undef S2I_5
#undef S2I_6
#undef S2I_7
#undef S2I_8
#undef S2I_9
#undef S2I_no
#undef SMALL_SIZE
};

__attribute__((__aligned__(64)))
constexpr uint32_t kSmallIndex2Size[] = {
#define SMALL_SIZE(index, lg_grp, lg_delta, ndelta, lg_delta_lookup, ncontig) \
  ((uint32_t{1}<<lg_grp) + (uint32_t{ndelta}<<lg_delta)),
  SMALL_SIZES
#undef SMALL_SIZE
};

constexpr uint32_t kMaxSmallSizeLookup = 4096;

constexpr unsigned kLgSlabSize = 21;
constexpr uint32_t kSlabSize = uint32_t{1} << kLgSlabSize;
constexpr unsigned kLgSmallSizeQuantum = 4;
constexpr uint32_t kSmallSizeAlign = 1u << kLgSmallSizeQuantum;
constexpr uint32_t kSmallSizeAlignMask = kSmallSizeAlign - 1;

constexpr unsigned kLgSizeClassesPerDoubling = 2;

/*
 * The maximum size where we use our custom allocator for request-local memory.
 *
 * Allocations larger than this size go to the underlying malloc implementation,
 * and certain specialized allocator functions have preconditions about the
 * requested size being above or below this number to avoid checking at runtime.
 *
 * We want kMaxSmallSize to be the largest size-class less than kSlabSize.
 */
constexpr uint32_t kNumSmallSizes = 63;
static_assert(kNumSmallSizes <= (1 << 6),
              "only 6 bits available in HeaderWord");

constexpr uint32_t kMaxSmallSize = kSmallIndex2Size[kNumSmallSizes-1];
static_assert(kMaxSmallSize > kSmallSizeAlign * 2,
              "Too few size classes");
static_assert(kMaxSmallSize < kSlabSize, "fix kNumSmallSizes or kLgSlabSize");
static_assert(kNumSmallSizes <= sizeof(kSmallSize2Index),
              "Extend SMALL_SIZES table");

constexpr unsigned kSmallPreallocCountLimit = 8;
constexpr uint32_t kSmallPreallocBytesLimit = uint32_t{1} << 9;

/*
 * Constants for the various debug junk-filling of different types of
 * memory in hhvm.
 *
 * jemalloc uses 0x5a to fill freed memory, so we use 0x6a for the
 * request-local allocator so it is easy to tell the difference when
 * debugging.  There's also 0x7a for junk-filling some cases of
 * ex-TypedValue memory (evaluation stack).
 */
constexpr char kSmallFreeFill   = 0x6a;
constexpr char kTVTrashFill     = 0x7a; // used by interpreter
constexpr char kTVTrashFill2    = 0x7b; // used by req::ptr dtors
constexpr char kTVTrashJITStk   = 0x7c; // used by the JIT for stack slots
constexpr char kTVTrashJITFrame = 0x7d; // used by the JIT for stack frames
constexpr char kTVTrashJITHeap  = 0x7e; // used by the JIT for heap
constexpr uintptr_t kSmallFreeWord = 0x6a6a6a6a6a6a6a6aLL;
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
  HeaderWord<> hdr;
  uint32_t& index() { return hdr.hi32; }
};

// Header used for small req::malloc allocations (but not *Size allocs)
struct SmallNode {
  size_t padbytes;
  HeaderWord<> hdr;
};

// all FreeList entries are parsed by inspecting this header.
struct FreeNode {
  FreeNode* next;
  HeaderWord<> hdr;
  uint32_t& size() { return hdr.hi32; }
  uint32_t size() const { return hdr.hi32; }
};

// header for HNI objects with NativeData payloads. see native-data.h
// for details about memory layout.
struct NativeNode {
  uint32_t sweep_index; // index in MM::m_natives
  uint32_t obj_offset; // byte offset from this to ObjectData*
  HeaderWord<> hdr;
};

// header for Resumable objects. See layout comment in resumable.h
struct ResumableNode {
  size_t framesize;
  HeaderWord<> hdr;
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
  template<class Fn> void iterate(Fn);

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
   * Return the size class for a given requested small-allocation size.
   *
   * The return value is greater than or equal to the parameter, and
   * less than or equal to kMaxSmallSize.
   *
   * Pre: requested <= kMaxSmallSize
   */
  static uint32_t smallSizeClass(uint32_t requested);

  /*
   * Return a lower bound estimate of the capacity that will be returned for
   * the requested size.
   */
  static uint32_t estimateCap(uint32_t requested);

  /*
   * Allocate/deallocate a small memory block in a given small size class.
   * You must be able to tell the deallocation function how big the
   * allocation was.
   *
   * The size passed to mallocSmallSize does not need to be an exact
   * size class (although stats accounting may undercount in this
   * case).  The size passed to freeSmallSize must be the exact size
   * that was passed to mallocSmallSize for that allocation.
   *
   * The returned pointer is guaranteed to be 16-byte aligned.
   *
   * Pre: size > 0 && size <= kMaxSmallSize
   */
  void* mallocSmallSize(uint32_t size);
  void freeSmallSize(void* p, uint32_t size);

  /*
   * Allocate/deallocate memory that is too big for the small size classes.
   *
   * Returns a pointer and the actual size of the allocation, which
   * amay be larger than the requested size.  The returned pointer is
   * guaranteed to be 16-byte aligned.
   *
   * The size passed to freeBigSize must either be the size that was
   * passed to mallocBigSize, or the value that was returned as the
   * actual allocation size.
   *
   * Pre: size > kMaxSmallSize
   */
  template<bool callerSavesActualSize>
  MemBlock mallocBigSize(size_t size);
  void freeBigSize(void* vp, size_t size);

  /*
   * Allocate/deallocate objects when the size is not known to be
   * above or below kMaxSmallSize without a runtime check.
   *
   * These functions use the same underlying allocator as
   * malloc{Small,Big}Size, and it is safe to return allocations using
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
   * Allocate/deallocate by size class index.  This is useful when size
   * class is already calculated at the call site.
   */
  void* mallocSmallIndex(size_t index, uint32_t size);
  void freeSmallIndex(void* ptr, size_t index, uint32_t size);

  /*
   * These functions are useful when working directly with size classes outside
   * this class.
   *
   * Note that we intentionally use size_t for size class index here, so that
   * gcc would not generate inefficient code.
   */
  static size_t computeSmallSize2Index(uint32_t size);
  static size_t lookupSmallSize2Index(uint32_t size);
  static size_t smallSize2Index(uint32_t size);
  static uint32_t smallIndex2Size(size_t index);

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
   * req::ptr, etc. or other handle class.
   */
  template <typename T> RootId addRoot(req::ptr<T>&& ptr);
  template <typename T> RootId addRoot(const req::ptr<T>& ptr);
  template <typename T> req::ptr<T> lookupRoot(RootId tok) const;
  template <typename T> bool removeRoot(const req::ptr<T>& ptr);
  template <typename T> bool removeRoot(const T* ptr);
  template <typename T> req::ptr<T> removeRoot(RootId token);
  template <typename F> void scanRootMaps(F& m) const;

  /*
   * Heap iterator methods.
   */
  template<class Fn> void iterate(Fn);
  template<class Fn> void forEachHeader(Fn);
  template<class Fn> void forEachObject(Fn);

  /*
   * Run the experimental collector.
   * Has no effect other than possibly asserting.
   */
  void collect();

  /////////////////////////////////////////////////////////////////////////////

private:
  friend void* req::malloc(size_t nbytes);
  friend void* req::calloc(size_t count, size_t bytes);
  friend void* req::realloc(void* ptr, size_t nbytes);
  friend void  req::free(void* ptr);

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
      req::ptr<T>,
      std::hash<RootId>,
      std::equal_to<RootId>,
      req::Allocator<std::pair<const RootId,req::ptr<T>>>
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
  void* newSlab(uint32_t nbytes);
  void storeTail(void* tail, uint32_t tailBytes);
  void splitTail(void* tail, uint32_t tailBytes, unsigned nSplit,
                 uint32_t splitUsable, unsigned splitInd);
  void* mallocSmallSizeSlow(uint32_t bytes, unsigned index);
  void  updateBigStats();
  void* mallocBig(size_t nbytes);
  void* callocBig(size_t nbytes);
  void* malloc(size_t nbytes);
  void* realloc(void* ptr, size_t nbytes);
  void  free(void* ptr);

  static uint32_t bsr(uint32_t x);

  static void threadStatsInit();
  static void threadStats(uint64_t*&, uint64_t*&, size_t*&, size_t&);
  void refreshStats();
  template<bool live> void refreshStatsImpl(MemoryUsageStats& stats);
  void refreshStatsHelperExceeded();
  void refreshStatsHelperStop();

  void resetStatsImpl(bool isInternalCall);

  void logAllocation(void*, size_t);
  void logDeallocation(void*);

  void checkHeap();
  void initHole(void* ptr, uint32_t size);
  void initHole();
  void initFree();

  void dropRootMaps();
  void deleteRootMaps();

  template <typename T>
  typename std::enable_if<
    std::is_base_of<ResourceData,T>::value,
    RootMap<ResourceData>&
  >::type getRootMap() {
    if (UNLIKELY(!m_resourceRoots)) {
      m_resourceRoots = req::make_raw<RootMap<ResourceData>>();
    }
    return *m_resourceRoots;
  }

  template <typename T>
  typename std::enable_if<
    std::is_base_of<ObjectData,T>::value,
    RootMap<ObjectData>&
  >::type getRootMap() {
    if (UNLIKELY(!m_objectRoots)) {
      m_objectRoots = req::make_raw<RootMap<ObjectData>>();
    }
    return *m_objectRoots;
  }

  template <typename T>
  typename std::enable_if<
    std::is_base_of<ResourceData,T>::value,
    const RootMap<ResourceData>&
  >::type getRootMap() const {
    if (UNLIKELY(!m_resourceRoots)) {
      m_resourceRoots = req::make_raw<RootMap<ResourceData>>();
    }
    return *m_resourceRoots;
  }

  template <typename T>
  typename std::enable_if<
    std::is_base_of<ObjectData,T>::value,
    const RootMap<ObjectData>&
  >::type getRootMap() const {
    if (UNLIKELY(!m_objectRoots)) {
      m_objectRoots = req::make_raw<RootMap<ObjectData>>();
    }
    return *m_objectRoots;
  }

  /////////////////////////////////////////////////////////////////////////////

private:
  TRACE_SET_MOD(mm);

  void* m_front;
  void* m_limit;
  std::array<FreeList,kNumSmallSizes> m_freelists;
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
  bool m_needInitFree{false}; // true after free(), false after initFree()

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
