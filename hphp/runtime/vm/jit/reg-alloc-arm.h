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

#ifndef incl_HPHP_VM_REG_ALLOC_ARM_H_
#define incl_HPHP_VM_REG_ALLOC_ARM_H_

#include "hphp/runtime/vm/jit/reg-alloc.h"
#include "hphp/runtime/vm/jit/native-calls.h"

namespace HPHP {  namespace JIT {

using NativeCalls::CallMap;

namespace ARM {

// Return true if the CodeGenerator method for this instruction can
// handle an immediate for the ith source operand, usually by selecting
// a special form of the necessary instruction.  The value of the immediate
// can affect this decision; we look at the value here, and trust it
// blindly in CodeGenerator.
bool mayUseConst(const IRInstruction& inst, unsigned i) {
  assert(inst.src(i)->isConst());
  union {
    int64_t cint;
    double cdouble;
  };
  auto type = inst.src(i)->type();
  cint = type.hasRawVal() ? type.rawVal() : 0;
  // (almost?) any instruction that accepts a GPR, can accept XZR in
  // place of an immediate zero. TODO #3827905
  switch (inst.op()) {
  case GuardRefs:
    if (i == 1) return inst.src(2)->intVal() == 0; // nParams
    if (i == 3) { // mask64
      return vixl::Assembler::IsImmLogical(cint, vixl::kXRegSize);
    }
    if (i == 4) { // vals64
      return vixl::Assembler::IsImmArithmetic(cint);
    }
    break;
  case AddInt:
  case SubInt:
  case EqInt:
  case NeqInt:
  case LtInt:
  case GtInt:
  case LteInt:
  case GteInt:
    if (i == 1) {
      return vixl::Assembler::IsImmArithmetic(cint);
    }
    break;

  //TODO: t3944093 add constraints for existing arm codegen
  default:
    break;
  }
  if (CallMap::hasInfo(inst.op())) {
    // shuffleArgs() knows what to do with immediates.
    // TODO: #3634984 ... but it needs a scratch register
    return true;
  }
  return false;
}

Constraint srcConstraint(const IRInstruction& inst, unsigned i) {
  Constraint c { Constraint::GP };
  if (inst.src(i)->isConst() && mayUseConst(inst, i)) {
    c |= Constraint::IMM;
  }
  if (inst.src(i)->type() <= Type::Dbl) {
    c |= Constraint::SIMD;
  }
  return c;
}

Constraint dstConstraint(const IRInstruction& inst, unsigned i) {
  Constraint c { Constraint::GP };
  if (inst.dst(i)->type() <= Type::Dbl) {
    c |= Constraint::SIMD;
  }
  return c;
}

}}}

#endif
