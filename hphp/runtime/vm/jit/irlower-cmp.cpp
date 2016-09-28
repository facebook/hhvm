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

#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/code-gen-cf.h"
#include "hphp/runtime/vm/jit/code-gen-helpers.h"
#include "hphp/runtime/vm/jit/ir-instruction.h"
#include "hphp/runtime/vm/jit/ir-opcode.h"
#include "hphp/runtime/vm/jit/ssa-tmp.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/jit/type.h"
#include "hphp/runtime/vm/jit/vasm-gen.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"
#include "hphp/runtime/vm/jit/vasm-reg.h"

#include "hphp/util/asm-x64.h"
#include "hphp/util/trace.h"

#include <utility>

namespace HPHP { namespace jit { namespace irlower {

TRACE_SET_MOD(irlower);

///////////////////////////////////////////////////////////////////////////////

namespace {

///////////////////////////////////////////////////////////////////////////////

void implCmp(IRLS& env, const IRInstruction* inst, ConditionCode cc) {
  auto const s0 = srcLoc(env, inst, 0).reg();
  auto const s1 = srcLoc(env, inst, 1).reg();
  auto const d  = dstLoc(env, inst, 0).reg();
  auto& v = vmain(env);
  auto const sf = v.makeReg();

  v << cmpq{s1, s0, sf};
  v << setcc{cc, sf, d};
}

void implCmpBool(IRLS& env, const IRInstruction* inst, ConditionCode cc) {
  auto const s0 = srcLoc(env, inst, 0).reg();
  auto const s1 = srcLoc(env, inst, 1).reg();
  auto const d  = dstLoc(env, inst, 0).reg();
  auto& v = vmain(env);
  auto const sf = v.makeReg();

  assertx(inst->src(0)->type() <= TBool);
  assertx(inst->src(1)->type() <= TBool);

  v << cmpb{s1, s0, sf};
  v << setcc{cc, sf, d};
}

void implCmpEqDbl(IRLS& env, const IRInstruction* inst, ComparisonPred pred) {
  auto const s0 = srcLoc(env, inst, 0).reg();
  auto const s1 = srcLoc(env, inst, 1).reg();
  auto const d  = dstLoc(env, inst, 0).reg();

  auto& v = vmain(env);
  auto const cmpsd_res = v.makeReg();
  auto const movtdb_res = v.makeReg();

  v << cmpsd{pred, s0, s1, cmpsd_res};
  v << movtdb{cmpsd_res, movtdb_res};
  v << andbi{1, movtdb_res, d, v.makeReg()};
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

void cgCheckRange(IRLS& env, const IRInstruction* inst) {
  auto valTmp = inst->src(0);
  auto dst = dstLoc(env, inst, 0).reg();
  auto val = srcLoc(env, inst, 0).reg();
  auto limit = srcLoc(env, inst, 1).reg();
  auto& v = vmain(env);

  ConditionCode cc;
  auto const sf = v.makeReg();

  if (valTmp->hasConstVal()) {
    // Try to put the constant in a position that can get imm-folded.  A
    // suffiently smart imm-folder could handle this for us.  Note that this is
    // an arch-agnostic API bleed.
    v << cmpq{val, limit, sf};
    cc = CC_A;
  } else {
    v << cmpq{limit, val, sf};
    cc = CC_B;
  }
  v << setcc{cc, sf, dst};
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
 *
 * ARM has condition codes that test Gt, Gte, Lt, Lte without running into
 * "unordered" ambiguity. We can just use those directly. See convertCC()
 * in "abi-arm.h".
 */
#if !defined(__aarch64__)
#define CMP_DBL_OPS         \
  CDO(Gt,   CC_A,   false)  \
  CDO(Gte,  CC_AE,  false)  \
  CDO(Lt,   CC_A,   true)   \
  CDO(Lte,  CC_AE,  true)
#else
#define CMP_DBL_OPS         \
  CDO(Gt,   CC_G,   false)  \
  CDO(Gte,  CC_GE,  false)  \
  CDO(Lt,   CC_B,   false)  \
  CDO(Lte,  CC_BE,  false)
#endif

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

IMPL_OPCODE_CALL(GtStr);
IMPL_OPCODE_CALL(GteStr);
IMPL_OPCODE_CALL(LtStr);
IMPL_OPCODE_CALL(LteStr);
IMPL_OPCODE_CALL(EqStr);
IMPL_OPCODE_CALL(NeqStr);
IMPL_OPCODE_CALL(SameStr);
IMPL_OPCODE_CALL(NSameStr);
IMPL_OPCODE_CALL(CmpStr);

IMPL_OPCODE_CALL(GtStrInt);
IMPL_OPCODE_CALL(GteStrInt);
IMPL_OPCODE_CALL(LtStrInt);
IMPL_OPCODE_CALL(LteStrInt);
IMPL_OPCODE_CALL(EqStrInt);
IMPL_OPCODE_CALL(NeqStrInt);
IMPL_OPCODE_CALL(CmpStrInt);

IMPL_OPCODE_CALL(GtObj);
IMPL_OPCODE_CALL(GteObj);
IMPL_OPCODE_CALL(LtObj);
IMPL_OPCODE_CALL(LteObj);
IMPL_OPCODE_CALL(EqObj);
IMPL_OPCODE_CALL(NeqObj);
IMPL_OPCODE_CALL(CmpObj);

IMPL_OPCODE_CALL(GtArr);
IMPL_OPCODE_CALL(GteArr);
IMPL_OPCODE_CALL(LtArr);
IMPL_OPCODE_CALL(LteArr);
IMPL_OPCODE_CALL(EqArr);
IMPL_OPCODE_CALL(NeqArr);
IMPL_OPCODE_CALL(SameArr);
IMPL_OPCODE_CALL(NSameArr);
IMPL_OPCODE_CALL(CmpArr);

IMPL_OPCODE_CALL(GtVec);
IMPL_OPCODE_CALL(GteVec);
IMPL_OPCODE_CALL(LtVec);
IMPL_OPCODE_CALL(LteVec);
IMPL_OPCODE_CALL(EqVec);
IMPL_OPCODE_CALL(NeqVec);
IMPL_OPCODE_CALL(SameVec);
IMPL_OPCODE_CALL(NSameVec);
IMPL_OPCODE_CALL(CmpVec);

IMPL_OPCODE_CALL(EqDict);
IMPL_OPCODE_CALL(NeqDict);
IMPL_OPCODE_CALL(SameDict);
IMPL_OPCODE_CALL(NSameDict);

IMPL_OPCODE_CALL(EqKeyset);
IMPL_OPCODE_CALL(NeqKeyset);

IMPL_OPCODE_CALL(GtRes);
IMPL_OPCODE_CALL(GteRes);
IMPL_OPCODE_CALL(LtRes);
IMPL_OPCODE_CALL(LteRes);
IMPL_OPCODE_CALL(CmpRes);

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

void cgEqFunc(IRLS& env, const IRInstruction* inst) {
  auto const s0 = srcLoc(env, inst, 0).reg();
  auto const s1 = srcLoc(env, inst, 1).reg();
  auto const d  = dstLoc(env, inst, 0).reg();

  auto& v = vmain(env);
  auto const sf = v.makeReg();

  emitCmpLowPtr<Func>(v, sf, s1, s0);
  v << setcc{CC_E, sf, d};
}

void cgDbgAssertARFunc(IRLS& env, const IRInstruction* inst) {
  auto const sp = srcLoc(env, inst, 0).reg();
  auto const func = srcLoc(env, inst, 1).reg(0);
  auto const off = cellsToBytes(inst->extra<DbgAssertARFunc>()->offset.offset);

  auto& v = vmain(env);

  auto const sf = v.makeReg();
  v << cmpqm{func, sp[off + AROFF(m_func)], sf};

  ifThen(v, CC_NE, sf, [&](Vout& v) { v << ud2{}; });
}

///////////////////////////////////////////////////////////////////////////////

void cgEqStrPtr(IRLS& env, const IRInstruction* inst) {
  assertx(inst->src(0)->type() <= TStr);
  assertx(inst->src(1)->type() <= TStr);
  implCmp(env, inst, CC_E);
}

///////////////////////////////////////////////////////////////////////////////

}}}
