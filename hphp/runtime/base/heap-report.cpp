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

#include <sstream>

#include "hphp/runtime/base/heap-graph.h"
#include "hphp/runtime/base/heap-algorithms.h"
#include "hphp/runtime/base/header-kind.h"
#include "hphp/util/trace.h"
#include "hphp/util/assertions.h"
#include "hphp/runtime/base/container-functions.h"

TRACE_SET_MOD(heapreport)

namespace HPHP {
namespace {

size_t count_reached(const HeapGraph& g, const std::vector<int>& root_nodes) {
  size_t count{0};
  dfs_nodes(g, root_nodes, [&](int /*n*/) { count++; });
  return count;
}

// make a short string describing an object
DEBUG_ONLY std::string describe(const HeapGraph& g, int n) {
  std::ostringstream out;
  auto h = g.nodes[n].h;
  out << n;
  if (haveCount(h->kind())) {
    out << "#" << static_cast<const MaybeCountable*>(h)->count();
  }
  out << ":" << header_names[int(h->kind())];
  switch (h->kind()) {
    case HeaderKind::Dict:
    case HeaderKind::Vec:
    case HeaderKind::Keyset:
    case HeaderKind::BespokeVec:
    case HeaderKind::BespokeDict:
    case HeaderKind::BespokeKeyset:
      out << "[" << static_cast<const ArrayData*>(h)->size() << "]";
      break;
    case HeaderKind::String:
      out << "[" << static_cast<const StringData*>(h)->size() << "]";
      break;
    case HeaderKind::RFunc: {
      auto const rfunc = static_cast<const RFuncData*>(h);
      out << ":" << rfunc->m_func->name()->data()
          << "[" << rfunc->m_arr->size() << "]";
      break;
    }
    case HeaderKind::Resource:
    case HeaderKind::ClsMeth:
      break;
    case HeaderKind::RClsMeth: {
      auto const rclsmeth = static_cast<const RClsMethData*>(h);
      out << ":" << rclsmeth->m_cls->name()->data()
          << "::" << rclsmeth->m_func->name()->data()
          << "[" << rclsmeth->m_arr->size() << "]";
      break;
    }
    case HeaderKind::Object:
    case HeaderKind::NativeObject:
    case HeaderKind::Closure:
    case HeaderKind::WaitHandle:
    case HeaderKind::AsyncFuncWH:
    case HeaderKind::AwaitAllWH:
    case HeaderKind::ConcurrentWH:
      out << ":" << static_cast<const ObjectData*>(h)->classname_cstr();
      break;
    case HeaderKind::Vector:
    case HeaderKind::Map:
    case HeaderKind::Set:
    case HeaderKind::Pair:
    case HeaderKind::ImmVector:
    case HeaderKind::ImmMap:
    case HeaderKind::ImmSet: {
      auto obj = const_cast<ObjectData*>(
          static_cast<const ObjectData*>(h)
      );
      out << "[" << getContainerSize(make_tv<KindOfObject>(obj)) << "]";
      break;
    }
    case HeaderKind::Cpp:
    case HeaderKind::BigMalloc:
    case HeaderKind::SmallMalloc:
      out << "[" << static_cast<const MallocNode*>(h)->nbytes << "]";
      break;
    case HeaderKind::AsyncFuncFrame:
    case HeaderKind::NativeData:
    case HeaderKind::ClosureHdr:
    case HeaderKind::MemoData:
      break;
    case HeaderKind::Free:
      out << "[" << static_cast<const FreeNode*>(h)->size() << "]";
      break;
    case HeaderKind::Slab:
    case HeaderKind::Hole:
      not_reached();
  }
  out << " " << (void*)h;
  return out.str();
}

const char* ptrSym[] = {
    "<--", // Counted
    "<~~", // Ambiguous
};

DEBUG_ONLY
std::string describePtr(const HeapGraph& g, const HeapGraph::Ptr& ptr) {
  std::ostringstream out;
  out << " " << ptrSym[(unsigned)ptr.ptr_kind];
  auto& from = g.nodes[ptr.from];
  if (!from.is_root) out << describe(g, ptr.from);
  else out << type_scan::getName(from.tyindex);
  return out.str();
}

void reportUndead(const HeapGraph& g, int node) {
  TRACE(3, "HG: undead object %s\n",
        describe(g, node).c_str());
  g.eachPred(node, [&](const HeapGraph::Ptr& ptr) {
    TRACE(3, "  %s\n", describePtr(g, ptr).c_str());
  });
}

// print path from root to n
void reportPathFromRoot(const HeapGraph& g, std::vector<int>& parents, int n) {
  TRACE(2, "  %s", describe(g, n).c_str());
  walkParents(g, parents, n, [&](const HeapGraph::Ptr& ptr) {
    TRACE(2, "%s", describePtr(g, ptr).c_str());
  });
  TRACE(2, "\n");
}
} // anon namespace

void printHeapReport(const HeapGraph& g, const char* phase) {
  TRACE(2, "HG: printHeapReport %s\n", phase);
  size_t allocd = 0; // non-free nodes in the heap
  size_t freed = 0; // free in the heap
  size_t live = 0; // non-free reachable nodes
  size_t undead = 0; // free but still reachable
  auto count = [&](int n, size_t& c1, size_t& c2) {
    if (g.nodes[n].h->kind() != HeaderKind::Free) c1++;
    else c2++;
  };
  for (int i = 0; i < g.nodes.size(); i++) {
    count(i, allocd, freed);
  }
  std::vector<int> parents(g.nodes.size(), -1);
  dfs_ptrs(g, g.root_ptrs, [&](int node, int ptr) {
    parents[node] = ptr;
    count(node, live, undead);
    auto h = g.nodes[node].h;
    if (h->kind() == HeaderKind::Free) {
      reportUndead(g, node);
    }
  });
  TRACE(2, "HG: allocd %lu freed %lu live %lu undead %lu leaked %lu\n",
        allocd, freed, live, undead, allocd-live);
  auto report_cycle = [&](const char* kind, NodeRange cycle) {
    TRACE(2, "HG: %s cycle of %lu nodes:\n", kind, cycle.size());
    reportPathFromRoot(g, parents, cycle[0]);
    for (size_t i = 1; i < cycle.size(); ++i) {
      TRACE(2, "  %s\n", describe(g, cycle[i]).c_str());
    }
  };
  // Cycle analysis:
  std::vector<int> live_cycle_nodes, leaked_cycle_nodes;
  size_t num_live_cycles{0}, num_leaked_cycles{0};
  findHeapCycles(g,
    /* live */ [&](NodeRange cycle) {
      report_cycle("live", cycle);
      num_live_cycles++;
      live_cycle_nodes.insert(live_cycle_nodes.end(),
                              cycle.begin(), cycle.end());
    },
    /* leaked */ [&](NodeRange cycle) {
      report_cycle("leaked", cycle);
      num_leaked_cycles++;
      leaked_cycle_nodes.insert(leaked_cycle_nodes.end(),
                                cycle.begin(), cycle.end());
    });
  if (!live_cycle_nodes.empty()) {
    DEBUG_ONLY auto reached = count_reached(g, live_cycle_nodes);
    TRACE(2, "HG: %ld live in %lu cycles hold %lu/%lu(%.0f)%% "
          "of live objects\n",
          live_cycle_nodes.size(), num_live_cycles, reached, live,
          100.0*reached/live);
  } else {
    TRACE(2, "HG: no live cycles found\n");
  }
  if (!leaked_cycle_nodes.empty()) {
    DEBUG_ONLY auto leaked = allocd - live;
    DEBUG_ONLY auto reached = count_reached(g, leaked_cycle_nodes);
    TRACE(2, "HG: %ld leaked in %lu cycles hold %lu/%lu(%.0f)%%"
          "of leaked objects\n",
          leaked_cycle_nodes.size(), num_leaked_cycles, reached, leaked,
          100.0*reached/leaked);
  } else {
    TRACE(2, "HG: no leaked cycles found.\n");
  }
}

static void traceToRoot(const HeapGraph& g, int n, const std::string& ind) {
  const std::string indent(ind + " ");
  if (indent.length() > 200) {
    TRACE(1, "%s ## level limit exceeded ##\n", indent.c_str());
    return;
  }
  g.eachPred(n, [&](const HeapGraph::Ptr& ptr) {
    TRACE(1, "%s%s\n", indent.c_str(), describePtr(g, ptr).c_str());
    if (ptr.from == -1) return;
    if (ptr.ptr_kind != HeapGraph::Counted) {
      TRACE(1, "%s   ## only tracing ref-counted references ##\n",
            indent.c_str());
    } else {
      traceToRoot(g, ptr.from, indent);
    }
  });
}

bool checkPointers(const HeapGraph& g, const char* phase) {
  UNUSED auto found_dangling = false;
  for (size_t n = 0; n < g.nodes.size(); ++n) {
    auto& node = g.nodes[n];
    if (!haveCount(node.h->kind())) continue;
    auto count = static_cast<const MaybeCountable*>(node.h)->count();
    assertx(count >= 0); // static things shouldn't be in the heap.
    unsigned num_counted{0}, num_ambig{0};
    g.eachPred(n, [&](const HeapGraph::Ptr& ptr) {
      switch (ptr.ptr_kind) {
        case HeapGraph::Counted: num_counted++; break;
        case HeapGraph::Ambiguous: num_ambig++; break;
        case HeapGraph::Weak: break;
      }
    });
    auto num_ptrs = num_counted + num_ambig;
    if (num_ptrs < count) {
      // missed at least one counted pointer, or refcount too high
      // no assert, because without gc the only effect is a leak.
      TRACE(1, "HG: %s missing %d pointers to %s\n", phase, count - num_ptrs,
            describe(g, n).c_str());
      g.eachPred(n, [&](const HeapGraph::Ptr& ptr) {
        TRACE(1, "  %s\n", describePtr(g, ptr).c_str());
      });
    } else if (num_counted > count) {
      // traced too many exact ptrs. buggy scan() or refcount is too low.
      found_dangling = true;
      TRACE(1, "HG: %s dangling %d pointers to %s\n",
            phase, num_counted - count, describe(g, n).c_str());
      traceToRoot(g, n, "");
    }
  }
  assertx(!found_dangling && "found dangling pointers");
  return true;
}

}
