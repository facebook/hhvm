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

namespace HPHP { namespace jit { namespace irlower {

TRACE_SET_MOD(irlower);

///////////////////////////////////////////////////////////////////////////////

namespace {
template<typename T>
constexpr Type getType();
template<>
constexpr Type getType<Class>() { return TCls; }
template<>
constexpr Type getType<RecordDesc>() { return TRecDesc; }

template<typename T>
const T* constVal(SSATmp*);
template<>
const Class* constVal<Class>(SSATmp* cls) { return cls->clsVal(); }
template<>
const RecordDesc* constVal<RecordDesc>(SSATmp* rec) { return rec->recVal(); }

template<typename T>
void implVerifyType(IRLS& env, const IRInstruction* inst) {
  auto const type = inst->src(0);
  auto const constraint = inst->src(1);
  auto& v = vmain(env);

  if (type->hasConstVal() && constraint->hasConstVal(getType<T>())) {
    if (constVal<T>(type) != constVal<T>(constraint)) {
      cgCallNative(v, env, inst);
    }
    return;
  }

  auto const rtype = srcLoc(env, inst, 0).reg();
  auto const rconstraint = srcLoc(env, inst, 1).reg();
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
  implVerifyType<Class>(env, inst);
}
void cgVerifyRetCls(IRLS& env, const IRInstruction* inst) {
  implVerifyType<Class>(env, inst);
}

void cgVerifyParamRecDesc(IRLS& env, const IRInstruction* inst) {
  implVerifyType<RecordDesc>(env, inst);
}
void cgVerifyRetRecDesc(IRLS& env, const IRInstruction* inst) {
  implVerifyType<RecordDesc>(env, inst);
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

static void verifyPropRecDescImpl(const Class* objCls,
                                  const RecordDesc* constraint,
                                  RecordData* val,
                                  Slot slot,
                                  const TypeConstraint* tc) {
  assertx(slot < objCls->numDeclProperties());
  assertx(tc && tc->isRecord());
  auto const success = [&]{
    auto const valRec = val->record();
    if (LIKELY(constraint != nullptr)) return valRec->recordDescOf(constraint);
    return tc->checkTypeAliasRecord(valRec);
  }();
  if (!success) {
    verifyPropFailImpl(objCls, make_tv<KindOfRecord>(val), slot, tc);
  }
}

static void verifyStaticPropRecDescImpl(const Class* objCls,
                                        const RecordDesc* constraint,
                                        RecordData* val,
                                        Slot slot,
                                        const TypeConstraint* tc) {
  assertx(slot < objCls->numStaticProperties());
  assertx(tc && tc->isRecord());
  auto const success = [&]{
    auto const valRec = val->record();
    if (LIKELY(constraint != nullptr)) return valRec->recordDescOf(constraint);
    return tc->checkTypeAliasRecord(valRec);
  }();
  if (!success) {
    verifyStaticPropFailImpl(objCls, make_tv<KindOfRecord>(val), slot, tc);
  }
}


static void verifyPropClsImpl(const Class* objCls,
                              const Class* constraint,
                              ObjectData* val,
                              Slot slot,
                              const TypeConstraint* tc) {
  assertx(RuntimeOption::EvalCheckPropTypeHints > 0);
  assertx(slot < objCls->numDeclProperties());
  assertx(tc && tc->isObject());
  auto const success = [&]{
    auto const valCls = val->getVMClass();
    if (LIKELY(constraint != nullptr)) return valCls->classof(constraint);
    return tc->checkTypeAliasObj(valCls);
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
  assertx(tc && tc->isObject());
  auto const success = [&]{
    auto const valCls = val->getVMClass();
    if (LIKELY(constraint != nullptr)) return valCls->classof(constraint);
    return tc->checkTypeAliasObj(valCls);
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
  auto const& prop = cls->declProperties()[slot];
  if (tc->isCheckable()) tc->verifyProperty(&val, cls, prop.cls, prop.name);
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
  }
  if (RuntimeOption::EvalEnforceGenericsUB > 0) {
    for (auto const& ub : prop.ubs) {
      if (ub.isCheckable()) {
        val = verifyPropImpl(cls, slot, &ub, val);
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
  auto const& prop = cls->staticProperties()[slot];
  if (tc->isCheckable()) {
    tc->verifyStaticProperty(&val, cls, prop.cls, prop.name);
  }
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
  }
  if (RuntimeOption::EvalEnforceGenericsUB > 0) {
    for (auto const& ub : prop.ubs) {
      if (ub.isCheckable()) {
        val = verifySPropImpl(cls, slot, &ub, val);
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

void cgVerifyPropRecDesc(IRLS& env, const IRInstruction* inst) {
  auto const extra = inst->extra<TypeConstraintData>();
  cgCallHelper(
    vmain(env),
    env,
    inst->src(4)->boolVal()
      ? CallSpec::direct(verifyStaticPropRecDescImpl)
      : CallSpec::direct(verifyPropRecDescImpl),
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

void cgVerifyProp(IRLS& env, const IRInstruction* inst) {
  auto const extra = inst->extra<TypeConstraintData>();
  cgCallHelper(
    vmain(env),
    env,
    inst->src(3)->boolVal()
      ? CallSpec::direct(verifySPropImpl)
      : CallSpec::direct(verifyPropImpl),
    kVoidDest,
    SyncOptions::Sync,
    argGroup(env, inst)
      .ssa(0)
      .ssa(1)
      .immPtr(extra->tc)
      .typedValue(2)
  );
}

void cgVerifyPropAll(IRLS& env, const IRInstruction* inst) {
  cgCallHelper(
    vmain(env),
    env,
    inst->src(3)->boolVal()
      ? CallSpec::direct(verifySPropAll)
      : CallSpec::direct(verifyPropAll),
    kVoidDest,
    SyncOptions::Sync,
    argGroup(env, inst)
      .ssa(0)
      .ssa(1)
      .typedValue(2)
  );
}

void cgVerifyPropCoerce(IRLS& env, const IRInstruction* inst) {
  auto const extra = inst->extra<TypeConstraintData>();
  cgCallHelper(
    vmain(env),
    env,
    inst->src(3)->boolVal()
      ? CallSpec::direct(verifySPropImpl)
      : CallSpec::direct(verifyPropImpl),
    callDestTV(env, inst),
    SyncOptions::Sync,
    argGroup(env, inst)
      .ssa(0)
      .ssa(1)
      .immPtr(extra->tc)
      .typedValue(2)
  );
}

void cgVerifyPropCoerceAll(IRLS& env, const IRInstruction* inst) {
  cgCallHelper(
    vmain(env),
    env,
    inst->src(3)->boolVal()
      ? CallSpec::direct(verifySPropAll)
      : CallSpec::direct(verifyPropAll),
    callDestTV(env, inst),
    SyncOptions::Sync,
    argGroup(env, inst)
      .ssa(0)
      .ssa(1)
      .typedValue(2)
  );
}

void cgVerifyRetFail(IRLS& env, const IRInstruction* inst) {
  auto const extra = inst->extra<ParamWithTCData>();
  cgCallHelper(
    vmain(env),
    env,
    CallSpec::direct(VerifyRetTypeFail),
    kVoidDest,
    SyncOptions::Sync,
    argGroup(env, inst)
      .imm(extra->paramId)
      .ssa(0)
      .immPtr(extra->tc)
  );
}

void cgVerifyRetFailHard(IRLS& env, const IRInstruction* inst) {
  cgVerifyRetFail(env, inst);
}

void cgVerifyParamFail(IRLS& env, const IRInstruction* inst) {
  auto const extra = inst->extra<ParamWithTCData>();
  cgCallHelper(
    vmain(env),
    env,
    CallSpec::direct(VerifyParamTypeFail),
    kVoidDest,
    SyncOptions::Sync,
    argGroup(env, inst)
      .imm(extra->paramId)
      .immPtr(extra->tc)
  );
}

void cgVerifyParamFailHard(IRLS& env, const IRInstruction* inst) {
  cgVerifyParamFail(env, inst);
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

}}}
