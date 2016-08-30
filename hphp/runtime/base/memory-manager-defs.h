/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/runtime/ext/asio/ext_async-function-wait-handle.h"
#include "hphp/runtime/ext/collections/ext_collections-pair.h"
#include "hphp/runtime/ext/collections/ext_collections-vector.h"
#include "hphp/runtime/ext/collections/hash-collection.h"

#include <algorithm>

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

// union of all the possible header types, and some utilities
struct Header {
  HeaderKind kind() const {
    assert(unsigned(hdr_.kind) <= NumHeaderKinds);
    return hdr_.kind;
  }

  size_t size() const;

  size_t allocSize() const {
    // Hole and Free have exact sizes, so don't round them.
    auto const sz = hdr_.kind == HeaderKind::Hole ||
                    hdr_.kind == HeaderKind::Free
      ? free_.size()
      : MemoryManager::smallSizeClass(size());

    assertx(sz % 16 == 0);
    return sz;
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
    return obj;
  }
  ObjectData* nativeObj() {
    assert(kind() == HeaderKind::NativeData);
    auto obj = Native::obj(&native_);
    assert(isObjectKind(obj->headerKind()));
    return obj;
  }

  // if this header is one of the types that contains an ObjectData,
  // return the (possibly inner ptr) ObjectData*
  const ObjectData* obj() const {
    return isObjectKind(kind()) ? &obj_ :
           kind() == HeaderKind::AsyncFuncFrame ? asyncFuncWH() :
           kind() == HeaderKind::NativeData ? nativeObj() :
           nullptr;
  }

public:
  union {
    struct {
      uint64_t q;
      HeaderWord<> hdr_;
    };
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
  };
};

inline size_t Header::size() const {
  switch (kind()) {
    case HeaderKind::Packed:
    case HeaderKind::VecArray:
      return PackedArray::heapSize(&arr_);
    case HeaderKind::Mixed:
    case HeaderKind::Dict:
      return mixed_.heapSize();
    case HeaderKind::Keyset:
      return set_.heapSize();
    case HeaderKind::Empty:
      return sizeof(ArrayData);
    case HeaderKind::Apc:
      return sizeof(APCLocalArray);
    case HeaderKind::Globals:
      return sizeof(GlobalsArray);
    case HeaderKind::Proxy:
      return sizeof(ProxyArray);
    case HeaderKind::String:
      return str_.heapSize();
    case HeaderKind::Object:
    case HeaderKind::AsyncFuncWH:
      // [ObjectData][subclass][props]
      return obj_.heapSize();
    case HeaderKind::Vector:
    case HeaderKind::Map:
    case HeaderKind::Set:
    case HeaderKind::Pair:
    case HeaderKind::ImmVector:
    case HeaderKind::ImmMap:
    case HeaderKind::ImmSet:
      // [ObjectData][subclass]
      return collections::heapSize(kind());
    case HeaderKind::WaitHandle:
      // [ObjectData][subclass]
      return asio_object_size(&obj_);
    case HeaderKind::AwaitAllWH:
      // [ObjectData][children...]
      return awaitall_.heapSize();
    case HeaderKind::Resource:
      // [ResourceHdr][ResourceData subclass]
      return res_.heapSize();
    case HeaderKind::Ref:
      return sizeof(RefData);
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
  }
  return 0;
}

///////////////////////////////////////////////////////////////////////////////

template<class Fn> void BigHeap::iterate(Fn fn) {
  auto in_slabs = !m_slabs.empty();
  auto slab = std::begin(m_slabs);
  auto big = std::begin(m_bigs);

  auto const bounds_from_slab = [] (const MemBlock& slab) {
    return std::make_pair(
      reinterpret_cast<Header*>(slab.ptr),
      reinterpret_cast<Header*>(static_cast<char*>(slab.ptr) + slab.size)
    );
  };

  Header* hdr;
  Header* slab_end;

  if (in_slabs) {
    std::tie(hdr, slab_end) = bounds_from_slab(*slab);
  } else {
    hdr = slab_end = nullptr;
    if (big != std::end(m_bigs)) hdr = reinterpret_cast<Header*>(*big);
  }
  while (in_slabs || big != end(m_bigs)) {
    auto const h = hdr;

    if (in_slabs) {
      // Move to the next header in the slab.
      hdr = reinterpret_cast<Header*>(reinterpret_cast<char*>(hdr) +
                                      hdr->allocSize());
      if (hdr >= slab_end) {
        assert(hdr == slab_end && "hdr > slab_end indicates corruption");
        // move to next slab
        if (++slab != m_slabs.end()) {
          std::tie(hdr, slab_end) = bounds_from_slab(*slab);
        } else {
          // move to first big block
          in_slabs = false;
          if (big != std::end(m_bigs)) hdr = reinterpret_cast<Header*>(*big);
        }
      }
    } else {
      // move to next big block
      if (++big != std::end(m_bigs)) hdr = reinterpret_cast<Header*>(*big);
    }
    fn(h);
  }
}

template<class Fn> void MemoryManager::iterate(Fn fn) {
  m_heap.iterate([&](Header* h) {
    if (h->kind() == HeaderKind::BigObj) {
      // skip MallocNode
      h = reinterpret_cast<Header*>((&h->malloc_) + 1);
    } else if (h->kind() == HeaderKind::Hole) {
      // no valid pointer can point here.
      return; // continue iterating
    }
    fn(h);
  });
}

template<class Fn> void MemoryManager::forEachHeader(Fn fn) {
  initFree();
  iterate(fn);
}

template<class Fn> void MemoryManager::forEachObject(Fn fn) {
  if (debug) checkHeap("MM::forEachObject");
  std::vector<ObjectData*> ptrs;
  forEachHeader([&](Header* h) {
    switch (h->kind()) {
      case HeaderKind::Object:
      case HeaderKind::WaitHandle:
      case HeaderKind::AsyncFuncWH:
      case HeaderKind::AwaitAllWH:
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

  void insert(const Header* h) {
    assert(!sorted_);
    regions_.emplace_back(h, h->size());
  }

  const Region* region(const void* p) const {
    assert(sorted_);
    // Find the first region which begins beyond p.
    p = reinterpret_cast<void*>(uintptr_t(p) & Mask);
    auto it = std::upper_bound(regions_.begin(), regions_.end(), p,
      [](const void* p, const Region& region) {
        return p < region.first;
      });
    // If its the first region, p is before any region, so there's no
    // header. Otherwise, backup to the previous region.
    if (it == regions_.begin()) return nullptr;
    --it;
    // p can only potentially point within this previous region, so check that.
    return (uintptr_t(p) < uintptr_t(it->first) + it->second) ? &*it :
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
    assert(!sorted_);
    std::sort(regions_.begin(), regions_.end());
    assert(sanityCheck());
    sorted_ = true;
  }

  size_t size() const {
    return regions_.size();
  }

  template<class Fn> void iterate(Fn fn) const {
    for (auto& r : regions_) {
      fn(r.first, r.second);
    }
  }

private:
  bool sanityCheck() const {
    // Verify that all the regions are in increasing and non-overlapping order.
    void* last = nullptr;
    for (const auto& region : regions_) {
      if (!last || last <= region.first) {
        last = (void*)(uintptr_t(region.first) + region.second);
      } else {
        return false;
      }
    }
    return true;
  }

  std::vector<std::pair<const Header*, std::size_t>> regions_;
  bool sorted_ = false;
};

///////////////////////////////////////////////////////////////////////////////

}

#endif
