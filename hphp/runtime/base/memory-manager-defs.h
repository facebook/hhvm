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

#ifndef incl_HPHP_MEMORY_MANAGER_DEFS_H
#define incl_HPHP_MEMORY_MANAGER_DEFS_H

#include "hphp/runtime/base/apc-local-array.h"
#include "hphp/runtime/base/mixed-array-defs.h"
#include "hphp/runtime/base/packed-array-defs.h"
#include "hphp/runtime/base/proxy-array.h"
#include "hphp/runtime/base/set-array.h"
#include "hphp/runtime/base/apc-local-array-defs.h"
#include "hphp/runtime/vm/globals-array.h"
#include "hphp/runtime/vm/native-data.h"
#include "hphp/runtime/vm/resumable.h"

#include "hphp/runtime/ext/asio/ext_asio.h"
#include "hphp/runtime/ext/asio/ext_await-all-wait-handle.h"
#include "hphp/runtime/ext/asio/ext_condition-wait-handle.h"
#include "hphp/runtime/ext/asio/ext_external-thread-event-wait-handle.h"
#include "hphp/runtime/ext/asio/ext_reschedule-wait-handle.h"
#include "hphp/runtime/ext/asio/ext_sleep-wait-handle.h"
#include "hphp/runtime/ext/asio/ext_static-wait-handle.h"
#include "hphp/runtime/ext/asio/ext_async-generator-wait-handle.h"
#include "hphp/runtime/ext/asio/ext_wait-handle.h"
#include "hphp/runtime/ext/asio/ext_async-function-wait-handle.h"
#include "hphp/runtime/ext/collections/ext_collections-map.h"
#include "hphp/runtime/ext/collections/ext_collections-pair.h"
#include "hphp/runtime/ext/collections/ext_collections-set.h"
#include "hphp/runtime/ext/collections/ext_collections-vector.h"
#include "hphp/runtime/ext/collections/hash-collection.h"
#include "hphp/runtime/ext/std/ext_std_closure.h"
#include "hphp/util/bitops.h"

#include <algorithm>

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

/*
 * Struct Slab encapsulates the header attached to each large block of memory
 * used for allocating smaller blocks. The header contains a crossing map,
 * used for quickly locating the start of an object, given an interior pointer
 * (a pointer to somewhere in the body). The crossing map logically divides
 * the kSlabSize bytes into equal sized LineSize-byte "lines". Optimal LineSize
 * appears to be near average object size.
 */
template<size_t LineSize>
struct alignas(kSmallSizeAlign) SlabHeader: FreeNode {
  static_assert((LineSize & (LineSize-1)) == 0, "LineSize must be power of 2");

  char* init() {
    initHeader(HeaderKind::Slab, sizeof(*this));
    return start();
    static_assert(sizeof(*this) % kSmallSizeAlign == 0, "");
  }

  /*
   * call fn(h,size) on each object in the slab, return the first HdrBlock
   * when fn returns true
   */
  template<class Fn> HdrBlock find_if(HeapObject* h, Fn fn) const;

  /*
   * initialize the whole crossing map by iterating the slab in address order,
   * calling Fn after each non-free object is procesed.
   */
  template<class Fn> void initCrossingMap(Fn fn) {
    // initialization algorithm:
    // for each object h in address order:
    //   1. let i = line index of h, j = line index of (h+size)
    //   2. xmap[i] = offset of h within line i, in units of Q bytes
    //   3. xmap[i+1..j] = -1 * no. of lines from line i+1..j back to h
    //   4. call Fn(h,size) if h is not free space.
    find_if((HeapObject*)this, [&](HeapObject* h, size_t size) {
      auto s = pos(h);
      // store positive offset to start of object h
      xmap_[s.line] = s.off;
      for (auto i = s.line + 1, e = pos((char*)h + size).line; i < e; ++i) {
        // for objects that extend into subsequent lines, store number of lines
        // back to object start, saturated to -128
        xmap_[i] = std::max(ssize_t(s.line) - ssize_t(i), ssize_t(-128));
      }
      if (!isFreeKind(h->kind())) {
        fn(h, size);
      }
      return false;
    });
  }

