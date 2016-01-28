/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/vm/jit/irlower-internal.h"

#include "hphp/runtime/vm/jit/ir-instruction.h"
#include "hphp/runtime/vm/jit/ssa-tmp.h"
#include "hphp/runtime/vm/jit/vasm-gen.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"
#include "hphp/runtime/vm/jit/vasm-reg.h"

#include "hphp/util/asm-x64.h"

#include <utility>

namespace HPHP { namespace jit { namespace irlower {

///////////////////////////////////////////////////////////////////////////////

namespace {

///////////////////////////////////////////////////////////////////////////////

void implCmp(IRLS& env, const IRInstruction* inst, ConditionCode cc) {
  auto const s0 = srcLoc(env, inst, 0).reg();
  auto const s1 = srcLoc(env, inst, 1).reg();
  auto const d  = dstLoc(env, inst, 0).reg();
  auto& v = vmain(env);
  auto const sf = v.makeReg();

  // Vasm uses AT&T syntax, so this will set flags after doing `s0 - s1`.
  v << cmpq{s1, s0, sf};
  v << setcc{cc, sf, d};
}

void implCmpBool(IRLS& env, const IRInstruction* inst, ConditionCode cc) {
  auto const s0 = srcLoc(env, inst, 0).reg();
  auto const s1 = srcLoc(env, inst, 1).reg();
  auto const d  = dstLoc(env, inst, 0).reg();

  auto& v = vmain(env);
  auto const sf = v.makeReg();

  assert(inst->src(0)->type() <= TBool);
  assert(inst->src(1)->type() <= TBool);

  auto const ty1 = inst->src(1)->type();

  if (ty1.hasConstVal()) {
    // Emit testb when possible to enable more optimizations later on.
    if (cc == CC_E || cc == CC_NE) {
      if (ty1.boolVal()) {
        cc = ccNegate(cc);
      }
      v << testb{s0, s0, sf};
    } else {
      v << cmpbi{ty1.boolVal(), s0, sf};
    }
  } else {
    v << cmpb{s1, s0, sf};
  }
  v << setcc{cc, sf, d};
}

void implCmpEqDbl(IRLS& env, const IRInstruction* inst, ComparisonPred pred) {
  auto const s0 = srcLoc(env, inst, 0).reg();
  auto const s1 = srcLoc(env, inst, 1).reg();
  auto const d  = dstLoc(env, inst, 0).reg();

  auto& v = vmain(env);
  auto const tmp = v.makeReg();

  v << cmpsd{pred, s0, s1, tmp};
  v << andbi{1, tmp, d, v.makeReg()};
}

void implCmpRelDbl(IRLS& env, const IRInstruction* inst,
                 ConditionCode cc, bool invert) {
  auto s0 = srcLoc(env, inst, 0).reg();
  auto s1 = srcLoc(env, inst, 1).reg();
  auto const d = dstLoc(env, inst, 0).reg();
  auto& v = vmain(env);
  auto const sf = v.makeReg();

  if (invert) std::swap(s0, s1);
  v << ucomisd{s0, s1, sf};
  v << setcc{cc, sf, d};
}

///////////////////////////////////////////////////////////////////////////////

}

///////////////////////////////////////////////////////////////////////////////

#define CMP_OPS   \
  CO(Gt,  CC_G)   \
  CO(Gte, CC_GE)  \
  CO(Lt,  CC_L)   \
  CO(Lte, CC_LE)  \
  CO(Eq,  CC_E)   \
  CO(Neq, CC_NE)

#define CO(Inst, cc)                                            \
  void cg##Inst##Int(IRLS& env, const IRInstruction* inst) {    \
    implCmp(env, inst, cc);                                     \
  }                                                             \
  void cg##Inst##Bool(IRLS& env, const IRInstruction* inst) {   \
    implCmpBool(env, inst, cc);                                 \
  }
CMP_OPS
#undef CO

#undef CMP_OPS

void cgCmpBool(IRLS& env, const IRInstruction* inst) {
  auto const s0 = srcLoc(env, inst, 0).reg();
  auto const s1 = srcLoc(env, inst, 1).reg();
  auto const d  = dstLoc(env, inst, 0).reg();

  auto& v = vmain(env);
  auto const sf = v.makeReg();
  auto const extended0 = v.makeReg();
  auto const extended1 = v.makeReg();

  assert(inst->src(0)->type() <= TBool);
  assert(inst->src(1)->type() <= TBool);

  v << movzbq{s0, extended0};
  v << movzbq{s1, extended1};
  v << subq{extended1, extended0, d, sf};
}

void cgCmpInt(IRLS& env, const IRInstruction* inst) {
  auto const s0 = srcLoc(env, inst, 0).reg();
  auto const s1 = srcLoc(env, inst, 1).reg();
  auto const d  = dstLoc(env, inst, 0).reg();

  auto& v = vmain(env);
  auto const sf = v.makeReg();
  auto const tmp1 = v.makeReg();
  auto const tmp2 = v.makeReg();

  assert(inst->src(0)->type() <= TInt);
  assert(inst->src(1)->type() <= TInt);

  v << cmpq{s1, s0, sf};
  v << setcc{CC_G, sf, tmp1};
  v << movzbq{tmp1, tmp2};
  v << cmovq{CC_L, sf, tmp2, v.cns(-1), d};
}

///////////////////////////////////////////////////////////////////////////////

void cgEqDbl(IRLS& env, const IRInstruction* inst)  {
  implCmpEqDbl(env, inst, ComparisonPred::eq_ord);
}

void cgNeqDbl(IRLS& env, const IRInstruction* inst) {
  implCmpEqDbl(env, inst, ComparisonPred::ne_unord);
}

/*
 * This is a little tricky, because "unordered" is a thing.
 *
 *         ZF  PF  CF
 * x ?= y   1   1   1
 * x <  y   0   0   1
 * x == y   1   0   0
 * x >  y   0   0   0
 *
 * The condition codes B and BE are true if CF == 1, which it is in the
 * unordered case, which will give incorrect results.  So we just invert the
 * condition code (A and AE don't get set if CF == 1) and flip the operands.
 * This trick lets us avoid special-casing unordered.
 *
 * NB: This is clearly an x64-ism, and likely needs to be done differently for
 * other architectures.
 */
#define CMP_DBL_OPS         \
  CDO(Gt,   CC_A,   false)  \
  CDO(Gte,  CC_AE,  false)  \
  CDO(Lt,   CC_A,   true)   \
  CDO(Lte,  CC_AE,  true)

#define CDO(Inst, cc, invert) \
  void cg##Inst##Dbl(IRLS& env, const IRInstruction* inst) {  \
    implCmpRelDbl(env, inst, cc, invert);                     \
  }
