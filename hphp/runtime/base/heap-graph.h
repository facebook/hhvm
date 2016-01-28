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

#ifndef incl_HPHP_HEAP_GRAPH_H_
#define incl_HPHP_HEAP_GRAPH_H_

#include <vector>

namespace HPHP {

struct Header;

struct HeapGraph {
  enum PtrKind {
    Counted, // exactly-marked, ref-counted, pointer
    Implicit, // exactly-marked but not counted
    Ambiguous, // any ambiguous pointer into a valid object
  };
  struct Node {
    const Header* h;
    int succ, pred;
  };
  struct Ptr {
    int from, to, succ, pred; // if root, from == -1
    PtrKind kind;
    const char* seat;
  };
  std::vector<Node> nodes;
  std::vector<Ptr> ptrs;
  std::vector<int> roots; // ptr ids. ptr.from = -1, ptr.to = object

  template<class F> void eachSuccNode(int n, F f) const {
    eachSuccPtr(n, [&](int p) { f(ptrs[p].to); });
  }

  template<class F> void eachSuccPtr(int n, F f) const {
    for (int p = nodes[n].succ; p != -1; p = ptrs[p].succ) f(p);
  }

  template<class F> void eachPredPtr(int n, F f) const {
    for (int p = nodes[n].pred; p != -1; p = ptrs[p].pred) f(p);
  }

  template<class F> void eachPred(int n, F f) const {
    for (int p = nodes[n].pred; p != -1; p = ptrs[p].pred) f(ptrs[p]);
  }

  template<class F> void eachRoot(F f) const {
    for (auto p : roots) f(ptrs[p]);
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
HeapGraph makeHeapGraph();

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
