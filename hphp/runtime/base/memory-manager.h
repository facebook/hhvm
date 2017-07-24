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
#include "hphp/util/thread-local.h"
#include "hphp/util/trace.h"
#include "hphp/util/type-scan.h"

#include "hphp/runtime/base/memory-usage-stats.h"
#include "hphp/runtime/base/request-event-handler.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/sweepable.h"
#include "hphp/runtime/base/header-kind.h"
#include "hphp/runtime/base/req-containers.h"
#include "hphp/runtime/base/req-malloc.h"
#include "hphp/runtime/base/req-ptr.h"

namespace HPHP {

struct APCLocalArray;
struct MemoryManager;
struct ObjectData;
struct ResourceData;

namespace req {
struct root_handle;
void* malloc_big(size_t, type_scan::Index);
void* calloc_big(size_t, type_scan::Index);
void* realloc_big(void*, size_t);
void  free_big(void*);
}

//////////////////////////////////////////////////////////////////////

/*
 * Request local memory in HHVM is managed by a thread local object
 * called MemoryManager.
 *
 * The object may be accessed with MM(), but higher-level apis are
 * also provided.
 *
 * The MemoryManager serves the following functions in hhvm:
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
 * Slabs are consumed via bump allocation.  The individual allocations are
 * quantized into a fixed set of size classes, the sizes of which are an
 * implementation detail documented here to shed light on the algorithms that
 * compute size classes.  Request sizes are rounded up to the nearest size in
 * the relevant SIZE_CLASSES table; e.g. 17 is rounded up to 32.  There are
 * 4 size classes for each doubling of size
 * (ignoring the alignment-constrained smallest size classes), which limits
 * internal fragmentation to 20%.
 *
 * SIZE_CLASSES: Complete table of SIZE_CLASS(index, lg_grp, lg_delta, ndelta,
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
#define SIZE_CLASSES \
/*         index, lg_grp, lg_delta, ndelta, lg_delta_lookup, ncontig */ \
  SIZE_CLASS(  0,      4,        4,      0,  4,             128) \
  SIZE_CLASS(  1,      4,        4,      1,  4,             128) \
  SIZE_CLASS(  2,      4,        4,      2,  4,             128) \
  SIZE_CLASS(  3,      4,        4,      3,  4,              96) /* 64 */\
  \
  SIZE_CLASS(  4,      6,        4,      1,  4,              96) \
  SIZE_CLASS(  5,      6,        4,      2,  4,              96) \
  SIZE_CLASS(  6,      6,        4,      3,  4,              96) \
  SIZE_CLASS(  7,      6,        4,      4,  4,              64) /* 128 */\
  \
  SIZE_CLASS(  8,      7,        5,      1,  5,              64) \
  SIZE_CLASS(  9,      7,        5,      2,  5,              64) \
  SIZE_CLASS( 10,      7,        5,      3,  5,              64) \
  SIZE_CLASS( 11,      7,        5,      4,  5,              32) /* 256 */\
  \
  SIZE_CLASS( 12,      8,        6,      1,  6,              32) \
  SIZE_CLASS( 13,      8,        6,      2,  6,              32) \
  SIZE_CLASS( 14,      8,        6,      3,  6,              32) \
  SIZE_CLASS( 15,      8,        6,      4,  6,              16) /* 512 */\
  \
  SIZE_CLASS( 16,      9,        7,      1,  7,              16) \
  SIZE_CLASS( 17,      9,        7,      2,  7,              16) \
  SIZE_CLASS( 18,      9,        7,      3,  7,              16) \
  SIZE_CLASS( 19,      9,        7,      4,  7,               8) /* 1K */\
  \
  SIZE_CLASS( 20,     10,        8,      1,  8,               8) \
  SIZE_CLASS( 21,     10,        8,      2,  8,               8) \
  SIZE_CLASS( 22,     10,        8,      3,  8,               8) \
  SIZE_CLASS( 23,     10,        8,      4,  8,               4) /* 2K */\
  \
  SIZE_CLASS( 24,     11,        9,      1,  9,               4) \
  SIZE_CLASS( 25,     11,        9,      2,  9,               4) \
  SIZE_CLASS( 26,     11,        9,      3,  9,               4) \
  SIZE_CLASS( 27,     11,        9,      4,  9,               2) /* 4K */\
  \
  SIZE_CLASS( 28,     12,       10,      1, no,               2) \
  SIZE_CLASS( 29,     12,       10,      2, no,               2) \
  SIZE_CLASS( 30,     12,       10,      3, no,               2) \
  SIZE_CLASS( 31,     12,       10,      4, no,               1) /* 8K */\
  \
  SIZE_CLASS( 32,     13,       11,      1, no,               1) \
  SIZE_CLASS( 33,     13,       11,      2, no,               1) \
  SIZE_CLASS( 34,     13,       11,      3, no,               1) \
  SIZE_CLASS( 35,     13,       11,      4, no,               1) /* 16K */\
  \
  SIZE_CLASS( 36,     14,       12,      1, no,               1) \
  SIZE_CLASS( 37,     14,       12,      2, no,               1) \
  SIZE_CLASS( 38,     14,       12,      3, no,               1) \
  SIZE_CLASS( 39,     14,       12,      4, no,               1) /* 32K */\
  \
  SIZE_CLASS( 40,     15,       13,      1, no,               1) \
  SIZE_CLASS( 41,     15,       13,      2, no,               1) \
  SIZE_CLASS( 42,     15,       13,      3, no,               1) \
  SIZE_CLASS( 43,     15,       13,      4, no,               1) /* 64K */\
  \
  SIZE_CLASS( 44,     16,       14,      1, no,               1) \
  SIZE_CLASS( 45,     16,       14,      2, no,               1) \
  SIZE_CLASS( 46,     16,       14,      3, no,               1) \
  SIZE_CLASS( 47,     16,       14,      4, no,               1) /* 128K */\
  \
  SIZE_CLASS( 48,     17,       15,      1, no,               1) \
  SIZE_CLASS( 49,     17,       15,      2, no,               1) \
  SIZE_CLASS( 50,     17,       15,      3, no,               1) \
  SIZE_CLASS( 51,     17,       15,      4, no,               1) /* 256K */\
  \
  SIZE_CLASS( 52,     18,       16,      1, no,               1) \
  SIZE_CLASS( 53,     18,       16,      2, no,               1) \
  SIZE_CLASS( 54,     18,       16,      3, no,               1) \
  SIZE_CLASS( 55,     18,       16,      4, no,               1) /* 512K */\
  \
  SIZE_CLASS( 56,     19,       17,      1, no,               1) \
  SIZE_CLASS( 57,     19,       17,      2, no,               1) \
  SIZE_CLASS( 58,     19,       17,      3, no,               1) \
  SIZE_CLASS( 59,     19,       17,      4, no,               1) /* 1M */\
  \
  SIZE_CLASS( 60,     20,       18,      1, no,               1) \
  SIZE_CLASS( 61,     20,       18,      2, no,               1) \
  SIZE_CLASS( 62,     20,       18,      3, no,               1) \
  SIZE_CLASS( 63,     20,       18,      4, no,               1) /* 2M */\
  \
  SIZE_CLASS( 64,     21,       19,      1, no,               1) \
  SIZE_CLASS( 65,     21,       19,      2, no,               1) \
  SIZE_CLASS( 66,     21,       19,      3, no,               1) \
  SIZE_CLASS( 67,     21,       19,      4, no,               1) /* 4M */\
  \
  SIZE_CLASS( 68,     22,       20,      1, no,               1) \
  SIZE_CLASS( 69,     22,       20,      2, no,               1) \
  SIZE_CLASS( 70,     22,       20,      3, no,               1) \
  SIZE_CLASS( 71,     22,       20,      4, no,               1) /* 8M */\
  \
  SIZE_CLASS( 72,     23,       21,      1, no,               1) \
  SIZE_CLASS( 73,     23,       21,      2, no,               1) \
  SIZE_CLASS( 74,     23,       21,      3, no,               1) \
  SIZE_CLASS( 75,     23,       21,      4, no,               1) /* 16M */\
  \
  SIZE_CLASS( 76,     24,       22,      1, no,               1) \
  SIZE_CLASS( 77,     24,       22,      2, no,               1) \
  SIZE_CLASS( 78,     24,       22,      3, no,               1) \
  SIZE_CLASS( 79,     24,       22,      4, no,               1) /* 32M */\
  \
  SIZE_CLASS( 80,     25,       23,      1, no,               1) \
  SIZE_CLASS( 81,     25,       23,      2, no,               1) \
  SIZE_CLASS( 82,     25,       23,      3, no,               1) \
  SIZE_CLASS( 83,     25,       23,      4, no,               1) /* 64M */\
  \
  SIZE_CLASS( 84,     26,       24,      1, no,               1) \
  SIZE_CLASS( 85,     26,       24,      2, no,               1) \
  SIZE_CLASS( 86,     26,       24,      3, no,               1) \
  SIZE_CLASS( 87,     26,       24,      4, no,               1) /* 128M */\
  \
  SIZE_CLASS( 88,     27,       25,      1, no,               1) \
  SIZE_CLASS( 89,     27,       25,      2, no,               1) \
  SIZE_CLASS( 90,     27,       25,      3, no,               1) \
  SIZE_CLASS( 91,     27,       25,      4, no,               1) /* 256M */\
  \
  SIZE_CLASS( 92,     28,       26,      1, no,               1) \
  SIZE_CLASS( 93,     28,       26,      2, no,               1) \
  SIZE_CLASS( 94,     28,       26,      3, no,               1) \
  SIZE_CLASS( 95,     28,       26,      4, no,               1) /* 512M */\
  \
  SIZE_CLASS( 96,     29,       27,      1, no,               1) \
  SIZE_CLASS( 97,     29,       27,      2, no,               1) \
  SIZE_CLASS( 98,     29,       27,      3, no,               1) \
  SIZE_CLASS( 99,     29,       27,      4, no,               1) /* 1G */\
  \
  SIZE_CLASS(100,     30,       28,      1, no,               1) \
  SIZE_CLASS(101,     30,       28,      2, no,               1) \
  SIZE_CLASS(102,     30,       28,      3, no,               1) \
  SIZE_CLASS(103,     30,       28,      4, no,               1) /* 2G */\
  \
  SIZE_CLASS(104,     31,       29,      1, no,               1) \
  SIZE_CLASS(105,     31,       29,      2, no,               1) \
  SIZE_CLASS(106,     31,       29,      3, no,               1) \
  SIZE_CLASS(107,     31,       29,      4, no,               1) /* 4G */\
  \
  SIZE_CLASS(108,     32,       30,      1, no,               1) \
  SIZE_CLASS(109,     32,       30,      2, no,               1) \
  SIZE_CLASS(110,     32,       30,      3, no,               1) \
  SIZE_CLASS(111,     32,       30,      4, no,               1) /* 8G */\
  \
  SIZE_CLASS(112,     33,       31,      1, no,               1) \
  SIZE_CLASS(113,     33,       31,      2, no,               1) \
  SIZE_CLASS(114,     33,       31,      3, no,               1) \
  SIZE_CLASS(115,     33,       31,      4, no,               1) /* 16G */\
  \
  SIZE_CLASS(116,     34,       32,      1, no,               1) \
  SIZE_CLASS(117,     34,       32,      2, no,               1) \
  SIZE_CLASS(118,     34,       32,      3, no,               1) \
  SIZE_CLASS(119,     34,       32,      4, no,               1) /* 32G */\
  \
  SIZE_CLASS(120,     35,       33,      1, no,               1) \
  SIZE_CLASS(121,     35,       33,      2, no,               1) \
  SIZE_CLASS(122,     35,       33,      3, no,               1) \
  SIZE_CLASS(123,     35,       33,      4, no,               1) /* 64G */\
  \
  SIZE_CLASS(124,     36,       34,      1, no,               1) \
  SIZE_CLASS(125,     36,       34,      2, no,               1) \
  SIZE_CLASS(126,     36,       34,      3, no,               1) \
  SIZE_CLASS(127,     36,       34,      4, no,               1) /* 128G */\
  \
  SIZE_CLASS(128,     37,       35,      1, no,               1) \
  SIZE_CLASS(129,     37,       35,      2, no,               1) \
  SIZE_CLASS(130,     37,       35,      3, no,               1) \
  SIZE_CLASS(131,     37,       35,      4, no,               1) /* 256G */\
  \
  SIZE_CLASS(132,     38,       36,      1, no,               1) \
  SIZE_CLASS(133,     38,       36,      2, no,               1) \
  SIZE_CLASS(134,     38,       36,      3, no,               1) \
  SIZE_CLASS(135,     38,       36,      4, no,               1) /* 512G */\
  \
  SIZE_CLASS(136,     39,       37,      1, no,               1) \
  SIZE_CLASS(137,     39,       37,      2, no,               1) \
  SIZE_CLASS(138,     39,       37,      3, no,               1) \
  SIZE_CLASS(139,     39,       37,      4, no,               1) /* 1T */\
  \
  SIZE_CLASS(140,     40,       38,      1, no,               1) \
  SIZE_CLASS(141,     40,       38,      2, no,               1) \
  SIZE_CLASS(142,     40,       38,      3, no,               1) \
  SIZE_CLASS(143,     40,       38,      4, no,               1) /* 2T */\
  \
  SIZE_CLASS(144,     41,       39,      1, no,               1) \
  SIZE_CLASS(145,     41,       39,      2, no,               1) \
  SIZE_CLASS(146,     41,       39,      3, no,               1) \
  SIZE_CLASS(147,     41,       39,      4, no,               1) /* 4T */\
  \
  SIZE_CLASS(148,     42,       40,      1, no,               1) \
  SIZE_CLASS(149,     42,       40,      2, no,               1) \
  SIZE_CLASS(150,     42,       40,      3, no,               1) \
  SIZE_CLASS(151,     42,       40,      4, no,               1) /* 8T */\
  \
  SIZE_CLASS(152,     43,       41,      1, no,               1) \
  SIZE_CLASS(153,     43,       41,      2, no,               1) \
  SIZE_CLASS(154,     43,       41,      3, no,               1) \
  SIZE_CLASS(155,     43,       41,      4, no,               1) /* 16T */\
  \
  SIZE_CLASS(156,     44,       42,      1, no,               1) \
  SIZE_CLASS(157,     44,       42,      2, no,               1) \
  SIZE_CLASS(158,     44,       42,      3, no,               1) \
  SIZE_CLASS(159,     44,       42,      4, no,               1) /* 32T */\
  \
  SIZE_CLASS(160,     45,       43,      1, no,               1) \
  SIZE_CLASS(161,     45,       43,      2, no,               1) \
  SIZE_CLASS(162,     45,       43,      3, no,               1) \
  SIZE_CLASS(163,     45,       43,      4, no,               1) /* 64T */\
  \
  SIZE_CLASS(164,     46,       44,      1, no,               1) \
  SIZE_CLASS(165,     46,       44,      2, no,               1) \
  SIZE_CLASS(166,     46,       44,      3, no,               1) \
  SIZE_CLASS(167,     46,       44,      4, no,               1) /* 128T */\
  \
  SIZE_CLASS(168,     47,       45,      1, no,               1) \
  SIZE_CLASS(169,     47,       45,      2, no,               1) \
  SIZE_CLASS(170,     47,       45,      3, no,               1) \
  SIZE_CLASS(171,     47,       45,      4, no,               1) /* 256T */\

constexpr size_t kNumSizeClasses = 172;

alignas(64) constexpr uint8_t kSmallSize2Index[] = {
#define S2I_4(i)  i,
#define S2I_5(i)  S2I_4(i) S2I_4(i)
#define S2I_6(i)  S2I_5(i) S2I_5(i)
#define S2I_7(i)  S2I_6(i) S2I_6(i)
#define S2I_8(i)  S2I_7(i) S2I_7(i)
#define S2I_9(i)  S2I_8(i) S2I_8(i)
#define S2I_no(i)
#define SIZE_CLASS(index, lg_grp, lg_delta, ndelta, lg_delta_lookup, ncontig) \
  S2I_##lg_delta_lookup(index)
  SIZE_CLASSES
#undef S2I_4
#undef S2I_5
#undef S2I_6
#undef S2I_7
#undef S2I_8
#undef S2I_9
#undef S2I_no
#undef SIZE_CLASS
};

constexpr size_t kSizeIndex2Size[] = {
#define SIZE_CLASS(index, lg_grp, lg_delta, ndelta, lg_delta_lookup, ncontig) \
  ((size_t{1}<<lg_grp) + (size_t{ndelta}<<lg_delta)),
  SIZE_CLASSES
#undef SIZE_CLASS
};

alignas(64) constexpr unsigned kNContigTab[] = {
#define SIZE_CLASS(index, lg_grp, lg_delta, ndelta, lg_delta_lookup, ncontig) \
  ncontig,
  SIZE_CLASSES
#undef SIZE_CLASS
};

constexpr size_t kMaxSmallSizeLookup = 4096;

constexpr unsigned kLgSlabSize = 21;
constexpr uint32_t kSlabSize = uint32_t{1} << kLgSlabSize;
constexpr unsigned kLgSmallSizeQuantum = 4;
constexpr size_t kSmallSizeAlign = 1u << kLgSmallSizeQuantum;
constexpr size_t kSmallSizeAlignMask = kSmallSizeAlign - 1;

constexpr unsigned kLgSizeClassesPerDoubling = 2;
constexpr size_t kSizeClassesPerDoubling = (1u << kLgSizeClassesPerDoubling);

/*
 * The maximum size where we use our custom allocator for request-local memory.
 *
 * Allocations larger than this size go to the underlying malloc implementation,
 * and certain specialized allocator functions have preconditions about the
 * requested size being above or below this number to avoid checking at runtime.
 *
 * We want kMaxSmallSize to be the largest size-class less than kSlabSize.
 */
constexpr size_t kNumSmallSizes = 63;
static_assert(kNumSmallSizes <= (1 << 6),
              "only 6 bits available in HeapObject");
static_assert(kNumSmallSizes < kNumSizeClasses,
              "small sizes should be a proper subset of all sizes");
static_assert(kNumSizeClasses <= (sizeof(kSizeIndex2Size) / sizeof(size_t)),
              "Extend SIZE_CLASSES macro");

constexpr size_t kMaxSmallSize = kSizeIndex2Size[kNumSmallSizes-1];
static_assert(kMaxSmallSize > kSmallSizeAlign * 2,
              "Too few size classes");
static_assert(kMaxSmallSize < kSlabSize, "fix kNumSmallSizes or kLgSlabSize");

constexpr size_t kMaxSizeClass = kSizeIndex2Size[kNumSizeClasses-1];
static_assert(kMaxSmallSize < kMaxSizeClass,
              "largest small size should be smaller than largest overall size");

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
constexpr char kRDSTrashFill    = 0x6b; // used by RDS for "normal" section
constexpr char kTrashClsRef     = 0x6c; // used for class-ref slots
constexpr char kTrashCufIter    = 0x6d; // used for cuf-iters
constexpr char kTVTrashFill     = 0x7a; // used by interpreter
constexpr char kTVTrashFill2    = 0x7b; // used by req::ptr dtors
constexpr char kTVTrashJITStk   = 0x7c; // used by the JIT for stack slots
constexpr char kTVTrashJITFrame = 0x7d; // used by the JIT for stack frames
constexpr char kTVTrashJITHeap  = 0x7e; // used by the JIT for heap
constexpr char kTVTrashJITRetVal = 0x7f; // used by the JIT for ActRec::m_r
constexpr uintptr_t kSmallFreeWord = 0x6a6a6a6a6a6a6a6aLL;
constexpr uintptr_t kMallocFreeWord = 0x5a5a5a5a5a5a5a5aLL;

//////////////////////////////////////////////////////////////////////

// Header MemoryManager uses for StringDatas that wrap APCHandle
struct StringDataNode {
  StringDataNode* next;
  StringDataNode* prev;
};

static_assert(std::numeric_limits<type_scan::Index>::max() <=
              std::numeric_limits<uint16_t>::max(),
              "type_scan::Index must be no greater than 16-bits "
              "to fit into HeapObject");

// This is the header MemoryManager uses to remember large allocations
// so they can be auto-freed in MemoryManager::reset(), as well as large/small
// req::malloc()'d blocks, which must track their size internally.
struct MallocNode : HeapObject {
  size_t nbytes;
  uint32_t& index() { return m_aux32; }
  uint16_t& typeIndex() { return m_aux16; }
  uint16_t typeIndex() const { return m_aux16; }
};

// all FreeList entries are parsed by inspecting this header.
struct FreeNode : HeapObject {
  FreeNode* next;
  uint32_t& size() { return m_aux32; }
  uint32_t size() const { return m_aux32; }
  static FreeNode* InitFrom(void* addr, uint32_t size, HeaderKind);
  static FreeNode* UninitFrom(void* addr, FreeNode* next);
};

static_assert(sizeof(FreeNode) <= kSmallSizeAlign,
              "FreeNode must fit into the smallest size class");

// header for HNI objects with NativeData payloads. see native-data.h
// for details about memory layout.
struct NativeNode : HeapObject,
                    type_scan::MarkCountable<NativeNode> {
  NativeNode(HeaderKind k, uint32_t off) : obj_offset(off) {
    initHeader(k, 0);
  }
  uint32_t sweep_index; // index in MM::m_natives
  uint32_t obj_offset; // byte offset from this to ObjectData*
  uint16_t& typeIndex() { return m_aux16; }
  uint16_t typeIndex() const { return m_aux16; }
  uint32_t arOff() const { return m_count; } // from this to ActRec, or 0
};

// POD type for tracking arbitrary memory ranges
template<class T> struct MemRange {
  T ptr;
  size_t size; // bytes
};

using MemBlock = MemRange<void*>;
using HdrBlock = MemRange<HeapObject*>;

///////////////////////////////////////////////////////////////////////////////

/*
 * Allocator for slabs and big blocks.
 */
struct BigHeap {
  BigHeap() {}
  ~BigHeap();

