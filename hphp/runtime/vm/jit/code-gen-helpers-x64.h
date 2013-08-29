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

namespace HPHP {
struct Func;
namespace JIT {

class SSATmp;

using HPHP::Transl::PhysReg;

/*
 * This namespace is intended to house stateless code emission helpers (as a
 * complement to code-gen which maintains state.)
 */
namespace CodeGenHelpersX64 {

typedef Transl::X64Assembler Asm;

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
void emitLea(Asm& as, Transl::MemoryRef mr, PhysReg dst);

void emitLdObjClass(Asm& as, PhysReg objReg, PhysReg dstReg);
void emitLdClsCctx(Asm& as, PhysReg srcReg, PhysReg dstReg);

void emitExitSlowStats(Asm& as, const Func* func, SrcKey dest);

template<class Mem>
void emitLoadReg(Asm& as, Mem mem, PhysReg reg) {
  assert(reg != Transl::InvalidReg);
  if (reg.isGP()) {
    as. loadq(mem, reg);
  } else {
    as. movsd(mem, reg);
  }
}

template<class Mem>
void emitStoreReg(Asm& as, PhysReg reg, Mem mem) {
  assert(reg != Transl::InvalidReg);
  if (reg.isGP()) {
    as. storeq(reg, mem);
  } else {
    as. movsd(reg, mem);
  }
}

void shuffle2(Asm& as, PhysReg s0, PhysReg s1, PhysReg d0, PhysReg d1);

void zeroExtendIfBool(Asm& as, const SSATmp* src, PhysReg reg);

Transl::ConditionCode opToConditionCode(Opcode opc);

};

}}

#endif
