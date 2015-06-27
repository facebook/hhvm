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
#include "hphp/runtime/vm/jit/abi-x64.h"
#include "hphp/runtime/vm/jit/code-gen-cf.h"
#include "hphp/runtime/vm/jit/code-gen-helpers.h"
#include "hphp/runtime/vm/jit/cpp-call.h"
#include "hphp/runtime/vm/jit/ir-opcode.h"
#include "hphp/runtime/vm/jit/phys-reg.h"
#include "hphp/runtime/vm/jit/service-requests-x64.h"
#include "hphp/runtime/vm/jit/service-requests.h"
#include "hphp/runtime/vm/jit/translator.h"
#include "hphp/runtime/vm/jit/vasm-emit.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"
#include "hphp/runtime/vm/jit/vasm-reg.h"

namespace HPHP {
//////////////////////////////////////////////////////////////////////

struct Func;

namespace jit {
//////////////////////////////////////////////////////////////////////

struct Fixup;
struct SSATmp;

namespace x64 {
//////////////////////////////////////////////////////////////////////

typedef X64Assembler Asm;

constexpr size_t kJmpTargetAlign = 16;

void moveToAlign(CodeBlock& cb, size_t alignment = kJmpTargetAlign);

void emitEagerSyncPoint(Vout& v, const Op* pc, Vreg rds, Vreg vmfp, Vreg vmsp);
void emitGetGContext(Asm& as, PhysReg dest);
void emitGetGContext(Vout& as, Vreg dest);

void emitTransCounterInc(Asm& a);
void emitTransCounterInc(Vout&);

/*
 * Emit a decrement on the m_count field of `base', which must contain a
 * reference counted heap object.  This helper also conditionally makes some
 * sanity checks on the reference count of the object.
 *
 * Returns: the status flags register for the decrement instruction.
 */
Vreg emitDecRef(Vout& v, Vreg base);

/*
 * Assuming rData is the data pointer for a refcounted (but possibly static)
 * value, emit a static check and DecRef, executing the code emitted by
 * `destroy' if the count would go to zero.
 */
template<class Destroy>
void emitDecRefWork(Vout& v, Vout& vcold, Vreg rData,
                    Destroy destroy, bool unlikelyDestroy) {
  auto const sf = v.makeReg();
  v << cmplim{1, rData[FAST_REFCOUNT_OFFSET], sf};
  ifThenElse(
    v, vcold, CC_E, sf,
    destroy,
    [&] (Vout& v) {
      /*
       * If it's not static, actually reduce the reference count.  This does
       * another branch using the same status flags from the cmplim above.
       */
      ifThen(
        v, CC_NL, sf,
        [&] (Vout& v) {
          emitDecRef(v, rData);
        }
      );
    },
    unlikelyDestroy
  );
}

void emitIncRef(Asm& as, PhysReg base);
void emitIncRef(Vout& v, Vreg base);
void emitIncRefCheckNonStatic(Asm& as, PhysReg base, DataType dtype);
void emitIncRefGenericRegSafe(Asm& as, PhysReg base, int disp, PhysReg tmpReg);

void emitAssertFlagsNonNegative(Vout& v, Vreg sf);
void emitAssertRefCount(Vout& v, Vreg base);

void emitMovRegReg(Asm& as, PhysReg srcReg, PhysReg dstReg);
void emitLea(Asm& as, MemoryRef mr, PhysReg dst);

Vreg emitLdObjClass(Vout& v, Vreg objReg, Vreg dstReg);
Vreg emitLdClsCctx(Vout& v, Vreg srcReg, Vreg dstReg);

void emitCall(Asm& as, TCA dest, RegSet args);
void emitCall(Asm& as, CppCall call, RegSet args);
void emitCall(Vout& v, CppCall call, RegSet args);

// store imm to the 8-byte memory location at ref. Warning: don't use this
// if you wanted an atomic store; large imms cause two stores.
void emitImmStoreq(Vout& v, Immed64 imm, Vptr ref);
void emitImmStoreq(Asm& as, Immed64 imm, MemoryRef ref);

void emitRB(Vout& v, Trace::RingBufferType t, const char* msg);

void emitCheckSurpriseFlagsEnter(Vout& main, Vout& cold, Vreg fp, Vreg rds,
                                 Fixup fixup, Vlabel catchBlock);

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
inline Vptr getTLSPtr(const T& data) {
  uintptr_t virtualAddress = uintptr_t(&data) - tlsBase();
  return Vptr{baseless(virtualAddress), Vptr::FS};
}

template<typename T>
inline void
emitTLSLoad(Vout& v, const ThreadLocalNoCheck<T>& datum, Vreg reg) {
  v << load{getTLSPtr(datum.m_node.m_p), reg};
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
  v << ldimmq{datum.m_key, argNumToRegName[0]};
  const CodeAddress addr = (CodeAddress)pthread_getspecific;
  if (deltaFits((uintptr_t)addr, sz::dword)) {
    v << call{addr, argSet(1)};
  } else {
    v << ldimmq{addr, reg::rax};
    v << callr{reg::rax, argSet(1)};
  }
  if (dest != Vreg(reg::rax)) {
    v << copy{reg::rax, dest};
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
void emitLdLowPtr(Vout& v, Vptr mem, Vreg reg, size_t size);

void emitCmpClass(Vout& v, Vreg sf, const Class* c, Vptr mem);
void emitCmpClass(Vout& v, Vreg sf, Vreg reg, Vptr mem);
void emitCmpClass(Vout& v, Vreg sf, Vreg reg1, Vreg reg2);

void emitCmpVecLen(Vout& v, Vreg sf, Vptr mem, Immed val);

void copyTV(Vout& v, Vloc src, Vloc dst, Type destType);
void pack2(Vout& v, Vreg s0, Vreg s1, Vreg d0);

Vreg zeroExtendIfBool(Vout& v, const SSATmp* src, Vreg reg);

template<ConditionCode Jcc, class Lambda>
void jccBlock(Asm& a, Lambda body) {
  Label exit;
  exit.jcc8(a, Jcc);
  body();
  asm_label(a, exit);
}

/*
 * lookupDestructor --
 *
 * Return a MemoryRef pointer to the destructor for the type in typeReg.
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

inline Vptr lookupDestructor(Vout& v, Vreg typeReg) {
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
  auto shiftedType = v.makeReg();
  v << shrli{kShiftDataTypeToDestrIndex, typeReg, shiftedType, v.makeReg()};
  return Vptr{Vreg{}, shiftedType, 8, safe_cast<int>(table)};
}

inline ptrdiff_t genOffset(bool isAsync) {
  return isAsync ? AsyncGenerator::objectOff() : Generator::objectOff();
}

//////////////////////////////////////////////////////////////////////

}}}

#endif