  /*
   * Is the heap empty?
   */
  bool empty() const;

  /*
   * Whether `ptr' refers to slab-allocated memory.
   *
   * Note that memory in big blocks is explicitly excluded.
   */
  bool contains(void* ptr) const;

  /*
   * Allocate a MemBlock of at least size bytes, track in m_slabs.
   */
  MemBlock allocSlab(size_t size);

  /*
   * Allocation API for big blocks.
   */
  MemBlock allocBig(size_t size, HeaderKind kind, type_scan::Index tyindex);
  MemBlock callocBig(size_t size, HeaderKind kind, type_scan::Index tyindex);
  MemBlock resizeBig(void* p, size_t size);
  void freeBig(void*);

  /*
   * Free all slabs and big blocks.
   */
  void reset();

  /*
   * Release auxiliary structures to prepare to be idle for a while.
   *
   * @requires: empty()
   */
  void flush();

  /*
   * Iterate over all the slabs and bigs, calling Fn on each block, including
   * all of the blocks in each slab.
   */
  template<class Fn> void iterate(Fn);

  /*
   * call OnBig() on each BigObj & BigMalloc header, and OnSlab() on
   * each slab, without iterating the blocks within each slab.
   */
  template<class OnBig, class OnSlab> void iterate(OnBig, OnSlab);