  /*
   * If ptr points within a non-free HeapObject, return the HeapObject* start
   * and size. Otherwise, return {nullptr,0} indicating that ptr points within
   * the slab header or free space.
   */
  HdrBlock find(const void* ptr) const {
    // search algorithm:
    // 1. let i = line containing ptr
    // 2. if the last object in line i starts after ptr, back up 1 line
    //    (and possibly fail)
    // 3. while xmap[i] < 0, search backwards for the line containing a
    //    nonnegagive offset, indicating an object start.
    // 4. iterate over each object, forwards, until we find the object
    //    enclosing ptr.
    auto p = pos(ptr);
    auto i = p.line;
    auto d = xmap_[i];
    if (d > p.off) {
      // last object in line i starts after ptr; back up to previous line.
      // if sizeof(*this) >= LineSize, then line 0 is fully covered by this
      // slab header, and xmap_[i] == 0, so we can't get here.
      if (sizeof(*this) < LineSize && i == 0) {
        return {nullptr, 0}; // ptr is in the slab header
      }
      d = xmap_[--i];
    }
    // if d >= 0, then object at offset d contains ptr.
    // if d < 0, search backwards for object start
    while (d < 0) {
      assert(i+d >= 0 && i+d < NumLines);
      d = xmap_[i += d]; // back up, since d < 0
    }
    // compute object address, given line index i and offset d.
    auto h0 = reinterpret_cast<HeapObject*>((char*)this + i * LineSize + d * Q);
    // search forwards looking for the enclosing object.
    auto r = find_if(h0, [&](HeapObject* h, size_t size) {
      return ptr < (char*)h + size; // found it!
    });
    assert(r.ptr); // forward search can't fail without heap corruption.
    return !isFreeKind(r.ptr->kind()) ? r : HdrBlock{nullptr, 0};
  }

  static SlabHeader<LineSize>* fromHeader(HeapObject* h) {
    assert(h->kind() == HeaderKind::Slab);
    return reinterpret_cast<SlabHeader<LineSize>*>(h);
  }

  char* start() { return (char*)(this + 1); }
  char* end() { return (char*)this + kSlabSize; }
  const char* start() const { return (const char*)(this + 1); }
  const char* end() const { return (const char*)this + kSlabSize; }

private:
  // LineSize=128 would be the same overhead as 1 bit per 16 bytes
  static const size_t Q = kSmallSizeAlign; // 16
  static const size_t NumLines = kSlabSize / LineSize;
  static_assert(kSlabSize % LineSize == 0, "NumLines cannot be fraction");

  struct Pos { size_t line, off; };
  Pos pos(const void* p) const {
    assert(p >= (char*)this && p <= end());
    auto off = (char*)p - (char*)this;
    return {off / LineSize, (off % LineSize) / Q};
    static_assert(LineSize/Q <= 128, "positive offset overflows int8_t");
  }

  // Crossing map state: each byte in the crossing map corresponds to
  // LineSize bytes in the slab, both indexed from "this". A non-negative value
  // d in slot i provides the offset of the last object beginning in the
  // corresponding line i. A negative value d indicates no object starts in
  // this line, but the object that covers this line begins d lines earlier,
  // indexed from the start of this line. If d is -128, the start of the object
  // is even further back.
  //
  // d=xmap[i]   meaning
  // ---------   --------
  // 0..127      d*Q is offset of last object starting on this line
  // -127..-1    line i is inside last object starting on line i+d
  // -128        saturation. object starts on or before i+d
  int8_t xmap_[NumLines];
};

// LineSize of 256 was chosen experimentally as tradeoff between
// SlabHeader overhead and lookup costs.
using Slab = SlabHeader<256>;

