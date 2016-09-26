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
#include <folly/Range.h>

namespace HPHP {

const char* root_kind_names[] = {
  "NotARoot",
  "RdsNormal",
  "RdsLocal",
  "RdsPersistent",
  "PhpStack",
  "ExecutionContext",
  "ThreadInfo",
  "CppStack",
  "CppTls",
  "ThreadLocalManager",
  "Extensions",
  "RootMaps",
  "SweepLists",
  "AsioSession",
  "GetServerNote",
  "EzcResources"
};

namespace {
template<class F>
struct PtrFilter: F {
  template <class... Args> explicit PtrFilter(Args&&... args)
    : F(std::forward<Args>(args)...)
  {
    rds_ = folly::Range<const char*>((char*)rds::header(),
                                   RuntimeOption::EvalJitTargetCacheSize);
  }

  // classes containing counted pointers
  void operator()(const String& p) { (*this)(p.get()); }
  void operator()(const Array& p) { (*this)(p.get()); }
  void operator()(const ArrayNoDtor& p) { (*this)(p.arr()); }
  void operator()(const Object& p) { (*this)(p.get()); }
  void operator()(const Resource& p) { (*this)(p.hdr()->data()); }
  void operator()(const Variant& p) { (*this)(*p.asTypedValue()); }
  void operator()(const StringBuffer& p) { p.scan(*this); }
  void operator()(const NameValueTable& p) { p.scan(*this); }
  void operator()(const VarEnv& venv) { (*this)(&venv); }

  void operator()(const RequestEventHandler& p) { p.scan(*this); }
  void operator()(const RequestEventHandler* p) { p->scan(*this); }

  // mark a TypedValue or TypedValueAux. taking tv by value would exclude aux.
  void operator()(const TypedValueAux& v) { (*this)(*(const TypedValue*)&v); }
  void operator()(const TypedValue& tv) {
    switch (tv.m_type) {
      case KindOfString: // never null, sometimes counted
        if (tv.m_data.pstr->isRefCounted()) F::counted(tv.m_data.pstr);
        break;
      case KindOfVec:
      case KindOfDict:
      case KindOfKeyset:
      case KindOfArray: // never null, sometimes counted
        if (tv.m_data.parr->isRefCounted()) F::counted(tv.m_data.parr);
        break;
      case KindOfRef: // sometimes points into rds
        if (!inRds(tv.m_data.pref)) F::counted(tv.m_data.pref);
        break;
      case KindOfObject:    return F::counted(tv.m_data.pobj);
      case KindOfResource:  return F::counted(tv.m_data.pres);
      case KindOfUninit:
      case KindOfNull:
      case KindOfBoolean:
      case KindOfInt64:
      case KindOfDouble:
      case KindOfPersistentString:
      case KindOfPersistentVec:
      case KindOfPersistentDict:
      case KindOfPersistentKeyset:
      case KindOfPersistentArray:
      case KindOfClass: // only in eval stack
        return;
    }
  }

  // todo: implement me
  void operator()(const ActRec&) {}
  void operator()(const Stack&) {}

  void operator()(const StringData* p) {
    if (p && p->isRefCounted()) F::counted(p);
  }
  void operator()(const ArrayData* p) {
    if (p && p->isRefCounted()) F::counted(p);
  }
  void operator()(const ObjectData* p) { if (p) F::counted(p); }
  void operator()(const ResourceData* p) { if (p) F::counted(p->hdr()); }
  void operator()(const ResourceHdr* p) { if (p) F::counted(p); }
  void operator()(const RefData* p) { if (p && !inRds(p)) F::counted(p); }

  void implicit(const ObjectData* p) { if (p) F::implicit(p); }
  void implicit(const ResourceHdr* p) { if (p) F::implicit(p); }
  void implicit(const Array& p) {
    if (!p.isNull() && p.get()->isRefCounted()) F::implicit(p.get());
  }

