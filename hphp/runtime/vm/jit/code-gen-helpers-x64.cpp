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

#include "hphp/util/asm-x64.h"
#include "hphp/util/ringbuffer.h"
#include "hphp/util/trace.h"

#include "hphp/runtime/base/arch.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/stats.h"
#include "hphp/runtime/base/types.h"
#include "hphp/runtime/vm/jit/back-end.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/jit/mc-generator.h"
#include "hphp/runtime/vm/jit/mc-generator-internal.h"
#include "hphp/runtime/vm/jit/translator.h"
#include "hphp/runtime/vm/jit/ir.h"
#include "hphp/runtime/vm/jit/code-gen-x64.h"

namespace HPHP { namespace JIT { namespace X64 {

//////////////////////////////////////////////////////////////////////

using namespace JIT::reg;

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

void emitEagerSyncPoint(Asm& as, const Op* pc) {
  // we can use rAsm because we don't clobber it in X64Assembler
  as.  storeq(rVmFp, rVmTl[RDS::kVmfpOff]);
  as.  storeq(rVmSp, rVmTl[RDS::kVmspOff]);
  emitImmStoreq(as, intptr_t(pc), rVmTl[RDS::kVmpcOff]);
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
  assert(!kSpecialCrossTraceRegs.contains(rdi));

  as.   storeq (rVmSp, rVmTl[RDS::kVmspOff]);
  if (savePC) {
    // We're going to temporarily abuse rVmSp to hold the current unit.
    Reg64 rBC = rVmSp;
    as. push   (rBC);
    // m_fp -> m_func -> m_unit -> m_bc + pcReg
    as. loadq  (rVmFp[AROFF(m_func)], rBC);
    as. loadq  (rBC[Func::unitOff()], rBC);
    as. loadq  (rBC[Unit::bcOff()], rBC);
    as. addq   (rBC, pcReg);
    as. storeq (pcReg, rVmTl[RDS::kVmpcOff]);
    as. pop    (rBC);
  }
  if (saveFP) {
    as. storeq (rVmFp, rVmTl[RDS::kVmfpOff]);
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
  if (!tx->isTransDBEnabled()) return;

  a.    movq (tx->getTransCounterAddr(), rAsm);
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
    as.   loadq  (base[disp + TVOFF(m_data)], tmpReg);
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
  ifThen(as, CC_NLE, [&] {
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

void emitLdObjClass(Asm& as, PhysReg objReg, PhysReg dstReg) {
  emitLdLowPtr(as, objReg[ObjectData::getVMClassOffset()],
               dstReg, sizeof(LowClassPtr));
}

void emitLdClsCctx(Asm& as, PhysReg srcReg, PhysReg dstReg) {
  emitMovRegReg(as, srcReg, dstReg);
  as.   decq(dstReg);
}

void emitCall(Asm& a, TCA dest) {
  if (a.jmpDeltaFits(dest) && !Stats::enabled()) {
    a.    call(dest);
  } else {
    dest = mcg->getNativeTrampoline(dest);
    if (a.jmpDeltaFits(dest)) {
      a.call(dest);
    } else {
      // can't do a near call; store address in data section.
      // call by loading the address using rip-relative addressing.  This
      // assumes the data section is near the current code section.  Since
      // this sequence is directly in-line, rip-relative like this is
      // more compact than loading a 64-bit immediate.
      TCA* addr = mcg->allocData<TCA>(sizeof(TCA), 1);
      *addr = dest;
      a.call(rip[(intptr_t)addr]);
      assert(((int32_t*)a.frontier())[-1] + a.frontier() == (TCA)addr);
    }
  }
}

void emitCall(Asm& a, CppCall call) {
  switch (call.kind()) {
  case CppCall::Kind::Direct:
    return emitCall(a, static_cast<TCA>(call.address()));
  case CppCall::Kind::Virtual:
    // Virtual call.
    // Load method's address from proper offset off of object in rdi,
    // using rax as scratch.
    a.  loadq   (*rdi, rax);
    a.  call    (rax[call.vtableOffset()]);
    return;
  case CppCall::Kind::Indirect:
    a.  call    (call.reg());
    return;
  case CppCall::Kind::ArrayVirt:
    {
      auto const addr = reinterpret_cast<intptr_t>(call.arrayTable());
      always_assert_flog(
        deltaFits(addr, sz::dword),
        "Array data vtables are expected to be in the data "
        "segment, with addresses less than 2^31"
      );
      a.    loadzbl (rdi[ArrayData::offsetofKind()], eax);
      a.    call    (baseless(rax*8 + addr));
    }
    return;
  }
  not_reached();
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
  Asm a { cb };
  // call to a trace function
  a.    lea    (rip[(int64_t)a.frontier()], rcx);
  a.    movq   (rVmFp, rdi);
  a.    movq   (rVmSp, rsi);
  a.    movq   (pcOff, rdx);
  // do the call; may use a trampoline
  emitCall(a, reinterpret_cast<TCA>(traceCallback));
}

void emitTestSurpriseFlags(Asm& a) {
  static_assert(RequestInjectionData::LastFlag < (1LL << 32),
                "Translator assumes RequestInjectionFlags fit in 32-bit int");
  a.    testl((int32_t)0xffffffff, rVmTl[RDS::kConditionFlagsOff]);
}

void emitCheckSurpriseFlagsEnter(CodeBlock& mainCode, CodeBlock& coldCode,
                                 Fixup fixup) {
  Asm a { mainCode };
  Asm acold { coldCode };

  emitTestSurpriseFlags(a);
  a.  jnz  (coldCode.frontier());

  acold.  movq  (rVmFp, argNumToRegName[0]);
  emitCall(acold, tx->uniqueStubs.functionEnterHelper);
  mcg->recordSyncPoint(coldCode.frontier(),
                       fixup.m_pcOffset, fixup.m_spOffset);
  acold.  jmp   (mainCode.frontier());
}

template<class Mem>
void emitCmpClass(Asm& as, Reg64 reg, Mem mem) {
  auto size = sizeof(LowClassPtr);

  if (size == 8) {
    as.   cmpq    (reg, mem);
  } else if (size == 4) {
    as.   cmpl    (r32(reg), mem);
  } else {
    not_implemented();
  }
}

template void emitCmpClass<MemoryRef>(Asm& as, Reg64 reg, MemoryRef mem);

void emitCmpClass(Asm& as, Reg64 reg1, PhysReg reg2) {
  auto size = sizeof(LowClassPtr);

  if (size == 8) {
    as.   cmpq    (reg1, reg2);
  } else if (size == 4) {
    as.   cmpl    (r32(reg1), r32(reg2));
  } else {
    not_implemented();
  }
}

void shuffle2(Asm& as, PhysReg s0, PhysReg s1, PhysReg d0, PhysReg d1) {
  if (s0 == InvalidReg && s1 == InvalidReg &&
      d0 == InvalidReg && d1 == InvalidReg) return;
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
    as.   movq_rx(s0, d0);
    as.   movq_rx(s1, rCgXMM0);
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
  case JmpGtInt:              return CC_G;
  case JmpGteInt:             return CC_GE;
  case JmpLtInt:              return CC_L;
  case JmpLteInt:             return CC_LE;
  case JmpEqInt:              return CC_E;
  case JmpNeqInt:             return CC_NE;
  case JmpSame:               return CC_E;
  case JmpNSame:              return CC_NE;
  case JmpInstanceOfBitmask:  return CC_NZ;
  case JmpNInstanceOfBitmask: return CC_Z;
  case JmpZero:               return CC_Z;
  case JmpNZero:              return CC_NZ;
  case ReqBindJmpGt:                 return CC_G;
  case ReqBindJmpGte:                return CC_GE;
  case ReqBindJmpLt:                 return CC_L;
  case ReqBindJmpLte:                return CC_LE;
  case ReqBindJmpEq:                 return CC_E;
  case ReqBindJmpNeq:                return CC_NE;
  case ReqBindJmpGtInt:              return CC_G;
  case ReqBindJmpGteInt:             return CC_GE;
  case ReqBindJmpLtInt:              return CC_L;
  case ReqBindJmpLteInt:             return CC_LE;
  case ReqBindJmpEqInt:              return CC_E;
  case ReqBindJmpNeqInt:             return CC_NE;
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
  case SideExitJmpGtInt:              return CC_G;
  case SideExitJmpGteInt:             return CC_GE;
  case SideExitJmpLtInt:              return CC_L;
  case SideExitJmpLteInt:             return CC_LE;
  case SideExitJmpEqInt:              return CC_E;
  case SideExitJmpNeqInt:             return CC_NE;
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