  /*
   * Find the HeapObject* which contains `p', else nullptr if `p' is
   * not contained in any heap allocation.
   */
  HeapObject* find(const void* p);

  /*
   * Sorting helpers
   */
  void sortSlabs();
  void sortBigs();

 protected:
  void enlist(MallocNode*, HeaderKind kind, size_t size, type_scan::Index);

 protected:
  std::vector<MemBlock> m_slabs;
  std::vector<MallocNode*> m_bigs;
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
  static size_t smallSizeClass(size_t requested);

  /*
   * Return a lower bound estimate of the capacity that will be returned for
   * the requested size.
   */
  static size_t estimateCap(size_t requested);

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
  void* mallocSmallSize(size_t size);
  void freeSmallSize(void* p, size_t size);

  /*
   * Allocate/deallocate memory that is too big for the small size classes.
   *
   * Returns a pointer and the actual size of the allocation, which
   * may be larger than the requested size.  The returned pointer is
   * guaranteed to be 16-byte aligned.
   *
   * The size passed to freeBigSize must either be the requested size that was
   * passed to mallocBigSize, or the MemBlock size that was returned as the
   * actual allocation size.
   *
   * Mode of ZeroFreeActual is the same as FreeActual, but zeros memory.
   *
   * Pre: size > kMaxSmallSize
   */
  enum MBS {
    FreeRequested, // caller frees requested size
    FreeActual,    // caller frees actual size returned in MemBlock
    ZeroFreeActual // calloc & FreeActual
  };
  template<MBS Mode>
  void* mallocBigSize(size_t size, HeaderKind kind = HeaderKind::BigObj,
                      type_scan::Index tyindex = 0);
  void freeBigSize(void* vp, size_t size);
  void* resizeBig(MallocNode* n, size_t nbytes);

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
  void* mallocSmallIndex(size_t index);
  void freeSmallIndex(void* ptr, size_t index);

