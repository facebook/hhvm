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

#ifndef incl_HPHP_HEAP_GRAPH_H_
#define incl_HPHP_HEAP_GRAPH_H_

#include "hphp/util/type-scan.h"
#include <vector>
#include <cstdint>
#include <cstddef>

namespace HPHP {

struct HeapObject;

// Graph representation of the heap. The heap consists of some objects
// (Nodes), and directed pointers (Ptrs) from Node to Node. For each
// node, we maintain two singly linked lists:
//
// 1. a list of pointers from this node to other nodes (out-ptrs)
// 2. a list of pointers from other nodes to this node (in-ptrs).
//
// Each pointer is a member of both lists, except root pointers which
// are only a member of a node's in-ptr list.
//
// Additionally, each Ptr records it's from and to nodes. This allows
// traversing the heap graph in either direction (roots toward leaves,
// or vice-versa). However, each list is singly linked; the order of
// pointers in the least is meaningless and we don't need to support
// deleting pointers (or nodes).

struct HeapGraph {
  enum PtrKind : uint8_t {
    Counted, // exactly-marked, ref-counted, pointer
    Ambiguous, // any ambiguous pointer into a valid object
    Weak, // a weak pointer to a possibly dead object
  };
  static constexpr auto NumPtrKinds = 3;
  struct Node {
    union {
      const void* ptr;
      const HeapObject* h;
    };
    size_t size;
    bool is_root;
    type_scan::Index tyindex;
    int first_out;
    int first_in; // first out-ptr and in-ptr, respectively
  };
  struct Ptr {
    int from, to; // node ids. if root, from == -1
    int next_out, next_in; // from's next out-ptr, to's next in-ptr
    int offset; // byte offset of ptr within from node. (0 if unknown)
    PtrKind ptr_kind;
  };
  std::vector<Node> nodes;
  std::vector<Ptr> ptrs;
  std::vector<int> root_ptrs; // ptr ids
  std::vector<int> root_nodes; // node ids

  template<class F> void eachSuccNode(int n, F f) const {
    eachOutPtr(n, [&](int p) { f(ptrs[p].to); });
  }

  template<class F> void eachOutPtr(int n, F f) const {
    for (int p = nodes[n].first_out; p != -1; p = ptrs[p].next_out) {
      f(p);
    }
  }

  template<class F> void eachInPtr(int n, F f) const {
    for (int p = nodes[n].first_in; p != -1; p = ptrs[p].next_in) {
      f(p);
    }
  }

  template<class F> void eachPred(int n, F f) const {
    for (int p = nodes[n].first_in; p != -1; p = ptrs[p].next_in) {
      f(ptrs[p]);
    }
  }

  template<class F> void eachRootPtr(F f) const {
    for (auto p : root_ptrs) f(ptrs[p]);
  }
};

// Summary of heap cycles found in a HeapGraph
struct HeapCycles {
  using NodeList = std::vector<int>;
  std::vector<NodeList> live_cycles, leaked_cycles;
};

// Make a snapshot of the heap. It will contain pointers to objects
// in the heap so their properties or contents can be inspected.
// With great power comes great responsibility; if you invoke anything
// that frees or moves objects, pointers in this snapshot will be stale.
// if include_free is true, include free blocks, allowing dangling pointers
HeapGraph makeHeapGraph(bool include_free = false);

// Analyze the graph for cycles, then TRACE interesting things about cycles.
void printHeapReport(const HeapGraph&, const char* phase);

// Run a DFS over the heap, remember the first pointer id to each
// reachable node, aka its "parent". The tree formed by the parent
// edges is a spanning tree for the reachable nodes.
// Given a node, you can walk the parents towards roots to find out
// why the node is reachable. parent[k] == -1 for unreachable nodes.
std::vector<int> makeParentTree(const HeapGraph&);

// integrity check pointers and refcounts
bool checkPointers(const HeapGraph& g, const char* phase);

}

#endif
