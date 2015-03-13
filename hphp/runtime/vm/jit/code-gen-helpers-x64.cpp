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

#include "hphp/runtime/vm/jit/code-gen-helpers-x64.h"

#include "hphp/runtime/base/arch.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/stats.h"
#include "hphp/runtime/base/types.h"
#include "hphp/runtime/vm/jit/back-end.h"
#include "hphp/runtime/vm/jit/code-gen-x64.h"
#include "hphp/runtime/vm/jit/ir-opcode.h"
#include "hphp/runtime/vm/jit/mc-generator-internal.h"
#include "hphp/runtime/vm/jit/mc-generator.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/jit/translator.h"
#include "hphp/runtime/vm/jit/vasm-emit.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"
#include "hphp/runtime/vm/jit/vasm-reg.h"

#include "hphp/util/asm-x64.h"
#include "hphp/util/ringbuffer.h"
#include "hphp/util/trace.h"

namespace HPHP { namespace jit { namespace x64 {

//////////////////////////////////////////////////////////////////////

using namespace jit::reg;

TRACE_SET_MOD(hhir);

//////////////////////////////////////////////////////////////////////

/*
 * It's not normally ok to directly use tracelet abi registers in
 * codegen, unless you're directly dealing with an instruction that
 * does near-end-of-tracelet glue.  (Or also we sometimes use them
 * just for some static_assertions relating to calls to helpers from
 * mcg that hardcode these registers.)
 */

/*
 * Satisfy an alignment constraint. Bridge the gap with int3's.
 */
void moveToAlign(CodeBlock& cb,
                 const size_t align /* =kJmpTargetAlign */) {
  X64Assembler a { cb };
  assert(folly::isPowTwo(align));
  size_t leftInBlock = align - ((align - 1) & uintptr_t(cb.frontier()));
  if (leftInBlock == align) return;
  if (leftInBlock > 2) {
    a.ud2();
    leftInBlock -= 2;
  }
  if (leftInBlock > 0) {
    a.emitInt3s(leftInBlock);
  }
}

void emitEagerSyncPoint(Vout& v, const Op* pc, Vreg rds, Vreg vmfp, Vreg vmsp) {
  v << store{vmfp, rds[rds::kVmfpOff]};
  v << store{vmsp, rds[rds::kVmspOff]};
  emitImmStoreq(v, intptr_t(pc), rds[rds::kVmpcOff]);
}

// emitEagerVMRegSave --
//   Inline. Saves regs in-place in the TC. This is an unusual need;
//   you probably want to lazily save these regs via recordCall and
//   its ilk.
void emitEagerVMRegSave(Asm& as, PhysReg rds, RegSaveFlags flags) {
  bool saveFP = bool(flags & RegSaveFlags::SaveFP);
  bool savePC = bool(flags & RegSaveFlags::SavePC);
  assert((flags & ~(RegSaveFlags::SavePC | RegSaveFlags::SaveFP)) ==
         RegSaveFlags::None);

  Reg64 pcReg = rdi;
  assert(!kCrossCallRegs.contains(rdi));

  as.   storeq (rVmSp, rds[rds::kVmspOff]);
  if (savePC) {
    // We're going to temporarily abuse rVmSp to hold the current unit.
    Reg64 rBC = rVmSp;
    as. push   (rBC);
    // m_fp -> m_func -> m_unit -> m_bc + pcReg
    as. loadq  (rVmFp[AROFF(m_func)], rBC);
    as. loadq  (rBC[Func::unitOff()], rBC);
    as. loadq  (rBC[Unit::bcOff()], rBC);
    as. addq   (rBC, pcReg);
    as. storeq (pcReg, rds[rds::kVmpcOff]);
    as. pop    (rBC);
  }
  if (saveFP) {
    as. storeq (rVmFp, rds[rds::kVmfpOff]);
  }
}

// Save vmsp, and optionally vmfp and vmpc. If saving vmpc,
// the bytecode offset is expected to be in rdi and is clobbered
void emitEagerVMRegSave(Vout& v, Vreg rds, RegSaveFlags flags) {
  bool saveFP = bool(flags & RegSaveFlags::SaveFP);
  bool savePC = bool(flags & RegSaveFlags::SavePC);
  assert((flags & ~(RegSaveFlags::SavePC | RegSaveFlags::SaveFP)) ==
         RegSaveFlags::None);

  assert(!kCrossCallRegs.contains(rdi));

  v << store{rVmSp, rds[rds::kVmspOff]};
  if (savePC) {
    PhysReg pc{rdi};
    auto func = v.makeReg();
    auto unit = v.makeReg();
    auto bc = v.makeReg();
    // m_fp -> m_func -> m_unit -> m_bc + pcReg
    v << load{rVmFp[AROFF(m_func)], func};
    v << load{func[Func::unitOff()], unit};
    v << load{unit[Unit::bcOff()], bc};
    v << addq{bc, pc, pc, v.makeReg()};
    v << store{pc, rds[rds::kVmpcOff]};
  }
  if (saveFP) {
    v << store{rVmFp, rds[rds::kVmfpOff]};
  }
}

void emitGetGContext(Vout& v, Vreg dest) {
  emitTLSLoad<ExecutionContext>(v, g_context, dest);
}

void emitGetGContext(Asm& as, PhysReg dest) {
  emitGetGContext(Vauto(as.code()).main(), dest);
}

// IfCountNotStatic --
//   Emits if (%reg->_count < 0) { ... }.
//   This depends on UncountedValue and StaticValue
//   being the only valid negative refCounts and both indicating no
//   ref count is needed.
//   May short-circuit this check if the type is known to be
//   static already.
struct IfCountNotStatic {
  typedef CondBlock<FAST_REFCOUNT_OFFSET,
                    0,
                    CC_S,
                    int32_t> NonStaticCondBlock;
  static_assert(UncountedValue < 0 && StaticValue < 0, "");
  NonStaticCondBlock *m_cb; // might be null
  IfCountNotStatic(Asm& as, PhysReg reg,
                   MaybeDataType t = folly::none) {

    // Objects and variants cannot be static
    if (t != KindOfObject && t != KindOfResource && t != KindOfRef) {
      m_cb = new NonStaticCondBlock(as, reg);
    } else {
      m_cb = nullptr;
    }
  }