  /*
   * Allocate/deallocate by size class index, when the index is not known to
   * be < kNumSmallSizes This is useful when size class is already calculated
   * at the call site, but might not be a small size class.
   *
   * The index passed to objFreeIndex must be the same one passed to
   * objMallocIndex.
   */
  void* objMallocIndex(size_t index);
  void objFreeIndex(void* ptr, size_t index);

  /*
   * These functions are useful when working directly with size classes outside
   * this class.
   *
   * Note that we intentionally use size_t for size class index here, so that
   * gcc would not generate inefficient code.
   */
  static size_t computeSize2Index(size_t size);
  static size_t lookupSmallSize2Index(size_t size);
  static size_t smallSize2Index(size_t size);
  static size_t size2Index(size_t size);
  static size_t sizeIndex2Size(size_t index);

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
   *
   * Note that this explicitly excludes allocations that are made through the
   * big alloc API.
   */
  bool contains(void* p) const;
  bool checkContains(void* p) const;

  /*
   * Heap iterator methods.  `fn' takes a HeapObject* argument.
   *
   * initFree(): prepare to iterate by initializing free block headers.
   * iterate(): Raw iterator loop over every HeapObject in the heap.
   *            Skips BigObj because it's just a detail of which sub-heap we
   *            used to allocate something based on its size, and it can prefix
   *            almost any other header kind, and also skips Hole. Clients can
   *            call this directly to avoid unnecessary initFree()s.
   * forEachHeapObject(): Like iterate(), but with an eager initFree().
   * forEachObject(): Iterate just the ObjectDatas, including the kinds with
   *                  prefixes (NativeData, AsyncFuncFrame, and ClosureHdr).
   */
  void initFree();
  template<class Fn> void iterate(Fn fn);
  template<class Fn> void forEachHeapObject(Fn fn);
  template<class Fn> void forEachObject(Fn fn);

