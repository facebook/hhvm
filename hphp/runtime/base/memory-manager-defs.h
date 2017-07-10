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

#include <algorithm>

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

struct HdrBlock {
  HeapObject* ptr;
  size_t size;
};

/*
 * Struct Slab encapsulates the header attached to each large block of memory
 * used for allocating smaller blocks. The header contains a crossing map,
 * used for quickly locating the start of an object, given an interior pointer
 * (a pointer to somewhere in the body). The crossing map logically divides
 * the kSlabSize bytes into equal sized LineSize-byte "lines". Optimal LineSize
 * appears to be near average object size.
 */
template<size_t LineSize> struct SlabHeader: FreeNode {
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
} __attribute__((__aligned__(kSmallSizeAlign)));

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

// Return the size (in bytes) without rounding up to MM size class.
inline size_t heapSize(const HeapObject* h) {
  // Ordering depends on ext_wait-handle.h.
  static const uint32_t waithandle_sizes[] = {
    sizeof(c_StaticWaitHandle),
    0, /* AsyncFunction */
    sizeof(c_AsyncGeneratorWaitHandle),
    0, /* AwaitAll */
    sizeof(c_ConditionWaitHandle),
    sizeof(c_RescheduleWaitHandle),
    sizeof(c_SleepWaitHandle),
    sizeof(c_ExternalThreadEventWaitHandle),
  };

  // Ordering depends on header-kind.h.
  static constexpr uint32_t kind_sizes[] = {
    0, /* Packed */
    0, /* Mixed */
    sizeof(ArrayData), /* Empty */
    0, /* APCLocalArray */
    sizeof(GlobalsArray),
    sizeof(ProxyArray),
    0, /* Dict */
    0, /* VecArray */
    0, /* KeySet */
    0, /* String */
    0, /* Resource */
    sizeof(RefData),
    0, /* Object */
    0, /* WaitHandle */
    sizeof(c_AsyncFunctionWaitHandle),
    0, /* AwaitAllWH */
    0, /* Closure */
    sizeof(c_Vector),
    sizeof(c_Map),
    sizeof(c_Set),
    sizeof(c_Pair),
    sizeof(c_ImmVector),
    sizeof(c_ImmMap),
    sizeof(c_ImmSet),
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
#define CHECKSIZE(knd, size) \
  static_assert(kind_sizes[(int)HeaderKind::knd] == size, #knd);
  CHECKSIZE(Empty, sizeof(ArrayData))
  CHECKSIZE(Globals, sizeof(GlobalsArray))
  CHECKSIZE(AsyncFuncWH, sizeof(c_AsyncFunctionWaitHandle))
  CHECKSIZE(Vector, sizeof(c_Vector))
  CHECKSIZE(Map, sizeof(c_Map))
  CHECKSIZE(Set, sizeof(c_Set))
  CHECKSIZE(Pair, sizeof(c_Pair))
  CHECKSIZE(ImmVector, sizeof(c_ImmVector))
  CHECKSIZE(ImmMap, sizeof(c_ImmMap))
  CHECKSIZE(ImmSet, sizeof(c_ImmSet))
  CHECKSIZE(Ref, sizeof(RefData))
  CHECKSIZE(Apc, 0)
  CHECKSIZE(AsyncFuncFrame, 0)
  CHECKSIZE(AwaitAllWH, 0)
  CHECKSIZE(Closure, 0)
  CHECKSIZE(WaitHandle, 0)
  CHECKSIZE(Resource, 0)
  CHECKSIZE(NativeData, 0)
  CHECKSIZE(ClosureHdr, 0)
  CHECKSIZE(SmallMalloc, 0)
  CHECKSIZE(BigMalloc, 0)
  CHECKSIZE(BigObj, 0)
  CHECKSIZE(Object, 0)
  CHECKSIZE(Hole, 0)
  CHECKSIZE(Free, 0)
  CHECKSIZE(Slab, 0)
#undef CHECKSIZE

  auto kind = h->kind();
  if (auto size = kind_sizes[(int)kind]) return size;

  switch (kind) {
    case HeaderKind::Packed:
    case HeaderKind::VecArray:
      return PackedArray::heapSize(static_cast<const ArrayData*>(h));
    case HeaderKind::Mixed:
    case HeaderKind::Dict:
      return static_cast<const MixedArray*>(h)->heapSize();
    case HeaderKind::Keyset:
      return static_cast<const SetArray*>(h)->heapSize();
    case HeaderKind::Apc:
      return static_cast<const APCLocalArray*>(h)->heapSize();
    case HeaderKind::String:
      return static_cast<const StringData*>(h)->heapSize();
    case HeaderKind::Closure:
    case HeaderKind::Object:
      // [ObjectData][props]
      return static_cast<const ObjectData*>(h)->heapSize();
    case HeaderKind::ClosureHdr:
      // [ClosureHdr][ObjectData][use vars]
      return static_cast<const ClosureHdr*>(h)->size();
    case HeaderKind::WaitHandle:
    {
      // [ObjectData][subclass]
      auto obj = static_cast<const ObjectData*>(h);
      auto whKind = wait_handle<c_WaitHandle>(obj)->getKind();
      if (auto whSize = waithandle_sizes[(int)whKind]) return whSize;
      return asio_object_size(obj);
    }
    case HeaderKind::AwaitAllWH:
      // [ObjectData][children...]
      return static_cast<const c_AwaitAllWaitHandle*>(h)->heapSize();
    case HeaderKind::Resource:
      // [ResourceHdr][ResourceData subclass]
      return static_cast<const ResourceHdr*>(h)->heapSize();
    case HeaderKind::SmallMalloc: // [MallocNode][bytes...]
    case HeaderKind::BigMalloc:   // [MallocNode][bytes...]
    case HeaderKind::BigObj:      // [MallocNode][Header...]
      return static_cast<const MallocNode*>(h)->nbytes;
    case HeaderKind::AsyncFuncFrame:
      // [NativeNode][locals][Resumable][c_AsyncFunctionWaitHandle]
      return static_cast<const NativeNode*>(h)->obj_offset +
             sizeof(c_AsyncFunctionWaitHandle);
    case HeaderKind::NativeData: {
      // [NativeNode][NativeData][ObjectData][props] is one allocation.
      // Generators -
      // [NativeNode][NativeData<locals><Resumable><GeneratorData>][ObjectData]
      auto native = static_cast<const NativeNode*>(h);
      return native->obj_offset + Native::obj(native)->heapSize();
    }
    case HeaderKind::Free:
    case HeaderKind::Hole:
    case HeaderKind::Slab:
      return static_cast<const FreeNode*>(h)->size();
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
      assertx(false &&
              "Constant header sizes should be handled by the lookup table.");
  }
  return 0;
}

inline size_t allocSize(const HeapObject* h) {
  auto const sz = heapSize(h);
  switch (h->kind()) {
    case HeaderKind::Slab:
    case HeaderKind::Hole:
    case HeaderKind::Free:
    case HeaderKind::BigObj:
    case HeaderKind::BigMalloc:
      // these don't need rounding up to size classes.
      assertx(sz % 16 == 0);
      return sz;
    default:
      return MemoryManager::smallSizeClass(sz);
  }
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
  std::vector<ObjectData*> ptrs;
  forEachHeapObject([&](HeapObject* h, size_t) {
    switch (h->kind()) {
      case HeaderKind::Object:
      case HeaderKind::WaitHandle:
      case HeaderKind::AsyncFuncWH:
      case HeaderKind::AwaitAllWH:
      case HeaderKind::Closure:
      case HeaderKind::Vector:
      case HeaderKind::Map:
      case HeaderKind::Set:
      case HeaderKind::Pair:
      case HeaderKind::ImmVector:
      case HeaderKind::ImmMap:
      case HeaderKind::ImmSet:
        ptrs.push_back(static_cast<ObjectData*>(h));
        break;
      case HeaderKind::AsyncFuncFrame:
        ptrs.push_back(asyncFuncWH(h));
        break;
      case HeaderKind::NativeData:
        ptrs.push_back(Native::obj(static_cast<NativeNode*>(h)));
        break;
      case HeaderKind::ClosureHdr:
        ptrs.push_back(closureObj(h));
        break;
      case HeaderKind::Packed:
      case HeaderKind::Mixed:
      case HeaderKind::Dict:
      case HeaderKind::Empty:
      case HeaderKind::VecArray:
      case HeaderKind::Keyset:
      case HeaderKind::Apc:
      case HeaderKind::Globals:
      case HeaderKind::Proxy:
      case HeaderKind::String:
      case HeaderKind::Resource:
      case HeaderKind::Ref:
      case HeaderKind::SmallMalloc:
      case HeaderKind::BigMalloc:
      case HeaderKind::Free:
        break;
      case HeaderKind::BigObj:
      case HeaderKind::Hole:
      case HeaderKind::Slab:
        assert(false && "forEachHeapObject skips these kinds");
        break;
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
