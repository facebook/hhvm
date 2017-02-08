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

// union of all the possible header types, and some utilities
struct Header {
  HeaderKind kind() const {
    assert(unsigned(hdr_.kind()) <= NumHeaderKinds);
    return hdr_.kind();
  }

  public:
  size_t allocSize() const {
    auto const sz = size();
    switch (kind()) {
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

  const Resumable* resumable() const {
    assert(kind() == HeaderKind::AsyncFuncFrame);
    return reinterpret_cast<const Resumable*>(
      (const char*)this + native_.obj_offset - sizeof(Resumable)
    );
  }
  Resumable* resumable() {
    assert(kind() == HeaderKind::AsyncFuncFrame);
    return reinterpret_cast<Resumable*>(
      (char*)this + native_.obj_offset - sizeof(Resumable)
    );
  }
  const ObjectData* asyncFuncWH() const {
    assert(resumable()->actRec()->func()->isAsyncFunction());
    auto obj = reinterpret_cast<const ObjectData*>(
      (const char*)this + native_.obj_offset
    );
    assert(obj->headerKind() == HeaderKind::AsyncFuncWH);
    return obj;
  }
  ObjectData* asyncFuncWH() {
    assert(resumable()->actRec()->func()->isAsyncFunction());
    auto obj = reinterpret_cast<ObjectData*>(
      (char*)this + native_.obj_offset
    );
    assert(obj->headerKind() == HeaderKind::AsyncFuncWH);
    return obj;
  }
  const ObjectData* nativeObj() const {
    assert(kind() == HeaderKind::NativeData);
    auto obj = Native::obj(&native_);
    assert(isObjectKind(obj->headerKind()));
    assert(obj->getAttribute(ObjectData::HasNativeData));
    return obj;
  }
  ObjectData* nativeObj() {
    assert(kind() == HeaderKind::NativeData);
    auto obj = Native::obj(&native_);
    assert(isObjectKind(obj->headerKind()));
    return obj;
  }
  const ObjectData* closureObj() const {
    assert(kind() == HeaderKind::ClosureHdr);
    auto obj = reinterpret_cast<const ObjectData*>(&closure_hdr_ + 1);
    assert(obj->headerKind() == HeaderKind::Closure);
    return obj;
  }
  ObjectData* closureObj() {
    assert(kind() == HeaderKind::ClosureHdr);
    auto obj = reinterpret_cast<ObjectData*>(&closure_hdr_ + 1);
    assert(obj->headerKind() == HeaderKind::Closure);
    return obj;
  }

  // if this header is one of the types that contains an ObjectData,
  // return the (possibly inner ptr) ObjectData*
  const ObjectData* obj() const {
    return isObjectKind(kind()) ? &obj_ :
           kind() == HeaderKind::AsyncFuncFrame ? asyncFuncWH() :
           kind() == HeaderKind::NativeData ? nativeObj() :
           kind() == HeaderKind::ClosureHdr ? closureObj() :
           nullptr;
  }

public:
  union {
    MaybeCountable hdr_;
    StringData str_;
    ArrayData arr_;
    MixedArray mixed_;
    SetArray set_;
    APCLocalArray apc_;
    ProxyArray proxy_;
    GlobalsArray globals_;
    ObjectData obj_;
    c_Pair pair_;
    BaseVector vector_;
    HashCollection hashcoll_;
    ResourceHdr res_;
    RefData ref_;
    MallocNode malloc_;
    FreeNode free_;
    NativeNode native_;
    c_AwaitAllWaitHandle awaitall_;
    ClosureHdr closure_hdr_;
    c_Closure closure_;
  };

private:
  size_t size() const;

};

// Return the size (in bytes) without rounding up to MM size class.
inline size_t Header::size() const {
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
    sizeof(APCLocalArray),
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
  };
#define CHECKSIZE(knd, size) \
  static_assert(kind_sizes[(int)HeaderKind::knd] == size, #knd);
  CHECKSIZE(Empty, sizeof(ArrayData))
  CHECKSIZE(Apc, sizeof(APCLocalArray))
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
#undef CHECKSIZE

  HeaderKind kindVar = kind();
  if (auto size = kind_sizes[(int)kindVar]) return size;

  switch (kindVar) {
    case HeaderKind::Packed:
    case HeaderKind::VecArray:
      return PackedArray::heapSize(&arr_);
    case HeaderKind::Mixed:
    case HeaderKind::Dict:
      return mixed_.heapSize();
    case HeaderKind::Keyset:
      return set_.heapSize();
    case HeaderKind::String:
      return str_.heapSize();
    case HeaderKind::Closure:
    case HeaderKind::Object:
      // [ObjectData][props]
      return obj_.heapSize();
    case HeaderKind::ClosureHdr:
      // [ClosureHdr][ObjectData][use vars]
      return closure_hdr_.size();
    case HeaderKind::WaitHandle:
    {
      // [ObjectData][subclass]
      auto whKind = wait_handle<c_WaitHandle>(&obj_)->getKind();
      if (auto whSize = waithandle_sizes[(int)whKind]) return whSize;
      return asio_object_size(&obj_);
    }
    case HeaderKind::AwaitAllWH:
      // [ObjectData][children...]
      return awaitall_.heapSize();
    case HeaderKind::Resource:
      // [ResourceHdr][ResourceData subclass]
      return res_.heapSize();
    case HeaderKind::SmallMalloc: // [MallocNode][bytes...]
    case HeaderKind::BigMalloc:   // [MallocNode][bytes...]
    case HeaderKind::BigObj:      // [MallocNode][Header...]
      return malloc_.nbytes;
    case HeaderKind::AsyncFuncFrame:
      // [NativeNode][locals][Resumable][c_AsyncFunctionWaitHandle]
      return native_.obj_offset + sizeof(c_AsyncFunctionWaitHandle);
    case HeaderKind::NativeData:
      // [NativeNode][NativeData][ObjectData][props] is one allocation.
      // Generators -
      // [NativeNode][NativeData<locals><Resumable><GeneratorData>][ObjectData]
      return native_.obj_offset + nativeObj()->heapSize();
    case HeaderKind::Free:
    case HeaderKind::Hole:
      return free_.size();
    case HeaderKind::AsyncFuncWH:
    case HeaderKind::Empty:
    case HeaderKind::Apc:
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

///////////////////////////////////////////////////////////////////////////////

template<class Fn> void BigHeap::iterate(Fn fn) {
  // slabs and bigs are sorted; walk through both in address order
  const auto SENTINEL = (Header*) ~0LL;
  auto slab = std::begin(m_slabs);
  auto big = std::begin(m_bigs);
  auto slabend = std::end(m_slabs);
  auto bigend = std::end(m_bigs);
  while (slab != slabend || big != bigend) {
    Header* slab_hdr = slab != slabend ? (Header*)slab->ptr : SENTINEL;
    Header* big_hdr = big != bigend ? (Header*)*big : SENTINEL;
    assert(slab_hdr < SENTINEL || big_hdr < SENTINEL);
    Header *h, *end;
    if (slab_hdr < big_hdr) {
      h = slab_hdr;
      end = (Header*)((char*)h + slab->size);
      ++slab;
      assert(end <= big_hdr); // otherwise slab overlaps next big
    } else {
      h = big_hdr;
      end = nullptr; // ensure we don't loop below
      ++big;
    }
    do {
      auto size = h->allocSize();
      fn(h, size);
      h = (Header*)((char*)h + size);
    } while (h < end);
    assert(!end || h == end); // otherwise, last object was truncated
  }
}

template<class Fn> void MemoryManager::iterate(Fn fn) {
  m_heap.iterate([&](Header* h, size_t allocSize) {
    if (h->kind() == HeaderKind::BigObj) {
      // skip MallocNode
      h = reinterpret_cast<Header*>((&h->malloc_) + 1);
      allocSize -= sizeof(MallocNode);
    } else if (h->kind() == HeaderKind::Hole) {
      // no valid pointer can point here.
      return; // continue iterating
    }
    fn(h, allocSize);
  });
}

template<class Fn> void MemoryManager::forEachHeader(Fn fn) {
  initFree();
  iterate(fn);
}

template<class Fn> void MemoryManager::forEachObject(Fn fn) {
  if (debug) checkHeap("MM::forEachObject");
  std::vector<ObjectData*> ptrs;
  forEachHeader([&](Header* h, size_t) {
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
        ptrs.push_back(&h->obj_);
        break;
      case HeaderKind::AsyncFuncFrame:
        ptrs.push_back(h->asyncFuncWH());
        break;
      case HeaderKind::NativeData:
        ptrs.push_back(h->nativeObj());
        break;
      case HeaderKind::ClosureHdr:
        ptrs.push_back(h->closureObj());
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
        assert(false && "forEachHeader skips these kinds");
        break;
    }
  });
  for (auto ptr : ptrs) {
    fn(ptr);
  }
}

///////////////////////////////////////////////////////////////////////////////

// information about heap objects, indexed by valid object starts.
struct PtrMap {
  using Region = std::pair<const Header*, std::size_t>;
  static constexpr auto Mask = 0xffffffffffffULL; // 48 bit address space

  void insert(const Header* h, size_t size) {
    sorted_ &= regions_.empty() || h > regions_.back().first;
    regions_.emplace_back(h, size);
  }

  const Region* region(const void* p) const {
    assert(sorted_);
    if (uintptr_t(p) - uintptr_t(span_.first) >= span_.second) {
      return nullptr;
    }
    // Find the first region which begins beyond p.
    p = reinterpret_cast<void*>(uintptr_t(p) & Mask);
    auto it = std::upper_bound(regions_.begin(), regions_.end(), p,
      [](const void* p, const Region& region) {
        return p < region.first;
      });
    // If it == first region, p is before any region, which we already
    // checked above.
    assert(it != regions_.begin());
    --it; // backup to the previous region.
    // p can only potentially point within this previous region, so check that.
    return uintptr_t(p) - uintptr_t(it->first) < it->second ? &*it :
           nullptr;
  }

  const Header* header(const void* p) const {
    auto r = region(p);
    return r ? r->first : nullptr;
  }

  bool isHeader(const void* p) const {
    auto h = header(p);
    return h && h == p;
  }

  size_t index(const Region* r) const {
    return r - &regions_[0];
  }

  // where does this header sit in the regions_ vector?
  size_t index(const Header* h) const {
    assert(header(h));
    return region(h) - &regions_[0];
  }

  void prepare() {
    if (!sorted_) {
      std::sort(regions_.begin(), regions_.end());
      sorted_ = true;
    }
    if (!regions_.empty()) {
      auto& front = regions_.front();
      auto& back = regions_.back();
      span_ = Region{
        front.first,
        (const char*)back.first + back.second - (const char*)front.first
      };
    }
    assert(sanityCheck());
  }

  size_t size() const {
    return regions_.size();
  }

  template<class Fn> void iterate(Fn fn) const {
    for (auto& r : regions_) {
      fn(r.first, r.second);
    }
  }

  Region span() const {
    return span_;
  }

private:
  bool sanityCheck() const {
    // Verify that all the regions are in increasing and non-overlapping order.
    DEBUG_ONLY void* last = nullptr;
    for (const auto& region : regions_) {
      assert(!last || last <= region.first);
      last = (void*)(uintptr_t(region.first) + region.second);
    }
    return true;
  }

  Region span_{nullptr, 0};
  std::vector<std::pair<const Header*, std::size_t>> regions_;
  bool sorted_{true};
};

///////////////////////////////////////////////////////////////////////////////

}

#endif
