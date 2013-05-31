/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#include "hphp/util/stacktrace_profiler.h"
#include "hphp/util/stack_trace.h"
#include <execinfo.h>
#include <algorithm>

namespace HPHP {

const bool enable = getenv("STACKTRACE_PROFILER") != nullptr;
const int kMaxDepth = 10;

// Make a new caller node and link it into n's caller list.
StackTraceProfiler::Node* StackTraceProfiler::makeCaller(Node* n, void* addr) {
  Node* caller = new (m_arena) Node(addr);
  caller->next = n->callers;
  n->callers = caller;
  return caller;
}

// find a caller node with the given address, or make a new one.
// The new node is placed or moved to the front of n's caller list,
// in the hopes that the next search on n will be the same addr.
StackTraceProfiler::Node* StackTraceProfiler::findCaller(Node* n, void* addr) {
  if (Node* caller = n->callers) {
    if (caller->addr == addr) return caller;
    Node** prev = &caller->next;
    caller = caller->next;
    while (caller) {
      if (caller->addr == addr) {
        *prev = caller->next;
        caller->next = n->callers;
        return n->callers = caller;
      }
      prev = &caller->next;
      caller = caller->next;
    }
  }
  return makeCaller(n, addr);
}

void StackTraceProfiler::count() {
  if (!enable) return;
  void* addrs[kMaxDepth];
  auto count = backtrace(addrs, kMaxDepth);
  if (count <= 0) return;
  std::lock_guard<std::mutex> lock(m_mutex);
  m_root.hits++;
  Node* node = &m_root;
  for (int i = m_skip; i < count; i++) {
    Node* caller = findCaller(node, addrs[i]);
    caller->hits++;
    node = caller;
  }
}

bool StackTraceProfiler::compareNodes(Node* a, Node* b) {
  return a->hits > b->hits;
}

int StackTraceProfiler::numLeaves(Node* n) {
  if (!n->callers) return 1;
  int count = 0;
  for (Node* c = n->callers; c; c = c->next) count += numLeaves(c);
  return count;
}

void StackTraceProfiler::print(Node* n, std::string indent) {
  fprintf(stderr, "%s%d ", indent.c_str(), n->hits);
  if (n->addr) {
    std::string s = StackTrace::Translate(n->addr)->toString();
    fprintf(stderr, "%s\n", s.c_str());
  } else {
    fprintf(stderr, "%s\n", m_name);
  }
  if (numLeaves(n) <= 1) return;
  indent += "  ";
  std::vector<Node*> v;
  for (Node* caller = n->callers; caller; caller = caller->next) {
    v.push_back(caller);
  }
  std::sort(v.begin(), v.end(), compareNodes);
  for (auto i : v) {
    if (i->hits > n->hits / 100.0) print(i, indent);
  }
}

StackTraceProfiler::~StackTraceProfiler() {
  if (!enable) return;
  print(&m_root, "");
}

}