CMP_DBL_OPS
#undef CDO

#undef CMP_DBL_OPS

void cgCmpDbl(IRLS& env, const IRInstruction* inst) {
  auto const s0 = srcLoc(env, inst, 0).reg();
  auto const s1 = srcLoc(env, inst, 1).reg();
  auto const d  = dstLoc(env, inst, 0).reg();

  auto& v = vmain(env);
  auto const sf = v.makeReg();
  auto const tmp1 = v.makeReg();
  auto const tmp2 = v.makeReg();

  assert(inst->src(0)->type() <= TDbl);
  assert(inst->src(1)->type() <= TDbl);

  v << ucomisd{s0, s1, sf};
  v << cmovq{CC_A, sf, v.cns(-1), v.cns(1), tmp1};
  v << cmovq{CC_NE, sf, v.cns(0), tmp1, tmp2};
  v << cmovq{CC_P, sf, tmp2, v.cns(-1), d};
}

///////////////////////////////////////////////////////////////////////////////

#define CMP_DATA_OPS        \
  CDO(Obj,  Same,   CC_E)   \
  CDO(Obj,  NSame,  CC_NE)  \
  CDO(Res,  Eq,     CC_E)   \
  CDO(Res,  Neq,    CC_NE)

#define CDO(Ty, Inst, cc)                                     \
  void cg##Inst##Ty(IRLS& env, const IRInstruction* inst) {   \
    assertx(inst->src(0)->type() <= T##Ty);                   \
    assertx(inst->src(1)->type() <= T##Ty);                   \
    implCmp(env, inst, cc);                                   \
  }
CMP_DATA_OPS
#undef CDO

#undef CMP_DATA_OPS

///////////////////////////////////////////////////////////////////////////////

}}}
