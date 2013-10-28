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
                                 bool inTracelet, Transl::FixupMap& fixupMap,
                                 Transl::Fixup fixup) {
  vixl::MacroAssembler a { mainCode };
  vixl::MacroAssembler astubs { stubsCode };

  emitTestSurpriseFlags(a);
  emitSmashableJump(mainCode, stubsCode.frontier(), CC_NZ);

  astubs.  Mov  (argReg(0), rVmFp);
  void (*helper)(const ActRec*) = functionEnterHelper;

  auto fixupAddr = emitCall(astubs, CppCall(helper));
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

  // Read the inner object.
  a.   Ldr   (rAsm, dataReg[disp + TVOFF(m_data)]);
  // Check the count for staticness.
  static_assert(sizeof(RefCount) == 4, "");
  auto const& rCount = rAsm.W();
  a.   Ldr   (rCount, rAsm[FAST_REFCOUNT_OFFSET]);
  // Careful: tbnz can only test a single bit, so you pass a bit position
  // instead of a full-blown immediate. 0 = lsb, 63 = msb.
  a.   Tbnz  (rCount.X(), RefCountStaticBitPos, &dontCount);
  // Increment and store count.
  a.   Add   (rCount, rCount, 1);
  a.   Str   (rCount, rAsm[FAST_REFCOUNT_OFFSET]);

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
