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

#include "hphp/runtime/vm/jit/stub-alloc.h"

#include "hphp/runtime/base/stats.h"
#include "hphp/runtime/vm/debug/debug.h"
#include "hphp/runtime/vm/jit/cg-meta.h"
#include "hphp/runtime/vm/jit/code-cache.h"
#include "hphp/runtime/vm/jit/tc.h"

TRACE_SET_MOD(mcg);

namespace HPHP { namespace jit {

namespace {

struct FreeStubList {
  struct StubNode {
    StubNode* m_next;
    uint64_t  m_freed;
  };
  static const uint64_t kStubFree = 0;
  FreeStubList() : m_list(nullptr) {}
  TCA peek() { return (TCA)m_list; }
  TCA maybePop();
  void push(TCA stub);
private:
  StubNode* m_list;
};

std::mutex s_stubLock;

void FreeStubList::push(TCA stub) {
  always_assert(tc::isValidCodeAddress(stub));
  std::unique_lock<std::mutex> l(s_stubLock);

  StubNode* n = reinterpret_cast<StubNode*>(stub);
  /*
   * We should never try to free a stub twice, but if the code to do
   * so is not careful, two threads running through the stub at the
   * same time could both think they need to smash it, and treadmill
   * the stub. We use the m_freed field to opportunistically catch
   * that kind of error (its still possible that its pushed, then
   * popped, scribbled over and then pushed again).
   */
  always_assert(n->m_freed != kStubFree &&
                n->m_freed != ~kStubFree);
  n->m_freed = kStubFree;
  n->m_next = m_list;
  TRACE(1, "free stub %p (-> %p)\n", stub, m_list);
  m_list = n;
}

TCA FreeStubList::maybePop() {
  std::unique_lock<std::mutex> l(s_stubLock);

  StubNode* ret = m_list;
  if (ret) {
    TRACE(1, "alloc stub %p\n", ret);
    always_assert(ret->m_next == nullptr ||
                  tc::isValidCodeAddress((TCA)ret->m_next));
    m_list = ret->m_next;
    ret->m_freed = ~kStubFree;
  }
  return (TCA)ret;
}

FreeStubList s_freeStubs;

////////////////////////////////////////////////////////////////////////////////
}

std::set<TCA> getFreeTCStubs() {
  std::set<TCA> deadStubs;
  auto stub = (FreeStubList::StubNode*)s_freeStubs.peek();
  while (stub) {
    deadStubs.insert((TCA)stub);
    stub = stub->m_next;
  }
  return deadStubs;
}

TCA allocTCStub(CodeBlock& frozen, CGMeta* fixups, bool* isReused) {
  TCA ret = s_freeStubs.maybePop();
  if (isReused) *isReused = ret;

  if (ret) {
    Stats::inc(Stats::Astub_Reused);
    TRACE(1, "recycle stub %p\n", ret);
  } else {
    ret = frozen.frontier();
    Stats::inc(Stats::Astub_New);
    TRACE(1, "alloc new stub %p\n", ret);
  }

  if (fixups) {
    fixups->reusedStubs.emplace_back(ret);
  }
  return ret;
}

void markStubFreed(TCA stub) {
  Debug::DebugInfo::Get()->recordRelocMap(stub, 0, "FreeStub");
  s_freeStubs.push(stub);
}

}}