  /*
   * Iterate over the roots owned by MemoryManager.
   * call fn(ptr, size, type_scan::Index) for each root
   */
  template<class Fn> void iterateRoots(Fn) const;

  /*
   * Find the HeapObject* which contains `p', else nullptr if `p' is
   * not contained in any heap allocation.
   */
  HeapObject* find(const void* p);

  /////////////////////////////////////////////////////////////////////////////
  // Stats.

  /*
   * Update the request-memory limit.
   */
  void setMemoryLimit(size_t limit);
  int64_t getMemoryLimit() const;

  /*
   * Update the tracked stats in the MemoryManager object, then return
   * a copy of the stats.
   */
  MemoryUsageStats getStats();

  /*
   * Get most recent stats data, as one would with getStats(), but without
   * altering the underlying data stored in the MemoryManager, triggering
   * OOM, or triggering the memory threshold callback. Used for obtaining
   * allocation counters passively.
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
  int64_t currentUsage() const;

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
  static bool exiting();
  static void setExiting();

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
  template<class Fn> void sweepApcArrays(Fn fn);
  template<class Fn> void sweepApcStrings(Fn fn);

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
   * Installs a PHP callback for the specified peak memory watermarks
   * Each request may have 1 memory callback for 1 specific threshold
   * at a given time
   */
  void setMemThresholdCallback(size_t threshold);

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

