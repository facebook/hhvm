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
#include "hphp/runtime/base/proxy-array.h"
#include "hphp/runtime/vm/native-data.h"
#include "hphp/runtime/base/packed-array-defs.h"
#include "hphp/runtime/base/mixed-array-defs.h"
#include "hphp/runtime/base/struct-array.h"
#include "hphp/runtime/vm/globals-array.h"
#include "hphp/runtime/vm/resumable.h"
#include "hphp/runtime/ext/asio/ext_asio.h"
#include "hphp/runtime/ext/asio/ext_await-all-wait-handle.h"
#include "hphp/runtime/ext/collections/ext_collections-pair.h"

namespace HPHP {

// union of all the possible header types, and some utilities
struct Header {
  size_t size() const;
  HeaderKind kind() const {
    assert(unsigned(hdr_.kind) <= NumHeaderKinds);
    return hdr_.kind;
  }
  union {
    struct {
      uint64_t q;
      HeaderWord<> hdr_;
    };
    StringData str_;
    ArrayData arr_;
    MixedArray mixed_;
    StructArray struct_;
    APCLocalArray apc_;
    ProxyArray proxy_;
    GlobalsArray globals_;
    ObjectData obj_;
    c_Pair pair_;
    ResourceHdr res_;
    RefData ref_;
    SmallNode small_;
    BigNode big_;
    FreeNode free_;
    ResumableNode resumable_;
    NativeNode native_;
    c_AwaitAllWaitHandle awaitall_;
  };

