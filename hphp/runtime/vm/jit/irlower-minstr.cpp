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

#include "hphp/runtime/base/array-data.h"
#include "hphp/runtime/base/header-kind.h"
#include "hphp/runtime/base/mixed-array.h"
#include "hphp/runtime/base/packed-array.h"
#include "hphp/runtime/base/rds.h"
#include "hphp/runtime/base/typed-value.h"
#include "hphp/runtime/vm/member-operations.h"
#include "hphp/runtime/vm/unit.h"

#include "hphp/runtime/vm/jit/abi.h"
#include "hphp/runtime/vm/jit/arg-group.h"
#include "hphp/runtime/vm/jit/array-offset-profile.h"
#include "hphp/runtime/vm/jit/bc-marker.h"
#include "hphp/runtime/vm/jit/code-gen-cf.h"
#include "hphp/runtime/vm/jit/code-gen-helpers.h"
#include "hphp/runtime/vm/jit/code-gen-internal.h"
#include "hphp/runtime/vm/jit/ir-instruction.h"
#include "hphp/runtime/vm/jit/minstr-helpers.h"
#include "hphp/runtime/vm/jit/translator-inline.h"

#include "hphp/util/immed.h"
#include "hphp/util/stack-trace.h"
#include "hphp/util/struct-log.h"
#include "hphp/util/trace.h"

// This file does ugly things with macros so include last.
#include "hphp/runtime/vm/jit/irlower-minstr-internal.h"

