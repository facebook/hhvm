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

#ifndef incl_HPHP_MEMORY_MANAGER_DEFS_H
#define incl_HPHP_MEMORY_MANAGER_DEFS_H

#include "hphp/runtime/base/apc-local-array.h"
#include "hphp/runtime/base/proxy-array.h"
#include "hphp/runtime/vm/native-data.h"
#include "hphp/runtime/base/packed-array-defs.h"
#include "hphp/runtime/base/mixed-array-defs.h"
#include "hphp/runtime/vm/globals-array.h"
#include "hphp/runtime/vm/resumable.h"

namespace HPHP {

// union of all the possible header types, and some utilities
struct Header {
  struct DummySweepable: Sweepable, ObjectData { void sweep() {} };
  size_t size() const;
  HeaderKind kind() const {
    assert(unsigned(kind_) <= NumHeaderKinds);
    return kind_;
  }
  union {
    struct {
      uint64_t q;
      uint8_t b[3];
      HeaderKind kind_;
    };
    StringData str_;
    ArrayData arr_;
    MixedArray mixed_;
    ObjectData obj_;
    ResourceData res_;
    RefData ref_;
    SmallNode small_;
    BigNode big_;
    FreeNode free_;
    ResumableNode resumable_;
    NativeNode native_;
    DebugHeader debug_;
    DummySweepable sweepable_;
  };

  Resumable* resumable() const {
    return reinterpret_cast<Resumable*>(
      (char*)this + sizeof(ResumableNode) + resumable_.framesize
    );
  }
  ObjectData* resumableObj() const {
    return reinterpret_cast<ObjectData*>(resumable() + 1);
  }
};

inline size_t Header::size() const {
  auto resourceSize = [](const ResourceData* r) {
    // explicitly virtual-call ResourceData::heapSize() through a pointer
    assert(r->heapSize());
    return r->heapSize();
  };
  switch (kind()) {
    case HeaderKind::Packed: case HeaderKind::VPacked:
      return PackedArray::heapSize(&arr_);
    case HeaderKind::Mixed: case HeaderKind::StrMap: case HeaderKind::IntMap:
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
    case HeaderKind::Resource:
      // [ResourceData][subclass]
      return resourceSize(&res_);
    case HeaderKind::Ref:
      return sizeof(RefData);
    case HeaderKind::SmallMalloc:
      return small_.padbytes;
    case HeaderKind::BigMalloc:
    case HeaderKind::BigObj:
      // [BigNode][bytes...] if smartMallocBig, or
      // [BigNode][Header...] if smartMallocSizeBig
      return big_.nbytes;
    case HeaderKind::Free:
      return free_.size;
    case HeaderKind::Resumable:
      // [ResumableNode][locals][Resumable][ObjectData<ResumableObj>]
      return resumable()->size();
    case HeaderKind::Native:
      // [NativeNode][NativeData][ObjectData][props] is one allocation.
      return native_.obj_offset + Native::obj(&native_)->heapSize();
    case HeaderKind::Sweepable:
      // [Sweepable][ObjectData][subclass][props] or
      // [Sweepable][ResourceData][subclass]
      return sizeof(Sweepable) + sweepable_.heapSize();
    case HeaderKind::Hole:
      return free_.size;
    case HeaderKind::Debug:
      assert(debug_.allocatedMagic == DebugHeader::kAllocatedMagic);
      return sizeof(DebugHeader);
  }
  return 0;
}

// Iterator over all the slabs and bigs
struct BigHeap::iterator {
  struct Headiter {
    explicit Headiter(void* p) : raw_((char*)p) {}
    Headiter& operator=(void* p) {
      raw_ = (char*)p;
      return *this;
    }
    Headiter& operator++() {
      assert(h_->size() > 0);
      raw_ += MemoryManager::smartSizeClass(h_->size());
      return *this;
    }
    Header& operator*() { return *h_; }
    Header* operator->() { return h_; }
    bool operator<(Headiter i) const { return raw_ < i.raw_; }
    bool operator==(Headiter i) const { return raw_ == i.raw_; }
  private:
    union {
      Header* h_;
      char* raw_;
    };
  };
  enum State { Slabs, Bigs };
  using slab_iter = std::vector<MemBlock>::iterator;
  using big_iter  = std::vector<BigNode*>::iterator;
  explicit iterator(slab_iter slab, BigHeap& heap)
    : m_state{Slabs}
    , m_slab{slab}
    , m_header{slab->ptr} // should never be at end.
    , m_heap(heap)
  {}
  explicit iterator(big_iter big, BigHeap& heap)
    : m_state{Bigs}
    , m_big{big}
    , m_header{big != heap.m_bigs.end() ? *big : nullptr}
    , m_heap(heap)
  {}
  bool operator==(const iterator& it) const {
    if (m_state != it.m_state) return false;
    switch (m_state) {
      case Slabs: return m_slab == it.m_slab && m_header == it.m_header;
      case Bigs: return m_big == it.m_big;
    };
    not_reached();
  }
  bool operator!=(const iterator& it) const {
    return !(*this == it);
  }
  iterator& operator++() {
    switch (m_state) {
      case Slabs: {
        auto end = Headiter{static_cast<char*>(m_slab->ptr) + m_slab->size};
        if (++m_header < end) {
          return *this;
        }
        if (++m_slab != m_heap.m_slabs.end()) {
          m_header = m_slab->ptr;
          return *this;
        }
        m_state = Bigs;
        m_big = m_heap.m_bigs.begin();
        m_header = m_big != m_heap.m_bigs.end() ? *m_big : nullptr;
        return *this;
      }
      case Bigs:
        ++m_big;
        m_header = m_big != m_heap.m_bigs.end() ? *m_big : nullptr;
        return *this;
    }
    not_reached();
  }
  Header& operator*() { return *m_header; }
  Header* operator->() { return &*m_header; }
 private:
  State m_state;
  union {
    slab_iter m_slab;
    big_iter m_big;
  };
  Headiter m_header;
  BigHeap& m_heap;
};

template<class Fn> void MemoryManager::forEachObject(Fn fn) {
  if (debug) checkHeap();
  std::vector<ObjectData*> ptrs;
  for (auto i = begin(), lim = end(); i != lim; ++i) {
    switch (i->kind()) {
      case HeaderKind::Object:
      case HeaderKind::ResumableObj:
        ptrs.push_back(&i->obj_);
        break;
      case HeaderKind::Resumable:
        ptrs.push_back(i->resumableObj());
        break;
      case HeaderKind::Native:
        ptrs.push_back(Native::obj(&i->native_));
        break;
      case HeaderKind::BigObj: {
        Header* h = reinterpret_cast<Header*>(&i->big_ + 1);
        if (h->kind() == HeaderKind::Object) {
          ptrs.push_back(reinterpret_cast<ObjectData*>(h));
          break;
        }
        continue;
      }
      case HeaderKind::Packed:
      case HeaderKind::Mixed:
      case HeaderKind::StrMap:
      case HeaderKind::IntMap:
      case HeaderKind::VPacked:
      case HeaderKind::Empty:
      case HeaderKind::Apc:
      case HeaderKind::Globals:
      case HeaderKind::Proxy:
      case HeaderKind::String:
      case HeaderKind::Resource:
      case HeaderKind::Ref:
      case HeaderKind::Sweepable:
      case HeaderKind::SmallMalloc:
      case HeaderKind::BigMalloc:
      case HeaderKind::Free:
      case HeaderKind::Hole:
      case HeaderKind::Debug:
        continue;
    }
  }
  for (auto ptr : ptrs) {
    fn(ptr);
  }
}

}

#endif
