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
#include "hphp/runtime/vm/jit/code-gen-helpers-arm.h"

#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/jit/abi-arm.h"
#include "hphp/runtime/vm/jit/back-end.h"
#include "hphp/runtime/vm/jit/mc-generator.h"
#include "hphp/runtime/vm/jit/vasm.h"
#include "hphp/runtime/vm/jit/vasm-emit.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"
#include "hphp/runtime/vm/jit/vasm-reg.h"

namespace HPHP { namespace jit { namespace arm {

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

TCA emitCall(vixl::MacroAssembler& a, CppCall call) {
  switch (call.kind()) {
  case CppCall::Kind::Direct:
    a. Mov  (rHostCallReg, reinterpret_cast<intptr_t>(call.address()));
    break;
  case CppCall::Kind::Virtual:
    a. Ldr  (rHostCallReg, argReg(0)[0]);
    a. Ldr  (rHostCallReg, rHostCallReg[call.vtableOffset()]);
    break;
  case CppCall::Kind::ArrayVirt:
  case CppCall::Kind::Destructor:
    not_implemented();
    always_assert(0);
    break;
  }

  using namespace vixl;
  auto fixupAddr = a.frontier();
  a.   HostCall(6);

  // Note that the fixup address for a HostCall is directly *before* the
  // HostCall, not after as in the native case. This is because, in simulation
  // mode we look at the simulator's PC at the time the fixup is invoked, and it
  // will still be pointing to the HostCall; it's not advanced past it until the
  // host call returns. In the native case, by contrast, we'll be looking at
  // return addresses, which point after the call.
  return fixupAddr;
}

Vpoint emitCall(Vout& v, CppCall call, RegSet args) {
  PhysReg arg0(argReg(0));
  PhysReg rHostCall(rHostCallReg);
  switch (call.kind()) {
  case CppCall::Kind::Direct:
    v << ldimmq{reinterpret_cast<intptr_t>(call.address()), rHostCall};
    break;
  case CppCall::Kind::Virtual:
    v << load{arg0[0], rHostCall};
    v << load{rHostCall[call.vtableOffset()], rHostCall};
    break;
  case CppCall::Kind::ArrayVirt:
  case CppCall::Kind::Destructor:
    not_implemented();
    always_assert(0);
    break;
  }
  uint8_t argc = args.size();
  args.add(rHostCall);
  auto fixupAddr = v.makePoint();
  v << hostcall{args, argc, fixupAddr};
  return fixupAddr;
}

TCA emitCallWithinTC(vixl::MacroAssembler& a, TCA call) {
  a.   Mov     (rHostCallReg, reinterpret_cast<intptr_t>(call));

  a.   Blr     (rHostCallReg);
  auto fixupAddr = a.frontier();

  return fixupAddr;
}

//////////////////////////////////////////////////////////////////////

void emitXorSwap(vixl::MacroAssembler& a,
                 const vixl::Register& r1, const vixl::Register& r2) {
  a.  Eor  (r1, r1, r2);
  a.  Eor  (r2, r1, r2);
  a.  Eor  (r1, r1, r2);
}

void emitRegRegMove(vixl::MacroAssembler& a, const vixl::CPURegister& dst,
                    const vixl::CPURegister& src) {
  using namespace vixl;
  if (dst.IsRegister() && src.IsRegister()) {
    a.  Mov  (Register{dst}, Register{src});
  } else if (dst.IsFPRegister() && src.IsFPRegister()) {
    a.  Fmov (FPRegister{dst}, FPRegister{src});
  } else if (dst.IsRegister() && src.IsFPRegister()) {
    a.  Fmov (Register{dst}, FPRegister{src});
  } else {
    a.  Fmov (FPRegister{dst}, Register{src});
  }
}


//////////////////////////////////////////////////////////////////////

void emitTestSurpriseFlags(vixl::MacroAssembler& a, PhysReg rds) {
  not_reached();
}

Vreg emitTestSurpriseFlags(Vout& v, Vreg rds) {
  not_reached();
}

void emitCheckSurpriseFlagsEnter(CodeBlock& mainCode, CodeBlock& coldCode,
                                 PhysReg rds, jit::Fixup fixup) {
  // keep this in sync with vasm version below
  vixl::MacroAssembler a { mainCode };
  vixl::MacroAssembler acold { coldCode };

  emitTestSurpriseFlags(a, rds);
  mcg->backEnd().emitSmashableJump(mainCode, coldCode.frontier(), CC_NZ);

  acold.  Mov  (argReg(0), rVmFp);

  auto fixupAddr =
      emitCallWithinTC(acold, mcg->tx().uniqueStubs.functionEnterHelper);
  mcg->recordSyncPoint(fixupAddr, fixup);
  mcg->backEnd().emitSmashableJump(coldCode, mainCode.frontier(), CC_None);
}

void emitCheckSurpriseFlagsEnter(Vout& v, Vout& vc, Vreg rds,
                                 jit::Fixup fixup) {
  // keep this in sync with arm version above
  PhysReg fp{rVmFp}, arg0{argReg(0)};
  auto surprise = vc.makeBlock();
  auto done = v.makeBlock();
  auto sf = emitTestSurpriseFlags(v, rds);
  v << jcc{CC_NZ, sf, {done, surprise}};

  vc = surprise;
  vc << copy{fp, arg0};
  vc << callr{vc.cns(mcg->tx().uniqueStubs.functionEnterHelper), argSet(1)};
  vc << syncpoint{fixup};
  vc << jmp{done};
  v = done;
}

//////////////////////////////////////////////////////////////////////

void emitTransCounterInc(vixl::MacroAssembler& a) {
  if (!mcg->tx().isTransDBEnabled()) return;

  // TODO(#3057328): this is not thread-safe. This should be a "load-exclusive,
  // increment, store-exclusive, loop" sequence, but vixl doesn't yet support
  // the exclusive-access instructions.
  a.   Mov   (rAsm, mcg->tx().getTransCounterAddr());
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
