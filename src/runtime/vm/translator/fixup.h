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

#ifndef incl_FIXUP_H_
#define incl_FIXUP_H_

#include <util/util.h>
#include <runtime/vm/translator/types.h>
#include <runtime/base/execution_context.h>
#include "runtime/vm/tread_hash_map.h"
#include "runtime/vm/translator/types.h"
#include "util/atomic.h"

namespace HPHP {

class ExecutionContext;

namespace VM { namespace Transl {

/*
 * The Fixup map allows us to reconstruct the state of the VM registers
 * from an up-stack invocation record. Each range of bytes in the
 * translation cache is associated with a "distance" in both stack cells
 * and opcode bytes from the beginning of the function. These are
 * statically known at translation time.
 *
 */
struct Fixup {
  int32 m_pcOffset;
  int32 m_spOffset;

  Fixup(int32_t pcOff, int32_t spOff) : m_pcOffset(pcOff), m_spOffset(spOff)
  {
    ASSERT(m_pcOffset >= 0);
    ASSERT(m_spOffset >= 0);
  }
  Fixup() : m_pcOffset(-1), m_spOffset(-1) { }
};

class FixupMap {
  static const uint kInitCapac = 128;
  TRACE_SET_MOD(fixup);

public:
  struct VMRegs {
    const Opcode* m_pc;
    TypedValue* m_sp;
    const ActRec* m_fp;
  };

  static bool RunningUnitTest;

  FixupMap() : m_fixups(kInitCapac) {}

private:
  const Opcode* pc(const ActRec* ar, const Func* f, const Fixup& fixup) const {
    ASSERT(f);
    return f->getEntry() + fixup.m_pcOffset;
  }

  void regsFromActRec(CTCA tca, const ActRec* ar, const Fixup& fixup,
                      VMRegs* outRegs) const {
    const Func* f = ar->m_func;
    ASSERT(f);
    TRACE(3, "regsFromActRec:: tca %p -> (pcOff %d, spOff %d)\n",
          (void*)tca, fixup.m_pcOffset, fixup.m_spOffset);
    ASSERT(fixup.m_spOffset >= 0);
    outRegs->m_pc = pc(ar, f, fixup);
    outRegs->m_fp = ar;

    if (UNLIKELY(f->isGenerator())) {
      TypedValue* genStackBase = Stack::generatorStackBase(ar);
      outRegs->m_sp = genStackBase - fixup.m_spOffset;
    } else {
      outRegs->m_sp = (TypedValue*)ar - fixup.m_spOffset;
    }
  }

  TreadHashMap<CTCA,Fixup,ctca_identity_hash> m_fixups;

public:
  void recordFixup(CTCA tca, const Fixup& fixup) {
    TRACE(1, "FixupMapImpl::recordFixup: tca %p -> (pcOff %d, spOff %d)\n",
          tca, fixup.m_pcOffset, fixup.m_spOffset);
    m_fixups.insert(tca, fixup);
  }

  bool getFrameRegs(const ActRec* ar, VMRegs* outVMRegs) const {
    CTCA tca = (CTCA)ar->m_savedRip;
    // Non-obvious off-by-one fun: if the *return address* points into the TC,
    // then the frame we were running on in the TC is actually the previous
    // frame.
    ar = (const ActRec*)ar->m_savedRbp;
    const Fixup *fixup = m_fixups.find(tca);
    if (!fixup) return false;
    regsFromActRec(tca, ar, *fixup, outVMRegs);
    return true;
  }
};

struct FixupMapUnitTest {
  static const int kNumReaders = 7;
  static const int kNumToCheck = 50 * 1000;
  static const int kSpMul = 13;
  static const int kPcMul = 17;

  static void* writer(void* that);
  static void* reader(void* that);
  FixupMapUnitTest();
};

}}}

#endif