  /*
   * Setup/teardown profiling for current request.  This causes all allocation
   * requests to be passed through to the underlying memory allocator so that
   * heap profiling can capture backtraces for individual allocations rather
   * than slab allocations.
   */
  static void setupProfiling();
  static void teardownProfiling();

  /////////////////////////////////////////////////////////////////////////////
  // Garbage collection.

  /*
   * Returns ptr to head node of m_strings linked list. This used by
   * StringData during a reset, enlist, and delist
   */
  StringDataNode& getStringList();

  /*
   * Run the experimental collector.
   */
  void collect(const char* phase);
  void resetGC();
  void updateNextGc();

  bool isGCEnabled();
  void setGCEnabled(bool isGCEnabled);

  struct FreeList {
    void* maybePop();
    void push(void*);
    FreeNode* head{nullptr};
  };
  using FreelistArray = std::array<FreeList,kNumSmallSizes>;

  /*
   * beginQuarantine() swaps out the normal freelists. endQuarantine()
   * fills everything freed with holes, then restores the original freelists.
   */
  FreelistArray beginQuarantine();
  void endQuarantine(FreelistArray&&);

  /*
   * Run an integrity check on the heap
   */
  void checkHeap(const char* phase);

  /////////////////////////////////////////////////////////////////////////////

private:
  friend struct req::root_handle; // access m_root_handles

