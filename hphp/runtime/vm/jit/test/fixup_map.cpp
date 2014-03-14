/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/runtime/vm/jit/fixup.h"

#include <gtest/gtest.h>
#include "hphp/hhvm/process-init.h"

namespace HPHP { namespace JIT {

//////////////////////////////////////////////////////////////////////

namespace {

constexpr int kNumReaders = 7;
constexpr int kNumToCheck = 50 * 1000;
constexpr int kSpMul = 13;
constexpr int kPcMul = 17;

void* writer(void* that) {
  // Give the readers time to wind up.
  usleep(1000);
  FixupMap* m = (FixupMap*)that;
  for (int i = 1; i < kNumToCheck; i++) {
    Fixup fxp(kPcMul * i, kSpMul * i);
    m->recordFixup(TCA(uintptr_t(i)), fxp);
  }
  return nullptr;
}

void* reader(void* that) {
  FixupMap* m = (FixupMap*)that;
  const StringData* sd = makeStaticString("test");
  // ar2: a mock actrec, requires a Func, which can't be builtin.
  Unit u;
  Func f(u, 1, nullptr, 1, 1, 0, 0, sd, AttrNone, true, nullptr, 0);
  ActRec ar2;
  ar2.m_savedRip = 0xdeadbeef;
  ar2.m_savedRbp = 0;
  ar2.m_func = &f;
  for (int i = 1; i < kNumToCheck; i++) {
    // ar1(i) -> ar2(DONTCARE) -> NULL
    ActRec ar1;
    ar1.m_savedRbp = (uint64_t)&ar2;
    ar1.m_savedRip = i;
    FixupMap::VMRegs vmr = { 0, 0, 0 };
    while (!m->getFrameRegs(&ar1, 0, &vmr)) {
      // Has to succeed eventually.
    }
    assert(vmr.m_pc == reinterpret_cast<const Op*>(u.entry() + i * kPcMul));
    assert(vmr.m_fp == &ar2);
    assert(vmr.m_sp == (TypedValue*)&ar2 - i * kSpMul);
  }
  return nullptr;
}

}

//////////////////////////////////////////////////////////////////////

/*
 * Hammer a fixup map with a bunch of concurrent readers and hope it
 * works.
 */
TEST(FixupMap, Concurrent) {
  FixupMap m;
  void* vm = (void*)&m;
  pthread_t threads[kNumReaders];
  for (int i = 0; i < kNumReaders; i++) {
    pthread_create(&threads[i], nullptr, reader, vm);
  }
  writer(vm);
  for (int i = 0; i < kNumReaders; i++) {
    pthread_join(threads[i], nullptr);
  }
}

//////////////////////////////////////////////////////////////////////

}}
