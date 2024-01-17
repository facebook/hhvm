/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#include "hphp/runtime/base/runtime-error.h"
#include "hphp/runtime/base/tv-conversions.h"

#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/arg-group.h"
#include "hphp/runtime/vm/jit/call-spec.h"
#include "hphp/runtime/vm/jit/code-gen-cf.h"
#include "hphp/runtime/vm/jit/extra-data.h"
#include "hphp/runtime/vm/jit/ir-instruction.h"
#include "hphp/runtime/vm/jit/ir-opcode.h"
#include "hphp/runtime/vm/jit/ssa-tmp.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/jit/type.h"
#include "hphp/runtime/vm/jit/vasm-gen.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"
#include "hphp/runtime/vm/jit/vasm-reg.h"

#include "hphp/util/trace.h"

namespace HPHP::jit::irlower {

TRACE_SET_MOD(irlower);

///////////////////////////////////////////////////////////////////////////////

namespace {

void implVerifyType(IRLS& env, const IRInstruction* inst) {
  auto const type = inst->src(1);
  auto const constraint = inst->src(2);
  auto& v = vmain(env);

  if (type->hasConstVal() && constraint->hasConstVal(TCls)) {
    if (type->clsVal() != constraint->clsVal()) {
      cgCallNative(v, env, inst);
    }
    return;
  }

  auto const rtype = srcLoc(env, inst, 1).reg();
  auto const rconstraint = srcLoc(env, inst, 2).reg();
  auto const sf = v.makeReg();

  v << cmpq{rconstraint, rtype, sf};

  // The native call for this instruction is the slow path that does proper
  // subtype checking.  The comparisons above are just to short-circuit the
  // overhead when the Classes are an exact match.
  ifThen(v, CC_NE, sf, [&](Vout& v) { cgCallNative(v, env, inst); });
}

}

IMPL_OPCODE_CALL(VerifyParamCallable)
IMPL_OPCODE_CALL(VerifyRetCallable)
IMPL_OPCODE_CALL(VerifyReifiedLocalType)
IMPL_OPCODE_CALL(VerifyReifiedReturnType)

void cgVerifyParamCls(IRLS& env, const IRInstruction* inst) {
  implVerifyType(env, inst);
}
void cgVerifyRetCls(IRLS& env, const IRInstruction* inst) {
  implVerifyType(env, inst);
}

///////////////////////////////////////////////////////////////////////////////

static void verifyPropFailImpl(const Class* objCls, TypedValue val, Slot slot,
                               const TypeConstraint* tc) {
  assertx(RuntimeOption::EvalCheckPropTypeHints > 0);
  assertx(tvIsPlausible(val));
  assertx(slot < objCls->numDeclProperties());
  auto const& prop = objCls->declProperties()[slot];
  assertx(tc && tc->isCheckable());
  tc->verifyPropFail(
    objCls,
    prop.cls,
    &val,
    prop.name,
    false
  );
}

static void verifyStaticPropFailImpl(const Class* objCls, TypedValue val,
                                     Slot slot, const TypeConstraint* tc) {
  assertx(RuntimeOption::EvalCheckPropTypeHints > 0);
  assertx(tvIsPlausible(val));
  assertx(slot < objCls->numStaticProperties());
  auto const& sprop = objCls->staticProperties()[slot];
  assertx(tc && tc->isCheckable());
  tc->verifyPropFail(
    objCls,
    sprop.cls,
    &val,
    sprop.name,
    true
  );
}

static void verifyPropClsImpl(const Class* objCls,
                              const Class* constraint,
                              ObjectData* val,
                              Slot slot,
                              const TypeConstraint* tc) {
  assertx(RuntimeOption::EvalCheckPropTypeHints > 0);
  assertx(slot < objCls->numDeclProperties());
  assertx(tc && (tc->isSubObject() || tc->isUnresolved()));
  auto const success = [&]{
    auto const valCls = val->getVMClass();
    if (LIKELY(constraint != nullptr)) return valCls->classof(constraint);
    return tc->isUnresolved() && tc->checkTypeAliasObj(valCls);
  }();
  if (!success) verifyPropFailImpl(objCls, make_tv<KindOfObject>(val), slot, tc);
}

static void verifyStaticPropClsImpl(const Class* objCls,
                                    const Class* constraint,
                                    ObjectData* val,
                                    Slot slot,
                                    const TypeConstraint* tc) {
  assertx(RuntimeOption::EvalCheckPropTypeHints > 0);
  assertx(slot < objCls->numStaticProperties());
  assertx(tc && (tc->isSubObject() || tc->isUnresolved()));
  auto const success = [&]{
    auto const valCls = val->getVMClass();
    if (LIKELY(constraint != nullptr)) return valCls->classof(constraint);
    return tc->isUnresolved() && tc->checkTypeAliasObj(valCls);
  }();
  if (!success) {
    verifyStaticPropFailImpl(objCls, make_tv<KindOfObject>(val), slot, tc);
  }
}

static TypedValue verifyPropImpl(const Class* cls,
                                 Slot slot,
                                 const TypeConstraint* tc,
                                 TypedValue val) {
  assertx(RuntimeOption::EvalCheckPropTypeHints > 0);
  assertx(slot < cls->numDeclProperties());
  assertx(tvIsPlausible(val));
  assertx(tc->isCheckable());
  auto const& prop = cls->declProperties()[slot];
  tc->verifyProperty(&val, cls, prop.cls, prop.name);
  assertx(tvIsPlausible(val));
  return val;
}

static TypedValue verifyPropAll(const Class* cls, Slot slot, TypedValue val) {
  assertx(RuntimeOption::EvalCheckPropTypeHints > 0);
  assertx(slot < cls->numDeclProperties());
  assertx(tvIsPlausible(val));
  auto const& prop = cls->declProperties()[slot];
  auto const& tc = prop.typeConstraint;
  if (tc.isCheckable()) {
    val = verifyPropImpl(cls, slot, &tc, val);
    assertx(tvIsPlausible(val));
  }
  if (RuntimeOption::EvalEnforceGenericsUB > 0) {
    for (auto const& ub : prop.ubs.m_constraints) {
      if (ub.isCheckable()) {
        val = verifyPropImpl(cls, slot, &ub, val);
        assertx(tvIsPlausible(val));
      }
    }
  }
  return val;
}


static TypedValue verifySPropImpl(const Class* cls,
                                  Slot slot,
                                  const TypeConstraint* tc,
                                  TypedValue val) {
  assertx(RuntimeOption::EvalCheckPropTypeHints > 0);
  assertx(slot < cls->numStaticProperties());
  assertx(tvIsPlausible(val));
  assertx(tc->isCheckable());
  auto const& prop = cls->staticProperties()[slot];
  tc->verifyStaticProperty(&val, cls, prop.cls, prop.name);
  assertx(tvIsPlausible(val));
  return val;
}

static TypedValue verifySPropAll(const Class* cls, Slot slot, TypedValue val) {
  assertx(RuntimeOption::EvalCheckPropTypeHints > 0);
  assertx(slot < cls->numStaticProperties());
  assertx(tvIsPlausible(val));
  auto const& prop = cls->staticProperties()[slot];
  auto const& tc = prop.typeConstraint;
  if (tc.isCheckable()) {
    val = verifySPropImpl(cls, slot, &tc, val);
    assertx(tvIsPlausible(val));
  }
  if (RuntimeOption::EvalEnforceGenericsUB > 0) {
    for (auto const& ub : prop.ubs.m_constraints) {
      if (ub.isCheckable()) {
        val = verifySPropImpl(cls, slot, &ub, val);
        assertx(tvIsPlausible(val));
      }
    }
  }
  return val;
}

void cgVerifyPropFail(IRLS& env, const IRInstruction* inst) {
  auto const extra = inst->extra<TypeConstraintData>();
  cgCallHelper(
    vmain(env),
    env,
    inst->src(3)->boolVal()
      ? CallSpec::direct(verifyStaticPropFailImpl)
      : CallSpec::direct(verifyPropFailImpl),
    kVoidDest,
    SyncOptions::Sync,
    argGroup(env, inst)
      .ssa(0)
      .typedValue(2)
      .ssa(1)
      .immPtr(extra->tc)
  );
}

void cgVerifyPropFailHard(IRLS& env, const IRInstruction* inst) {
  cgVerifyPropFail(env, inst);
}

void cgVerifyPropCls(IRLS& env, const IRInstruction* inst) {
  auto const extra = inst->extra<TypeConstraintData>();
  cgCallHelper(
    vmain(env),
    env,
    inst->src(4)->boolVal()
      ? CallSpec::direct(verifyStaticPropClsImpl)
      : CallSpec::direct(verifyPropClsImpl),
    kVoidDest,
    SyncOptions::Sync,
    argGroup(env, inst)
      .ssa(0)
      .ssa(2)
      .ssa(3)
      .ssa(1)
      .immPtr(extra->tc)
  );
}

namespace {

void implVerifyProp(IRLS& env, const IRInstruction* inst, bool coerce) {
  auto const extra = inst->extra<TypeConstraintData>();
  cgCallHelper(
    vmain(env),
    env,
    inst->src(3)->boolVal()
      ? CallSpec::direct(verifySPropImpl)
      : CallSpec::direct(verifyPropImpl),
    coerce ? callDestTV(env, inst) : kVoidDest,
    SyncOptions::Sync,
    argGroup(env, inst)
      .ssa(0)
      .ssa(1)
      .immPtr(extra->tc)
      .typedValue(2)
  );
}

void implVerifyPropAll(IRLS& env, const IRInstruction* inst, bool coerce) {
  cgCallHelper(
    vmain(env),
    env,
    inst->src(3)->boolVal()
      ? CallSpec::direct(verifySPropAll)
      : CallSpec::direct(verifyPropAll),
    coerce ? callDestTV(env, inst) : kVoidDest,
    SyncOptions::Sync,
    argGroup(env, inst)
      .ssa(0)
      .ssa(1)
      .typedValue(2)
  );
}

void implVerifyRet(IRLS& env, const IRInstruction* inst, bool coerce) {
  auto const extra = inst->extra<FuncParamWithTCData>();
  cgCallHelper(
    vmain(env),
    env,
    CallSpec::direct(VerifyRetType),
    coerce ? callDestTV(env, inst) : kVoidDest,
    SyncOptions::Sync,
    argGroup(env, inst)
      .typedValue(0)
      .ssa(1)
      .immPtr(extra->func)
      .imm(extra->paramId)
      .immPtr(extra->tc)
  );
}

void implVerifyRetFail(IRLS& env, const IRInstruction* inst) {
  auto const extra = inst->extra<FuncParamWithTCData>();
  cgCallHelper(
    vmain(env),
    env,
    CallSpec::direct(VerifyRetTypeFail),
    kVoidDest,
    SyncOptions::Sync,
    argGroup(env, inst)
      .typedValue(0)
      .ssa(1)
      .immPtr(extra->func)
      .imm(extra->paramId)
      .immPtr(extra->tc)
  );
}

void implVerifyParam(IRLS& env, const IRInstruction* inst, bool coerce) {
  auto const extra = inst->extra<FuncParamWithTCData>();
  cgCallHelper(
    vmain(env),
    env,
    CallSpec::direct(VerifyParamType),
    coerce ? callDestTV(env, inst) : kVoidDest,
    SyncOptions::Sync,
    argGroup(env, inst)
      .typedValue(0)
      .ssa(1)
      .immPtr(extra->func)
      .imm(extra->paramId)
      .immPtr(extra->tc)
  );
}

void implVerifyParamFail(IRLS& env, const IRInstruction* inst) {
  auto const extra = inst->extra<FuncParamWithTCData>();
  cgCallHelper(
    vmain(env),
    env,
    CallSpec::direct(VerifyParamTypeFail),
    kVoidDest,
    SyncOptions::Sync,
    argGroup(env, inst)
      .typedValue(0)
      .ssa(1)
      .immPtr(extra->func)
      .imm(extra->paramId)
      .immPtr(extra->tc)
  );
}

}

void cgVerifyProp(IRLS& env, const IRInstruction* inst) {
  implVerifyProp(env, inst, false);
}

void cgVerifyPropCoerce(IRLS& env, const IRInstruction* inst) {
  implVerifyProp(env, inst, true);
}

void cgVerifyPropAll(IRLS& env, const IRInstruction* inst) {
  implVerifyPropAll(env, inst, false);
}

void cgVerifyPropCoerceAll(IRLS& env, const IRInstruction* inst) {
  implVerifyPropAll(env, inst, true);
}

void cgVerifyRet(IRLS& env, const IRInstruction* inst) {
  implVerifyRet(env, inst, false);
}

void cgVerifyRetCoerce(IRLS& env, const IRInstruction* inst) {
  implVerifyRet(env, inst, true);
}

void cgVerifyRetFail(IRLS& env, const IRInstruction* inst) {
  implVerifyRetFail(env, inst);
}

void cgVerifyRetFailHard(IRLS& env, const IRInstruction* inst) {
  implVerifyRetFail(env, inst);
}

void cgVerifyParam(IRLS& env, const IRInstruction* inst) {
  implVerifyParam(env, inst, false);
}

void cgVerifyParamCoerce(IRLS& env, const IRInstruction* inst) {
  implVerifyParam(env, inst, true);
}

void cgVerifyParamFail(IRLS& env, const IRInstruction* inst) {
  implVerifyParamFail(env, inst);
}

void cgVerifyParamFailHard(IRLS& env, const IRInstruction* inst) {
  implVerifyParamFail(env, inst);
}

///////////////////////////////////////////////////////////////////////////////

void cgRaiseStrToClassNotice(IRLS& env, const IRInstruction* inst) {
  cgCallHelper(
    vmain(env),
    env,
    CallSpec::direct(raise_str_to_class_notice),
    callDest(env, inst),
    SyncOptions::Sync,
    argGroup(env, inst).ssa(0)
  );
}

///////////////////////////////////////////////////////////////////////////////

}
