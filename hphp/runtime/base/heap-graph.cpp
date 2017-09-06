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
#include "hphp/runtime/base/heap-graph.h"
#include "hphp/runtime/base/heap-algorithms.h"
#include "hphp/runtime/base/types.h"
#include "hphp/runtime/base/req-containers.h"
#include "hphp/runtime/base/memory-manager-defs.h"
#include "hphp/runtime/base/heap-scan.h"
#include "hphp/runtime/base/thread-info.h"
#include "hphp/runtime/base/container-functions.h"
#include "hphp/runtime/base/tv-mutate.h"
#include "hphp/runtime/base/tv-variant.h"
#include "hphp/util/alloc.h"
#include "hphp/util/ptr-map.h"

#include <vector>
#include <folly/Range.h>

namespace HPHP {

template<class Fn> void FOLLY_DISABLE_ADDRESS_SANITIZER
conservativeScan(const void* start, size_t len, Fn fn) {
  const uintptr_t M{7}; // word size - 1
  auto s = (const void**)((uintptr_t(start) + M) & ~M); // round up
  auto e = (const void**)((uintptr_t(start) + len) & ~M); // round down
  for (; s < e; s++) {
    // Mask off the upper 16-bits to handle things like
    // DiscriminatedPtr which stores things up there.
    fn(s, (const void*)(uintptr_t(*s) & (-1ULL >> 16)));
  }
}

namespace {

size_t addPtr(HeapGraph& g, int from, int to, HeapGraph::PtrKind kind,
            ptrdiff_t offset) {
  auto& from_node = g.nodes[from];
  auto& to_node = g.nodes[to];
  auto e = g.ptrs.size();
  g.ptrs.push_back(
    HeapGraph::Ptr{from, to, from_node.first_out, to_node.first_in,
                   (int)offset, kind}
  );
  from_node.first_out = to_node.first_in = e;
  return e;
}

void addRootNode(HeapGraph& g, const PtrMap<const HeapObject*>& blocks,
                 type_scan::Scanner& scanner,
                 const void* h, size_t size, type_scan::Index ty) {
  auto from = g.nodes.size();
  g.nodes.push_back(
    HeapGraph::Node{h, size, true, ty, -1, -1}
  );
  g.root_nodes.push_back(from);
  scanner.scanByIndex(ty, h, size);
  scanner.finish(
    [&](const void* p, std::size_t size) {
      conservativeScan(p, size, [&](const void** addr, const void* ptr) {
        if (auto r = blocks.region(ptr)) {
          auto to = blocks.index(r);
          auto offset = uintptr_t(addr) - uintptr_t(h);
          auto e = addPtr(g, from, to, HeapGraph::Ambiguous, offset);
          g.root_ptrs.push_back(e);
        }
      });
    },
    [&](const void** addr) {
      if (auto r = blocks.region(*addr)) {
        auto to = blocks.index(r);
        auto offset = uintptr_t(addr) - uintptr_t(h);
        auto e = addPtr(g, from, to, HeapGraph::Counted, offset);
        g.root_ptrs.push_back(e);
      }
    }
  );
}

} // anon namespace

// Run a DFS over the heap, remember the first pointer id to each
// reachable node, aka its "parent". The tree formed by the parent
// edges is a spanning tree for the reachable nodes.
// Given a node, you can walk the parents towards roots to find out
// why the node is reachable. parent[k] == -1 for unreachable nodes.
std::vector<int> makeParentTree(const HeapGraph& g) {
  std::vector<int> parents(g.nodes.size(), -1);
  dfs_ptrs(g, g.root_ptrs, [&](int node, int ptr) {
    parents[node] = ptr;
  });
  return parents;
}

// parse the heap to find valid objects and initialize metadata, then
// add edges for every known root pointer and every known obj->obj ptr.
HeapGraph makeHeapGraph(bool include_free) {
  HeapGraph g;
  PtrMap<const HeapObject*> blocks;

  // parse the heap once to create a PtrMap for pointer filtering. Create
  // one node for every parsed block, including NativeData and AsyncFuncFrame
  // blocks. Only include free blocks if requested.
  MM().forEachHeapObject([&](HeapObject* h, size_t alloc_size) {
    if (h->kind() != HeaderKind::Free || include_free) {
      blocks.insert(h, alloc_size); // adds interval [h, h+alloc_size[
    }
  });
  blocks.prepare();

  // initialize nodes by iterating over PtrMap's regions
  g.nodes.reserve(blocks.size());
  blocks.iterate([&](const HeapObject* h, size_t size) {
    type_scan::Index ty;
    switch (h->kind()) {
      case HeaderKind::NativeData:
        ty = static_cast<const NativeNode*>(h)->typeIndex();
        break;
      case HeaderKind::Resource:
        ty = static_cast<const ResourceHdr*>(h)->typeIndex();
        break;
      case HeaderKind::SmallMalloc:
      case HeaderKind::BigMalloc:
        ty = static_cast<const MallocNode*>(h)->typeIndex();
        break;
      default:
        ty = type_scan::kIndexUnknown;
        break;
    }
    g.nodes.push_back(
      HeapGraph::Node{h, size, false, ty, -1, -1}
    );
  });

  // find root nodes
  type_scan::Scanner scanner;
  iterateRoots([&](const void* h, size_t size, type_scan::Index tyindex) {
    // it's important that we actually scan each root node before
    // returning, since at least one will be the C++ stack, and some
    // nodes will only exist for the duration of the call to this lambda,
    // for example EphemeralPtrWrapper<T>.
    addRootNode(g, blocks, scanner, h, size, tyindex);
  });

  // find heap->heap pointers
  for (size_t i = 0, n = g.nodes.size(); i < n; i++) {
    if (g.nodes[i].is_root) continue;
    auto h = g.nodes[i].h;
    scanHeapObject(h, scanner);
    auto from = blocks.index(h);
    assert(from == i);
    scanner.finish(
      [&](const void* p, std::size_t size) {
        conservativeScan(p, size, [&](const void** addr, const void* ptr) {
          if (auto r = blocks.region(ptr)) {
            auto to = blocks.index(r);
            auto offset = uintptr_t(addr) - uintptr_t(h);
            addPtr(g, from, to, HeapGraph::Ambiguous, offset);
          }
        });
      },
      [&](const void** addr) {
        if (auto r = blocks.region(*addr)) {
          auto to = blocks.index(r);
          auto offset = uintptr_t(addr) - uintptr_t(h);
          addPtr(g, from, to, HeapGraph::Counted, offset);
        }
      }
    );
  }
  g.nodes.shrink_to_fit();
  g.ptrs.shrink_to_fit();
  g.root_ptrs.shrink_to_fit();
  g.root_nodes.shrink_to_fit();
  return g;
}

}
