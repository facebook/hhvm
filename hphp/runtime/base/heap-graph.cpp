/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2015 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/runtime/base/heap-graph.h"
#include "hphp/runtime/base/heap-algorithms.h"
#include "hphp/runtime/base/types.h"
#include "hphp/runtime/base/req-containers.h"
#include "hphp/runtime/base/memory-manager-defs.h"
#include "hphp/runtime/base/heap-scan.h"
#include "hphp/runtime/base/thread-info.h"
#include "hphp/runtime/base/container-functions.h"
#include "hphp/runtime/base/tv-helpers.h"
#include "hphp/util/alloc.h"

#include <vector>
#include <boost/dynamic_bitset.hpp>
#include <unordered_map>
#include <folly/Range.h>

namespace HPHP {

template<class F>
struct PtrFilter: F {
  template <class... Args> explicit PtrFilter(Args&&... args)
    : F(std::forward<Args>(args)...)
  {}

  // classes containing exact pointers
  void operator()(const String& p) { (*this)(p.get()); }
  void operator()(const Array& p) { (*this)(p.get()); }
  void operator()(const ArrayNoDtor& p) { (*this)(p.arr()); }
  void operator()(const Object& p) { (*this)(p.get()); }
  void operator()(const Resource& p) { (*this)(p.hdr()->data()); }
  void operator()(const Variant& p) { (*this)(*p.asTypedValue()); }
  void operator()(const StringBuffer& p) { p.scan(*this); }
  void operator()(const NameValueTable& p) { p.scan(*this); }
  void operator()(const ArrayIter& p) { p.scan(*this); }
  void operator()(const MArrayIter& p) { p.scan(*this); }
  void operator()(const VarEnv& venv) { (*this)(&venv); }

  void operator()(const RequestEventHandler& p) { p.scan(*this); }
  void operator()(const RequestEventHandler* p) { p->scan(*this); }

  // TODO (6512343): this needs to be hooked into scan methods for Extensions.
  void operator()(const Extension&) { }

  void operator()(const AsioContext& p) { scanner().scan(p, *this); }

  // mark a TypedValue or TypedValueAux. taking tv by value would exclude aux.
  void operator()(const TypedValueAux& v) { (*this)(*(const TypedValue*)&v); }
  void operator()(const TypedValue& tv) {
    switch (tv.m_type) {
      case KindOfString: // never null, sometimes counted
        if (counted(tv.m_data.pstr)) next()(tv.m_data.pstr);
        break;
      case KindOfArray: // never null, sometimes counted
        if (counted(tv.m_data.parr)) next()(tv.m_data.parr);
        break;
      case KindOfRef: // sometimes points into rds
        return next()(tv.m_data.pref);
      case KindOfObject:    return next()(tv.m_data.pobj);
      case KindOfResource:  return next()(tv.m_data.pres);
      case KindOfUninit:
      case KindOfNull:
      case KindOfBoolean:
      case KindOfInt64:
      case KindOfDouble:
      case KindOfStaticString:
      case KindOfClass: // only in eval stack
        return;
    }
  }

  // todo: implement me
  void operator()(const ActRec&) {}
  void operator()(const Stack&) {}

  void operator()(const StringData* p) { if (p && counted(p)) next()(p); }
  void operator()(const ArrayData* p) { if (p && counted(p)) next()(p); }
  void operator()(const ObjectData* p) { if (p) next()(p); }
  void operator()(const ResourceData* p) { if (p) next()(p->hdr()); }
  void operator()(const ResourceHdr* p) { if (p) next()(p); }
  void operator()(const RefData* p) { if (p) next()(p); }

  // collections of scannable fields
  template<class T> void operator()(const req::vector<T>& c) {
    for (auto& e : c) (*this)(e);
  }
  template<class T> void operator()(const req::set<T>& c) {
    for (auto& e : c) (*this)(e);
  }
  template<class T,class U> void operator()(const std::pair<T,U>& p) {
    (*this)(p.first);
    (*this)(p.second);
  }
  template<class T,class U,class V,class W>
  void operator()(const req::hash_map<T,U,V,W>& c) {
    for (auto& e : c) (*this)(e); // each element is pair<T,U>
  }

  void operator()(const VarEnv* p) { if (p) p->scan(*this); }

  // Explicitly ignored field types.
  void operator()(const LowPtr<Class>&) {}
  void operator()(const Unit*) {}
  void operator()(const Func*) {}
  void operator()(const Class*) {}
  void operator()(const std::string&) {}
  void operator()(int) {}

  // NVTs live inside VarEnv, and GlobalsArray has an interior ptr to one.
  // ignore the interior pointer; NVT should be scanned by VarEnv::scan.
  void operator()(const NameValueTable* p) {}