  // head node of the doubly-linked list of Sweepables
  struct SweepableList : Sweepable {
    SweepableList() : Sweepable(Init{}) {}
    void sweep() override {}
    void* owner() override { return nullptr; }
  };

  /*
   * Request-local heap profiling context.
   */
  struct ReqProfContext {
    bool flag{false};
    bool prof_active{false};
    bool thread_prof_active{false};
    std::string filename{};
  };

  /////////////////////////////////////////////////////////////////////////////

private:
  MemoryManager();
  MemoryManager(const MemoryManager&) = delete;
  MemoryManager& operator=(const MemoryManager&) = delete;
  ~MemoryManager();

private:
  void storeTail(void* tail, uint32_t tailBytes);
  void splitTail(void* tail, uint32_t tailBytes, unsigned nSplit,
                 uint32_t splitUsable, unsigned splitInd);
  void* slabAlloc(uint32_t bytes, size_t index);
  void* newSlab(uint32_t nbytes);
  void* mallocSmallSizeFast(size_t bytes, size_t index);
  void* mallocSmallSizeSlow(size_t bytes, size_t index);
  void  updateBigStats();

  static size_t bsrq(size_t x);

  static void threadStatsInit();
  static void threadStats(uint64_t*&, uint64_t*&);
  void refreshStats();
  void refreshStatsImpl(MemoryUsageStats& stats);
  void refreshStatsHelperExceeded();
  void resetAllStats();
  void traceStats(const char* when);

