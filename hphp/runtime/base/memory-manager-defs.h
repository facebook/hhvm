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

#pragma once

#include "hphp/runtime/base/bespoke-array.h"
#include "hphp/runtime/base/mixed-array-defs.h"
#include "hphp/runtime/base/packed-array-defs.h"
#include "hphp/runtime/base/set-array.h"

#include "hphp/runtime/vm/class-meth-data.h"
#include "hphp/runtime/vm/native-data.h"
#include "hphp/runtime/vm/resumable.h"
#include "hphp/runtime/vm/rfunc-data.h"
#include "hphp/runtime/vm/rclass-meth-data.h"

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
#include "hphp/util/bitset-utils.h"

#include <algorithm>

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

/*
 * Struct Slab encapsulates the header attached to each large block of memory
 * used for allocating smaller blocks. The header contains a start-bit map,
 * used for quickly locating the start of an object, given an interior pointer
 * (a pointer to somewhere in the body). The start-bit map marks the location
 * of the start of each object. One bit represents kSmallSizeAlign bytes.
 */
struct Slab : HeapObject {

  char* init() {
    initHeader_32(HeaderKind::Slab, 0);
    clearStarts();
    return start();
  }

  /*
   * call fn(h) on each object in the slab, without calling allocSize(),
   * by scanning through the start-bits alone.
   */
  template<class Fn> void iter_starts(Fn fn) const;

  void setStart(const void* p) {
    bitvec_set(starts_, start_index(p));
  }

  /*
   * set start bits for as many objects of size class index, that fit
   * between [start,end).
   */
  void setStarts(const void* start, const void* end, size_t nbytes,
                 size_t index);

  bool isStart(const void* p) const {
    return bitvec_test(starts_, start_index(p));
  }

  HeapObject* find(const void* ptr) const;

  static Slab* fromHeader(HeapObject* h) {
    assertx(h->kind() == HeaderKind::Slab);
    return reinterpret_cast<Slab*>(h);
  }

  size_t size() const {
    return kSlabSize;
  }

  static Slab* fromPtr(const void* p) {
    static_assert(kSlabAlign == kSlabSize, "");
    auto slab = reinterpret_cast<Slab*>(uintptr_t(p) & ~(kSlabAlign - 1));
    assertx(slab->kind() == HeaderKind::Slab);
    return slab;
  }

  static char* align(const void* addr) {
    auto const a = reinterpret_cast<intptr_t>(addr);
    auto const mask = RO::EvalSlabAllocAlign - 1;
    assertx((mask & (mask + 1)) == 0);
    return reinterpret_cast<char*>(a + (-a & mask));
  }

  char* start() { return align(this + 1); }
  char* end() { return (char*)this + size(); }
  const char* start() const { return align(this + 1); }
  const char* end() const { return (const char*)this + size(); }

  struct InitMasks;

  // access whole start-bits word, for testing
  uint64_t start_bits(const void* p) const {
    return starts_[start_index(p) / kBitsPerStart];
  }

private:
  void clearStarts() {
    memset(starts_, 0, sizeof(starts_));
  }

  static size_t start_index(const void* p) {
    return ((size_t)p & (kSlabSize - 1)) / kSmallSizeAlign;
  }

private:
  static constexpr size_t kBitsPerStart = 64; // bits per starts_[] entry
  static constexpr auto kNumStarts = kSlabSize / kBitsPerStart /
                                     kSmallSizeAlign;

  // tables for bulk-initializing start bits in setStarts(). kNumMasks
  // covers the small size classes that span <= 64 start bits (up to 1K).
  static constexpr size_t kNumMasks = 20;
  static std::array<uint64_t,kNumMasks> masks_;
  static std::array<uint8_t,kNumMasks> shifts_;

  // Start-bit state:
  // 1 = a HeapObject starts at corresponding address
  // 0 = in the middle of an object or free space
  uint64_t starts_[kNumStarts];
};

static_assert(kMaxSmallSize < kSlabSize - sizeof(Slab),
              "kMaxSmallSize must fit in Slab");

inline const Resumable* resumable(const HeapObject* h) {
  assertx(h->kind() == HeaderKind::AsyncFuncFrame);
  auto native = static_cast<const NativeNode*>(h);
  return reinterpret_cast<const Resumable*>(
    (const char*)native + native->obj_offset - sizeof(Resumable)
  );
}