namespace HPHP { namespace jit { namespace irlower {

TRACE_SET_MOD(irlower);

///////////////////////////////////////////////////////////////////////////////

void cgBaseG(IRLS& env, const IRInstruction* inst) {
  auto const mode = inst->extra<MOpModeData>()->mode;
  BUILD_OPTAB(BASE_G_HELPER_TABLE, mode);

  auto const args = argGroup(env, inst).typedValue(0);

  auto& v = vmain(env);
  cgCallHelper(v, env, CallSpec::direct(opFunc), callDest(env, inst),
               SyncOptions::Sync, args);
}

void cgFinishMemberOp(IRLS&, const IRInstruction*) {}

///////////////////////////////////////////////////////////////////////////////

namespace {

/*
 * Make an ArgGroup for prop instructions that takes:
 *    1/ the context Class*
 *    2/ the ObjectData* (or pointer to a KindOfObject TV)
 */
ArgGroup propArgs(IRLS& env, const IRInstruction* inst) {
  return argGroup(env, inst)
    .immPtr(inst->marker().func()->cls())
    .ssa(0);
}

void implProp(IRLS& env, const IRInstruction* inst) {
  auto const mode    = inst->extra<MOpModeData>()->mode;
  auto const base    = inst->src(0);
  auto const key     = inst->src(1);
  auto const keyType = getKeyTypeNoInt(key);

  void (*helper)();
  if (base->isA(TObj)) {
    BUILD_OPTAB(PROP_OBJ_HELPER_TABLE, mode, keyType);
    helper = opFunc;
  } else {
    BUILD_OPTAB(PROP_HELPER_TABLE, mode, keyType);
    helper = opFunc;
  }

  auto const args = propArgs(env, inst)
    .memberKeyS(1)
    .ssa(2);

  auto& v = vmain(env);
  cgCallHelper(v, env, CallSpec::direct(helper), callDest(env, inst),
               SyncOptions::Sync, args);
}

void implIssetEmptyProp(IRLS& env, const IRInstruction* inst) {
  auto const isEmpty = inst->op() == EmptyProp;
  auto const base = inst->src(0);
  auto const key = inst->src(1);
  auto const keyType = getKeyTypeNoInt(key);

  void (*helper)();
  if (base->isA(TObj)) {
    BUILD_OPTAB(ISSET_EMPTY_OBJ_PROP_HELPER_TABLE, keyType, isEmpty);
    helper = opFunc;
  } else {
    BUILD_OPTAB(ISSET_EMPTY_PROP_HELPER_TABLE, keyType, isEmpty);
    helper = opFunc;
  }

  auto const args = propArgs(env, inst).memberKeyS(1);

  auto& v = vmain(env);
  cgCallHelper(v, env, CallSpec::direct(helper), callDest(env, inst),
               SyncOptions::Sync, args);
}

}

///////////////////////////////////////////////////////////////////////////////

void cgPropX(IRLS& env, const IRInstruction* i)  { implProp(env, i); }
void cgPropDX(IRLS& env, const IRInstruction* i) { implProp(env, i); }

void cgPropQ(IRLS& env, const IRInstruction* inst) {
  using namespace MInstrHelpers;

  auto const args = propArgs(env, inst).ssa(1).ssa(2);

  auto helper = inst->src(0)->isA(TObj)
    ? CallSpec::direct(propCOQ)
    : CallSpec::direct(propCQ);

  auto& v = vmain(env);
  cgCallHelper(v, env, helper, callDest(env, inst), SyncOptions::Sync, args);
}

void cgCGetProp(IRLS& env, const IRInstruction* inst) {
  auto const mode    = inst->extra<MOpModeData>()->mode;
  auto const base    = inst->src(0);
  auto const key     = inst->src(1);
  auto const keyType = getKeyTypeNoInt(key);

  void (*helper)();
  if (base->isA(TObj)) {
    BUILD_OPTAB(CGET_OBJ_PROP_HELPER_TABLE, keyType, mode);
    helper = opFunc;
  } else {
    BUILD_OPTAB(CGET_PROP_HELPER_TABLE, keyType, mode);
    helper = opFunc;
  }

  auto const args = propArgs(env, inst).memberKeyS(1);

  auto& v = vmain(env);
  cgCallHelper(v, env, CallSpec::direct(helper), callDestTV(env, inst),
               SyncOptions::Sync, args);
}

void cgCGetPropQ(IRLS& env, const IRInstruction* inst) {
  using namespace MInstrHelpers;

  auto args = propArgs(env, inst).ssa(1);

  auto& v = vmain(env);

  if (inst->src(0)->isA(TObj)) {
    return cgCallHelper(v, env, CallSpec::direct(cGetPropSOQ),
                        callDestTV(env, inst), SyncOptions::Sync, args);
  }
  cgCallHelper(v, env, CallSpec::direct(cGetPropSQ),
               callDestTV(env, inst), SyncOptions::Sync, args);
}

void cgVGetProp(IRLS& env, const IRInstruction* inst) {
  auto const base    = inst->src(0);
  auto const key     = inst->src(1);
  auto const keyType = getKeyTypeNoInt(key);

  void (*helper)();
  if (base->isA(TObj)) {
    BUILD_OPTAB(VGET_OBJ_PROP_HELPER_TABLE, keyType);
    helper = opFunc;
  } else {
    BUILD_OPTAB(VGET_PROP_HELPER_TABLE, keyType);
    helper = opFunc;
  }

  auto const args = propArgs(env, inst).memberKeyS(1);

  auto& v = vmain(env);
  cgCallHelper(v, env, CallSpec::direct(helper), callDest(env, inst),
               SyncOptions::Sync, args);
}

void cgBindProp(IRLS& env, const IRInstruction* inst) {
  auto const base = inst->src(0);

  auto helper = base->isA(TObj)
    ? CallSpec::direct(MInstrHelpers::bindPropCO)
    : CallSpec::direct(MInstrHelpers::bindPropC);

  auto const args = propArgs(env, inst).typedValue(1).ssa(2);

  auto& v = vmain(env);
  cgCallHelper(v, env, helper, callDest(env, inst), SyncOptions::Sync, args);
}

void cgSetProp(IRLS& env, const IRInstruction* inst) {
  auto const base = inst->src(0);
  auto const key = inst->src(1);
  auto const keyType = getKeyTypeNoInt(key);

  void (*helper)();
  if (base->isA(TObj)) {
    BUILD_OPTAB(SETPROP_OBJ_HELPER_TABLE, keyType);
    helper = opFunc;
  } else {
    BUILD_OPTAB(SETPROP_HELPER_TABLE, keyType);
    helper = opFunc;
  }

  auto const args = propArgs(env, inst)
    .memberKeyS(1)
    .typedValue(2);

  auto& v = vmain(env);
  cgCallHelper(v, env, CallSpec::direct(helper),
               kVoidDest, SyncOptions::Sync, args);
}

void cgUnsetProp(IRLS& env, const IRInstruction* inst) {
  auto const base = inst->src(0);

  auto helper = base->isA(TObj)
    ? CallSpec::direct(MInstrHelpers::unsetPropCO)
    : CallSpec::direct(MInstrHelpers::unsetPropC);

  auto const args = propArgs(env, inst).typedValue(1);

  auto& v = vmain(env);
  cgCallHelper(v, env, helper, kVoidDest, SyncOptions::Sync, args);
}

void cgSetOpProp(IRLS& env, const IRInstruction* inst) {
  auto const base = inst->src(0);
  auto const extra = inst->extra<SetOpProp>();

  auto helper = base->isA(TObj)
    ? CallSpec::direct(MInstrHelpers::setOpPropCO)
    : CallSpec::direct(MInstrHelpers::setOpPropC);

  auto const args = propArgs(env, inst)
    .typedValue(1)
    .typedValue(2)
    .imm(static_cast<int32_t>(extra->op));

  auto& v = vmain(env);
  cgCallHelper(v, env, helper, callDestTV(env, inst), SyncOptions::Sync, args);
}

void cgIncDecProp(IRLS& env, const IRInstruction* inst) {
  auto const base = inst->src(0);
  auto const extra = inst->extra<IncDecProp>();

  auto helper = base->isA(TObj)
    ? CallSpec::direct(MInstrHelpers::incDecPropCO)
    : CallSpec::direct(MInstrHelpers::incDecPropC);

  auto const args = propArgs(env, inst)
    .typedValue(1)
    .imm(static_cast<int32_t>(extra->op));

  auto& v = vmain(env);
  cgCallHelper(v, env, helper, callDestTV(env, inst),
               SyncOptions::Sync, args);
}

void cgIssetProp(IRLS& env, const IRInstruction* i) {
  implIssetEmptyProp(env, i);
}
void cgEmptyProp(IRLS& env, const IRInstruction* i) {
  implIssetEmptyProp(env, i);
}

///////////////////////////////////////////////////////////////////////////////

namespace {

/*
 * Make an ArgGroup for elem instructions that takes:
 *    1/ the pointer to a KindOfArray TV
 *    2/ the index key, as a raw value or a TypedValue depending on whether
 *       the type is known
 */
ArgGroup elemArgs(IRLS& env, const IRInstruction* inst) {
  return argGroup(env, inst)
    .ssa(0)
    .memberKeyIS(1);
}

void implElem(IRLS& env, const IRInstruction* inst) {
  auto const mode  = inst->extra<MOpModeData>()->mode;
  auto const key   = inst->src(1);
  BUILD_OPTAB(ELEM_HELPER_TABLE, getKeyType(key), mode,
              RuntimeOption::EvalHackArrCompatNotices);

  auto const args = elemArgs(env, inst).ssa(2);

  auto& v = vmain(env);
  cgCallHelper(v, env, CallSpec::direct(opFunc), callDest(env, inst),
               SyncOptions::Sync, args);
}

void implIssetEmptyElem(IRLS& env, const IRInstruction* inst) {
  auto const isEmpty = inst->op() == EmptyElem;
  auto const key     = inst->src(1);
  BUILD_OPTAB(ISSET_EMPTY_ELEM_HELPER_TABLE, getKeyType(key),
              isEmpty, RuntimeOption::EvalHackArrCompatNotices);

  auto& v = vmain(env);
  cgCallHelper(v, env, CallSpec::direct(opFunc), callDest(env, inst),
               SyncOptions::Sync, elemArgs(env, inst));
}

}

///////////////////////////////////////////////////////////////////////////////

void cgElemX(IRLS& env, const IRInstruction* i)  { implElem(env, i); }
void cgElemDX(IRLS& env, const IRInstruction* i) { implElem(env, i); }
void cgElemUX(IRLS& env, const IRInstruction* i) { implElem(env, i); }

void cgCGetElem(IRLS& env, const IRInstruction* inst) {
  auto const mode  = inst->extra<MOpModeData>()->mode;
  auto const key   = inst->src(1);
  BUILD_OPTAB(CGETELEM_HELPER_TABLE, getKeyType(key), mode,
              RuntimeOption::EvalHackArrCompatNotices);

  auto& v = vmain(env);
  cgCallHelper(v, env, CallSpec::direct(opFunc), callDestTV(env, inst),
               SyncOptions::Sync, elemArgs(env, inst));
}

void cgVGetElem(IRLS& env, const IRInstruction* inst) {
  auto const key = inst->src(1);
  BUILD_OPTAB(VGETELEM_HELPER_TABLE, getKeyType(key),
              RuntimeOption::EvalHackArrCompatNotices);

  auto& v = vmain(env);
  cgCallHelper(v, env, CallSpec::direct(opFunc), callDest(env, inst),
               SyncOptions::Sync, elemArgs(env, inst));
}

void cgSetElem(IRLS& env, const IRInstruction* inst) {
  auto const key = inst->src(1);
  BUILD_OPTAB(SETELEM_HELPER_TABLE, getKeyType(key),
              RuntimeOption::EvalHackArrCompatNotices);

  auto& v = vmain(env);
  cgCallHelper(v, env, CallSpec::direct(opFunc), callDest(env, inst),
               SyncOptions::Sync, elemArgs(env, inst).typedValue(2));
}

IMPL_OPCODE_CALL(SetNewElem);
IMPL_OPCODE_CALL(BindNewElem);

void cgSetWithRefElem(IRLS& env, const IRInstruction* inst) {
  auto& v = vmain(env);

  auto const target = RuntimeOption::EvalHackArrCompatNotices
    ? CallSpec::direct(MInstrHelpers::setWithRefElem<true>)
    : CallSpec::direct(MInstrHelpers::setWithRefElem<false>);

  auto const args = argGroup(env, inst)
    .ssa(0)
    .typedValue(1)
    .typedValue(2);

  cgCallHelper(v, env, target, kVoidDest, SyncOptions::Sync, args);
}

void cgBindElem(IRLS& env, const IRInstruction* inst) {
  auto& v = vmain(env);

  auto const target = RuntimeOption::EvalHackArrCompatNotices
    ? CallSpec::direct(MInstrHelpers::bindElemC<true>)
    : CallSpec::direct(MInstrHelpers::bindElemC<false>);

  auto const args = argGroup(env, inst)
    .ssa(0)
    .typedValue(1)
    .ssa(2);

  cgCallHelper(v, env, target, kVoidDest, SyncOptions::Sync, args);
}

void cgSetOpElem(IRLS& env, const IRInstruction* inst) {
  auto& v = vmain(env);

  auto const target = RuntimeOption::EvalHackArrCompatNotices
    ? CallSpec::direct(MInstrHelpers::setOpElem<true>)
    : CallSpec::direct(MInstrHelpers::setOpElem<false>);

  auto const args = argGroup(env, inst)
    .ssa(0)
    .typedValue(1)
    .typedValue(2)
    .imm(uint32_t(inst->extra<SetOpElem>()->op));

  cgCallHelper(v, env, target, callDestTV(env, inst),
               SyncOptions::Sync, args);
}

void cgIncDecElem(IRLS& env, const IRInstruction* inst) {
  auto& v = vmain(env);

  auto const target = RuntimeOption::EvalHackArrCompatNotices
    ? CallSpec::direct(MInstrHelpers::incDecElem<true>)
    : CallSpec::direct(MInstrHelpers::incDecElem<false>);

  auto const args = argGroup(env, inst)
    .ssa(0)
    .typedValue(1)
    .imm(uint32_t(inst->extra<IncDecElem>()->op));

  cgCallHelper(v, env, target, callDestTV(env, inst),
               SyncOptions::Sync, args);
}

void cgUnsetElem(IRLS& env, const IRInstruction* inst) {
  auto const key = inst->src(1);
  BUILD_OPTAB(UNSET_ELEM_HELPER_TABLE, getKeyType(key),
              RuntimeOption::EvalHackArrCompatNotices);

  auto& v = vmain(env);
  cgCallHelper(v, env, CallSpec::direct(opFunc), kVoidDest,
               SyncOptions::Sync, elemArgs(env, inst));
}

void cgIssetElem(IRLS& env, const IRInstruction* i) {
  implIssetEmptyElem(env, i);
}
void cgEmptyElem(IRLS& env, const IRInstruction* i) {
  implIssetEmptyElem(env, i);
}

///////////////////////////////////////////////////////////////////////////////

namespace {

/*
 * Make an ArgGroup for array elem instructions that takes:
 *    1/ the ArrayData* (or pointer to a KindOfArray TV)
 *    2/ the index key
 */
ArgGroup arrArgs(IRLS& env, const IRInstruction* inst,
                 const ArrayKeyInfo& keyInfo) {
  auto args = argGroup(env, inst).ssa(0);
  if (keyInfo.converted) {
    args.imm(keyInfo.convertedInt);
  } else {
    args.ssa(1);
  }
  return args;
}

}

///////////////////////////////////////////////////////////////////////////////
// Offset profiling.

namespace {

template<class OpFunc>
void implProfileHackArrayOffset(IRLS& env, const IRInstruction* inst,
                                OpFunc opFunc) {
  auto& v = vmain(env);

  auto const rprof = v.makeReg();
  v << lea{rvmtl()[inst->extra<RDSHandleData>()->handle], rprof};

  auto args = argGroup(env, inst).ssa(0).ssa(1).reg(rprof);

  cgCallHelper(v, env, CallSpec::direct(opFunc), kVoidDest,
               SyncOptions::Sync, args);
}

void implCheckMixedArrayLikeOffset(IRLS& env, const IRInstruction* inst,
                                   KeyType key_type) {
  auto const arr = srcLoc(env, inst, 0).reg();
  auto const key = srcLoc(env, inst, 1).reg();
  auto const branch = label(env, inst->taken());
  auto const pos = inst->extra<IndexData>()->index;
  auto& v = vmain(env);

  auto const elmOff = MixedArray::elmOff(pos);
  using Elm = MixedArray::Elm;

  { // Also fail if our predicted position exceeds bounds.
    auto const sf = v.makeReg();
    v << cmplim{safe_cast<int32_t>(pos), arr[MixedArray::usedOff()], sf};
    ifThen(v, CC_LE, sf, branch);
  }
  { // Fail if the Elm key value doesn't match.
    auto const sf = v.makeReg();
    v << cmpqm{key, arr[elmOff + Elm::keyOff()], sf};
    ifThen(v, CC_NE, sf, branch);
  }
  { // Fail if the Elm key type doesn't match.
    auto const sf = v.makeReg();
    v << cmplim{0, arr[elmOff + Elm::dataOff() + TVOFF(m_aux)], sf};

    assertx(key_type != KeyType::Any);

    // Note that if `key' actually is an integer-ish string, we'd fail this
    // check (and most likely would have failed the previous check also), but
    // this false negative is allowed.
    auto const is_str_key = key_type == KeyType::Str;
    ifThen(v, is_str_key ? CC_L : CC_GE, sf, branch);
  }
  { // Fail if the Elm is a tombstone.  See MixedArray::isTombstone().
    auto const sf = v.makeReg();
    v << cmpbim{KindOfUninit, arr[elmOff + Elm::dataOff() + TVOFF(m_type)], sf};
    ifThen(v, CC_L, sf, branch);
  }
}

}

void cgProfileMixedArrayOffset(IRLS& env, const IRInstruction* inst) {
  auto const arr = inst->src(0);
  auto const key = inst->src(1);
  auto const keyInfo = checkStrictlyInteger(arr->type(), key->type());

  BUILD_OPTAB(PROFILE_MIXED_ARRAY_OFFSET_HELPER_TABLE,
              keyInfo.type,
              keyInfo.checkForInt);
  auto& v = vmain(env);

  auto const rprof = v.makeReg();
  v << lea{rvmtl()[inst->extra<ProfileMixedArrayOffset>()->handle], rprof};

  cgCallHelper(v, env, CallSpec::direct(opFunc), kVoidDest, SyncOptions::Sync,
               arrArgs(env, inst, keyInfo).reg(rprof));
}

void cgProfileDictOffset(IRLS& env, const IRInstruction* inst) {
  BUILD_OPTAB(PROFILE_DICT_OFFSET_HELPER_TABLE, getKeyType(inst->src(1)));
  implProfileHackArrayOffset(env, inst, opFunc);
}

void cgProfileKeysetOffset(IRLS& env, const IRInstruction* inst) {
  BUILD_OPTAB(PROFILE_KEYSET_OFFSET_HELPER_TABLE, getKeyType(inst->src(1)));
  implProfileHackArrayOffset(env, inst, opFunc);
}

void cgCheckMixedArrayOffset(IRLS& env, const IRInstruction* inst) {
  implCheckMixedArrayLikeOffset(
    env, inst,
    checkStrictlyInteger(inst->src(0)->type(), inst->src(1)->type()).type
  );
}

void cgCheckDictOffset(IRLS& env, const IRInstruction* inst) {
  implCheckMixedArrayLikeOffset(env, inst, getKeyType(inst->src(1)));
}

void cgCheckKeysetOffset(IRLS& env, const IRInstruction* inst) {
  auto const keyset = srcLoc(env, inst, 0).reg();
  auto const key = srcLoc(env, inst, 1).reg();
  auto const branch = label(env, inst->taken());
  auto const pos = inst->extra<CheckKeysetOffset>()->index;
  auto& v = vmain(env);
  auto const tvOff = SetArray::tvOff(pos);

  { // Fail if our predicted position exceeds bounds.
    auto const sf = v.makeReg();
    v << cmplim{safe_cast<int32_t>(pos), keyset[SetArray::usedOff()], sf};
    ifThen(v, CC_LE, sf, branch);
  }
  { // Fail if the Elm key value doesn't match.
    auto const sf = v.makeReg();
    v << cmpqm{key, keyset[tvOff + TVOFF(m_data)], sf};
    ifThen(v, CC_NE, sf, branch);
  }
  { // Fail if the Elm key type doesn't match.
    auto const sf = v.makeReg();
    v << cmplim{0, keyset[tvOff + TVOFF(m_aux)], sf};

    auto const key_type = getKeyType(inst->src(1));
    assertx(key_type != KeyType::Any);

    auto const is_str_key = key_type == KeyType::Str;
    ifThen(v, is_str_key ? CC_L : CC_GE, sf, branch);
  }
  { // Fail if the Elm is a tombstone.  See SetArray::isTombstone().
    auto const sf = v.makeReg();
    v << cmpbim{KindOfUninit, keyset[tvOff + TVOFF(m_type)], sf};
    ifThen(v, CC_L, sf, branch);
  }
}

void cgCheckArrayCOW(IRLS& env, const IRInstruction* inst) {
  auto const arr = srcLoc(env, inst, 0).reg();
  auto& v = vmain(env);

  auto const sf = emitCmpRefCount(v, OneReference, arr);
  ifThen(v, CC_NE, sf, label(env, inst->taken()));
}

///////////////////////////////////////////////////////////////////////////////
// Array.

namespace {

void implElemArray(IRLS& env, const IRInstruction* inst) {
  auto const arr = inst->src(0);
  auto const key = inst->src(1);
  auto const mode = inst->op() == ElemArrayW ? MOpMode::Warn : MOpMode::None;
  auto const keyInfo = checkStrictlyInteger(arr->type(), key->type());
  BUILD_OPTAB(ELEM_ARRAY_HELPER_TABLE,
              keyInfo.type, keyInfo.checkForInt, mode,
              RuntimeOption::EvalHackArrCompatNotices);

  auto& v = vmain(env);
  cgCallHelper(v, env, CallSpec::direct(opFunc), callDest(env, inst),
               SyncOptions::Sync, arrArgs(env, inst, keyInfo));
}

void implArraySet(IRLS& env, const IRInstruction* inst) {
  bool const setRef  = inst->op() == ArraySetRef;
  auto const arr     = inst->src(0);
  auto const key     = inst->src(1);
  auto const keyInfo = checkStrictlyInteger(arr->type(), key->type());
  BUILD_OPTAB(ARRAYSET_HELPER_TABLE,
              keyInfo.type,
              keyInfo.checkForInt,
              setRef,
              RuntimeOption::EvalHackArrCompatNotices);


  auto args = arrArgs(env, inst, keyInfo);
  args.typedValue(2);
  if (setRef) args.ssa(3);

  auto& v = vmain(env);
  cgCallHelper(v, env, CallSpec::direct(opFunc), callDest(env, inst),
               SyncOptions::Sync, args);
}

}

void cgElemArray(IRLS& env, const IRInstruction* i)  { implElemArray(env, i); }
void cgElemArrayW(IRLS& env, const IRInstruction* i) { implElemArray(env, i); }

void cgElemArrayD(IRLS& env, const IRInstruction* inst) {
  auto const key     = inst->src(1);
  auto const keyInfo = checkStrictlyInteger(inst->typeParam(), key->type());
  BUILD_OPTAB(ELEM_ARRAY_D_HELPER_TABLE, keyInfo.type,
              RuntimeOption::EvalHackArrCompatNotices);

  auto& v = vmain(env);
  cgCallHelper(v, env, CallSpec::direct(opFunc), callDest(env, inst),
               SyncOptions::Sync, arrArgs(env, inst, keyInfo));
}

void cgElemArrayU(IRLS& env, const IRInstruction* inst) {
  auto const key     = inst->src(1);
  auto const keyInfo = checkStrictlyInteger(inst->typeParam(), key->type());
  BUILD_OPTAB(ELEM_ARRAY_U_HELPER_TABLE, keyInfo.type,
              RuntimeOption::EvalHackArrCompatNotices);

  auto& v = vmain(env);
  cgCallHelper(v, env, CallSpec::direct(opFunc), callDest(env, inst),
               SyncOptions::Sync, arrArgs(env, inst, keyInfo));
}

void cgElemMixedArrayK(IRLS& env, const IRInstruction* inst) {
  auto const arr = srcLoc(env, inst, 0).reg();
  auto const dst = dstLoc(env, inst, 0);
  auto const pos = inst->extra<ElemMixedArrayK>()->index;
  auto const off = MixedArray::elmOff(pos) + MixedArray::Elm::dataOff();

  auto& v = vmain(env);

  assertx(dst.numAllocated() == 1);
  v << lea{arr[off], dst.reg()};
}

void cgArrayGet(IRLS& env, const IRInstruction* inst) {
  auto const arr = inst->src(0);
  auto const key = inst->src(1);
  auto const keyInfo = checkStrictlyInteger(arr->type(), key->type());
  BUILD_OPTAB(ARRAYGET_HELPER_TABLE, keyInfo.type, keyInfo.checkForInt,
                      RuntimeOption::EvalHackArrCompatNotices);

  auto& v = vmain(env);
  cgCallHelper(v, env, CallSpec::direct(opFunc), callDestTV(env, inst),
               SyncOptions::Sync, arrArgs(env, inst, keyInfo));
}

void cgMixedArrayGetK(IRLS& env, const IRInstruction* inst) {
  auto const arr = srcLoc(env, inst, 0).reg();
  auto const pos = inst->extra<MixedArrayGetK>()->index;
  auto const off = MixedArray::elmOff(pos) + MixedArray::Elm::dataOff();

  auto& v = vmain(env);
  loadTV(v, inst->dst(0), dstLoc(env, inst, 0), arr[off]);
}

void cgArraySet(IRLS& env, const IRInstruction* i)    { implArraySet(env, i); }
void cgArraySetRef(IRLS& env, const IRInstruction* i) { implArraySet(env, i); }

IMPL_OPCODE_CALL(SetNewElemArray);

IMPL_OPCODE_CALL(AddElemIntKey);
IMPL_OPCODE_CALL(AddNewElem);

static ArrayData* addNewElemKeysetImpl(ArrayData* keyset, Cell v) {
  assertx(keyset->isKeyset());
  auto out = SetArray::Append(keyset, v, keyset->cowCheck());
  if (keyset != out) decRefArr(keyset);
  return out;
}

static ArrayData* addNewElemVecImpl(ArrayData* vec, Cell v) {
  assertx(vec->isVecArray());
  auto out = PackedArray::AppendVec(vec, v, vec->cowCheck());
  if (vec != out) decRefArr(vec);
  return out;
}

void cgAddNewElemKeyset(IRLS& env, const IRInstruction* inst) {
  cgCallHelper(
    vmain(env),
    env,
    CallSpec::direct(addNewElemKeysetImpl),
    callDest(env, inst),
    SyncOptions::Sync,
    argGroup(env, inst).ssa(0).typedValue(1)
  );
}

void cgAddNewElemVec(IRLS& env, const IRInstruction* inst) {
  cgCallHelper(
    vmain(env),
    env,
    CallSpec::direct(addNewElemVecImpl),
    callDest(env, inst),
    SyncOptions::None,
    argGroup(env, inst).ssa(0).typedValue(1)
  );
}

void cgAddElemStrKey(IRLS& env, const IRInstruction* inst) {
  auto& v = vmain(env);

  auto const target = RuntimeOption::EvalHackArrCompatNotices
    ? CallSpec::direct(addElemStringKeyHelper<true>)
    : CallSpec::direct(addElemStringKeyHelper<false>);

  cgCallHelper(v, env, target,
               callDest(env, inst), SyncOptions::Sync,
               argGroup(env, inst).ssa(0).ssa(1).typedValue(2));
}

void cgArrayIsset(IRLS& env, const IRInstruction* inst) {
  auto const arr     = inst->src(0);
  auto const key     = inst->src(1);
  auto const keyInfo = checkStrictlyInteger(arr->type(), key->type());
  BUILD_OPTAB(ARRAY_ISSET_HELPER_TABLE, keyInfo.type, keyInfo.checkForInt,
              RuntimeOption::EvalHackArrCompatNotices);

  auto& v = vmain(env);
  cgCallHelper(v, env, CallSpec::direct(opFunc), callDest(env, inst),
               SyncOptions::Sync, arrArgs(env, inst, keyInfo));
}

void cgArrayIdx(IRLS& env, const IRInstruction* inst) {
  auto const arr     = inst->src(0);
  auto const key     = inst->src(1);
  auto const keyInfo = checkStrictlyInteger(arr->type(), key->type());

  auto const target = [&] () -> CallSpec {
    if (keyInfo.checkForInt) {
      return RuntimeOption::EvalHackArrCompatNotices
        ? CallSpec::direct(arrayIdxSi<true>)
        : CallSpec::direct(arrayIdxSi<false>);
    }
    if (keyInfo.type == KeyType::Int) {
      return CallSpec::direct(arrayIdxI);
    }
    return CallSpec::direct(arrayIdxS);
  }();

  auto& v = vmain(env);
  cgCallHelper(v, env, target, callDestTV(env, inst), SyncOptions::Sync,
               arrArgs(env, inst, keyInfo).typedValue(2));
}

///////////////////////////////////////////////////////////////////////////////
// Vec.

namespace {

Vptr implPackedLayoutElemAddr(IRLS& env, Vloc arrLoc,
                              Vloc idxLoc, const SSATmp* idx) {
  auto const rarr = arrLoc.reg();
  auto const ridx = idxLoc.reg();
  auto& v = vmain(env);

  static_assert(sizeof(TypedValue) == 16, "");

  if (idx->hasConstVal()) {
    auto const offset = PackedArray::entriesOffset() +
                        idx->intVal() * sizeof(TypedValue);
    if (deltaFits(offset, sz::dword)) {
      return rarr[offset];
    }
  }

  /*
   * Compute `rarr + ridx * sizeof(TypedValue) + PackedArray::entriesOffset()`.
   *
   * The logic of `scaledIdx * 16` is split in the following two instructions,
   * in order to save a byte in the shl instruction.
   *
   * TODO(#7728856): We should really move this into vasm-x64.cpp...
   */
  auto idxl = v.makeReg();
  auto scaled_idxl = v.makeReg();
  auto scaled_idx = v.makeReg();
  v << movtql{ridx, idxl};
  v << shlli{1, idxl, scaled_idxl, v.makeReg()};
  v << movzlq{scaled_idxl, scaled_idx};
  return rarr[scaled_idx * int(sizeof(TypedValue) / 2)
              + PackedArray::entriesOffset()];
}

void implVecSet(IRLS& env, const IRInstruction* inst) {
  bool const setRef = inst->op() == VecSetRef;
  BUILD_OPTAB(VECSET_HELPER_TABLE, setRef);

  auto args = argGroup(env, inst).
    ssa(0).
    ssa(1).
    typedValue(2);
  if (setRef) args.ssa(3);

  auto& v = vmain(env);
  cgCallHelper(v, env, CallSpec::direct(opFunc), callDest(env, inst),
               SyncOptions::Sync, args);
}

/*
 * Thread-local RDS packed array access sampling counter.
 */
rds::Link<uint32_t> s_counter{rds::kInvalidHandle};

}

void record_packed_access(const ArrayData* ad) {
  assertx(s_counter.bound());
  *s_counter = RuntimeOption::EvalProfPackedArraySampleFreq;

  auto record = StructuredLogEntry{};
  record.setInt("size", ad->size());

  auto const st = StackTrace(StackTrace::Force{});
  auto frames = std::vector<folly::StringPiece>{};
  folly::split("\n", st.toString(), frames);
  record.setVec("stacktrace", frames);

  FTRACE_MOD(Trace::prof_array, 1,
             "prof_array: {}\n", show(record).c_str());
  StructuredLog::log("hhvm_arrays", record);
}

void cgLdPackedArrayDataElemAddr(IRLS& env, const IRInstruction* inst) {
  auto const arrLoc = srcLoc(env, inst, 0);
  auto const idxLoc = srcLoc(env, inst, 1);
  auto& v = vmain(env);
  auto& vc = vcold(env);

  if (UNLIKELY(RuntimeOption::EvalProfPackedArraySampleFreq > 0)) {
    auto const arrTy = inst->src(0)->type();
    auto const packedTy = Type::Array(ArrayData::kPackedKind);

    if (arrTy.maybe(packedTy)) {
      s_counter.bind(rds::Mode::Local);

      auto const profile = [&] (Vout& v) {
        auto const handle = s_counter.handle();
        auto const sf = v.makeReg();
        v << declm{rvmtl()[handle], sf};

        unlikelyIfThen(v, vc, CC_LE, sf, [&] (Vout& v) {
          // Log this array access.
          v << vcall{CallSpec::direct(record_packed_access),
                     v.makeVcallArgs({{arrLoc.reg()}}), v.makeTuple({})};
        });
      };

      if (arrTy <= packedTy) {
        profile(v);
      } else {
        auto const sf = v.makeReg();
        v << cmpbim{ArrayData::kPackedKind, arrLoc.reg()[HeaderKindOffset], sf};
        ifThen(v, CC_E, sf, profile);
      }
    }
  }

  auto const addr = implPackedLayoutElemAddr(env, arrLoc, idxLoc, inst->src(1));
  vmain(env) << lea{addr, dstLoc(env, inst, 0).reg()};
}

void cgLdVecElem(IRLS& env, const IRInstruction* inst) {
  auto const arrLoc = srcLoc(env, inst, 0);
  auto const idxLoc = srcLoc(env, inst, 1);
  auto const addr = implPackedLayoutElemAddr(env, arrLoc, idxLoc, inst->src(1));
  loadTV(vmain(env), inst->dst(), dstLoc(env, inst, 0), addr);
}

IMPL_OPCODE_CALL(ElemVecD)
IMPL_OPCODE_CALL(ElemVecU)

void cgVecSet(IRLS& env, const IRInstruction* i)    { implVecSet(env, i); }
void cgVecSetRef(IRLS& env, const IRInstruction* i) { implVecSet(env, i); }

IMPL_OPCODE_CALL(SetNewElemVec);

void cgReservePackedArrayDataNewElem(IRLS& env, const IRInstruction* i) {
  static_assert(ArrayData::sizeofSize() == 4, "");

  auto& v = vmain(env);
  auto arrayData = srcLoc(env, i, 0).reg();
  auto const sizePtr = arrayData[ArrayData::offsetofSize()];

  // If the check below succeeds, we'll end up returning the original size
  // so just use the destination register to hold the orignal size
  auto const size = dstLoc(env, i, 0).reg();
  v << loadzlq{sizePtr, size};

  { // Bail out unless size < cap
    auto const indexb = v.makeReg();
    v << loadb{arrayData[PackedArray::SizeIndexOffset], indexb};
    auto const index = v.makeReg();
    v << movzbq{indexb, index};
    auto const cap = v.makeReg();
    auto const table =
      reinterpret_cast<uintptr_t>(kSizeIndex2PackedArrayCapacity);
    if (table < std::numeric_limits<int>::max()) {
      v << loadzlq{baseless(index * 4 + table), cap};
    } else {
      auto const base = v.cns(table);
      v << loadzlq{base[index * 4], cap};
    }

    auto const sf = v.makeReg();
    v << cmpq{size, cap, sf};
    ifThen(v, CC_BE, sf, label(env, i->taken()));
  }

  v << inclm{sizePtr, v.makeReg()};
}

///////////////////////////////////////////////////////////////////////////////
// Dict.

namespace {

void implElemDict(IRLS& env, const IRInstruction* inst) {
  auto const key = inst->src(1);
  auto const mode = inst->op() == ElemDictW ? MOpMode::Warn : MOpMode::None;
  BUILD_OPTAB(ELEM_DICT_HELPER_TABLE, getKeyType(key), mode);

  auto args = argGroup(env, inst).ssa(0).ssa(1);

  auto& v = vmain(env);
  cgCallHelper(v, env, CallSpec::direct(opFunc), callDest(env, inst),
               SyncOptions::Sync, args);
}

void implDictGet(IRLS& env, const IRInstruction* inst) {
  auto const key = inst->src(1);
  auto const mode =
    (inst->op() == DictGetQuiet) ? MOpMode::None : MOpMode::Warn;
  BUILD_OPTAB(DICTGET_HELPER_TABLE, getKeyType(key), mode);

  auto args = argGroup(env, inst).ssa(0).ssa(1);

  auto& v = vmain(env);
  cgCallHelper(v, env, CallSpec::direct(opFunc), callDestTV(env, inst),
               SyncOptions::Sync, args);
}

void implDictSet(IRLS& env, const IRInstruction* inst) {
  auto const key = inst->src(1);
  bool const setRef  = inst->op() == DictSetRef;
  BUILD_OPTAB(DICTSET_HELPER_TABLE, getKeyType(key), setRef);

  auto args = argGroup(env, inst).
    ssa(0).
    ssa(1).
    typedValue(2);
  if (setRef) args.ssa(3);

  auto& v = vmain(env);
  cgCallHelper(v, env, CallSpec::direct(opFunc), callDest(env, inst),
               SyncOptions::Sync, args);
}

void implDictIsset(IRLS& env, const IRInstruction* inst) {
  auto const key     = inst->src(1);
  auto const empty   = inst->op() == DictEmptyElem;
  BUILD_OPTAB(DICT_ISSET_EMPTY_ELEM_HELPER_TABLE, getKeyType(key), empty);

  auto args = argGroup(env, inst).ssa(0).ssa(1);

  auto& v = vmain(env);
  cgCallHelper(v, env, CallSpec::direct(opFunc), callDest(env, inst),
               SyncOptions::Sync, args);
}

}

void cgElemDict(IRLS& env, const IRInstruction* i)  { implElemDict(env, i); }
void cgElemDictW(IRLS& env, const IRInstruction* i) { implElemDict(env, i); }

void cgElemDictD(IRLS& env, const IRInstruction* inst) {
  auto const key     = inst->src(1);
  BUILD_OPTAB(ELEM_DICT_D_HELPER_TABLE, getKeyType(key));

  auto args = argGroup(env, inst).ssa(0).ssa(1);

  auto& v = vmain(env);
  cgCallHelper(v, env, CallSpec::direct(opFunc), callDest(env, inst),
               SyncOptions::Sync, args);
}

void cgElemDictU(IRLS& env, const IRInstruction* inst) {
  auto const key     = inst->src(1);
  BUILD_OPTAB(ELEM_DICT_U_HELPER_TABLE, getKeyType(key));

  auto args = argGroup(env, inst).ssa(0).ssa(1);

  auto& v = vmain(env);
  cgCallHelper(v, env, CallSpec::direct(opFunc), callDest(env, inst),
               SyncOptions::Sync, args);
}

void cgElemDictK(IRLS& env, const IRInstruction* inst) {
  auto const dict = srcLoc(env, inst, 0).reg();
  auto const dst = dstLoc(env, inst, 0);
  auto const pos = inst->extra<ElemDictK>()->index;
  auto const off = MixedArray::elmOff(pos) + MixedArray::Elm::dataOff();

  auto& v = vmain(env);

  assertx(dst.numAllocated() == 1);
  v << lea{dict[off], dst.reg()};
}

void cgDictGet(IRLS& env, const IRInstruction* inst) {
  implDictGet(env, inst);
}
void cgDictGetQuiet(IRLS& env, const IRInstruction* inst) {
  implDictGet(env, inst);
}

void cgDictGetK(IRLS& env, const IRInstruction* inst) {
  auto const dict = srcLoc(env, inst, 0).reg();
  auto const pos = inst->extra<DictGetK>()->index;
  auto const off = MixedArray::elmOff(pos) + MixedArray::Elm::dataOff();

  auto& v = vmain(env);
  loadTV(v, inst->dst(0), dstLoc(env, inst, 0), dict[off]);
}

void cgDictSet(IRLS& env, const IRInstruction* i)    { implDictSet(env, i); }
void cgDictSetRef(IRLS& env, const IRInstruction* i) { implDictSet(env, i); }

IMPL_OPCODE_CALL(DictAddElemIntKey);
IMPL_OPCODE_CALL(DictAddElemStrKey);

void cgDictIsset(IRLS& env, const IRInstruction* inst) {
  implDictIsset(env, inst);
}
void cgDictEmptyElem(IRLS& env, const IRInstruction* inst) {
  implDictIsset(env, inst);
}

void cgDictIdx(IRLS& env, const IRInstruction* inst) {
  auto const key     = inst->src(1);
  auto const target  = getKeyType(key) == KeyType::Int
    ? CallSpec::direct(dictIdxI)
    : CallSpec::direct(dictIdxS);
  auto args = argGroup(env, inst).ssa(0).ssa(1).typedValue(2);
  auto& v = vmain(env);
  cgCallHelper(v, env, target, callDestTV(env, inst),
               SyncOptions::Sync, args);
}

///////////////////////////////////////////////////////////////////////////////
// Keyset.

namespace {

void implElemKeyset(IRLS& env, const IRInstruction* inst) {
  auto const key = inst->src(1);
  auto const mode = inst->op() == ElemKeysetW
    ? MOpMode::Warn
    : MOpMode::None;
  BUILD_OPTAB(ELEM_KEYSET_HELPER_TABLE, getKeyType(key), mode);

  auto args = argGroup(env, inst).ssa(0).ssa(1);

  auto& v = vmain(env);
  cgCallHelper(v, env, CallSpec::direct(opFunc), callDest(env, inst),
               SyncOptions::Sync, args);
}

void implKeysetGet(IRLS& env, const IRInstruction* inst) {
  auto const key = inst->src(1);
  auto const mode = inst->op() == KeysetGetQuiet
    ? MOpMode::None
    : MOpMode::Warn;
  BUILD_OPTAB(KEYSETGET_HELPER_TABLE, getKeyType(key), mode);

  auto args = argGroup(env, inst).ssa(0).ssa(1);

  auto& v = vmain(env);
  cgCallHelper(v, env, CallSpec::direct(opFunc), callDestTV(env, inst),
               SyncOptions::Sync, args);
}

void implKeysetIsset(IRLS& env, const IRInstruction* inst) {
  auto const key = inst->src(1);
  auto const empty = inst->op() == KeysetEmptyElem;
  BUILD_OPTAB(KEYSET_ISSET_EMPTY_ELEM_HELPER_TABLE, getKeyType(key), empty);

  auto args = argGroup(env, inst).ssa(0).ssa(1);

  auto& v = vmain(env);
  cgCallHelper(v, env, CallSpec::direct(opFunc), callDest(env, inst),
               SyncOptions::Sync, args);
}

}

void cgElemKeyset(IRLS& e, const IRInstruction* i)  { implElemKeyset(e, i); }
void cgElemKeysetW(IRLS& e, const IRInstruction* i) { implElemKeyset(e, i); }

void cgElemKeysetU(IRLS& env, const IRInstruction* inst) {
  auto const key = inst->src(1);
  BUILD_OPTAB(ELEM_KEYSET_U_HELPER_TABLE, getKeyType(key));

  auto args = argGroup(env, inst).ssa(0).ssa(1);

  auto& v = vmain(env);
  cgCallHelper(v, env, CallSpec::direct(opFunc), callDest(env, inst),
               SyncOptions::Sync, args);
}

void cgElemKeysetK(IRLS& env, const IRInstruction* inst) {
  auto const keyset = srcLoc(env, inst, 0).reg();
  auto const dst = dstLoc(env, inst, 0);
  auto const pos = inst->extra<ElemKeysetK>()->index;
  auto const off = SetArray::tvOff(pos);

  auto& v = vmain(env);
  assertx(dst.numAllocated() == 1);
  v << lea{keyset[off], dst.reg()};
}

void cgKeysetGet(IRLS& env, const IRInstruction* inst) {
  implKeysetGet(env, inst);
}
void cgKeysetGetQuiet(IRLS& env, const IRInstruction* inst) {
  implKeysetGet(env, inst);
}

void cgKeysetGetK(IRLS& env, const IRInstruction* inst) {
  auto const keyset = srcLoc(env, inst, 0).reg();
  auto const pos = inst->extra<KeysetGetK>()->index;
  auto const off = SetArray::tvOff(pos);

  auto& v = vmain(env);
  loadTV(v, inst->dst(0), dstLoc(env, inst, 0), keyset[off]);
}

void cgSetNewElemKeyset(IRLS& env, const IRInstruction* inst) {
  auto const key     = inst->src(1);
  BUILD_OPTAB(KEYSET_SETNEWELEM_HELPER_TABLE, getKeyType(key));

  auto args = argGroup(env, inst).ssa(0).ssa(1);

  auto& v = vmain(env);
  cgCallHelper(v, env, CallSpec::direct(opFunc), callDest(env, inst),
               SyncOptions::Sync, args);
}

void cgKeysetIsset(IRLS& env, const IRInstruction* inst) {
  implKeysetIsset(env, inst);
}
void cgKeysetEmptyElem(IRLS& env, const IRInstruction* inst) {
  implKeysetIsset(env, inst);
}

void cgKeysetIdx(IRLS& env, const IRInstruction* inst) {
  auto const key     = inst->src(1);
  auto const target  = getKeyType(key) == KeyType::Int
    ? CallSpec::direct(keysetIdxI)
    : CallSpec::direct(keysetIdxS);
  auto args = argGroup(env, inst).ssa(0).ssa(1).typedValue(2);
  auto& v = vmain(env);
  cgCallHelper(v, env, target, callDestTV(env, inst),
               SyncOptions::Sync, args);
}

///////////////////////////////////////////////////////////////////////////////
// Collections.

IMPL_OPCODE_CALL(PairIsset);
IMPL_OPCODE_CALL(VectorIsset);

void cgMapGet(IRLS& env, const IRInstruction* inst) {
  auto const target = inst->src(1)->isA(TInt)
    ? CallSpec::direct(MInstrHelpers::mapGetImpl<KeyType::Int>)
    : CallSpec::direct(MInstrHelpers::mapGetImpl<KeyType::Str>);

  auto const args = argGroup(env, inst).ssa(0).ssa(1);

  auto& v = vmain(env);
  cgCallHelper(v, env, target, callDestTV(env, inst),
               SyncOptions::Sync, args);
}

void cgMapSet(IRLS& env, const IRInstruction* inst) {
  auto const target = inst->src(1)->isA(TInt)
    ? CallSpec::direct(MInstrHelpers::mapSetImpl<KeyType::Int>)
    : CallSpec::direct(MInstrHelpers::mapSetImpl<KeyType::Str>);

  auto const args = argGroup(env, inst)
    .ssa(0)
    .ssa(1)
    .typedValue(2);

  auto& v = vmain(env);
  cgCallHelper(v, env, target, kVoidDest, SyncOptions::Sync, args);
}

void cgMapIsset(IRLS& env, const IRInstruction* inst) {
  auto const target = inst->src(1)->isA(TInt)
    ? CallSpec::direct(MInstrHelpers::mapIssetImpl<KeyType::Int>)
    : CallSpec::direct(MInstrHelpers::mapIssetImpl<KeyType::Str>);

  auto const args = argGroup(env, inst).ssa(0).ssa(1);

  auto& v = vmain(env);
  cgCallHelper(v, env, target, callDest(env, inst),
               SyncOptions::Sync, args);
}

///////////////////////////////////////////////////////////////////////////////

void cgMemoGet(IRLS& env, const IRInstruction* inst) {
  auto const extra = inst->extra<MemoGet>();
  auto const fp    = srcLoc(env, inst, 0).reg();
  auto& v          = vmain(env);

  auto const args = argGroup(env, inst)
    .ssa(1)
    .addr(fp, localOffset(extra->locals.first))
    .imm(extra->locals.restCount + 1);
  cgCallHelper(
    v, env, CallSpec::direct(MixedArray::MemoGet),
    callDestTV(env, inst), SyncOptions::None, args
  );
}

void cgMemoSet(IRLS& env, const IRInstruction* inst) {
  auto const extra = inst->extra<MemoSet>();
  auto const fp    = srcLoc(env, inst, 0).reg();
  auto& v          = vmain(env);

  auto const args = argGroup(env, inst)
    .ssa(1)
    .addr(fp, localOffset(extra->locals.first))
    .imm(extra->locals.restCount + 1)
    .typedValue(2);
  cgCallHelper(
    v, env, CallSpec::direct(MixedArray::MemoSet),
    kVoidDest, SyncOptions::Sync, args
  );
}

///////////////////////////////////////////////////////////////////////////////

}}}