  const Resumable* resumable() const {
    assert(kind() == HeaderKind::ResumableFrame);
    return reinterpret_cast<const Resumable*>(
      (char*)this + sizeof(ResumableNode) + resumable_.framesize
    );
  }
  Resumable* resumable() {
    assert(kind() == HeaderKind::ResumableFrame);
    return reinterpret_cast<Resumable*>(
      (char*)this + sizeof(ResumableNode) + resumable_.framesize
    );
  }
  const ObjectData* resumableObj() const {
    DEBUG_ONLY auto const func = resumable()->actRec()->func();
    assert(func->isAsyncFunction());
    auto obj = reinterpret_cast<const ObjectData*>(resumable() + 1);
    assert(obj->headerKind() == HeaderKind::ResumableObj);
    return obj;
  }
  ObjectData* resumableObj() {
    DEBUG_ONLY auto const func = resumable()->actRec()->func();
    assert(func->isAsyncFunction());
    auto obj = reinterpret_cast<ObjectData*>(resumable() + 1);
    assert(obj->headerKind() == HeaderKind::ResumableObj);
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
           kind() == HeaderKind::ResumableFrame ? resumableObj() :
           kind() == HeaderKind::NativeData ? nativeObj() :
           nullptr;
  }
};

inline size_t Header::size() const {
  switch (kind()) {
    case HeaderKind::Packed:
      return PackedArray::heapSize(&arr_);
    case HeaderKind::Struct:
      return StructArray::heapSize(&arr_);
    case HeaderKind::Mixed:
      return mixed_.heapSize();
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
    case HeaderKind::ResumableObj:
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
    case HeaderKind::SmallMalloc:
      return small_.padbytes;
    case HeaderKind::BigMalloc: // [BigNode][bytes...]
    case HeaderKind::BigObj:    // [BigNode][Header...]
      return big_.nbytes;
    case HeaderKind::ResumableFrame:
      // Async functions -
      // [ResumableNode][locals][Resumable][ObjectData<WaitHandle>]
      return resumable()->size();
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

// Iterate over all the slabs and bigs
template<class Fn> void BigHeap::iterate(Fn fn) {
  auto in_slabs = !m_slabs.empty();
  auto slab = begin(m_slabs);
  auto big = begin(m_bigs);
  Header *hdr, *slab_end;
  if (in_slabs) {
    hdr = (Header*)slab->ptr;
    slab_end = (Header*)(static_cast<char*>(slab->ptr) + slab->size);
  } else {
    hdr = big != end(m_bigs) ? (Header*)*big : nullptr;
    slab_end = nullptr;
  }
  while (in_slabs || big != end(m_bigs)) {
    auto h = hdr;
    if (in_slabs) {
      // move to next header in slab. Hole and Free have exact sizes,
      // so don't round them.
      auto size = hdr->hdr_.kind == HeaderKind::Hole ||
                  hdr->hdr_.kind == HeaderKind::Free ? hdr->free_.size() :
                  MemoryManager::smallSizeClass(hdr->size());
      assert(size % 16 == 0);
      hdr = (Header*)((char*)hdr + size);
      if (hdr >= slab_end) {
        assert(hdr == slab_end && "hdr > slab_end indicates corruption");
        // move to next slab
        if (++slab != m_slabs.end()) {
          hdr = (Header*)slab->ptr;
          slab_end = (Header*)(static_cast<char*>(slab->ptr) + slab->size);
        } else {
          // move to first big block
          in_slabs = false;
          if (big != end(m_bigs)) hdr = (Header*)*big;
        }
      }
    } else {
      // move to next big block
      if (++big != end(m_bigs)) hdr = (Header*)*big;
    }
    fn(h);
  }
}

// Raw iterator loop over the headers of everything in the heap.  Skips BigObj
// because it's just a detail of which sub-heap we used to allocate something
// based on its size, and it can prefix almost any other header kind.  Clients
// can call this directly to avoid unnecessary initFree()s.
template<class Fn> void MemoryManager::iterate(Fn fn) {
  assert(!m_needInitFree);
  m_heap.iterate([&](Header* h) {
    if (h->kind() == HeaderKind::BigObj) {
      // skip BigNode
      h = reinterpret_cast<Header*>((&h->big_)+1);
    } else if (h->kind() == HeaderKind::Hole) {
      // no valid pointer can point here.
      return; // continue iterating
    }
    fn(h);
    assert(!m_needInitFree); // otherwise the heap is unparsable.
  });
}

// same as iterate(), but calls initFree first.
template<class Fn> void MemoryManager::forEachHeader(Fn fn) {
  initFree();
  iterate(fn);
}

// iterate just the ObjectDatas, including the kinds with prefixes.
// (NativeData and ResumableFrame).
template<class Fn> void MemoryManager::forEachObject(Fn fn) {
  if (debug) checkHeap("MM::forEachObject");
  std::vector<ObjectData*> ptrs;
  forEachHeader([&](Header* h) {
    switch (h->kind()) {
      case HeaderKind::Object:
      case HeaderKind::WaitHandle:
      case HeaderKind::ResumableObj:
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
      case HeaderKind::ResumableFrame:
        ptrs.push_back(h->resumableObj());
        break;
      case HeaderKind::NativeData:
        ptrs.push_back(h->nativeObj());
        break;
      case HeaderKind::Packed:
      case HeaderKind::Struct:
      case HeaderKind::Mixed:
      case HeaderKind::Empty:
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

// information about heap objects, indexed by valid object starts.
struct PtrMap {
  void insert(const Header* h) {
    assert(!sorted_);
    regions_.emplace_back(h, h->size());
  }

  const Header* header(const void* p) const {
    assert(sorted_);
    // Find the first region which begins beyond p.
    auto it =
      std::upper_bound(
        regions_.begin(),
        regions_.end(),
        p,
        [](const void* p,
           const std::pair<const Header*, std::size_t>& region) {
          return p < region.first;
        }
      );
    // If its the first region, p is before any region, so there's no
    // header. Otherwise, backup to the previous region.
    if (it == regions_.begin()) return nullptr;
    --it;
    // p can only potentially point within this previous region, so check that.
    return (uintptr_t(p) < uintptr_t(it->first) + it->second) ?
      it->first : nullptr;
  }

  bool isHeader(const void* p) const {
    auto h = header(p);
    return h && h == p;
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

}

#endif
