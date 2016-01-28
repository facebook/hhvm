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

#include "hphp/runtime/vm/jit/func-guard-arm.h"

#include "hphp/runtime/vm/jit/abi-arm.h"
#include "hphp/runtime/vm/jit/mc-generator.h"
#include "hphp/runtime/vm/jit/translator.h"
#include "hphp/runtime/vm/jit/smashable-instr-arm.h"
#include "hphp/runtime/vm/jit/unique-stubs.h"
#include "hphp/runtime/vm/jit/vasm-reg.h"

#include "hphp/util/data-block.h"

#include "hphp/vixl/a64/constants-a64.h"
#include "hphp/vixl/a64/macro-assembler-a64.h"

namespace HPHP { namespace jit { namespace arm {

///////////////////////////////////////////////////////////////////////////////

namespace {

///////////////////////////////////////////////////////////////////////////////

ALWAYS_INLINE bool isPrologueStub(TCA addr) {
  return addr == mcg->tx().uniqueStubs.fcallHelperThunk;
}

vixl::Register X(Vreg64 r) {
  PhysReg pr(r.asReg());
  return x2a(pr);
}

vixl::MemOperand M(Vptr p) {
  assertx(p.base.isValid() && !p.index.isValid());
  return X(p.base)[p.disp];
}

///////////////////////////////////////////////////////////////////////////////

}

///////////////////////////////////////////////////////////////////////////////

void emitFuncGuard(const Func* func, CodeBlock& cb) {
  vixl::MacroAssembler a { cb };
  vixl::Label after_data;
  vixl::Label target_data;

  assertx(arm::abi(CodeKind::CrossTrace).gpUnreserved.contains(vixl::x0));

  emitSmashableMovq(cb, uint64_t(func), vixl::x0);
  a.  Ldr   (rAsm, M(rvmfp()[AROFF(m_func)]));
  a.  Cmp   (vixl::x0, rAsm);
  a.  B     (&after_data, convertCC(CC_Z));

  a.  Ldr   (rAsm, &target_data);
  a.  Br    (rAsm);

  a.  bind  (&target_data);
  a.  dc64  (mcg->tx().uniqueStubs.funcPrologueRedispatch);
  a.  bind  (&after_data);
}

TCA funcGuardFromPrologue(TCA prologue, const Func* func) {
  if (isPrologueStub(prologue)) return prologue;
  return prologue - (smashableMovqLen() + (5 * 4) + 8);
}

bool funcGuardMatches(TCA guard, const Func* func) {
  if (isPrologueStub(guard)) return false;
  return smashableMovqImm(guard) == reinterpret_cast<uintptr_t>(func);
}

void clobberFuncGuard(TCA guard, const Func* func) {
  smashMovq(guard, 0);
}

///////////////////////////////////////////////////////////////////////////////

}}}
