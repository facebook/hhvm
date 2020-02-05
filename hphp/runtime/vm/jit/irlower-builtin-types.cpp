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

static void verifyPropFailImpl(const Class* objCls, TypedValue val, Slot slot) {
  assertx(RuntimeOption::EvalCheckPropTypeHints > 0);
  assertx(tvIsPlausible(val));
  assertx(slot < objCls->numDeclProperties());
  auto const& prop = objCls->declProperties()[slot];
  assertx(prop.typeConstraint.isCheckable());
  prop.typeConstraint.verifyPropFail(
    objCls,
    prop.cls,
    &val,
    prop.name,
    false
  );
}

static void verifyStaticPropFailImpl(const Class* objCls, TypedValue val, Slot slot) {
  assertx(RuntimeOption::EvalCheckPropTypeHints > 0);
  assertx(tvIsPlausible(val));
  assertx(slot < objCls->numStaticProperties());
  auto const& sprop = objCls->staticProperties()[slot];
  assertx(sprop.typeConstraint.isCheckable());
  sprop.typeConstraint.verifyPropFail(
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
                                  Slot slot) {
  assertx(slot < objCls->numDeclProperties());
  auto const& tc = objCls->declProperties()[slot].typeConstraint;
  assertx(tc.isRecord());
  auto const success = [&]{
    auto const valRec = val->record();
    if (LIKELY(constraint != nullptr)) return valRec->recordDescOf(constraint);
    return tc.checkTypeAliasRecord(valRec);
  }();
  if (!success) {
    verifyPropFailImpl(objCls, make_tv<KindOfRecord>(val), slot);
  }
}

static void verifyStaticPropRecDescImpl(const Class* objCls,
                                        const RecordDesc* constraint,
                                        RecordData* val,
                                        Slot slot) {
  assertx(slot < objCls->numStaticProperties());
  auto const& tc = objCls->staticProperties()[slot].typeConstraint;
  assertx(tc.isRecord());
  auto const success = [&]{
    auto const valRec = val->record();
    if (LIKELY(constraint != nullptr)) return valRec->recordDescOf(constraint);
    return tc.checkTypeAliasRecord(valRec);
  }();
  if (!success) {
    verifyStaticPropFailImpl(objCls, make_tv<KindOfRecord>(val), slot);
  }
}


static void verifyPropClsImpl(const Class* objCls,
                              const Class* constraint,
                              ObjectData* val,
                              Slot slot) {
  assertx(RuntimeOption::EvalCheckPropTypeHints > 0);
  assertx(slot < objCls->numDeclProperties());
  auto const& tc = objCls->declProperties()[slot].typeConstraint;
  assertx(tc.isObject());
  auto const success = [&]{
    auto const valCls = val->getVMClass();
    if (LIKELY(constraint != nullptr)) return valCls->classof(constraint);
    return tc.checkTypeAliasObj(valCls);
  }();
  if (!success) verifyPropFailImpl(objCls, make_tv<KindOfObject>(val), slot);
}

static void verifyStaticPropClsImpl(const Class* objCls,
                                    const Class* constraint,
                                    ObjectData* val,
                                    Slot slot) {
  assertx(RuntimeOption::EvalCheckPropTypeHints > 0);
  assertx(slot < objCls->numStaticProperties());
  auto const& tc = objCls->staticProperties()[slot].typeConstraint;
  assertx(tc.isObject());
  auto const success = [&]{
    auto const valCls = val->getVMClass();
    if (LIKELY(constraint != nullptr)) return valCls->classof(constraint);
    return tc.checkTypeAliasObj(valCls);
  }();
  if (!success) {
    verifyStaticPropFailImpl(objCls, make_tv<KindOfObject>(val), slot);
  }
}

static TypedValue verifyPropImpl(const Class* cls,
                                 Slot slot,
                                 TypedValue val) {
  assertx(RuntimeOption::EvalCheckPropTypeHints > 0);
  assertx(slot < cls->numDeclProperties());
  assertx(tvIsPlausible(val));
  auto const& prop = cls->declProperties()[slot];
  auto const& tc = prop.typeConstraint;
  if (tc.isCheckable()) tc.verifyProperty(&val, cls, prop.cls, prop.name);
  return val;
}

static TypedValue verifySPropImpl(const Class* cls,
                                  Slot slot,
                                  TypedValue val) {
  assertx(RuntimeOption::EvalCheckPropTypeHints > 0);
  assertx(slot < cls->numStaticProperties());
  assertx(tvIsPlausible(val));
  auto const& prop = cls->staticProperties()[slot];
  auto const& tc = prop.typeConstraint;
  if (tc.isCheckable()) tc.verifyStaticProperty(&val, cls, prop.cls, prop.name);
  return val;
}

void cgVerifyPropFail(IRLS& env, const IRInstruction* inst) {
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
  );
}

void cgVerifyPropFailHard(IRLS& env, const IRInstruction* inst) {
  cgVerifyPropFail(env, inst);
}

void cgVerifyPropCls(IRLS& env, const IRInstruction* inst) {
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
  );
}

void cgVerifyPropRecDesc(IRLS& env, const IRInstruction* inst) {
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
  );
}
void cgVerifyProp(IRLS& env, const IRInstruction* inst) {
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
      .typedValue(2)
  );
}