inline const Resumable* resumable(const HeapObject* h) {
  assert(h->kind() == HeaderKind::AsyncFuncFrame);
  auto native = static_cast<const NativeNode*>(h);
  return reinterpret_cast<const Resumable*>(
    (const char*)native + native->obj_offset - sizeof(Resumable)
  );
}

inline Resumable* resumable(HeapObject* h) {
  assert(h->kind() == HeaderKind::AsyncFuncFrame);
  auto native = static_cast<NativeNode*>(h);
  return reinterpret_cast<Resumable*>(
    (char*)native + native->obj_offset - sizeof(Resumable)
  );
}

inline const c_WaitHandle* asyncFuncWH(const HeapObject* h) {
  assert(resumable(h)->actRec()->func()->isAsyncFunction());
  auto native = static_cast<const NativeNode*>(h);
  auto obj = reinterpret_cast<const c_WaitHandle*>(
    (const char*)native + native->obj_offset
  );
  assert(obj->headerKind() == HeaderKind::AsyncFuncWH);
  return obj;
}

inline c_WaitHandle* asyncFuncWH(HeapObject* h) {
  assert(resumable(h)->actRec()->func()->isAsyncFunction());
  auto native = static_cast<NativeNode*>(h);
  auto obj = reinterpret_cast<c_WaitHandle*>(
    (char*)native + native->obj_offset
  );
  assert(obj->headerKind() == HeaderKind::AsyncFuncWH);
  return obj;
}

inline const ObjectData* closureObj(const HeapObject* h) {
  assert(h->kind() == HeaderKind::ClosureHdr);
  auto closure_hdr = static_cast<const ClosureHdr*>(h);
  auto obj = reinterpret_cast<const ObjectData*>(closure_hdr + 1);
  assert(obj->headerKind() == HeaderKind::Closure);
  return obj;
}

inline ObjectData* closureObj(HeapObject* h) {
  assert(h->kind() == HeaderKind::ClosureHdr);
  auto closure_hdr = static_cast<ClosureHdr*>(h);
  auto obj = reinterpret_cast<ObjectData*>(closure_hdr + 1);
  assert(obj->headerKind() == HeaderKind::Closure);
  return obj;
}

// if this header is one of the types that contains an ObjectData,
// return the (possibly inner ptr) ObjectData*
inline const ObjectData* innerObj(const HeapObject* h) {
  return isObjectKind(h->kind()) ? static_cast<const ObjectData*>(h) :
         h->kind() == HeaderKind::AsyncFuncFrame ? asyncFuncWH(h) :
         h->kind() == HeaderKind::NativeData ?
           Native::obj(static_cast<const NativeNode*>(h)) :
         h->kind() == HeaderKind::ClosureHdr ? closureObj(h) :
         nullptr;
}

// constexpr version of MemoryManager::smallSizeClass.
// requires sizeof(T) <= kMaxSmallSizeLookup
template<class T> constexpr size_t sizeClass() {
  return kSizeIndex2Size[
    kSmallSize2Index[(sizeof(T)-1) >> kLgSmallSizeQuantum]
  ];
}

/*
 * Return the size of h in bytes, rounded up to size class if necessary.
 */