inline Resumable* resumable(HeapObject* h) {
  assertx(h->kind() == HeaderKind::AsyncFuncFrame);
  auto native = static_cast<NativeNode*>(h);
  return reinterpret_cast<Resumable*>(
    (char*)native + native->obj_offset - sizeof(Resumable)
  );
}

inline const c_Awaitable* asyncFuncWH(const HeapObject* h) {
  assertx(resumable(h)->actRec()->func()->isAsyncFunction());
  auto native = static_cast<const NativeNode*>(h);
  auto obj = reinterpret_cast<const c_Awaitable*>(
    (const char*)native + native->obj_offset
  );
  assertx(obj->headerKind() == HeaderKind::AsyncFuncWH);
  return obj;
}

inline c_Awaitable* asyncFuncWH(HeapObject* h) {
  assertx(resumable(h)->actRec()->func()->isAsyncFunction());
  auto native = static_cast<NativeNode*>(h);
  auto obj = reinterpret_cast<c_Awaitable*>(
    (char*)native + native->obj_offset
  );
  assertx(obj->headerKind() == HeaderKind::AsyncFuncWH);
  return obj;
}

inline const ObjectData* closureObj(const HeapObject* h) {
  assertx(h->kind() == HeaderKind::ClosureHdr);
  auto closure_hdr = static_cast<const ClosureHdr*>(h);
  auto obj = reinterpret_cast<const ObjectData*>(closure_hdr + 1);
  assertx(obj->headerKind() == HeaderKind::Closure);
  return obj;
}

inline ObjectData* closureObj(HeapObject* h) {
  assertx(h->kind() == HeaderKind::ClosureHdr);
  auto closure_hdr = static_cast<ClosureHdr*>(h);
  auto obj = reinterpret_cast<ObjectData*>(closure_hdr + 1);
  assertx(obj->headerKind() == HeaderKind::Closure);
  return obj;
}

inline ObjectData* memoObj(HeapObject* h) {
  assertx(h->kind() == HeaderKind::MemoData);
  auto hdr = static_cast<MemoNode*>(h);
  auto obj = reinterpret_cast<ObjectData*>((char*)hdr + hdr->objOff());
  assertx(obj->headerKind() == HeaderKind::Object);
  assertx(obj->getVMClass()->hasMemoSlots());
  assertx(hdr->objOff() == ObjectData::objOffFromMemoNode(obj->getVMClass()));
  return obj;
}

inline const ObjectData* memoObj(const HeapObject* h) {
  return memoObj(const_cast<HeapObject*>(h));
}

// if this header is one of the types that contains an ObjectData,
// return the (possibly inner ptr) ObjectData*
inline const ObjectData* innerObj(const HeapObject* h) {
  return isObjectKind(h->kind()) ? static_cast<const ObjectData*>(h) :
         h->kind() == HeaderKind::AsyncFuncFrame ? asyncFuncWH(h) :
         h->kind() == HeaderKind::NativeData ?
           Native::obj(static_cast<const NativeNode*>(h)) :
         h->kind() == HeaderKind::ClosureHdr ? closureObj(h) :
         h->kind() == HeaderKind::MemoData ? memoObj(h) :
         nullptr;
}

// constexpr version of MemoryManager::sizeClass.
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
  static constexpr uint16_t waithandle_sizes[] = {
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
    0, /* Mixed */
    0, /* BespokeDArray */
    0, /* Packed */
    0, /* BespokeVArray */
    0, /* Dict */
    0, /* BespokeDict */
    0, /* Vec */
    0, /* BespokeVec */
    0, /* Keyset */
    0, /* BespokeKeyset */
    0, /* String */
    0, /* Resource */
    sizeClass<ClsMethData>(),
    sizeClass<RClsMethData>(),
    0, /* Record */
    sizeClass<RFuncData>(),
    0, /* Object */
    0, /* NativeObject */
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
    0, /* MemoData */
    0, /* Cpp */
    0, /* SmallMalloc */
    0, /* BigMalloc */
    0, /* Free */
    0, /* Hole */
    sizeof(Slab), /* Slab */
  };
