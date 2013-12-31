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

#include "hphp/runtime/vm/jit/code-gen-helpers-x64.h"

#include "hphp/util/asm-x64.h"
#include "hphp/util/ringbuffer.h"
#include "hphp/util/trace.h"

#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/stats.h"
#include "hphp/runtime/base/types.h"
#include "hphp/runtime/vm/jit/arch.h"
#include "hphp/runtime/vm/jit/jump-smash.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/jit/translator-x64.h"
#include "hphp/runtime/vm/jit/translator-x64-internal.h"
#include "hphp/runtime/vm/jit/translator.h"
#include "hphp/runtime/vm/jit/ir.h"

namespace HPHP { namespace JIT { namespace X64 {

//////////////////////////////////////////////////////////////////////

using namespace Util;
using namespace JIT::reg;

TRACE_SET_MOD(hhir);

//////////////////////////////////////////////////////////////////////

/*
 * It's not normally ok to directly use tracelet abi registers in
 * codegen, unless you're directly dealing with an instruction that
 * does near-end-of-tracelet glue.  (Or also we sometimes use them
 * just for some static_assertions relating to calls to helpers from
 * tx64 that hardcode these registers.)
 */

/*
 * Satisfy an alignment constraint. Bridge the gap with int3's.
 */
void moveToAlign(CodeBlock& cb,
                 const size_t align /* =kJmpTargetAlign */) {
  // TODO(2967396) implement properly, move function
  if (arch() == Arch::ARM) return;

  using namespace HPHP::Util;
  X64Assembler a { cb };
  assert(isPowerOfTwo(align));
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

void emitEagerSyncPoint(Asm& as, const HPHP::Opcode* pc, const Offset spDiff) {
  static COff spOff = offsetof(VMExecutionContext, m_stack) +
    Stack::topOfStackOffset();
  static COff fpOff = offsetof(VMExecutionContext, m_fp);
  static COff pcOff = offsetof(VMExecutionContext, m_pc);

  /* we can't use rAsm because the pc store uses it as a
     temporary */
  Reg64 rEC = reg::rdi;

  as.  push(rEC);
  emitGetGContext(as, rEC);
  as.  storeq(rVmFp, rEC[fpOff]);
  if (spDiff) {
    as.  lea(rVmSp[spDiff], rAsm);
    as.  storeq(rAsm, rEC[spOff]);
  } else {
    as.  storeq(rVmSp, rEC[spOff]);
  }
  as.  storeq(pc, rEC[pcOff]);
  as.  pop(rEC);
}

// emitEagerVMRegSave --
//   Inline. Saves regs in-place in the TC. This is an unusual need;
//   you probably want to lazily save these regs via recordCall and
//   its ilk.
void emitEagerVMRegSave(Asm& as, RegSaveFlags flags) {
  bool saveFP = bool(flags & RegSaveFlags::SaveFP);
  bool savePC = bool(flags & RegSaveFlags::SavePC);
  assert((flags & ~(RegSaveFlags::SavePC | RegSaveFlags::SaveFP)) ==
         RegSaveFlags::None);

  Reg64 pcReg = rdi;
  PhysReg rEC = rAsm;
  assert(!kSpecialCrossTraceRegs.contains(rdi));

  emitGetGContext(as, rEC);

  static COff spOff = offsetof(VMExecutionContext, m_stack) +
    Stack::topOfStackOffset();
  static COff fpOff = offsetof(VMExecutionContext, m_fp) - spOff;
  static COff pcOff = offsetof(VMExecutionContext, m_pc) - spOff;

  assert(spOff != 0);
  as.   addq   (spOff, r64(rEC));
  as.   storeq (rVmSp, *rEC);
  if (savePC) {
    // We're going to temporarily abuse rVmSp to hold the current unit.
    Reg64 rBC = rVmSp;
    as. push   (rBC);
    // m_fp -> m_func -> m_unit -> m_bc + pcReg
    as. loadq  (rVmFp[AROFF(m_func)], rBC);
    as. loadq  (rBC[Func::unitOff()], rBC);
    as. loadq  (rBC[Unit::bcOff()], rBC);
    as. addq   (rBC, pcReg);
    as. storeq (pcReg, rEC[pcOff]);
    as. pop    (rBC);
  }
  if (saveFP) {
    as. storeq (rVmFp, rEC[fpOff]);
  }
}

void emitGetGContext(Asm& as, PhysReg dest) {
  emitTLSLoad<ExecutionContext>(as, g_context, dest);
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
  IfCountNotStatic(Asm& as,
                   PhysReg reg,
                   DataType t = KindOfInvalid) {

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


void emitTransCounterInc(Asm& a) {
  if (!tx64->isTransDBEnabled()) return;

  a.    movq (tx64->getTransCounterAddr(), rAsm);
  a.    lock ();
  a.    incq (*rAsm);
}

void emitIncRef(Asm& as, PhysReg base) {
  if (RuntimeOption::EvalHHIRGenerateAsserts) {
    emitAssertRefCount(as, base);
  }
  // emit incref
  as.incl(base[FAST_REFCOUNT_OFFSET]);
  if (RuntimeOption::EvalHHIRGenerateAsserts) {
    // Assert that the ref count is greater than zero
    emitAssertFlagsNonNegative(as);
  }
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
    as.   load_reg64_disp_reg64(base, disp + TVOFF(m_data),
                                tmpReg);
    { // if !static
      IfCountNotStatic ins(as, tmpReg);
      as. incl(tmpReg[FAST_REFCOUNT_OFFSET]);
    } // endif
  } // endif
}

void emitAssertFlagsNonNegative(Asm& as) {
  ifThen(as, CC_NGE, [&] { as.ud2(); });
}

void emitAssertRefCount(Asm& as, PhysReg base) {
  as.cmpl(HPHP::StaticValue, base[FAST_REFCOUNT_OFFSET]);
  ifThen(as, CC_NBE, [&] {
      as.cmpl(HPHP::RefCountMaxRealistic, base[FAST_REFCOUNT_OFFSET]);
      ifThen(as, CC_NBE, [&] { as.ud2(); });
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
      as. mov_reg64_xmm(srcReg, dstReg);
    }
  } else {
    if (dstReg.isGP()) {                 // XMM => GP
      as. mov_xmm_reg64(srcReg, dstReg);
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

void emitLdObjClass(Asm& as, PhysReg objReg, PhysReg dstReg) {
  as.   loadq (objReg[ObjectData::getVMClassOffset()], dstReg);
}

void emitLdClsCctx(Asm& as, PhysReg srcReg, PhysReg dstReg) {
  emitMovRegReg(as, srcReg, dstReg);
  as.   decq(dstReg);
}

void emitCall(Asm& a, TCA dest) {
  if (a.jmpDeltaFits(dest) && !Stats::enabled()) {
    a.    call(dest);
  } else {
    a.    call(tx64->getNativeTrampoline(dest));
  }
}

void emitCall(Asm& a, CppCall call) {
  if (call.isDirect()) {
    return emitCall(a, (TCA)call.getAddress());
  }
  // Virtual call.
  // Load method's address from proper offset off of object in rdi,
  // using rax as scratch.
  a.  loadq  (*rdi, rax);
  a.  call   (rax[call.getOffset()]);
}

void emitJmpOrJcc(Asm& a, ConditionCode cc, TCA dest) {
  if (cc == CC_None) {
    a.   jmp(dest);
  } else {
    a.   jcc((ConditionCode)cc, dest);
  }
}

void emitRB(X64Assembler& a,
            Trace::RingBufferType t,
            const char* msg,
            RegSet toSave) {
  if (!Trace::moduleEnabledRelease(Trace::ringbuffer, 1)) {
    return;
  }
  PhysRegSaver save(a, toSave | kSpecialCrossTraceRegs);
  int arg = 0;
  a.    emitImmReg((uintptr_t)msg, argNumToRegName[arg++]);
  a.    emitImmReg(strlen(msg), argNumToRegName[arg++]);
  a.    emitImmReg(t, argNumToRegName[arg++]);
  a.    call((TCA)Trace::ringbufferMsg);
}

void emitTraceCall(CodeBlock& cb, int64_t pcOff) {
  // TODO(2967396) implement properly, move function
  if (arch() == Arch::ARM) return;

  Asm as { cb };
  // call to a trace function
  as.mov_imm64_reg((int64_t)as.frontier(), reg::rcx);
  as.mov_reg64_reg64(rVmFp, reg::rdi);
  as.mov_reg64_reg64(rVmSp, reg::rsi);
  as.mov_imm64_reg(pcOff, reg::rdx);
  // do the call; may use a trampoline
  emitCall(as, (TCA)traceCallback);
}

void emitTestSurpriseFlags(Asm& a) {
  static_assert(RequestInjectionData::LastFlag < (1 << 8),
                "Translator assumes RequestInjectionFlags fit in one byte");
  a.    testb((int8_t)0xff, rVmTl[RDS::kConditionFlagsOff]);
}

void emitCheckSurpriseFlagsEnter(CodeBlock& mainCode, CodeBlock& stubsCode,
                                 bool inTracelet, FixupMap& fixupMap,
                                 Fixup fixup) {
  Asm a { mainCode };
  Asm astubs { stubsCode };

  emitTestSurpriseFlags(a);
  a.  jnz  (stubsCode.frontier());

  astubs.  movq  (rVmFp, argNumToRegName[0]);
  emitCall(astubs, tx64->uniqueStubs.functionEnterHelper);
  if (inTracelet) {
    fixupMap.recordSyncPoint(stubsCode.frontier(),
                             fixup.m_pcOffset, fixup.m_spOffset);
  } else {
    // If we're being called while generating a func prologue, we
    // have to record the fixup directly in the fixup map instead of
    // going through the pending fixup path like normal.
    fixupMap.recordFixup(stubsCode.frontier(), fixup);
  }
  astubs.  jmp   (mainCode.frontier());
}

void shuffle2(Asm& as, PhysReg s0, PhysReg s1, PhysReg d0, PhysReg d1) {
  assert(s0 != s1);
  assert(!s0.isSIMD() || s1 == InvalidReg); // never 2 XMMs
  assert(!d0.isSIMD() || d1 == InvalidReg); // never 2 XMMs
  if (d0 == s1 && d1 != InvalidReg) {
    assert(d0 != d1);
    if (d1 == s0) {
      as.   xchgq (s1, s0);
    } else {
      as.   movq (s1, d1); // save s1 first; d1 != s0
      as.   movq (s0, d0);
    }
  } else if (d0.isSIMD() && s0.isGP() && s1.isGP()) {
    // move 2 gpr to 1 xmm
    assert(d0 != rCgXMM0); // xmm0 is reserved for scratch
    as.   mov_reg64_xmm(s0, d0);
    as.   mov_reg64_xmm(s1, rCgXMM0);
    as.   unpcklpd(rCgXMM0, d0); // s1 -> d0[1]
  } else {
    if (d0 != InvalidReg) emitMovRegReg(as, s0, d0); // d0 != s1
    if (d1 != InvalidReg) emitMovRegReg(as, s1, d1);
  }
}

void zeroExtendIfBool(CodeGenerator::Asm& as, const SSATmp* src, PhysReg reg) {
  if (src->isA(Type::Bool) && reg != InvalidReg) {
    // zero-extend the bool from a byte to a quad
    // note: movzbl actually extends the value to 64 bits.
    as.movzbl(rbyte(reg), r32(reg));
  }
}

ConditionCode opToConditionCode(Opcode opc) {

  switch (opc) {
  case JmpGt:                 return CC_G;
  case JmpGte:                return CC_GE;
  case JmpLt:                 return CC_L;
  case JmpLte:                return CC_LE;
  case JmpEq:                 return CC_E;
  case JmpNeq:                return CC_NE;
  case JmpSame:               return CC_E;
  case JmpNSame:              return CC_NE;
  case JmpInstanceOfBitmask:  return CC_NZ;
  case JmpNInstanceOfBitmask: return CC_Z;
  case JmpIsType:             return CC_NZ;
  case JmpIsNType:            return CC_Z;
  case JmpZero:               return CC_Z;
  case JmpNZero:              return CC_NZ;
  case ReqBindJmpGt:                 return CC_G;
  case ReqBindJmpGte:                return CC_GE;
  case ReqBindJmpLt:                 return CC_L;
  case ReqBindJmpLte:                return CC_LE;
  case ReqBindJmpEq:                 return CC_E;
  case ReqBindJmpNeq:                return CC_NE;
  case ReqBindJmpSame:               return CC_E;
  case ReqBindJmpNSame:              return CC_NE;
  case ReqBindJmpInstanceOfBitmask:  return CC_NZ;
  case ReqBindJmpNInstanceOfBitmask: return CC_Z;
  case ReqBindJmpZero:               return CC_Z;
  case ReqBindJmpNZero:              return CC_NZ;
  case SideExitJmpGt:                 return CC_G;
  case SideExitJmpGte:                return CC_GE;
  case SideExitJmpLt:                 return CC_L;
  case SideExitJmpLte:                return CC_LE;
  case SideExitJmpEq:                 return CC_E;
  case SideExitJmpNeq:                return CC_NE;
  case SideExitJmpSame:               return CC_E;
  case SideExitJmpNSame:              return CC_NE;
  case SideExitJmpInstanceOfBitmask:  return CC_NZ;
  case SideExitJmpNInstanceOfBitmask: return CC_Z;
  case SideExitJmpZero:               return CC_Z;
  case SideExitJmpNZero:              return CC_NZ;
  default:
    always_assert(0);
  }
}

}}}
