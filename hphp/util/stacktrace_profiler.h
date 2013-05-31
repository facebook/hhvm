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

#ifndef incl_HPHP_PROBE_H
#define incl_HPHP_PROBE_H

#include <string>
#include <mutex>
#include "hphp/util/arena.h"

namespace HPHP {

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
    int hits;
  };

public:
  explicit StackTraceProfiler(const char* name, int skip = 1) :
    m_name(name), m_root(nullptr), m_skip(skip) {
  }
  ~StackTraceProfiler();
  StackTraceProfiler(const StackTraceProfiler& other) = delete;
  StackTraceProfiler& operator=(const StackTraceProfiler& other) = delete;

  // count one call in this probe.
  void count();

private:
  static bool compareNodes(Node* a, Node* b);
  Node* findCaller(Node* n, void* addr);
  Node* makeCaller(Node* n, void* addr);
  void print(Node* n, std::string indent);
  int numLeaves(Node* n);

private:
  Arena m_arena;
  const char* m_name;
  std::mutex m_mutex;
  Node m_root;
  int m_skip;
};

}

#endif