  // mark ambigous pointers in the range [start,start+len). If the start or
  // end is a partial word, don't scan that word.
  void operator()(const void* start, size_t len) {
    const uintptr_t M{7}; // word size - 1
    auto s = (char**)((uintptr_t(start) + M) & ~M); // round up
    auto e = (char**)((uintptr_t(start) + len) & ~M); // round down
    for (; s < e; s++) next().ambig(*s);
  }

private:
  F& next() { return *this; }
  template<class T> static bool counted(T* p) {
    return p->getCount() >= 0;
  }
};

namespace {

using PtrMap = std::unordered_map<const Header*,int>;

void addNode(HeapGraph& g, PtrMap& ids, const Header* h) {
  ids[h] = g.nodes.size();
  g.nodes.push_back(HeapGraph::Node{h, -1, -1});
}

void addPtr(HeapGraph& g, int from, int to, HeapGraph::PtrKind kind) {
  auto& from_node = g.nodes[from];
  auto& to_node = g.nodes[to];
  auto e = g.ptrs.size();
  g.ptrs.push_back(
    HeapGraph::Ptr{from, to, from_node.succ, to_node.pred, kind}
  );
  from_node.succ = to_node.pred = e;
}

void addRoot(HeapGraph& g, int to, HeapGraph::PtrKind kind, const char* seat) {
  auto& to_node = g.nodes[to];
  auto e = g.ptrs.size();
  g.ptrs.push_back(HeapGraph::Ptr{-1, to, -1, to_node.pred, kind, seat});
  to_node.pred = e;
  g.roots.push_back(e);
}

// if p points to a valid Header, return it; otherwise return null
const Header* filter(PtrMap& ids, const void* p) {
  auto h = static_cast<const Header*>(p);
  auto it = ids.find(h);
  return it != end(ids) ? h : nullptr;
}

struct ObjMarker {
  HeapGraph& g_;
  PtrMap& ids_;
  int from_;
  explicit ObjMarker(HeapGraph& g, PtrMap& ids, const Header* h)
    : g_(g), ids_(ids), from_(ids_[h]) {
    assert(h);
  }
  void operator()(const void* p) {
    auto h = filter(ids_, p);
    assert(h && haveCount(h->kind()));
    addPtr(g_, from_, ids_[h], HeapGraph::Exact);
  }
  void ambig(const void* p) {
    if (auto h = filter(ids_, p)) {
      addPtr(g_, from_, ids_[h], HeapGraph::Ambiguous);
    }
  }
  void where(const char*) {}
};

struct RootMarker {
  HeapGraph& g_;
  PtrMap& ids_;
  const char* seat_{nullptr};
  explicit RootMarker(HeapGraph& g, PtrMap& ids): g_(g), ids_(ids) {}
  void operator()(const void* p) {
    auto h = filter(ids_, p);
    assert(h && haveCount(h->kind()));
    addRoot(g_, ids_[h], HeapGraph::Exact, seat_);
  }
  void ambig(const void* p) {
    if (auto h = filter(ids_, p)) {
      addRoot(g_, ids_[h], HeapGraph::Ambiguous, seat_);
    }
  }
  void where(const char* seat) {
    seat_ = seat;
  }
};

} // anon namespace

// Run a DFS over the heap, remember the first pointer id to each
// reachable node, aka its "parent". The tree formed by the parent
// edges is a spanning tree for the reachable nodes.
// Given a node, you can walk the parents towards roots to find out
// why the node is reachable. parent[k] == -1 for unreachable nodes.
std::vector<int> makeParentTree(const HeapGraph& g) {
  std::vector<int> parents(g.nodes.size(), -1);
  dfs_ptrs(g, g.roots, [&](int node, int ptr) {
    parents[node] = ptr;
  });
  return parents;
}

// parse the heap to find valid objects and initialize metadata, then
// add edges for every known root pointer and every known obj->obj ptr.
HeapGraph makeHeapGraph() {
  HeapGraph g;
  PtrMap ids;

  // parse the heap once to create nodes.
  MM().forEachHeader([&](Header* h) {
    addNode(g, ids, h);
    // NativeData and ResumableFrame headers wrap around an inner ObjectData,
    // which we also want to track.
    if (h->kind() == HeaderKind::NativeData) {
      addNode(g, ids, (Header*)h->nativeObj());
    } else if (h->kind() == HeaderKind::ResumableFrame) {
      addNode(g, ids, (Header*)h->resumableObj());
    }
  });

  // find roots
  PtrFilter<RootMarker> rmark(g, ids);
  scanRoots(rmark);

  // find heap->heap pointers
  for (size_t i = 0, n = g.nodes.size(); i < n; i++) {
    auto h = g.nodes[i].h;
    PtrFilter<ObjMarker> omark(g, ids, h);
    scanHeader(h, omark);
  }

  if (!g_context.isNull()) {
    // add a fake heap ptr from each obj with dyn props to its prop array
    for (const auto& e : g_context->dynPropTable) {
      auto obj = filter(ids, e.first); // ObjectData*
      auto props = filter(ids, e.second.arr().get()); // ArrayData*
      assert(obj && props);
      addPtr(g, ids[obj], ids[props], HeapGraph::DynProps);
    }
  }
  return g;
}

}
