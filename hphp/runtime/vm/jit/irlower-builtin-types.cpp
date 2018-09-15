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

/*
 * Attempt to convert `tv' to a given type, raising a warning and throwing a
 * TVCoercionException on failure.
 */
#define XX(kind, expkind)                                             \
void tvCoerceParamTo##kind##OrThrow(TypedValue* tv,                   \
                                    const Func* callee,               \
                                    unsigned int arg_num) {           \
  tvCoerceIfStrict(*tv, arg_num, callee);                             \
  auto ty = tv->m_type;                                               \
  if (LIKELY(tvCoerceParamTo##kind##InPlace(tv,                       \
                                            callee->isBuiltin()))) {  \
    if (RuntimeOption::EvalWarnOnCoerceBuiltinParams &&               \
        tv->m_type != KindOfNull &&                                   \
        !equivDataTypes(ty, KindOf##expkind)) {                       \
      raise_warning(                                                  \
        "Argument %i of type %s was passed to %s, "                   \
        "it was coerced to %s",                                       \
        arg_num,                                                      \
        getDataTypeString(ty).data(),                                 \
        callee->fullDisplayName()->data(),                            \
        getDataTypeString(KindOf##expkind).data()                     \
      );                                                              \
    }                                                                 \
    return;                                                           \
  }                                                                   \
  raise_param_type_warning(callee->displayName()->data(),             \
                           arg_num, KindOf##expkind, tv->m_type);     \
  throw TVCoercionException(callee, arg_num, tv->m_type,              \
                            KindOf##expkind);                         \
}
#define X(kind) XX(kind, kind)
X(Boolean)
X(Int64)
X(Double)
X(String)
X(Vec)
X(Dict)
X(Keyset)
X(Array)
X(Object)
XX(NullableObject, Object)
X(Resource)
#undef X
#undef XX

///////////////////////////////////////////////////////////////////////////////

namespace {

void implCast(IRLS& env, const IRInstruction* inst, Vreg base, int offset) {
  auto type = inst->typeParam();
  auto nullable = false;

  if (!type.isKnownDataType()) {
    assertx(TNull <= type);
    type -= TNull;
    assertx(type.isKnownDataType());
    nullable = true;
  }
  assertx(IMPLIES(nullable, type <= TObj));

  auto const args = argGroup(env, inst).addr(base, offset);

  auto const helper = [&]() -> void (*)(TypedValue*) {
    if (type <= TBool) {
      return tvCastToBooleanInPlace;
    } else if (type <= TInt) {
      return tvCastToInt64InPlace;
    } else if (type <= TDbl) {
      return tvCastToDoubleInPlace;
    } else if (type <= TArr) {
      return tvCastToArrayInPlace;
    } else if (type <= TVec) {
      return tvCastToVecInPlace;
    } else if (type <= TDict) {
      return tvCastToDictInPlace;
    } else if (type <= TKeyset) {
      return tvCastToKeysetInPlace;
    } else if (type <= TStr) {
      return tvCastToStringInPlace;
    } else if (type <= TObj) {
      return nullable ? tvCastToNullableObjectInPlace : tvCastToObjectInPlace;
    } else if (type <= TRes) {
      return tvCastToResourceInPlace;
    } else {
      not_reached();
    }
  }();
  cgCallHelper(vmain(env), env, CallSpec::direct(helper),
               kVoidDest, SyncOptions::Sync, args);
}

void implCoerce(IRLS& env, const IRInstruction* inst,
                Vreg base, int offset, Func const* callee, int argNum) {
  auto const type = inst->typeParam();
  assertx(type.isKnownDataType());

  auto args = argGroup(env, inst)
    .addr(base, offset)
    .imm(callee)
    .imm(argNum);

  auto const helper = [&]()
    -> void (*)(TypedValue*, const Func*, unsigned int)
  {
    if (type <= TBool) {
      return tvCoerceParamToBooleanOrThrow;
    } else if (type <= TInt) {
      return tvCoerceParamToInt64OrThrow;
    } else if (type <= TDbl) {
      return tvCoerceParamToDoubleOrThrow;
    } else if (type <= TArr) {
      return tvCoerceParamToArrayOrThrow;
    } else if (type <= TVec) {
      return tvCoerceParamToVecOrThrow;
    } else if (type <= TDict) {
      return tvCoerceParamToDictOrThrow;
    } else if (type <= TKeyset) {
      return tvCoerceParamToKeysetOrThrow;
    } else if (type <= TStr) {
      return tvCoerceParamToStringOrThrow;
    } else if (type <= TObj) {
      return tvCoerceParamToObjectOrThrow;
    } else if (type <= TRes) {
      return tvCoerceParamToResourceOrThrow;
    } else {
      not_reached();
    }
  }();

  cgCallHelper(vmain(env), env, CallSpec::direct(helper),
               kVoidDest, SyncOptions::Sync, args);
}

}

void cgCastStk(IRLS& env, const IRInstruction *inst) {
  auto const sp = srcLoc(env, inst, 0).reg();
  auto const offset = inst->extra<CastStk>()->offset;

  implCast(env, inst, sp, cellsToBytes(offset.offset));
}

void cgCastMem(IRLS& env, const IRInstruction *inst) {
  auto const ptr = srcLoc(env, inst, 0).reg();

  implCast(env, inst, ptr, 0);
}

void cgCoerceStk(IRLS& env, const IRInstruction *inst) {
  auto const extra = inst->extra<CoerceStk>();
  auto const sp = srcLoc(env, inst, 0).reg();
  auto const offset = cellsToBytes(extra->offset.offset);

  implCoerce(env, inst, sp, offset, extra->callee, extra->argNum);
}

void cgCoerceMem(IRLS& env, const IRInstruction *inst) {
  auto const extra = inst->extra<CoerceMem>();
  auto const ptr = srcLoc(env, inst, 0).reg();

  implCoerce(env, inst, ptr, 0, extra->callee, extra->argNum);
}

IMPL_OPCODE_CALL(CoerceCellToBool);
IMPL_OPCODE_CALL(CoerceCellToInt);
IMPL_OPCODE_CALL(CoerceCellToDbl);
IMPL_OPCODE_CALL(CoerceStrToDbl);
IMPL_OPCODE_CALL(CoerceStrToInt);

///////////////////////////////////////////////////////////////////////////////

namespace {

void implVerifyCls(IRLS& env, const IRInstruction* inst) {
  auto const cls = inst->src(0);
  auto const constraint = inst->src(1);
  auto& v = vmain(env);

  if (cls->hasConstVal() && constraint->hasConstVal(TCls)) {
    if (cls->clsVal() != constraint->clsVal()) {
      cgCallNative(v, env, inst);
    }
    return;
  }

  auto const rcls = srcLoc(env, inst, 0).reg();
  auto const rconstraint = srcLoc(env, inst, 1).reg();
  auto const sf = v.makeReg();

  v << cmpq{rconstraint, rcls, sf};

  // The native call for this instruction is the slow path that does proper
  // subtype checking.  The comparisons above are just to short-circuit the
  // overhead when the Classes are an exact match.
  ifThen(v, CC_NE, sf, [&](Vout& v) { cgCallNative(v, env, inst); });
}

}

IMPL_OPCODE_CALL(VerifyParamCallable)
IMPL_OPCODE_CALL(VerifyRetCallable)
IMPL_OPCODE_CALL(VerifyParamFail)
IMPL_OPCODE_CALL(VerifyParamFailHard)
IMPL_OPCODE_CALL(VerifyRetFail)
IMPL_OPCODE_CALL(VerifyRetFailHard)

void cgVerifyParamCls(IRLS& env, const IRInstruction* inst) {
  implVerifyCls(env, inst);
}
void cgVerifyRetCls(IRLS& env, const IRInstruction* inst) {
  implVerifyCls(env, inst);
}

///////////////////////////////////////////////////////////////////////////////

static void verifyPropFailImpl(const Class* objCls, Cell val, Slot slot) {
  assertx(RuntimeOption::EvalCheckPropTypeHints > 0);
  assertx(cellIsPlausible(val));
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

static void verifyStaticPropFailImpl(const Class* objCls, Cell val, Slot slot) {
  assertx(RuntimeOption::EvalCheckPropTypeHints > 0);
  assertx(cellIsPlausible(val));
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

static void verifyPropImpl(const Class* cls,
                            Slot slot,
                            Cell val) {
  assertx(RuntimeOption::EvalCheckPropTypeHints > 0);
  assertx(slot < cls->numDeclProperties());
  assertx(cellIsPlausible(val));
  auto const& prop = cls->declProperties()[slot];
  auto const& tc = prop.typeConstraint;
  if (tc.isCheckable()) tc.verifyProperty(&val, cls, prop.cls, prop.name);
}

static void verifySPropImpl(const Class* cls,
                            Slot slot,
                            Cell val) {
  assertx(RuntimeOption::EvalCheckPropTypeHints > 0);
  assertx(slot < cls->numStaticProperties());
  assertx(cellIsPlausible(val));
  auto const& prop = cls->staticProperties()[slot];
  auto const& tc = prop.typeConstraint;
  if (tc.isCheckable()) tc.verifyStaticProperty(&val, cls, prop.cls, prop.name);
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

///////////////////////////////////////////////////////////////////////////////

static void hackArrParamNoticeImpl(const Func* f, const ArrayData* a,
                                   int64_t type, int64_t param) {
  raise_hackarr_compat_type_hint_param_notice(f, a, AnnotType(type), param);
}

static void hackArrOutParamNoticeImpl(const Func* f, const ArrayData* a,
                                      int64_t type, int64_t param) {
  raise_hackarr_compat_type_hint_outparam_notice(f, a, AnnotType(type), param);
}

static void hackArrRetNoticeImpl(const Func* f, const ArrayData* a,
                                 int64_t type) {
  raise_hackarr_compat_type_hint_ret_notice(f, a, AnnotType(type));
}

template <bool IsStatic>
static void hackArrPropNoticeImpl(const Class* cls, const ArrayData* ad,
                                  Slot slot, int64_t type) {
  const Class* declCls;
  const StringData* name;
  if (IsStatic) {
    assertx(slot < cls->numStaticProperties());
    declCls = cls;
    name = cls->staticProperties()[slot].name;
  } else {
    assertx(slot < cls->numDeclProperties());
    auto const& prop = cls->declProperties()[slot];
    declCls = prop.cls;
    name = prop.name;
  }
  raise_hackarr_compat_type_hint_property_notice(
    declCls,
    ad,
    AnnotType(type),
    name,
    IsStatic
  );
}

void cgRaiseHackArrParamNotice(IRLS& env, const IRInstruction* inst) {
  auto const extra = inst->extra<RaiseHackArrParamNotice>();

  auto args = argGroup(env, inst).ssa(1).ssa(0).imm(int64_t(extra->type));
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

  cgCallHelper(
    vmain(env),
    env,
    target,
    kVoidDest,
    SyncOptions::Sync,
    args
  );
}

void cgRaiseHackArrPropNotice(IRLS& env, const IRInstruction* inst) {
  auto const extra = inst->extra<RaiseHackArrPropNotice>();
  cgCallHelper(
    vmain(env),
    env,
    inst->src(3)->boolVal()
      ? CallSpec::direct(hackArrPropNoticeImpl<true>)
      : CallSpec::direct(hackArrPropNoticeImpl<false>),
    kVoidDest,
    SyncOptions::Sync,
    argGroup(env, inst)
      .ssa(0)
      .ssa(1)
      .ssa(2)
      .imm(int64_t(extra->type))
  );
}

///////////////////////////////////////////////////////////////////////////////

}}}
