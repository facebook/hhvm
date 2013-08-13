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

#ifndef incl_HPHP_PROBE_H
#define incl_HPHP_PROBE_H

#include <string>
#include <mutex>
#include <atomic>
#include "hphp/util/arena.h"

namespace HPHP {

/*
 * One single stack trace sample.  These can be generated, stored,
 * then passed to a StackTraceProfiler after the fact.
 */
struct StackTraceSample {
  static const int kMaxDepth = 10;
  StackTraceSample();
  int depth;
  void* addrs[kMaxDepth];
};

/*
 * A StackTraceProfiler incrementally constructs a prof-style caller tree from
 * the native backtrace each time its count() method is called, then dumps the
 * tree to stderr when destructed.
 *
 * Example usage to get two value profiles based on a bool:
 * void myTweakedFunction(bool personality=false) {
 *   static StackTraceProfiler prof_true("myCoolFunction-true");
 *   static StackTraceProfiler prof_false("myCoolFunction-false");
 *   (personality ? &prof_true : &prof_false)->count();
 * }
 *
 * Sometimes it is useful to disable inlining & tail-calls to make
 * stack traces easier to understand:
 *   -O2 -fno-inline -fno-optimize-sibling-calls
 */
class StackTraceProfiler {
  struct Node {
    explicit Node(void* addr) :
      addr(addr), callers(nullptr), next(nullptr), hits(0) {
    }
    void* const addr;
    Node* callers;
    Node* next;
    size_t hits;
  };

public:
  explicit StackTraceProfiler(std::string name, int skip = 1);
  ~StackTraceProfiler();
  StackTraceProfiler(const StackTraceProfiler& other) = delete;
  StackTraceProfiler& operator=(const StackTraceProfiler& other) = delete;

  // count one call in this probe.
  void count();
  void count(const StackTraceSample&);
  size_t hits() const { return m_root.hits; }

private:
  static bool compareNodes(Node* a, Node* b);
  Node* findCaller(Node* n, void* addr);
  Node* makeCaller(Node* n, void* addr);
  void print(Node* n, std::string indent);
  int numLeaves(Node* n);

private:
  Arena m_arena;
  const std::string m_name;
  std::mutex m_mutex;
  std::atomic<bool> finishing;
  Node m_root;
  int m_skip;
};

/*
 * BoolProfiler is used to collect profiled samples of a bool variable.
 */
struct BoolProfiler {
  explicit BoolProfiler(std::string name);
  ~BoolProfiler();
  bool operator()(bool b);
private:
  const std::string name;
  StackTraceProfiler p1, p0;
};

/*
 * IntProfiler is used to collect profiled samples of an unsiged variable.
 * A histogram of samples are collected in power-of-2 buckets up to 64.
 */
struct IntProfiler {
  explicit IntProfiler(std::string name);
  ~IntProfiler();
  void operator()(unsigned i);
private:
  const std::string name;
  StackTraceProfiler pN, p64, p32, p16, p8, p4, p2, p1, p0;
};

extern const bool enable_stacktrace_profiler;

}

#endif
