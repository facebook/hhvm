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
#include <util/util.h>
#include <runtime/base/execution_context.h>
#include <runtime/vm/translator/translator-x64.h>
#include <runtime/vm/translator/fixup.h>

namespace HPHP {
namespace VM {

namespace Transl {

bool FixupMap::RunningUnitTest = false;

/*
 * We've taken a few risks in the name of performance. In debug builds,
 * exercise a few million racy reads/writes to make sure we don't publish
 * partial TCAs.
 */
void* FixupMapUnitTest::writer(void* that) {
  // Give the readers time to wind up.
  usleep(1000);
  FixupMap* m = (FixupMap*)that;
  for (int i = 1; i < kNumToCheck; i++) {
    Fixup fxp(kPcMul * i, kSpMul * i);
    m->recordFixup(TCA(uintptr_t(i)), fxp);
  }
  return NULL;
}

void* FixupMapUnitTest::reader(void* that) {
  FixupMap* m = (FixupMap*)that;
  const StringData* sd = StringData::GetStaticString("test");
  // ar2: a mock actrec, requires a Func, which can't be builtin.
  Unit u;
  Func f(u, 1, 1, 1, 0, 0, sd, AttrNone, true, NULL, 0, false);
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
    ASSERT(vmr.m_pc == u.entry() + i * kPcMul);
    ASSERT(vmr.m_fp == &ar2);
    ASSERT(vmr.m_sp == (TypedValue*)&ar2 - i * kSpMul);
  }
  return NULL;
}

FixupMapUnitTest::FixupMapUnitTest() {
  if (!hhvm) return;

  // This unit test runs at the end of process init, and it
  // directly spawns pthreads that don't init various HPHP
  // thread locals.

  // We set the 'RunningUnitTest' flag to true while we run
  // the unit test to disable the logic that frees old blocks
  // and to disable some ASSERTs about the translator's write
  // lease.

  FixupMap::RunningUnitTest = true;

  FixupMap m;
  void* vm = (void*)&m;
  pthread_t threads[kNumReaders];
  for (int i = 0; i < kNumReaders; i++) {
    pthread_create(&threads[i], NULL, reader, vm);
  }
  writer(vm);
  for (int i = 0; i < kNumReaders; i++) {
    pthread_join(threads[i], NULL);
  }

  FixupMap::RunningUnitTest = false;
}

} } } // HPHP::VM::Transl
