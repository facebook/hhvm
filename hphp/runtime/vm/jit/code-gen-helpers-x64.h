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

#ifndef incl_HPHP_VM_CODEGENHELPERS_X64_H_
#define incl_HPHP_VM_CODEGENHELPERS_X64_H_

#include "hphp/util/asm-x64.h"
#include "hphp/runtime/base/types.h"
#include "hphp/runtime/vm/jit/phys-reg.h"
#include "hphp/runtime/vm/jit/translator.h"
#include "hphp/runtime/vm/jit/code-gen-helpers.h"
#include "hphp/runtime/vm/jit/service-requests.h"
#include "hphp/runtime/vm/jit/abi-x64.h"

namespace HPHP {
struct Func;
namespace JIT {
struct SSATmp;
namespace X64 {

using namespace Transl; // XXX: this namespace should go away

//////////////////////////////////////////////////////////////////////

typedef X64Assembler Asm;

constexpr size_t kJmpTargetAlign = 16;

void moveToAlign(Asm &aa, const size_t alignment = kJmpTargetAlign,
                 const bool unreachable = true);

void emitEagerSyncPoint(Asm& as, const HPHP::Opcode* pc,
                                 const Offset spDiff);
void emitEagerVMRegSave(Asm& as, RegSaveFlags flags);
void emitGetGContext(Asm& as, PhysReg dest);

void emitIncRef(Asm& as, PhysReg base);
void emitIncRefCheckNonStatic(Asm& as, PhysReg base, DataType dtype);
void emitIncRefGenericRegSafe(Asm& as, PhysReg base, int disp, PhysReg tmpReg);

void emitAssertFlagsNonNegative(Asm& as);
void emitAssertRefCount(Asm& as, PhysReg base);

void emitMovRegReg(Asm& as, PhysReg srcReg, PhysReg dstReg);
void emitLea(Asm& as, PhysReg base, int disp, PhysReg dest);
void emitLea(Asm& as, MemoryRef mr, PhysReg dst);

void emitLdObjClass(Asm& as, PhysReg objReg, PhysReg dstReg);
void emitLdClsCctx(Asm& as, PhysReg srcReg, PhysReg dstReg);

void emitExitSlowStats(Asm& as, const Func* func, SrcKey dest);

void emitCall(Asm& as, TCA dest);
void emitCall(Asm& as, CppCall call);

/*
 * Tests the surprise flags for the current thread. Should be used
 * before a jnz to surprise handling code.
 */
void emitTestSurpriseFlags(Asm& as);

void emitCheckSurpriseFlagsEnter(CodeBlock& mainCode, CodeBlock& stubsCode,
                                 bool inTracelet, FixupMap& fixupMap,
                                 Fixup fixup);

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
  return scratch[typeReg*8];
}

inline void callDestructor(Asm& a, PhysReg typeReg, PhysReg scratch) {
  a.    call   (lookupDestructor(a, typeReg, scratch));
}

inline void jumpDestructor(Asm& a, PhysReg typeReg, PhysReg scratch) {
  a.    jmp    (lookupDestructor(a, typeReg, scratch));
}

void emitIncRef(Asm& a, PhysReg base);
void emitIncRefCheckNonStatic(Asm& a, PhysReg base, DataType dtype);
void emitIncRefGenericRegSafe(Asm& a, PhysReg base,
                              int disp, PhysReg tmpReg);

void emitAssertFlagsNonNegative(Asm& as);
void emitAssertRefCount(Asm& as, PhysReg base);

/*
 * Use the function templates below to conveniently build the arg vector.
 */
TCA emitServiceReqWork(Asm& as, TCA start, bool persist, SRFlags flags,
                       ServiceRequest req, const ServiceReqArgVec& argInfo);

template<typename... Arg>
TCA emitServiceReq(Asm& as, SRFlags flags, ServiceRequest sr, Arg... a) {
  // These should reuse stubs. Use emitEphemeralServiceReq.
  assert(sr != REQ_BIND_JMPCC_FIRST &&
         sr != REQ_BIND_JMPCC_SECOND &&
         sr != REQ_BIND_JMP);

  ServiceReqArgVec argv;
  packServiceReqArgs(argv, a...);
  return emitServiceReqWork(as, as.frontier(), true, flags, sr, argv);
}

template<typename... Arg>
TCA emitServiceReq(Asm& as, ServiceRequest sr, Arg... a) {
  return emitServiceReq(as, SRFlags::None, sr, a...);
}

template<typename... Arg>
TCA emitEphemeralServiceReq(Asm& as, TCA start, ServiceRequest sr,
                            Arg... a) {
  assert(sr == REQ_BIND_JMPCC_FIRST ||
         sr == REQ_BIND_JMPCC_SECOND ||
         sr == REQ_BIND_JMP);
  assert(as.contains(start));

  ServiceReqArgVec argv;
  packServiceReqArgs(argv, a...);
  return emitServiceReqWork(as, start, false, SRFlags::None, sr, argv);
}

//////////////////////////////////////////////////////////////////////

}}}

#endif
