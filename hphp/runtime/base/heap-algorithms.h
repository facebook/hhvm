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

#ifndef incl_HPHP_HEAP_ALGORITHMS_H_
#define incl_HPHP_HEAP_ALGORITHMS_H_

#include "hphp/runtime/base/heap-graph.h"
#include <folly/Range.h>
#include <boost/dynamic_bitset.hpp>
#include <vector>

namespace HPHP {
namespace detail {

// Recursive strongly connected components algorithm.
struct Scc {
  using range = folly::Range<std::vector<int>::iterator>;
  explicit Scc(const HeapGraph& g)
    : g_(g)
    , state(g.nodes.size(), State{-1, -1, false, false})
  {}

  // find SCCs reachable from roots
  template<class F> void visitRoots(F f) {
    g_.eachRoot([&](const HeapGraph::Ptr& ptr) {
      auto v = ptr.to;
      if (state[v].index == -1) visit(v, f);
    });
  }

  // find all SCCs
  template<class F> void visitAll(F f) {
    for (int v = 0; v < g_.nodes.size(); v++) {
      if (state[v].index == -1) visit(v, f);
    }
  }

private:
  const HeapGraph& g_;
  int index{0};
  struct State {
    int index, lowlink;
    bool onstack, selfptr;
  };
  std::vector<int> stack; // stack of node ids
  std::vector<State> state; // state for each node, indexed by node id

  // recursive visitor. Calls F on each SCC. "Trivial" SCCs are just single
  // nodes with no self pointers. "Interesting" SCCs are cycles involving two
  // or more nodes, or singleton nodes with at least one self pointer.
  // call f() on each interesting SCC.
  template<class F> void visit(int v, F f) {
    state[v] = {index, index, true, false};
    index++;
    stack.push_back(v);
    g_.eachSuccNode(v, [&](int w) {
      if (state[w].index == -1) {
        // successor w not yet visited; recurse on w.
        visit(w, f);
        state[v].lowlink = std::min(state[v].lowlink, state[w].lowlink);
      } else if (state[w].onstack) {
        // successor w is on stack, hence in current scc
        state[v].lowlink = std::min(state[v].lowlink, state[w].index);
        if (w == v) state[v].selfptr = true;
      }
    });
    // if v is a cycle-root, pop stack down to v & report scc
    if (state[v].lowlink == state[v].index) {
      auto e = end(stack), i = e;
      int w;
      do {
        w = *(--i);
        state[w].onstack = false;
      } while (w != v);
      auto cycle = range{i, e};
      if (cycle.size() > 1 || state[cycle[0]].selfptr) {
        f(cycle);
      }
      stack.erase(i, e);
    }
  }
};
}

using NodeRange = detail::Scc::range;

// Analyze a heap graph looking for cycles. Calls Live() if the cycle is
// reachable from a root, calls Leaked() otherwise. In both cases, the
// cycle is passed as a NodeRange whose underlying storage is invalidated
// when the Live or Leaked returns.
template<class Live, class Leaked>
void findHeapCycles(const HeapGraph& g, Live live, Leaked leaked) {
  detail::Scc scc(g);
  scc.visitRoots(live);
  scc.visitAll(leaked);
}

template<class Pre>
void dfs_nodes(
  const HeapGraph& g,
  const std::vector<int>& root_nodes,
  Pre pre
) {
  dfs_nodes(g, root_nodes, {}, pre);
}

// non-recursive depth-first-search over nodes, using a vector of
// nodes as the root set.
template<class Pre>
void dfs_nodes(
  const HeapGraph& g,
  const std::vector<int>& root_nodes,
  const std::vector<int>& skip_nodes,
  Pre pre
) {
  boost::dynamic_bitset<> marks(g.nodes.size());
  struct Action {
    enum { Start, Finish } cmd;
    int node;
  };
  std::vector<Action> work;
  for (auto r : root_nodes) work.push_back({Action::Start, r});
  for (auto s : skip_nodes) marks.set(s);
  while (!work.empty()) {
    auto cmd = work.back().cmd;
    auto n = work.back().node;
    work.pop_back();
    if (cmd == Action::Finish) continue;
    if (!marks.test(n)) {
      marks.set(n);
      pre(n);
      work.push_back({Action::Finish, n});
      g.eachSuccNode(n, [&](int to) {
        work.push_back({Action::Start, to});
      });
    }
  }
};
template<class Pre>
void dfs_ptrs(
  const HeapGraph& g,
  const std::vector<int>& root_ptrs,
  Pre pre
) {
  dfs_ptrs(g, root_ptrs, {}, pre);
}

// depth first search over nodes, using a vector of pointer ids as the
// root set (the "to" nodes are the effective root set).
template<class Pre>
void dfs_ptrs(
  const HeapGraph& g,
  const std::vector<int>& root_ptrs,
  const std::vector<int>& skip_nodes,
  Pre pre
) {
  boost::dynamic_bitset<> marks(g.nodes.size());
  struct Action {
    enum { Explore, Finish } cmd;
    int ptr;
  };
  std::vector<Action> work;
  for (auto r : root_ptrs) work.push_back({Action::Explore, r});
  for (auto s : skip_nodes) marks.set(s);
  while (!work.empty()) {
    auto cmd = work.back().cmd;
    auto ptr = work.back().ptr;
    work.pop_back();
    if (cmd == Action::Finish) continue;
    auto n = g.ptrs[ptr].to;
    if (!marks.test(n)) {
      marks.set(n);
      pre(n, ptr);
      work.push_back({Action::Finish, n});
      g.eachSuccPtr(n, [&](int p) {
        work.push_back({Action::Explore, p});
      });
    }
  }
};

template<class F>
void walkParents(const HeapGraph& g, const std::vector<int>& parents,
                 int n, F f) {
  auto p = parents[n];
  while (p != -1) {
    auto& ptr = g.ptrs[p];
    f(ptr);
    if (ptr.from != -1) {
      p = parents[ptr.from];
    } else {
      break;
    }
  }
}

}
#endif