  ~IfCountNotStatic() {
    delete m_cb;
  }
};

void emitTransCounterInc(Vout& v) {
  if (!mcg->tx().isTransDBEnabled()) return;
  auto t = v.cns(mcg->tx().getTransCounterAddr());
  v << incqmlock{*t, v.makeReg()};
}

void emitTransCounterInc(Asm& a) {
  emitTransCounterInc(Vauto(a.code()).main());
}

void emitDecRef(Vout& v, Vreg base) {
  auto const sf = v.makeReg();
  v << declm{base[FAST_REFCOUNT_OFFSET], sf};
  emitAssertFlagsNonNegative(v, sf);
}

void emitIncRef(Vout& v, Vreg base) {
  if (RuntimeOption::EvalHHIRGenerateAsserts) {
    emitAssertRefCount(v, base);
  }
  // emit incref
  auto const sf = v.makeReg();
  v << inclm{base[FAST_REFCOUNT_OFFSET], sf};
  emitAssertFlagsNonNegative(v, sf);
}

void emitIncRef(Asm& as, PhysReg base) {
  emitIncRef(Vauto(as.code()).main(), base);
}

void emitIncRefCheckNonStatic(Asm& as, PhysReg base, DataType dtype) {
  { // if !static then
    IfCountNotStatic ins(as, base, dtype);
    emitIncRef(as, base);
  } // endif
}

void emitIncRefGenericRegSafe(Asm& as, PhysReg base, int disp, PhysReg tmpReg) {
  { // if RC
    IfRefCounted irc(as, base, disp);
    as.   loadq  (base[disp + TVOFF(m_data)], tmpReg);
    { // if !static
      IfCountNotStatic ins(as, tmpReg);
      as. incl(tmpReg[FAST_REFCOUNT_OFFSET]);
    } // endif
  } // endif
}

void emitAssertFlagsNonNegative(Vout& v, Vreg sf) {
  if (!RuntimeOption::EvalHHIRGenerateAsserts) return;
  ifThen(v, CC_NGE, sf, [&](Vout& v) { v << ud2{}; });
}

void emitAssertRefCount(Vout& v, Vreg base) {
  auto const sf = v.makeReg();
  v << cmplim{StaticValue, base[FAST_REFCOUNT_OFFSET], sf};
  ifThen(v, CC_NLE, sf, [&](Vout& v) {
    auto const sf = v.makeReg();
    v << cmplim{RefCountMaxRealistic, base[FAST_REFCOUNT_OFFSET], sf};
    ifThen(v, CC_NBE, sf, [&](Vout& v) { v << ud2{}; });
  });
}

// Logical register move: ensures the value in src will be in dest
// after execution, but might do so in strange ways. Do not count on
// being able to smash dest to a different register in the future, e.g.
void emitMovRegReg(Asm& as, PhysReg srcReg, PhysReg dstReg) {
  assert(srcReg != InvalidReg);
  assert(dstReg != InvalidReg);

  if (srcReg == dstReg) return;

  if (srcReg.isGP()) {
    if (dstReg.isGP()) {                 // GP => GP
      as. movq(srcReg, dstReg);
    } else {                             // GP => XMM
      // This generates a movq x86 instruction, which zero extends
      // the 64-bit value in srcReg into a 128-bit XMM register
      as. movq_rx(srcReg, dstReg);
    }
  } else {
    if (dstReg.isGP()) {                 // XMM => GP
      as. movq_xr(srcReg, dstReg);
    } else {                             // XMM => XMM
      // This copies all 128 bits in XMM,
      // thus avoiding partial register stalls
      as. movdqa(srcReg, dstReg);
    }
  }
}

void emitLea(Asm& as, MemoryRef mr, PhysReg dst) {
  if (dst == InvalidReg) return;
  if (mr.r.disp == 0) {
    emitMovRegReg(as, mr.r.base, dst);
  } else {
    as. lea(mr, dst);
  }
}

Vreg emitLdObjClass(Vout& v, Vreg objReg, Vreg dstReg) {
  emitLdLowPtr(v, objReg[ObjectData::getVMClassOffset()],
               dstReg, sizeof(LowClassPtr));
  return dstReg;
}

Vreg emitLdClsCctx(Vout& v, Vreg src, Vreg dst) {
  v << decq{src, dst, v.makeReg()};
  return dst;
}

void emitCall(Asm& a, TCA dest, RegSet args) {
  // NB: Keep this in sync with Vgen::emit(call) in vasm-x64.cpp.
  if (a.jmpDeltaFits(dest)) {
    a.call(dest);
  } else {
    // can't do a near call; store address in data section.
    // call by loading the address using rip-relative addressing.  This
    // assumes the data section is near the current code section.  Since
    // this sequence is directly in-line, rip-relative like this is
    // more compact than loading a 64-bit immediate.
    auto addr = mcg->allocLiteral((uint64_t)dest);
    a.call(rip[(intptr_t)addr]);
  }
}

void emitCall(Asm& a, CppCall call, RegSet args) {
  emitCall(Vauto(a.code()).main(), call, args);
}

void emitCall(Vout& v, CppCall target, RegSet args) {
  switch (target.kind()) {
  case CppCall::Kind::Direct:
    v << call{static_cast<TCA>(target.address()), args};
    return;
  case CppCall::Kind::Virtual:
    // Virtual call.
    // Load method's address from proper offset off of object in rdi,
    // using rax as scratch.
    v << load{*rdi, rax};
    v << callm{rax[target.vtableOffset()], args};
    return;
  case CppCall::Kind::ArrayVirt: {
    auto const addr = reinterpret_cast<intptr_t>(target.arrayTable());
    always_assert_flog(
      deltaFits(addr, sz::dword),
      "deltaFits on ArrayData vtable calls needs to be checked before "
      "emitting them"
    );
    v << loadzbl{rdi[ArrayData::offsetofKind()], eax};
    v << callm{baseless(rax*8 + addr), args};
    return;
  }
  case CppCall::Kind::Destructor:
    // this movzbq is only needed because callers aren't required to
    // zero-extend the type.
    auto zextType = v.makeReg();
    v << movzbq{target.reg(), zextType};
    auto dtor_ptr = lookupDestructor(v, zextType);
    v << callm{dtor_ptr, args};
    return;
  }
  not_reached();
}

void emitImmStoreq(Vout& v, Immed64 imm, Vptr ref) {
  if (imm.fits(sz::dword)) {
    v << storeqi{imm.l(), ref};
  } else {
    // An alternative is two 32-bit immediate stores, but that's little-endian
    // specific and generates larger code on x64 (24 bytes vs. 18 bytes).
    v << store{v.cns(imm.q()), ref};
  }
}

void emitImmStoreq(Asm& a, Immed64 imm, MemoryRef ref) {
  if (imm.fits(sz::dword)) {
    a.storeq(imm.l(), ref);
  } else {
    a.storel(int32_t(imm.q()), ref);
    a.storel(int32_t(imm.q() >> 32), MemoryRef(ref.r + 4));
  }
}

void emitRB(Vout& v, Trace::RingBufferType t, const char* msg) {
  if (!Trace::moduleEnabled(Trace::ringbuffer, 1)) {
    return;
  }
  v << vcall{CppCall::direct(Trace::ringbufferMsg),
             v.makeVcallArgs({{v.cns(msg), v.cns(strlen(msg)), v.cns(t)}}),
             v.makeTuple({})};
}

void emitTraceCall(CodeBlock& cb, Offset pcOff) {
  Asm a { cb };
  // call to a trace function
  a.    lea    (rip[(int64_t)a.frontier()], rcx);
  a.    movq   (rVmFp, rdi);
  a.    movq   (rVmSp, rsi);
  a.    movq   (pcOff, rdx);
  // do the call; may use a trampoline
  emitCall(a, reinterpret_cast<TCA>(traceCallback),
           RegSet().add(rcx).add(rdi).add(rsi).add(rdx));
}

void emitTestSurpriseFlags(Asm& a, PhysReg rds) {
  static_assert(RequestInjectionData::LastFlag < (1LL << 32),
                "Translator assumes RequestInjectionFlags fit in 32-bit int");
  a.testl(-1, rds[rds::kConditionFlagsOff]);
}

Vreg emitTestSurpriseFlags(Vout& v, Vreg rds) {
  static_assert(RequestInjectionData::LastFlag < (1LL << 32),
                "Translator assumes RequestInjectionFlags fit in 32-bit int");
  auto const sf = v.makeReg();
  v << testlim{-1, rds[rds::kConditionFlagsOff], sf};
  return sf;
}

void emitCheckSurpriseFlagsEnter(CodeBlock& mainCode, CodeBlock& coldCode,
                                 PhysReg rds, Fixup fixup) {
  // warning: keep this in sync with the vasm version below.
  Asm a{mainCode}, acold{coldCode};

  emitTestSurpriseFlags(a, rds);
  a.  jnz(coldCode.frontier());

  acold.  movq  (rVmFp, argNumToRegName[0]);
  emitCall(acold, mcg->tx().uniqueStubs.functionEnterHelper, argSet(1));
  mcg->recordSyncPoint(acold.frontier(), fixup.pcOffset, fixup.spOffset);
  acold.  jmp   (a.frontier());
}

void emitCheckSurpriseFlagsEnter(Vout& v, Vout& vcold, Vreg rds, Fixup fixup) {
  // warning: keep this in sync with the x64 version above.
  auto cold = vcold.makeBlock();
  auto done = v.makeBlock();
  auto const sf = emitTestSurpriseFlags(v, rds);
  v << jcc{CC_NZ, sf, {done, cold}};

  auto helper = (void(*)())mcg->tx().uniqueStubs.functionEnterHelper;
  vcold = cold;
  vcold << vcall{CppCall::direct(helper),
                 v.makeVcallArgs({{rVmFp}}),
                 v.makeTuple({}),
                 Fixup{fixup.pcOffset, fixup.spOffset}};
  vcold << jmp{done};
  v = done;
}

void emitLdLowPtr(Vout& v, Vptr mem, Vreg reg, size_t size) {
  if (size == 8) {
    v << load{mem, reg};
  } else if (size == 4) {
    v << loadzlq{mem, reg};
  } else {
    not_implemented();
  }
}

void emitCmpClass(Vout& v, Vreg sf, const Class* c, Vptr mem) {
  auto size = sizeof(LowClassPtr);
  if (size == 8) {
    v << cmpqm{v.cns(c), mem, sf};
  } else if (size == 4) {
    v << cmplm{v.cns(safe_cast<uint32_t>(reinterpret_cast<intptr_t>(c))),
               mem, sf};
  } else {
    not_implemented();
  }
}

void emitCmpClass(Vout& v, Vreg sf, Vreg reg, Vptr mem) {
  auto size = sizeof(LowClassPtr);
  if (size == 8) {
    v << cmpqm{reg, mem, sf};
  } else if (size == 4) {
    auto lowCls = v.makeReg();
    v << movtql{reg, lowCls};
    v << cmplm{lowCls, mem, sf};
  } else {
    not_implemented();
  }
}

void emitCmpClass(Vout& v, Vreg sf, Vreg reg1, Vreg reg2) {
  auto size = sizeof(LowClassPtr);
  if (size == 8) {
    v << cmpq{reg1, reg2, sf};
  } else if (size == 4) {
    v << cmpl{reg1, reg2, sf};
  } else {
    not_implemented();
  }
}

void copyTV(Vout& v, Vloc src, Vloc dst, Type destType) {
  auto src_arity = src.numAllocated();
  auto dst_arity = dst.numAllocated();
  if (dst_arity == 2) {
    always_assert(src_arity == 2);
    v << copy2{src.reg(0), src.reg(1), dst.reg(0), dst.reg(1)};
    return;
  }
  always_assert(dst_arity == 1);
  if (src_arity == 2 && dst.isFullSIMD()) {
    pack2(v, src.reg(0), src.reg(1), dst.reg(0));
    return;
  }
  always_assert(src_arity >= 1);
  if (src_arity == 2 && destType <= Type::Bool) {
    v << movtqb{src.reg(0), dst.reg(0)};
  } else {
    v << copy{src.reg(0), dst.reg(0)};
  }
}

// copy 2 64-bit values into one 128-bit value
void pack2(Vout& v, Vreg s0, Vreg s1, Vreg d0) {
  auto prep = [&](Vreg r) {
    if (VregDbl::allowable(r)) return r;
    auto t = v.makeReg();
    v << copy{r, t};
    return t;
  };
  // s0 and s1 must be valid VregDbl registers; prep() takes care of it.
  v << unpcklpd{prep(s1), prep(s0), d0}; // s0,s1 -> d0[0],d0[1]
}

Vreg zeroExtendIfBool(Vout& v, const SSATmp* src, Vreg reg) {
  if (!src->isA(Type::Bool)) return reg;
  // zero-extend the bool from a byte to a quad
  auto extended = v.makeReg();
  v << movzbq{reg, extended};
  return extended;
}

}}}