  // collections of scannable fields
  template<class T> void operator()(const req::vector<T>& c) {
    for (auto& e : c) (*this)(e);
  }
  template<class T> void operator()(const req::set<T>& c) {
    for (auto& e : c) (*this)(e);
  }
  template<class T> void implicit(const req::set<T>& c) {
    for (auto& e : c) implicit(e);
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
  void FOLLY_DISABLE_ADDRESS_SANITIZER
  operator()(const void* start, size_t len) {
    const uintptr_t M{7}; // word size - 1
    auto s = (char**)((uintptr_t(start) + M) & ~M); // round up
    auto e = (char**)((uintptr_t(start) + len) & ~M); // round down
    for (; s < e; s++) F::ambig(*s);
  }

  void operator()(const void* start, const void* end) {
    assert(uintptr_t(end) >= uintptr_t(start));
    return (*this)(start, uintptr_t(end) - uintptr_t(start));
  }

 private:
  bool inRds(const void* vp) {
    auto p = reinterpret_cast<const char*>(vp);
    return p >= rds_.begin() && p < rds_.end();
  }

 private:
  folly::Range<const char*> rds_; // full mmap'd rds section.
};

void addPtr(HeapGraph& g, int from, int to, HeapGraph::PtrKind kind) {
  auto& from_node = g.nodes[from];
  auto& to_node = g.nodes[to];
  auto e = g.ptrs.size();
  g.ptrs.push_back(
    HeapGraph::Ptr{from, to, from_node.first_out, to_node.first_in, kind}
  );
  from_node.first_out = to_node.first_in = e;
}

void addRoot(HeapGraph& g, int to, HeapGraph::PtrKind ptr_kind,
             RootKind root_kind) {
  auto& to_node = g.nodes[to];
  auto e = g.ptrs.size();
  g.ptrs.push_back(
    HeapGraph::Ptr{-1, to, -1, to_node.first_in, ptr_kind, root_kind}
  );
  to_node.first_in = e;
  g.roots.push_back(e);
}

struct ObjMarker {
  explicit ObjMarker(HeapGraph& g, PtrMap& blocks, const Header* h)
    : g_(g), blocks_(blocks), from_(blocks.index(h)) {
    assert(h);
  }
  void counted(const void* p) { mark(p, HeapGraph::Counted); }
  void implicit(const void* p) { mark(p, HeapGraph::Implicit); }
  void ambig(const void* p) {
    if (auto r = blocks_.region(p)) {
      addPtr(g_, from_, blocks_.index(r), HeapGraph::Ambiguous);
    }
  }
  void where(RootKind) {}

 private:
  void mark(const void* p, HeapGraph::PtrKind kind) {
    assert(blocks_.header(p));
    auto r = blocks_.region(p);
    addPtr(g_, from_, blocks_.index(r), kind);
  }

 private:
  HeapGraph& g_;
  PtrMap& blocks_;
  int from_;
};

struct RootMarker {
  explicit RootMarker(HeapGraph& g, PtrMap& blocks)
    : g_(g), blocks_(blocks), root_kind_(RootKind::NotARoot)
  {}
  void counted(const void* p) { mark(p, HeapGraph::Counted); }
  void implicit(const void* p) { mark(p, HeapGraph::Implicit); }
  void ambig(const void* p) {
    if (auto r = blocks_.region(p)) {
      addRoot(g_, blocks_.index(r), HeapGraph::Ambiguous, root_kind_);
    }
  }
  void where(RootKind k) {
    root_kind_ = k;
  }

 private:
  void mark(const void* p, HeapGraph::PtrKind kind) {
    assert(blocks_.header(p));
    auto r = blocks_.region(p);
    addRoot(g_, blocks_.index(r), kind, root_kind_);
  }

 private:
  HeapGraph& g_;
  PtrMap& blocks_;
  RootKind root_kind_;
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
HeapGraph makeHeapGraph(bool include_free) {
  HeapGraph g;
  PtrMap blocks;

  // parse the heap once to create a PtrMap for pointer filtering. Create
  // one node for every parsed block, including NativeData and AsyncFuncFrame
  // blocks. Only include free blocks if requested.
  MM().forEachHeader([&](Header* h) {
    if (h->kind() != HeaderKind::Free || include_free) {
      blocks.insert(h); // adds interval [h, h+h->size[
    }
  });
  blocks.prepare();

  // initialize nodes by iterating over PtrMap's regions
  g.nodes.reserve(blocks.size());
  blocks.iterate([&](const Header* h, size_t size) {
    g.nodes.push_back(HeapGraph::Node{h, -1, -1});
  });

  // find roots
  PtrFilter<RootMarker> rmark(g, blocks);
  scanRoots(rmark);

  // find heap->heap pointers
  for (size_t i = 0, n = g.nodes.size(); i < n; i++) {
    auto h = g.nodes[i].h;
    PtrFilter<ObjMarker> omark(g, blocks, h);
    scanHeader(h, omark);
  }
  g.nodes.shrink_to_fit();
  g.ptrs.shrink_to_fit();
  g.roots.shrink_to_fit();
  return g;
}

}
