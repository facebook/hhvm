/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/util/stacktrace-profiler.h"
#include "hphp/util/stack-trace.h"
#include <execinfo.h>
#include <algorithm>

namespace HPHP {

extern const bool enable_stacktrace_profiler =
  getenv("STACKTRACE_PROFILER") != nullptr;

StackTraceProfiler::StackTraceProfiler(std::string name, int skip) :
  m_name(name), finishing(false), m_root(nullptr), m_skip(skip) {
}

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

StackTraceSample::StackTraceSample() {
  if (enable_stacktrace_profiler) {
    depth = backtrace(addrs, kMaxDepth);
  } else {
    depth = 0;
  }
}

void StackTraceProfiler::count(const StackTraceSample& sample) {
  if (!enable_stacktrace_profiler || finishing) return;
  if (sample.depth <= 0) return;
  std::lock_guard<std::mutex> lock(m_mutex);
  m_root.hits++;
  Node* node = &m_root;
  for (int i = m_skip; i < sample.depth; i++) {
    Node* caller = findCaller(node, sample.addrs[i]);
    caller->hits++;
    node = caller;
  }
}

void StackTraceProfiler::count() {
  if (!enable_stacktrace_profiler || finishing) return;
  StackTraceSample sample;
  count(sample);
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
  fprintf(stderr, "%s%lu ", indent.c_str(), n->hits);
  if (n->addr) {
    std::string s = StackTrace::Translate(n->addr)->toString();
    fprintf(stderr, "%s\n", s.c_str());
  } else {
    fprintf(stderr, "%s\n", m_name.c_str());
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
  if (!enable_stacktrace_profiler) return;
  finishing = true;
  print(&m_root, "");
}

BoolProfiler::BoolProfiler(std::string name)
  : name(name)
  , p1(name + "=true", 2)
  , p0(name + "=false", 2)
{}

BoolProfiler::~BoolProfiler() {
  if (!enable_stacktrace_profiler) return;
  auto total = p0.hits() + p1.hits();
  if (total) {
    fprintf(stderr, "%s: total=%lu false=%.1f%% true=%.1f%%\n", name.c_str(),
            total,
            100.0 * p0.hits() / total,
            100.0 * p1.hits() / total);
  }
}

bool BoolProfiler::operator()(bool b) {
  (b ? &p1 : &p0)->count();
  return b;
}

IntProfiler::IntProfiler(std::string name)
  : name(name)
  , pN(name + ">=65", 2)
  , p64(name + "=33-64", 2)
  , p32(name + "=17-32", 2)
  , p16(name + "=9-16", 2)
  , p8(name + "=5-8", 2)
  , p4(name + "=3-4", 2)
  , p2(name + "=2", 2)
  , p1(name + "=1", 2)
  , p0(name + "=0", 2)
{}

IntProfiler::~IntProfiler() {
  if (!enable_stacktrace_profiler) return;
  auto total = p0.hits() + p1.hits() + p2.hits() + p4.hits() + p8.hits() +
               p16.hits() + p32.hits() + p64.hits() + pN.hits();
  if (total) {
    fprintf(stderr, "%s: total=%lu 0=%.1f%% 1=%.1f%% 2=%.1f%% "
            "3-4=%.1f%% 5-8=%.1f%% 9-16=%.1f%% 17-32=%.1f%% 33-64=%.1f%% "
            "65+=%.1f%%\n", name.c_str(), total,
            100.0 * p0.hits() / total,
            100.0 * p1.hits() / total,
            100.0 * p2.hits() / total,
            100.0 * p4.hits() / total,
            100.0 * p8.hits() / total,
            100.0 * p16.hits() / total,
            100.0 * p32.hits() / total,
            100.0 * p64.hits() / total,
            100.0 * pN.hits() / total);
  }
}

void IntProfiler::operator()(unsigned i) {
  (i == 0 ? &p0 :
   i == 1 ? &p1 :
   i == 2 ? &p2 :
   i <= 4 ? &p4 :
   i <= 8 ? &p8 :
   i <= 16 ? &p16 :
   i <= 32 ? &p32 :
   i <= 64 ? &p64 :
   &pN)->count();
}

}