inline size_t allocSize(const HeapObject* h) {
  // Ordering depends on ext_wait-handle.h.
  static const uint16_t waithandle_sizes[] = {
    sizeClass<c_StaticWaitHandle>(),
    0, /* AsyncFunction */
    sizeClass<c_AsyncGeneratorWaitHandle>(),
    0, /* AwaitAll */
    sizeClass<c_ConditionWaitHandle>(),
    sizeClass<c_RescheduleWaitHandle>(),
    sizeClass<c_SleepWaitHandle>(),
    sizeClass<c_ExternalThreadEventWaitHandle>(),
  };

  // Ordering depends on header-kind.h.
  static constexpr uint16_t kind_sizes[] = {
    0, /* Packed */
    0, /* Mixed */
    sizeClass<ArrayData>(), /* Empty */
    0, /* APCLocalArray */
    sizeClass<GlobalsArray>(),
    sizeClass<ProxyArray>(),
    0, /* Dict */
    0, /* VecArray */
    0, /* KeySet */
    0, /* String */
    0, /* Resource */
    sizeClass<RefData>(),
    0, /* Object */
    0, /* WaitHandle */
    sizeClass<c_AsyncFunctionWaitHandle>(),
    0, /* AwaitAllWH */
    0, /* Closure */
    sizeClass<c_Vector>(),
    sizeClass<c_Map>(),
    sizeClass<c_Set>(),
    sizeClass<c_Pair>(),
    sizeClass<c_ImmVector>(),
    sizeClass<c_ImmMap>(),
    sizeClass<c_ImmSet>(),
    0, /* AsyncFuncFrame */
    0, /* NativeData */
    0, /* ClosureHdr */
    0, /* SmallMalloc */
    0, /* BigMalloc */
    0, /* BigObj */
    0, /* Free */
    0, /* Hole */
    0, /* Slab */
  };
#define CHECKSIZE(knd, type) \
  static_assert(kind_sizes[(int)HeaderKind::knd] == sizeClass<type>(), #knd);
  CHECKSIZE(Empty, ArrayData)
  CHECKSIZE(Globals, GlobalsArray)
  CHECKSIZE(Proxy, ProxyArray)
  CHECKSIZE(Ref, RefData)
  CHECKSIZE(AsyncFuncWH, c_AsyncFunctionWaitHandle)
  CHECKSIZE(Vector, c_Vector)
  CHECKSIZE(Map, c_Map)
  CHECKSIZE(Set, c_Set)
  CHECKSIZE(Pair, c_Pair)
  CHECKSIZE(ImmVector, c_ImmVector)
  CHECKSIZE(ImmMap, c_ImmMap)
  CHECKSIZE(ImmSet, c_ImmSet)
#undef CHECKSIZE
#define CHECKSIZE(knd)\
  static_assert(kind_sizes[(int)HeaderKind::knd] == 0, #knd);
  CHECKSIZE(Packed)
  CHECKSIZE(Mixed)
  CHECKSIZE(Apc)
  CHECKSIZE(Dict)
  CHECKSIZE(VecArray)
  CHECKSIZE(Keyset)
  CHECKSIZE(String)
  CHECKSIZE(Resource)
  CHECKSIZE(Object)
  CHECKSIZE(WaitHandle)
  CHECKSIZE(AwaitAllWH)
  CHECKSIZE(Closure)
  CHECKSIZE(AsyncFuncFrame)
  CHECKSIZE(NativeData)
  CHECKSIZE(ClosureHdr)
  CHECKSIZE(SmallMalloc)
  CHECKSIZE(BigMalloc)
  CHECKSIZE(BigObj)
  CHECKSIZE(Free)
  CHECKSIZE(Hole)
  CHECKSIZE(Slab)
#undef CHECKSIZE

  auto kind = h->kind();
  if (auto size = kind_sizes[(int)kind]) {
    assert(size == MemoryManager::smallSizeClass(size));
    return size;
  }

  // In the cases below, kinds that don't need size-class rounding will
  // return their size immediately; the rest break to the bottom, then
  // return a rounded-up size.
  size_t size = 0;
  switch (kind) {
    case HeaderKind::Packed:
    case HeaderKind::VecArray:
      // size = kSizeIndex2Size[h->aux16]
      size = PackedArray::heapSize(static_cast<const ArrayData*>(h));
      break;
    case HeaderKind::Mixed:
    case HeaderKind::Dict:
      // size = fn of h->m_scale
      size = static_cast<const MixedArray*>(h)->heapSize();
      break;
    case HeaderKind::Keyset:
      // size = fn of h->m_scale
      size = static_cast<const SetArray*>(h)->heapSize();
      break;
    case HeaderKind::Apc:
      // size = h->m_size * 16 + sizeof(APCLocalArray)
      size = static_cast<const APCLocalArray*>(h)->heapSize();
      break;
    case HeaderKind::String:
      // size = isFlat ? isRefCounted ? table[aux16] : m_size+C : proxy_size;
      size = static_cast<const StringData*>(h)->heapSize();
      break;
    case HeaderKind::Closure:
    case HeaderKind::Object:
      // size = h->m_cls->m_declProperties.m_extra * sz(TV) + sz(OD)
      // [ObjectData][props]
      size = static_cast<const ObjectData*>(h)->heapSize();
      break;
    case HeaderKind::ClosureHdr:
      // size = h->aux32
      // [ClosureHdr][ObjectData][use vars]
      size = static_cast<const ClosureHdr*>(h)->size();
      break;
    case HeaderKind::WaitHandle: {
      // size = table[h->whkind & mask]
      // [ObjectData][subclass]
      auto obj = static_cast<const ObjectData*>(h);
      auto whKind = wait_handle<c_WaitHandle>(obj)->getKind();
      size = waithandle_sizes[(int)whKind];
      assert(size != 0); // AsyncFuncFrame or AwaitAllWH
      assert(size == MemoryManager::smallSizeClass(size));
      return size;
    }
    case HeaderKind::AwaitAllWH:
      // size = h->m_cap * sz(Node) + sz(c_AAWH)
      // [ObjectData][children...]
      size = static_cast<const c_AwaitAllWaitHandle*>(h)->heapSize();
      break;
    case HeaderKind::Resource:
      // size = h->aux16
      // [ResourceHdr][ResourceData subclass]
      size = static_cast<const ResourceHdr*>(h)->heapSize();
      break;
    case HeaderKind::SmallMalloc: // [MallocNode][bytes...]
      // size = h->nbytes // 64-bit
      size = static_cast<const MallocNode*>(h)->nbytes;
      break;
    case HeaderKind::AsyncFuncFrame:
      // size = h->obj_offset + C // 32-bit
      // [NativeNode][locals][Resumable][c_AsyncFunctionWaitHandle]
      size = static_cast<const NativeNode*>(h)->obj_offset +
             sizeof(c_AsyncFunctionWaitHandle);
      break;
    case HeaderKind::NativeData: {
      // h->obj_offset + (h+h->obj_offset)->m_cls->m_extra * sz(TV) + sz(OD)
      // [NativeNode][NativeData][ObjectData][props] is one allocation.
      // Generators -
      // [NativeNode][NativeData<locals><Resumable><GeneratorData>][ObjectData]
      auto native = static_cast<const NativeNode*>(h);
      size = native->obj_offset + Native::obj(native)->heapSize();
      break;
    }
    // the following sizes are intentionally not rounded up to size class.
    case HeaderKind::BigObj:    // [MallocNode][HeapObject...]
    case HeaderKind::BigMalloc: // [MallocNode][raw bytes...]
      size = static_cast<const MallocNode*>(h)->nbytes;
      assert(size != 0);
      return size;
    case HeaderKind::Free:
    case HeaderKind::Hole:
    case HeaderKind::Slab:
      size = static_cast<const FreeNode*>(h)->size();
      assert(size != 0);
      return size;
    case HeaderKind::AsyncFuncWH:
    case HeaderKind::Empty:
    case HeaderKind::Globals:
    case HeaderKind::Proxy:
    case HeaderKind::Vector:
    case HeaderKind::Map:
    case HeaderKind::Set:
    case HeaderKind::Pair:
    case HeaderKind::ImmVector:
    case HeaderKind::ImmMap:
    case HeaderKind::ImmSet:
    case HeaderKind::Ref:
      // these have a constant size in kind_sizes[]
      not_reached();
  }
  return MemoryManager::smallSizeClass(size);
}

///////////////////////////////////////////////////////////////////////////////

// call fn(h,size) on each object in the slab, return the first HdrBlock
// when fn returns true
template<size_t LineSize> template<class Fn>
HdrBlock SlabHeader<LineSize>::find_if(HeapObject* h, Fn fn) const {
  auto end = (HeapObject*)this->end();
  do {
    auto size = allocSize(h);
    if (fn(h, size)) return {h, size};
    h = reinterpret_cast<HeapObject*>((char*)h + size);
  } while (h < end);
  assert(h == end); // otherwise, last object was truncated
  return {nullptr, 0};
}

template<class OnBig, class OnSlab>
void BigHeap::iterate(OnBig onBig, OnSlab onSlab) {
  // slabs and bigs are sorted; walk through both in address order
  const auto SENTINEL = (HeapObject*) ~0LL;
  auto slab = std::begin(m_slabs);
  auto big = std::begin(m_bigs);
  auto slabend = std::end(m_slabs);
  auto bigend = std::end(m_bigs);
  while (slab != slabend || big != bigend) {
    HeapObject* slab_hdr = slab != slabend ? (HeapObject*)slab->ptr : SENTINEL;
    HeapObject* big_hdr = big != bigend ? *big : SENTINEL;
    assert(slab_hdr < SENTINEL || big_hdr < SENTINEL);
    if (slab_hdr < big_hdr) {
      onSlab(slab_hdr, slab->size);
      ++slab;
    } else {
      assert(big_hdr < slab_hdr);
      onBig(big_hdr, allocSize(big_hdr));
      ++big;
    }
  }
}

template<class Fn> void BigHeap::iterate(Fn fn) {
  // slabs and bigs are sorted; walk through both in address order
  const auto SENTINEL = (HeapObject*) ~0LL;
  auto slab = std::begin(m_slabs);
  auto big = std::begin(m_bigs);
  auto slabend = std::end(m_slabs);
  auto bigend = std::end(m_bigs);
  while (slab != slabend || big != bigend) {
    HeapObject* slab_hdr = slab != slabend ? (HeapObject*)slab->ptr : SENTINEL;
    HeapObject* big_hdr = big != bigend ? *big : SENTINEL;
    assert(slab_hdr < SENTINEL || big_hdr < SENTINEL);
    HeapObject *h, *end;
    if (slab_hdr < big_hdr) {
      h = slab_hdr;
      end = (HeapObject*)((char*)h + slab->size);
      ++slab;
      assert(end <= big_hdr); // otherwise slab overlaps next big
    } else {
      h = big_hdr;
      end = nullptr; // ensure we don't loop below
      ++big;
    }
    do {
      auto size = allocSize(h);
      fn(h, size);
      h = (HeapObject*)((char*)h + size);
    } while (h < end);
    assert(!end || h == end); // otherwise, last object was truncated
  }
}

// The parameters onBigFn and onSlabFn are both lambda functions.
// When calling this function, iterate the whole heap from m_base to m_front,
// call onBigFn() on big blocks, call onSlabFn() on slabs.
// This function is used in GC Marker init and Marker sweep
template<class OnBigFn, class OnSlabFn>
void ContiguousBigHeap::iterate(OnBigFn onBigFn, OnSlabFn onSlabFn) {
  // [m_base,m_front) iterate the whole heap
  find_if(true, [&](HeapObject* h, size_t size) {
    switch (h->kind()) {
      case HeaderKind::BigObj:
      case HeaderKind::BigMalloc:
        onBigFn(h, size);
        break;
      case HeaderKind::Slab:
        onSlabFn(h, size);
        break;
      default:
        break;
    }
    return false;
  });
}

// The parameter Fn is a lambda function.
// When calling this function, iterate the whole heap, call fn() on every
// HeapObject(skip slab Header).
template<class Fn> void ContiguousBigHeap::iterate(Fn fn) {
  find_if(false, [&](HeapObject* h, size_t size) {
    fn(h, size);
    return false;
  });
}

// For each in-use chunk (using the bitmask), within [m_base, m_front),
// visit all the heap objects in the chunk by parsing headers.
// If wholeSlab is true, we would not go into the inside of slab,
// just take slab as a whole.
//
//bitmap:     0        0         1         0          1         1
//       [--------][--------][--------][--------][--------][--------]
//       ^usedStart          ^usedEnd                               ^endIndex
template<class Fn>
HeapObject* ContiguousBigHeap::find_if(bool wholeSlab, Fn fn) {
  assert(check_invariants());
  size_t endIndex   = chunk_index(m_front);
  size_t startIndex = chunk_index(m_base);
  size_t usedStart  = findFirst0(m_freebits, startIndex, endIndex);
  size_t usedEnd    = findFirst1(m_freebits, usedStart, endIndex);
  size_t size;
  while (usedStart < endIndex) {
    // in a contiguous allocated space, loop through all the header
    auto hdr    = m_base + usedStart * ChunkSize;
    auto endPtr = m_base + usedEnd * ChunkSize;
    while (hdr < endPtr) {
      if (((HeapObject*)hdr)->kind() == HeaderKind::Slab) {
        if (wholeSlab) {
          size = kSlabSize;
        } else{
          hdr += allocSize((HeapObject*)hdr);     // skip Slab header
          continue;
        }
      } else {
        size = allocSize((HeapObject*)hdr);
      }
      assert(hdr + size <= endPtr);
      if (fn((HeapObject*)hdr, size)) return (HeapObject*)hdr;
      hdr += size;
    }
    // go to next contiguous allocated space
    usedStart = findFirst0(m_freebits, usedEnd, endIndex);
    usedEnd   = findFirst1(m_freebits, usedStart, endIndex);
  }
  return nullptr;
}

template<class Fn> void MemoryManager::iterate(Fn fn) {
  m_heap.iterate([&](HeapObject* h, size_t allocSize) {
    if (h->kind() == HeaderKind::BigObj) {
      // skip MallocNode
      h = static_cast<MallocNode*>(h) + 1;
      allocSize -= sizeof(MallocNode);
    } else if (h->kind() >= HeaderKind::Hole) {
      assert(unsigned(h->kind()) < NumHeaderKinds);
      // no valid pointer can point here.
      return; // continue iterating
    }
    fn(h, allocSize);
  });
}

template<class Fn> void MemoryManager::forEachHeapObject(Fn fn) {
  initFree();
  iterate(fn);
}

template<class Fn> void MemoryManager::forEachObject(Fn fn) {
  if (debug) checkHeap("MM::forEachObject");
  std::vector<const ObjectData*> ptrs;
  forEachHeapObject([&](HeapObject* h, size_t) {
    if (auto obj = innerObj(h)) {
      ptrs.push_back(obj);
    }
  });
  for (auto ptr : ptrs) {
    fn(ptr);
  }
}

template<class Fn> void MemoryManager::sweepApcArrays(Fn fn) {
  for (size_t i = 0; i < m_apc_arrays.size();) {
    auto a = m_apc_arrays[i];
    if (fn(a)) {
      a->sweep();
      removeApcArray(a);
    } else {
      ++i;
    }
  }
}

template<class Fn> void MemoryManager::sweepApcStrings(Fn fn) {
  auto& head = getStringList();
  for (StringDataNode *next, *n = head.next; n != &head; n = next) {
    next = n->next;
    assert(next && uintptr_t(next) != kSmallFreeWord);
    assert(next && uintptr_t(next) != kMallocFreeWord);
    auto const s = StringData::node2str(n);
    if (fn(s)) {
      s->unProxy();
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
}
#endif
