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
#include "hphp/runtime/vm/jit/vasm-x64.h"
#include "hphp/runtime/vm/jit/phys-loc.h"

namespace HPHP {
struct Func;
namespace jit {
struct Fixup;
struct SSATmp;
namespace x64 {

//////////////////////////////////////////////////////////////////////

typedef X64Assembler Asm;

constexpr size_t kJmpTargetAlign = 16;

void moveToAlign(CodeBlock& cb, size_t alignment = kJmpTargetAlign);

void emitEagerSyncPoint(Asm& as, const Op* pc);
void emitEagerSyncPoint(Vout&, const Op* pc);
void emitEagerVMRegSave(Asm& as, RegSaveFlags flags);
void emitGetGContext(Asm& as, PhysReg dest);
void emitGetGContext(Vout& as, Vreg dest);

void emitTransCounterInc(Asm& a);
void emitTransCounterInc(Vout&);

void emitIncRef(Asm& as, PhysReg base);
void emitIncRef(Vout&, Vreg base);
void emitIncRefCheckNonStatic(Asm& as, PhysReg base, DataType dtype);
void emitIncRefGenericRegSafe(Asm& as, PhysReg base, int disp, PhysReg tmpReg);

void emitAssertFlagsNonNegative(Vout&);
void emitAssertRefCount(Vout&, Vreg base);

void emitMovRegReg(Asm& as, PhysReg srcReg, PhysReg dstReg);
void emitLea(Asm& as, MemoryRef mr, PhysReg dst);

void emitLdObjClass(Vout&, Vreg objReg, Vreg dstReg);
void emitLdClsCctx(Vout&, Vreg srcReg, Vreg dstReg);

void emitCall(Asm& as, TCA dest, RegSet args);
void emitCall(Asm& as, CppCall call, RegSet args);
void emitCall(Vout&, CppCall call, RegSet args);

// store imm to the 8-byte memory location at ref. Warning: don't use this
// if you wanted an atomic store; large imms cause two stores.
void emitImmStoreq(Vout& v, Immed64 imm, Vptr ref);
void emitImmStoreq(Asm& as, Immed64 imm, MemoryRef ref);

void emitJmpOrJcc(Asm& as, ConditionCode cc, TCA dest);

void emitRB(Asm& a, Trace::RingBufferType t, const char* msgm);

void emitTraceCall(CodeBlock& cb, Offset pcOff);

/*
 * Tests the surprise flags for the current thread. Should be used
 * before a jnz to surprise handling code.
 */
void emitTestSurpriseFlags(Asm& as);
void emitTestSurpriseFlags(Vout&);

void emitCheckSurpriseFlagsEnter(Vout& main, Vout& cold, Fixup fixup);
void emitCheckSurpriseFlagsEnter(CodeBlock& mainCode, CodeBlock& coldCode,
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
emitTLSLoad(Vout& v, const ThreadLocalNoCheck<T>& datum, Vreg reg) {
  uintptr_t virtualAddress = uintptr_t(&datum.m_node.m_p) - tlsBase();
  Vptr addr{baseless(virtualAddress), Vptr::FS};
  v << load{addr, reg};
}

template<typename T>
inline void
emitTLSLoad(X64Assembler& a, const ThreadLocalNoCheck<T>& datum, Reg64 reg) {
  uintptr_t virtualAddress = uintptr_t(&datum.m_node.m_p) - tlsBase();
  a.fs().loadq(baseless(virtualAddress), reg);
}

#else // USE_GCC_FAST_TLS

template<typename T>
inline void
emitTLSLoad(Vout& v, const ThreadLocalNoCheck<T>& datum, Vreg dest) {
  PhysRegSaver(v, kGPCallerSaved); // we don't know for sure what's alive
  v << ldimm{datum.m_key, argNumToRegName[0]};
  const CodeAddress addr = (CodeAddress)pthread_getspecific;
  if (deltaFits((uintptr_t)addr, sz::dword)) {
    v << call{addr, argSet(1)};
  } else {
    v << ldimm{addr, reg::rax};
    v << callr{reg::rax, argSet(1)};
  }
  if (dest != Vreg(reg::rax)) {
    v << movq{reg::rax, dest};
  }
}

template<typename T>
inline void
emitTLSLoad(X64Assembler& a, const ThreadLocalNoCheck<T>& datum, Reg64 dest) {
  PhysRegSaver(a, kGPCallerSaved); // we don't know for sure what's alive
  a.    emitImmReg(datum.m_key, argNumToRegName[0]);
  const TCA addr = (TCA)pthread_getspecific;
  if (deltaFits((uintptr_t)addr, sz::dword)) {
    a.    call(addr);
  } else {
    a.    movq(addr, reg::rax);
    a.    call(reg::rax);
  }
  if (dest != reg::rax) {
    a.    movq(reg::rax, dest);
  }
}

#endif // USE_GCC_FAST_TLS

// Emit a load of a low pointer.
void emitLdLowPtr(Vout&, Vptr mem, Vreg reg, size_t size);

void emitCmpClass(Vout&, const Class* c, Vptr mem);
void emitCmpClass(Vout&, Vreg reg, Vptr mem);
void emitCmpClass(Vout&, Vreg reg1, Vreg reg2);

void copyTV(Vout&, Vloc src, Vloc dst);
void pack2(Vout&, Vreg s0, Vreg s1, Vreg d0);

Vreg zeroExtendIfBool(Vout&, const SSATmp* src, Vreg reg);

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

inline MemoryRef lookupDestructor(X64Assembler& a, PhysReg typeReg) {
  auto const table = reinterpret_cast<intptr_t>(g_destructors);
  always_assert_flog(deltaFits(table, sz::dword),
    "Destructor function table is expected to be in the data "
    "segment, with addresses less than 2^31"
  );
  static_assert((KindOfString        >> kShiftDataTypeToDestrIndex == 1) &&
                (KindOfArray         >> kShiftDataTypeToDestrIndex == 2) &&
                (KindOfObject        >> kShiftDataTypeToDestrIndex == 3) &&
                (KindOfResource      >> kShiftDataTypeToDestrIndex == 4) &&
                (KindOfRef           >> kShiftDataTypeToDestrIndex == 5),
                "lookup of destructors depends on KindOf* values");
  a.    shrl   (kShiftDataTypeToDestrIndex, r32(typeReg));
  return baseless(typeReg*8 + table);
}

inline MemoryRef lookupDestructor(Vout& v, PhysReg typeReg) {
  auto const table = reinterpret_cast<intptr_t>(g_destructors);
  always_assert_flog(deltaFits(table, sz::dword),
    "Destructor function table is expected to be in the data "
    "segment, with addresses less than 2^31"
  );
  static_assert((KindOfString        >> kShiftDataTypeToDestrIndex == 1) &&
                (KindOfArray         >> kShiftDataTypeToDestrIndex == 2) &&
                (KindOfObject        >> kShiftDataTypeToDestrIndex == 3) &&
                (KindOfResource      >> kShiftDataTypeToDestrIndex == 4) &&
                (KindOfRef           >> kShiftDataTypeToDestrIndex == 5),
                "lookup of destructors depends on KindOf* values");
  v << shrli{kShiftDataTypeToDestrIndex, typeReg, typeReg};
  return baseless(typeReg*8 + table);
}

//////////////////////////////////////////////////////////////////////

}}}

#endif
