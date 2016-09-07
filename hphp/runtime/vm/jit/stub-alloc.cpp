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

void FreeStubList::push(TCA stub) {
  /*
   * A freed stub may be released by Treadmill more than once if multiple
   * threads execute the service request before it is freed. We detect
   * duplicates by marking freed stubs
   */
  StubNode* n = reinterpret_cast<StubNode*>(stub);
  if (n->m_freed == kStubFree) {
    TRACE(1, "already freed stub %p\n", stub);
    return;
  }
  n->m_freed = kStubFree;
  n->m_next = m_list;
  TRACE(1, "free stub %p (-> %p)\n", stub, m_list);
  m_list = n;
}

TCA FreeStubList::maybePop() {
  StubNode* ret = m_list;
  if (ret) {
    TRACE(1, "alloc stub %p\n", ret);
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
    always_assert(s_freeStubs.peek() == nullptr ||
                  tc::isValidCodeAddress(s_freeStubs.peek()));
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
  s_freeStubs.push(stub);
}

}}