void cgVerifyPropCoerce(IRLS& env, const IRInstruction* inst) {
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

static void hackArrParamNoticeImpl(const Func* f, const ArrayData* a,
                                   const StringData* name, int64_t param) {
  raise_hackarr_compat_type_hint_param_notice(f, a, name->data(), param);
}

static void hackArrOutParamNoticeImpl(const Func* f, const ArrayData* a,
                                      const StringData* name, int64_t param) {
  raise_hackarr_compat_type_hint_outparam_notice(f, a, name->data(), param);
}

static void hackArrRetNoticeImpl(const Func* f, const ArrayData* a,
                                 const StringData* name) {
  raise_hackarr_compat_type_hint_ret_notice(f, a, name->data());
}

template <bool IsStatic>
static void hackArrPropNoticeImpl(const Class* cls, const ArrayData* ad,
                                  Slot slot, const StringData* name) {
  const Class* declCls;
  const StringData* propName;
  if (IsStatic) {
    assertx(slot < cls->numStaticProperties());
    declCls = cls;
    propName = cls->staticProperties()[slot].name;
  } else {
    assertx(slot < cls->numDeclProperties());
    auto const& prop = cls->declProperties()[slot];
    declCls = prop.cls;
    propName = prop.name;
  }
  raise_hackarr_compat_type_hint_property_notice(
    declCls,
    ad,
    name->data(),
    propName,
    IsStatic
  );
}

namespace {

ArrayData::DVArray annotTypeToDVArrKind(AnnotType at) {
  switch (at) {
    case AnnotType::VArray: return ArrayData::kVArray;
    case AnnotType::DArray: return ArrayData::kDArray;
    case AnnotType::Array:  return ArrayData::kDVArrayMask;
    case AnnotType::VArrOrDArr: return ArrayData::kDVArrayMask;
    default: break;
  }
  not_reached();
}

void implRaiseHackArrTypehintNotice(IRLS& env, Vreg src,
                                    const RaiseHackArrTypehintNoticeData* extra,
                                    CallSpec target, const ArgGroup& args) {
  auto& v = vmain(env);
  auto const at = extra->tc.type();

  auto const do_notice = [&] (Vout& v) {
    cgCallHelper(v, env, target, kVoidDest, SyncOptions::Sync, args);
  };

  if (!RuntimeOption::EvalHackArrCompatTypeHintPolymorphism ||
      at != AnnotType::VArrOrDArr) {
    auto const dv = annotTypeToDVArrKind(at);
    auto const sf = v.makeReg();
    v << testbim{dv, src + ArrayData::offsetofDVArray(), sf};

    auto const cc = at == AnnotType::Array ? CC_NZ : CC_Z;

    return unlikelyIfThen(v, vcold(env), cc, sf, do_notice);
  }

  auto const dv = ArrayData::kDVArrayMask;
  auto const sf = v.makeReg();
  v << testbim{dv, src + ArrayData::offsetofDVArray(), sf};

  unlikelyIfThenElse(v, vcold(env), CC_Z, sf, do_notice, [&] (Vout& v) {
    implodingIFTE(v, v,
      [&] (Vout& v, Vlabel next, Vlabel taken) {
        auto const dv = ArrayData::kDArray;
        auto const sf = v.makeReg();
        v << testbim{dv, src + ArrayData::offsetofDVArray(), sf};
        v << jcc{CC_Z, sf, {next, taken}};
      },
      do_notice, do_notice
    );
  });
}

}

void cgRaiseHackArrParamNotice(IRLS& env, const IRInstruction* inst) {
  auto const src = srcLoc(env, inst, 0).reg();
  auto const extra = inst->extra<RaiseHackArrParamNotice>();

  auto args = argGroup(env, inst)
    .ssa(1)
    .ssa(0)
    .imm(makeStaticString(extra->tc.displayName()));

  auto const target = [&] {
    if (extra->isReturn) {
      if (extra->id == TypeConstraint::ReturnId) {
        return CallSpec::direct(hackArrRetNoticeImpl);
      }
      args.imm(extra->id);
      return CallSpec::direct(hackArrOutParamNoticeImpl);
    } else {
      args.imm(extra->id);
      return CallSpec::direct(hackArrParamNoticeImpl);
    }
  }();

  implRaiseHackArrTypehintNotice(env, src, extra, target, args);
}

void cgRaiseHackArrPropNotice(IRLS& env, const IRInstruction* inst) {
  auto const src = srcLoc(env, inst, 1).reg();
  auto const extra = inst->extra<RaiseHackArrPropNotice>();

  auto const target = inst->src(3)->boolVal()
    ? CallSpec::direct(hackArrPropNoticeImpl<true>)
    : CallSpec::direct(hackArrPropNoticeImpl<false>);

  auto args = argGroup(env, inst)
    .ssa(0)
    .ssa(1)
    .ssa(2)
    .imm(makeStaticString(extra->tc.displayName()));

  implRaiseHackArrTypehintNotice(env, src, extra, target, args);
}

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

void raiseArraySerializeImpl(SerializationSite src, const ArrayData* ad) {
  raise_array_serialization_notice(src, ad);
}

void cgRaiseArraySerializeNotice(IRLS& env, const IRInstruction* inst) {
  cgCallHelper(
    vmain(env),
    env,
    CallSpec::direct(raiseArraySerializeImpl),
    callDest(env, inst),
    SyncOptions::Sync,
    argGroup(env, inst).ssa(0).ssa(1)
  );
}

///////////////////////////////////////////////////////////////////////////////

}}}
