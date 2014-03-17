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

#ifndef incl_HPHP_VM_CODEGENHELPERS_X64_H_
#define incl_HPHP_VM_CODEGENHELPERS_X64_H_

#include "hphp/util/asm-x64.h"
#include "hphp/util/ringbuffer.h"

#include "hphp/runtime/base/types.h"
#include "hphp/runtime/vm/jit/phys-reg.h"
#include "hphp/runtime/vm/jit/translator.h"
#include "hphp/runtime/vm/jit/code-gen-helpers.h"
#include "hphp/runtime/vm/jit/service-requests.h"
#include "hphp/runtime/vm/jit/service-requests-x64.h"
#include "hphp/runtime/vm/jit/abi-x64.h"
#include "hphp/runtime/vm/jit/ir.h"

namespace HPHP {
struct Func;
namespace JIT {
struct SSATmp;
namespace X64 {

//////////////////////////////////////////////////////////////////////

typedef X64Assembler Asm;

constexpr size_t kJmpTargetAlign = 16;

void moveToAlign(CodeBlock& cb, size_t alignment = kJmpTargetAlign);

void emitEagerSyncPoint(Asm& as, const Op* pc);
void emitEagerVMRegSave(Asm& as, RegSaveFlags flags);
void emitGetGContext(Asm& as, PhysReg dest);

void emitTransCounterInc(Asm& a);

void emitIncRef(Asm& as, PhysReg base);
void emitIncRefCheckNonStatic(Asm& as, PhysReg base, DataType dtype);
void emitIncRefGenericRegSafe(Asm& as, PhysReg base, int disp, PhysReg tmpReg);

void emitAssertFlagsNonNegative(Asm& as);
void emitAssertRefCount(Asm& as, PhysReg base);

void emitMovRegReg(Asm& as, PhysReg srcReg, PhysReg dstReg);
void emitLea(Asm& as, MemoryRef mr, PhysReg dst);

void emitLdObjClass(Asm& as, PhysReg objReg, PhysReg dstReg);
void emitLdClsCctx(Asm& as, PhysReg srcReg, PhysReg dstReg);

void emitCall(Asm& as, TCA dest);
void emitCall(Asm& as, CppCall call);

// store imm to the 8-byte memory location at ref. Warning: don't use this
// if you wanted an atomic store; large imms cause two stores.
template<class Ref>
void emitImmStoreq(Asm& as, Immed64 imm, Ref ref) {
  if (imm.fits(sz::dword)) {
    as.storeq(imm.l(), ref); // sign-extend to 64-bit then storeq
  } else {
    as.storel(int32_t(imm.q()), ref);
    as.storel(int32_t(imm.q() >> 32), Ref(ref.r + 4));
  }
}

void emitJmpOrJcc(Asm& as, ConditionCode cc, TCA dest);

void emitRB(Asm& a, Trace::RingBufferType t, const char* msgm,
            RegSet toSave = RegSet());

void emitTraceCall(CodeBlock& cb, int64_t pcOff);

/*
 * Tests the surprise flags for the current thread. Should be used
 * before a jnz to surprise handling code.
 */
void emitTestSurpriseFlags(Asm& as);

void emitCheckSurpriseFlagsEnter(CodeBlock& mainCode, CodeBlock& stubsCode,
                                 bool inTracelet, FixupMap& fixupMap,
                                 Fixup fixup);

#ifdef USE_GCC_FAST_TLS

/*
 * TLS access: XXX we currently only support static-style TLS directly
 * linked off of FS.
 *
 * x86 terminology review: "Virtual addresses" are subject to both
 * segmented translation and paged translation. "Linear addresses" are
 * post-segmentation address, subject only to paging. C and C++ generally
 * only have access to bitwise linear addresses.
 *
 * TLS data live at negative virtual addresses off FS: the first datum
 * is typically at VA(FS:-sizeof(datum)). Linux's x64 ABI stores the linear
 * address of the base of TLS at VA(FS:0). While this is just a convention, it
 * is firm: gcc builds binaries that assume it when, e.g., evaluating
 * "&myTlsDatum".
 *
 * The virtual addresses of TLS data are not exposed to C/C++. To figure it
 * out, we take a datum's linear address, and subtract it from the linear
 * address where TLS starts.
 */
template<typename T>
inline void
emitTLSLoad(X64Assembler& a, const ThreadLocalNoCheck<T>& datum,
            RegNumber reg) {
  uintptr_t virtualAddress = uintptr_t(&datum.m_node.m_p) - tlsBase();
  a.    fs().loadq(baseless(virtualAddress), r64(reg));
}

#else // USE_GCC_FAST_TLS

template<typename T>
inline void
emitTLSLoad(X64Assembler& a, const ThreadLocalNoCheck<T>& datum,
            RegNumber reg) {
  PhysRegSaver(a, kGPCallerSaved); // we don't know for sure what's alive
  a.    emitImmReg(&datum.m_key, argNumToRegName[0]);
  a.    call((TCA)pthread_getspecific);
  if (reg != reg::rax) {
    a.    movq(reg::rax, r64(reg));
  }
}

#endif // USE_GCC_FAST_TLS


template<class Mem>
void emitLoadReg(Asm& as, Mem mem, PhysReg reg) {
  assert(reg != InvalidReg);
  if (reg.isGP()) {
    as. loadq(mem, reg);
  } else {
    as. movsd(mem, reg);
  }
}

template<class Mem>
void emitStoreReg(Asm& as, PhysReg reg, Mem mem) {
  assert(reg != InvalidReg);
  if (reg.isGP()) {
    as. storeq(reg, mem);
  } else {
    as. movsd(reg, mem);
  }
}

void shuffle2(Asm& as, PhysReg s0, PhysReg s1, PhysReg d0, PhysReg d1);

void zeroExtendIfBool(Asm& as, const SSATmp* src, PhysReg reg);

ConditionCode opToConditionCode(Opcode opc);

template<ConditionCode Jcc, class Lambda>
void jccBlock(Asm& a, Lambda body) {
  Label exit;
  exit.jcc8(a, Jcc);
  body();
  asm_label(a, exit);
}

/*
 * callDestructor/jumpDestructor --
 *
 * Emit a call or jump to the appropriate destructor for a dynamically
 * typed value.
 *
 * No registers are saved; most translated code should be using
 * emitDecRefGeneric{Reg,} instead of this.
 *
 *   Inputs:
 *
 *     - typeReg is destroyed and may not be argNumToRegName[0].
 *     - argNumToRegName[0] should contain the m_data for this value.
 *     - scratch is destoyed.
 */

inline IndexedMemoryRef lookupDestructor(X64Assembler& a,
                                         PhysReg typeReg,
                                         PhysReg scratch) {
  assert(typeReg != r32(argNumToRegName[0]));
  assert(scratch != argNumToRegName[0]);

  static_assert((KindOfString        >> kShiftDataTypeToDestrIndex == 1) &&
                (KindOfArray         >> kShiftDataTypeToDestrIndex == 2) &&
                (KindOfObject        >> kShiftDataTypeToDestrIndex == 3) &&
                (KindOfResource      >> kShiftDataTypeToDestrIndex == 4) &&
                (KindOfRef           >> kShiftDataTypeToDestrIndex == 5),
                "lookup of destructors depends on KindOf* values");

  a.    shrl   (kShiftDataTypeToDestrIndex, r32(typeReg));
  a.    movq   (&g_destructors, scratch);
  return scratch[typeReg * 8];
}

inline void callDestructor(Asm& a, PhysReg typeReg, PhysReg scratch) {
  a.    call   (lookupDestructor(a, typeReg, scratch));
}

inline void jumpDestructor(Asm& a, PhysReg typeReg, PhysReg scratch) {
  a.    jmp    (lookupDestructor(a, typeReg, scratch));
}

inline void loadDestructorFunc(X64Assembler& a,
                               PhysReg typeReg,
                               PhysReg dstReg) {
  static_assert((KindOfString        >> kShiftDataTypeToDestrIndex == 1) &&
                (KindOfArray         >> kShiftDataTypeToDestrIndex == 2) &&
                (KindOfObject        >> kShiftDataTypeToDestrIndex == 3) &&
                (KindOfResource      >> kShiftDataTypeToDestrIndex == 4) &&
                (KindOfRef           >> kShiftDataTypeToDestrIndex == 5),
                "lookup of destructors depends on KindOf* values");

  a.    movsbl (rbyte(typeReg), r32(typeReg));
  a.    shrl   (kShiftDataTypeToDestrIndex, r32(typeReg));
  a.    movq   (&g_destructors, dstReg);
  a.    loadq  (dstReg[typeReg * 8], dstReg);
}


//////////////////////////////////////////////////////////////////////

}}}

#endif
