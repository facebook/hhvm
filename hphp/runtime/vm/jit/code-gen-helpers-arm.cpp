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
#include "hphp/runtime/vm/jit/code-gen-helpers-arm.h"

#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/jit/abi-arm.h"
#include "hphp/runtime/vm/jit/jump-smash.h"
#include "hphp/runtime/vm/jit/translator-x64.h"

namespace HPHP { namespace JIT { namespace ARM {

void emitRegGetsRegPlusImm(vixl::MacroAssembler& as,
                           const vixl::Register& dstReg,
                           const vixl::Register& srcReg,
                           int64_t imm) {
  if (imm != 0) {
    as.  Add  (dstReg, srcReg, imm);
  } else if (dstReg.code() != srcReg.code()) {
    as.  Mov  (dstReg, srcReg);
  } // else nothing
}

//////////////////////////////////////////////////////////////////////

void emitStoreRetIntoActRec(vixl::MacroAssembler& a) {
  a.  Str  (rLinkReg, rStashedAR[AROFF(m_savedRip)]);
}

//////////////////////////////////////////////////////////////////////

TCA emitCall(vixl::MacroAssembler& a, CppCall call) {
  if (call.isDirect()) {
    a. Mov  (rHostCallReg, reinterpret_cast<intptr_t>(call.getAddress()));
  } else {
    a. Ldr  (rHostCallReg, argReg(0)[0]);
    a. Ldr  (rHostCallReg, rHostCallReg[call.getOffset()]);
  }

  using namespace vixl;
  a.   Push    (x30, x29);
  auto fixupAddr = a.frontier();
  a.   HostCall(5);
  a.   Pop     (x29, x30);

  // Note that the fixup address for a HostCall is directly *before* the
  // HostCall, not after as in the native case. This is because, in simulation
  // mode we look at the simulator's PC at the time the fixup is invoked, and it
  // will still be pointing to the HostCall; it's not advanced past it until the
  // host call returns. In the native case, by contrast, we'll be looking at
  // return addresses, which point after the call.
  return fixupAddr;
}

TCA emitCall(vixl::MacroAssembler& a, TCA call) {
  a. Mov  (rHostCallReg, reinterpret_cast<intptr_t>(call));

  using namespace vixl;
  a.   Push    (x30, x29);
  a.   Blr     (rHostCallReg);
  auto fixupAddr = a.frontier();
  a.   Pop     (x29, x30);

  return fixupAddr;
}

//////////////////////////////////////////////////////////////////////

void emitXorSwap(vixl::MacroAssembler& a,
                 const vixl::Register& r1, const vixl::Register& r2) {
  a.  Eor  (r1, r1, r2);
  a.  Eor  (r2, r1, r2);
  a.  Eor  (r1, r1, r2);
}

//////////////////////////////////////////////////////////////////////

void emitTestSurpriseFlags(vixl::MacroAssembler& a) {
  static_assert(RequestInjectionData::LastFlag < (1 << 8),
                "Translator assumes RequestInjectionFlags fit in one byte");
  a.  Ldrb  (rAsm, rVmTl[RDS::kConditionFlagsOff]);
  a.  Tst   (rAsm, 0xff);
}

void emitCheckSurpriseFlagsEnter(CodeBlock& mainCode, CodeBlock& stubsCode,
                                 bool inTracelet, JIT::FixupMap& fixupMap,
                                 JIT::Fixup fixup) {
  vixl::MacroAssembler a { mainCode };
  vixl::MacroAssembler astubs { stubsCode };

  emitTestSurpriseFlags(a);
  emitSmashableJump(mainCode, stubsCode.frontier(), CC_NZ);

  astubs.  Mov  (argReg(0), rVmFp);

  auto fixupAddr = emitCall(astubs, tx64->uniqueStubs.functionEnterHelper);
  if (inTracelet) {
    fixupMap.recordSyncPoint(fixupAddr,
                             fixup.m_pcOffset, fixup.m_spOffset);
  } else {
    // If we're being called while generating a func prologue, we
    // have to record the fixup directly in the fixup map instead of
    // going through the pending fixup path like normal.
    fixupMap.recordFixup(fixupAddr, fixup);
  }
  emitSmashableJump(stubsCode, mainCode.frontier(), CC_None);
}

//////////////////////////////////////////////////////////////////////

void emitTransCounterInc(vixl::MacroAssembler& a) {
  if (!tx64->isTransDBEnabled()) return;

  // TODO(#3057328): this is not thread-safe. This should be a "load-exclusive,
  // increment, store-exclusive, loop" sequence, but vixl doesn't yet support
  // the exclusive-access instructions.
  a.   Mov   (rAsm, tx64->getTransCounterAddr());
  a.   Ldr   (rAsm2, rAsm[0]);
  a.   Add   (rAsm2, rAsm2, 1);
  a.   Str   (rAsm2, rAsm[0]);
}

//////////////////////////////////////////////////////////////////////

void emitIncRefKnownType(vixl::MacroAssembler& a,
                         const vixl::Register& dataReg,
                         const size_t disp) {
  vixl::Label dontCount;

  auto const& rAddr = rAsm;
  auto const& rCount = rAsm2.W();

  // Read the inner object.
  a.   Ldr   (rAddr, dataReg[disp + TVOFF(m_data)]);
  // Check the count for staticness.
  static_assert(sizeof(RefCount) == 4, "");
  a.   Ldr   (rCount, rAddr[FAST_REFCOUNT_OFFSET]);
  // Careful: tbnz can only test a single bit, so you pass a bit position
  // instead of a full-blown immediate. 0 = lsb, 63 = msb.
  a.   Tbnz  (rCount.X(), UncountedBitPos, &dontCount);
  // Increment and store count.
  a.   Add   (rCount, rCount, 1);
  a.   Str   (rCount, rAddr[FAST_REFCOUNT_OFFSET]);

  a.   bind  (&dontCount);
}

void emitIncRefGeneric(vixl::MacroAssembler& a,
                       const vixl::Register& dataReg,
                       const size_t disp) {
  vixl::Label dontCount;

  // Read the type; bail if it's not refcounted.
  a.   Ldrb  (rAsm.W(), dataReg[disp + TVOFF(m_type)]);
  a.   Cmp   (rAsm.W(), KindOfRefCountThreshold);
  a.   B     (&dontCount, vixl::le);

  emitIncRefKnownType(a, dataReg, disp);

  a.   bind  (&dontCount);
}

}}}