#define CHECKSIZE(knd, type) \
  static_assert(kind_sizes[(int)HeaderKind::knd] == sizeClass<type>(), #knd);
  CHECKSIZE(ClsMeth, ClsMethData)
  CHECKSIZE(RClsMeth, RClsMethData)
  CHECKSIZE(AsyncFuncWH, c_AsyncFunctionWaitHandle)
  CHECKSIZE(Vector, c_Vector)
  CHECKSIZE(Map, c_Map)
  CHECKSIZE(Set, c_Set)
  CHECKSIZE(Pair, c_Pair)
  CHECKSIZE(ImmVector, c_ImmVector)
  CHECKSIZE(ImmMap, c_ImmMap)
  CHECKSIZE(ImmSet, c_ImmSet)
  CHECKSIZE(RFunc, RFuncData)
  static_assert(kind_sizes[(int)HeaderKind::Slab] == sizeof(Slab), "");
#undef CHECKSIZE
#define CHECKSIZE(knd)\
  static_assert(kind_sizes[(int)HeaderKind::knd] == 0, #knd);
  CHECKSIZE(Packed)
  CHECKSIZE(Mixed)
  CHECKSIZE(Dict)
  CHECKSIZE(Vec)
  CHECKSIZE(Keyset)
  CHECKSIZE(String)
  CHECKSIZE(Resource)
  CHECKSIZE(Record)
  CHECKSIZE(Object)
  CHECKSIZE(NativeObject)
  CHECKSIZE(WaitHandle)
  CHECKSIZE(AwaitAllWH)
  CHECKSIZE(Closure)
  CHECKSIZE(AsyncFuncFrame)
  CHECKSIZE(NativeData)
  CHECKSIZE(ClosureHdr)
  CHECKSIZE(MemoData)
  CHECKSIZE(Cpp)
  CHECKSIZE(SmallMalloc)
  CHECKSIZE(BigMalloc)
  CHECKSIZE(Free)
  CHECKSIZE(Hole)
#undef CHECKSIZE

  auto kind = h->kind();
  if (auto size = kind_sizes[(int)kind]) {
    return size;
  }

  // In the cases below, kinds that don't need size-class rounding will
  // return their size immediately; the rest break to the bottom, then
  // return a rounded-up size.
  size_t size = 0;
  switch (kind) {
    case HeaderKind::Packed:
    case HeaderKind::Vec:
      // size = kSizeIndex2Size[h->aux16>>8]
      size = PackedArray::heapSize(static_cast<const ArrayData*>(h));
      assertx(size == MemoryManager::sizeClass(size));
      return size;
    case HeaderKind::Mixed:
    case HeaderKind::Dict:
      // size = fn of h->m_scale
      size = static_cast<const MixedArray*>(h)->heapSize();
      break;
    case HeaderKind::Keyset:
      // size = fn of h->m_scale
      size = static_cast<const SetArray*>(h)->heapSize();
      break;
    case HeaderKind::BespokeVArray:
    case HeaderKind::BespokeDArray:
    case HeaderKind::BespokeVec:
    case HeaderKind::BespokeDict:
    case HeaderKind::BespokeKeyset:
      size = static_cast<const BespokeArray*>(h)->heapSize();
      break;
    case HeaderKind::String:
      // size = isFlat ? isRefCounted ? table[aux16] : m_size+C : proxy_size;
      size = static_cast<const StringData*>(h)->heapSize();
      break;
    case HeaderKind::Resource:
      // size = h->aux16
      // [ResourceHdr][ResourceData subclass]
      size = static_cast<const ResourceHdr*>(h)->heapSize();
      break;
    case HeaderKind::Record:
      // size = h->m_record->numFields() * sz(TV) + sz(RD)
      // [RecordData][fields]
      size = static_cast<const RecordData*>(h)->heapSize();
      break;
    case HeaderKind::Object:
    case HeaderKind::Closure:
      // size = h->m_cls->m_declProperties.m_extra * sz(TV) + sz(OD)
      // [ObjectData][props]
      size = static_cast<const ObjectData*>(h)->heapSize();
      break;
    case HeaderKind::WaitHandle: {
      // size = table[h->whkind & mask]
      // [ObjectData][subclass]
      auto obj = static_cast<const ObjectData*>(h);
      auto whKind = wait_handle<c_Awaitable>(obj)->getKind();
      size = waithandle_sizes[(int)whKind];
      assertx(size != 0); // AsyncFuncFrame or AwaitAllWH
      assertx(size == MemoryManager::sizeClass(size));
      return size;
    }
    case HeaderKind::AwaitAllWH:
      // size = h->m_cap * sz(Node) + sz(c_AAWH)
      // [ObjectData][children...]
      size = static_cast<const c_AwaitAllWaitHandle*>(h)->heapSize();
      break;
    case HeaderKind::AsyncFuncFrame:
      // size = h->obj_offset + C // 32-bit
      // [NativeNode][locals][Resumable][c_AsyncFunctionWaitHandle]
      size = static_cast<const NativeNode*>(h)->obj_offset +
             sizeof(c_AsyncFunctionWaitHandle);
      break;
    case HeaderKind::NativeData: {
      // h->obj_offset + (h+h->obj_offset)->m_cls->m_extra * sz(TV) + sz(OD)
      // [NativeNode][Memo Slots][NativeData][ObjectData][props] is one alloc.
      // Generators -
      // [NativeNode][NativeData<locals><Resumable><GeneratorData>][ObjectData]
      auto native = static_cast<const NativeNode*>(h);
      size = native->obj_offset + Native::obj(native)->heapSize();
      break;
    }
    case HeaderKind::ClosureHdr:
      // size = h->aux32
      // [ClosureHdr][ObjectData][use vars]
      size = static_cast<const ClosureHdr*>(h)->size();
      break;
    case HeaderKind::MemoData:
      size = static_cast<const MemoNode*>(h)->objOff() + memoObj(h)->heapSize();
      break;
    case HeaderKind::Cpp:
    case HeaderKind::SmallMalloc: // [MallocNode][bytes...]
      // size = h->nbytes // 64-bit
      size = static_cast<const MallocNode*>(h)->nbytes;
      break;
    // the following sizes are intentionally not rounded up to size class.
    case HeaderKind::BigMalloc: // [MallocNode][raw bytes...]
      size = static_cast<const MallocNode*>(h)->nbytes;
      assertx(size != 0);
      // not rounded up to size class, because we never need to iterate over
      // more than one allocated block. avoid calling nallocx().
      return size;
    case HeaderKind::Free:
    case HeaderKind::Hole:
      size = static_cast<const FreeNode*>(h)->size();
      assertx(size != 0);
      // Free objects are guaranteed to be already size-class aligned.
      // Holes are not size-class aligned, so neither need to be rounded up.
      return size;
    case HeaderKind::Slab:
    case HeaderKind::NativeObject:
    case HeaderKind::AsyncFuncWH:
    case HeaderKind::Vector:
    case HeaderKind::Map:
    case HeaderKind::Set:
    case HeaderKind::Pair:
    case HeaderKind::ImmVector:
    case HeaderKind::ImmMap:
    case HeaderKind::ImmSet:
    case HeaderKind::ClsMeth:
    case HeaderKind::RClsMeth:
    case HeaderKind::RFunc:
      not_reached();
  }
  return MemoryManager::sizeClass(size);
}

///////////////////////////////////////////////////////////////////////////////

// call fn(h) on each object in the slab, without calling allocSize()
template<class Fn>
void Slab::iter_starts(Fn fn) const {
  auto ptr = (char*)this;
  for (auto it = starts_, end = starts_ + kNumStarts; it < end; ++it) {
    for (auto bits = *it; bits != 0; bits &= (bits - 1)) {
      auto k = ffs64(bits);
      auto h = (HeapObject*)(ptr + k * kSmallSizeAlign);
      fn(h);
    }
    ptr += kBitsPerStart * kSmallSizeAlign;
  }
}

/*
 * Search from ptr backwards, for a HeapObject that contains ptr.
 * Returns the HeapObject* for the containing object, else nullptr.
 */
inline HeapObject* Slab::find(const void* ptr) const {
  auto const i = start_index(ptr);
  if (bitvec_test(starts_, i)) {
    return reinterpret_cast<HeapObject*>(
        uintptr_t(ptr) & ~(kSmallSizeAlign - 1)
    );
  }
  // compute a mask with 0s at i+1 and higher
  auto const mask = ~0ull >> (kBitsPerStart - 1 - i % kBitsPerStart);
  auto cursor = starts_ + i / kBitsPerStart;
  for (auto bits = *cursor & mask;; bits = *cursor) {
    if (bits) {
      auto k = fls64(bits);
      auto h = reinterpret_cast<HeapObject*>(
        (char*)this + ((cursor - starts_) * kBitsPerStart + k) * kSmallSizeAlign
      );
      auto size = allocSize(h);
      if ((char*)ptr >= (char*)h + size) break;
      return h;
    }
    if (--cursor < starts_) break;
  }
  return nullptr;
}

/*
 * Set multiple start bits. See implementation notes near Slab::InitMasks
 * in memory-manager.cpp
 */
inline void Slab::setStarts(const void* start, const void* end,
                            size_t nbytes, size_t index) {
  auto const start_bit = start_index(start);
  auto const end_bit = start_index((char*)end - kSmallSizeAlign) + 1;
  auto const nbits = nbytes / kSmallSizeAlign;
  if (nbits <= kBitsPerStart &&
      start_bit / kBitsPerStart < end_bit / kBitsPerStart) {
    assertx(index < kNumMasks);
    auto const mask = masks_[index];
    size_t const k = shifts_[index];
    // initially n = how much mask should be shifted to line up with start_bit
    auto n = start_bit % kBitsPerStart;
    // first word (containing start_bit) |= left-shifted mask
    auto s = &starts_[start_bit / kBitsPerStart];
    *s++ |= mask << n;
    // subsequently, n accumulates shifts required by the size class
    n = (n + k) % nbits;
    // store middle words with shifted mask, update shift amount each time
    for (auto e = &starts_[end_bit / kBitsPerStart]; s < e;) {
      *s++ = mask << n;
      n = n + k >= nbits ? n + k - nbits : n + k; // n = (n+k) % nbits
    }
    // last word (containing end_bit) |= mask with end_bit and higher zeroed
    *s |= (mask << n) & ~(~0ull << end_bit % kBitsPerStart);
  } else {
    // Either the size class is too large to fit 1+ bits per 64bit word,
    // or the ncontig range was too small. Set one bit at a time.
    for (auto i = start_bit; i < end_bit; i += nbits) {
      bitvec_set(starts_, i);
    }
  }
}

template<class OnBig, class OnSlab>
void SparseHeap::iterate(OnBig onBig, OnSlab onSlab) {
  // slabs and bigs are sorted; walk through both in address order
  m_bigs.iterate([&](HeapObject* h, size_t size) {
    if (h->kind() == HeaderKind::Slab) {
      onSlab(h, size);
    } else {
      onBig(h, size);
    }
  });
}

template<class Fn> void SparseHeap::iterate(Fn fn) {
  // slabs and bigs are sorted; walk through both in address order
  m_bigs.iterate([&](HeapObject* big, size_t big_size) {
    HeapObject *h, *end;
    if (big->kind() == HeaderKind::Slab) {
      assertx((char*)big + big_size == Slab::fromHeader(big)->end());
      h = (HeapObject*)Slab::fromHeader(big)->start();
      end = (HeapObject*)Slab::fromHeader(big)->end();
    } else {
      h = big;
      end = nullptr; // ensure we don't loop below
    }
    do {
      auto size = allocSize(h);
      fn(h, size);
      h = (HeapObject*)((char*)h + size);
    } while (h < end);
    assertx(!end || h == end); // otherwise, last object was truncated
  });
}

template<class Fn> void MemoryManager::iterate(Fn fn) {
  m_heap.iterate([&](HeapObject* h, size_t allocSize) {
    if (h->kind() >= HeaderKind::Hole) {
      assertx(unsigned(h->kind()) < NumHeaderKinds);
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
  if (debug) checkHeap("MemoryManager::forEachObject");
  std::vector<const ObjectData*> ptrs;
  forEachHeapObject([&](HeapObject* h, size_t) {
    if (auto obj = innerObj(h)) {
      if (!obj->hasUninitProps()) {
        ptrs.push_back(obj);
      }
    }
  });
  for (auto ptr : ptrs) {
    fn(ptr);
  }
}

template<class Fn> void MemoryManager::sweepApcStrings(Fn fn) {
  auto& head = getStringList();
  for (StringDataNode *next, *n = head.next; n != &head; n = next) {
    next = n->next;
    assertx(next && uintptr_t(next) != kSmallFreeWord);
    assertx(next && uintptr_t(next) != kMallocFreeWord);
    auto const s = StringData::node2str(n);
    if (fn(s)) {
      s->unProxy();
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
}