  static void initHole(void* ptr, uint32_t size);
  void initHole();

  void requestEagerGC();
  void resetEagerGC();
  void requestGC();

  /////////////////////////////////////////////////////////////////////////////

private:
  TRACE_SET_MOD(mm);

  void* m_front{nullptr};
  void* m_limit{nullptr};
  FreelistArray m_freelists;
  StringDataNode m_strings; // in-place node is head of circular list
  std::vector<APCLocalArray*> m_apc_arrays;
  int64_t m_nextGc; // request gc when heap usage reaches this size
  int64_t m_usageLimit; // OOM when m_stats.usage() > m_usageLimit
  MemoryUsageStats m_stats;
  BigHeap m_heap;
  std::vector<NativeNode*> m_natives;
  SweepableList m_sweepables;

  mutable std::vector<req::root_handle*> m_root_handles;

  bool m_exiting{false};
  bool m_statsIntervalActive;
  bool m_couldOOM{true};
  bool m_bypassSlabAlloc;
  bool m_gc_enabled{RuntimeOption::EvalEnableGC};
  bool m_enableStatsSync{false};

  ReqProfContext m_profctx;
  static std::atomic<ReqProfContext*> s_trigger;

  // Peak memory threshold callback (installed via setMemThresholdCallback)
  size_t m_memThresholdCallbackPeakUsage{SIZE_MAX};

  static void* TlsInitSetup;

  // pointers to jemalloc-maintained allocation counters
  uint64_t* m_allocated;
  uint64_t* m_deallocated;

  // previous values of *m_[de]allocated from last resetStats()
  uint64_t m_resetAllocated;
  uint64_t m_resetDeallocated;

  // true if mallctlnametomib() setup succeeded, which requires jemalloc
  static bool s_statsEnabled;

  int64_t m_req_start_micros;

  TYPE_SCAN_IGNORE_ALL; // heap-scan handles MM fields itself.
};

//////////////////////////////////////////////////////////////////////

}

#include "hphp/runtime/base/memory-manager-inl.h"

#endif
