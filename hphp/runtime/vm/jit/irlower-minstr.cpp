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

#include "hphp/runtime/base/mixed-array.h"
#include "hphp/runtime/vm/member-operations.h"
#include "hphp/runtime/vm/unit.h"

#include "hphp/runtime/vm/jit/abi.h"
#include "hphp/runtime/vm/jit/arg-group.h"
#include "hphp/runtime/vm/jit/bc-marker.h"
#include "hphp/runtime/vm/jit/code-gen-cf.h"
#include "hphp/runtime/vm/jit/code-gen-helpers.h"
#include "hphp/runtime/vm/jit/code-gen-internal.h"
#include "hphp/runtime/vm/jit/ir-instruction.h"
#include "hphp/runtime/vm/jit/minstr-helpers.h"
#include "hphp/runtime/vm/jit/array-offset-profile.h"

#include "hphp/util/trace.h"

// This file does ugly things with macros so include last.
#include "hphp/runtime/vm/jit/irlower-minstr-internal.h"

namespace HPHP { namespace jit { namespace irlower {

TRACE_SET_MOD(irlower);

///////////////////////////////////////////////////////////////////////////////

void cgBaseG(IRLS& env, const IRInstruction* inst) {
  auto const flags = inst->extra<MOpFlagsData>()->flags;
  BUILD_OPTAB(BASE_G_HELPER_TABLE, flags);

  auto const args = argGroup(env, inst).typedValue(0);

  auto& v = vmain(env);
  cgCallHelper(v, env, CallSpec::direct(opFunc), callDest(env, inst),
               SyncOptions::Sync, args);
}

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
  auto const flags   = inst->extra<MOpFlagsData>()->flags;
  auto const base    = inst->src(0);
  auto const key     = inst->src(1);
  auto const keyType = getKeyTypeNoInt(key);
  BUILD_OPTAB(PROP_HELPER_TABLE, flags, keyType, base->isA(TObj));

  auto const args = propArgs(env, inst)
    .memberKeyS(1)
    .ssa(2);

  auto& v = vmain(env);
  cgCallHelper(v, env, CallSpec::direct(opFunc), callDest(env, inst),
               SyncOptions::Sync, args);
}

void implIssetEmptyProp(IRLS& env, const IRInstruction* inst) {
  auto const isEmpty = inst->op() == EmptyProp;
  auto const base = inst->src(0);
  auto const key = inst->src(1);
  auto const keyType = getKeyTypeNoInt(key);
  BUILD_OPTAB(ISSET_EMPTY_PROP_HELPER_TABLE, keyType, isEmpty, base->isA(TObj));

  auto const args = propArgs(env, inst).memberKeyS(1);

  auto& v = vmain(env);
  cgCallHelper(v, env, CallSpec::direct(opFunc), callDest(env, inst),
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
  auto const flags   = inst->extra<MOpFlagsData>()->flags;
  auto const base    = inst->src(0);
  auto const key     = inst->src(1);
  auto const keyType = getKeyTypeNoInt(key);
  BUILD_OPTAB(CGETPROP_HELPER_TABLE, keyType, base->isA(TObj), flags);

  auto const args = propArgs(env, inst).memberKeyS(1);

  auto& v = vmain(env);
  cgCallHelper(v, env, CallSpec::direct(opFunc), callDestTV(env, inst),
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
  BUILD_OPTAB(VGETPROP_HELPER_TABLE, keyType, base->isA(TObj));

  auto const args = propArgs(env, inst).memberKeyS(1);

  auto& v = vmain(env);
  cgCallHelper(v, env, CallSpec::direct(opFunc), callDest(env, inst),
               SyncOptions::Sync, args);
}

void cgBindProp(IRLS& env, const IRInstruction* inst) {
  auto const base = inst->src(0);
  BUILD_OPTAB(BINDPROP_HELPER_TABLE, base->isA(TObj));

  auto const args = propArgs(env, inst).typedValue(1).ssa(2);

  auto& v = vmain(env);
  cgCallHelper(v, env, CallSpec::direct(opFunc), callDest(env, inst),
               SyncOptions::Sync, args);
}

void cgSetProp(IRLS& env, const IRInstruction* inst) {
  auto const base = inst->src(0);
  auto const key = inst->src(1);
  auto const keyType = getKeyTypeNoInt(key);
  BUILD_OPTAB(SETPROP_HELPER_TABLE, keyType, base->isA(TObj));

  auto const args = propArgs(env, inst)
    .memberKeyS(1)
    .typedValue(2);

  auto& v = vmain(env);
  cgCallHelper(v, env, CallSpec::direct(opFunc),
               kVoidDest, SyncOptions::Sync, args);
}

void cgUnsetProp(IRLS& env, const IRInstruction* inst) {
  auto const base = inst->src(0);
  BUILD_OPTAB(UNSETPROP_HELPER_TABLE, base->isA(TObj));

  auto const args = propArgs(env, inst).typedValue(1);

  auto& v = vmain(env);
  cgCallHelper(v, env, CallSpec::direct(opFunc),
               kVoidDest, SyncOptions::Sync, args);
}

void cgSetOpProp(IRLS& env, const IRInstruction* inst) {
  auto const base = inst->src(0);
  auto const extra = inst->extra<SetOpProp>();
  BUILD_OPTAB(SETOPPROP_HELPER_TABLE, base->isA(TObj));

  auto const args = propArgs(env, inst)
    .typedValue(1)
    .typedValue(2)
    .imm(static_cast<int32_t>(extra->op));

  auto& v = vmain(env);
  cgCallHelper(v, env, CallSpec::direct(opFunc), callDestTV(env, inst),
               SyncOptions::Sync, args);
}

void cgIncDecProp(IRLS& env, const IRInstruction* inst) {
  auto const base = inst->src(0);
  auto const extra = inst->extra<IncDecProp>();
  BUILD_OPTAB(INCDECPROP_HELPER_TABLE, base->isA(TObj));

  auto const args = propArgs(env, inst)
    .typedValue(1)
    .imm(static_cast<int32_t>(extra->op));

  auto& v = vmain(env);
  cgCallHelper(v, env, CallSpec::direct(opFunc), callDestTV(env, inst),
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
  auto const flags = inst->extra<MOpFlagsData>()->flags;
  auto const key   = inst->src(1);
  BUILD_OPTAB(ELEM_HELPER_TABLE, getKeyType(key), flags);

  auto const args = elemArgs(env, inst).ssa(2);

  auto& v = vmain(env);
  cgCallHelper(v, env, CallSpec::direct(opFunc), callDest(env, inst),
               SyncOptions::Sync, args);
}

void implIssetEmptyElem(IRLS& env, const IRInstruction* inst) {
  auto const isEmpty = inst->op() == EmptyElem;
  auto const key     = inst->src(1);
  BUILD_OPTAB(ISSET_EMPTY_ELEM_HELPER_TABLE, getKeyType(key), isEmpty);

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
  auto const flags = inst->extra<MOpFlagsData>()->flags;
  auto const key   = inst->src(1);
  BUILD_OPTAB(CGETELEM_HELPER_TABLE, getKeyType(key), flags);

  auto& v = vmain(env);
  cgCallHelper(v, env, CallSpec::direct(opFunc), callDestTV(env, inst),
               SyncOptions::Sync, elemArgs(env, inst));
}

void cgVGetElem(IRLS& env, const IRInstruction* inst) {
  auto const key = inst->src(1);
  BUILD_OPTAB(VGETELEM_HELPER_TABLE, getKeyType(key));

  auto& v = vmain(env);
  cgCallHelper(v, env, CallSpec::direct(opFunc), callDest(env, inst),
               SyncOptions::Sync, elemArgs(env, inst));
}

void cgSetElem(IRLS& env, const IRInstruction* inst) {
  auto const key = inst->src(1);
  BUILD_OPTAB(SETELEM_HELPER_TABLE, getKeyType(key));

  auto& v = vmain(env);
  cgCallHelper(v, env, CallSpec::direct(opFunc), callDest(env, inst),
               SyncOptions::Sync, elemArgs(env, inst).typedValue(2));
}

void cgUnsetElem(IRLS& env, const IRInstruction* inst) {
  auto const key = inst->src(1);
  BUILD_OPTAB(UNSET_ELEM_HELPER_TABLE, getKeyType(key));

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

void implElemArray(IRLS& env, const IRInstruction* inst) {
  auto const arr = inst->src(0);
  auto const key = inst->src(1);
  auto const flags = inst->op() == ElemArrayW ? MOpFlags::Warn : MOpFlags::None;
  auto const keyInfo = checkStrictlyInteger(arr->type(), key->type());
  BUILD_OPTAB(ELEM_ARRAY_HELPER_TABLE,
              keyInfo.type, keyInfo.checkForInt, flags);

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
              setRef);

  auto args = arrArgs(env, inst, keyInfo);
  args.typedValue(2);
  if (setRef) args.ssa(3);

  auto& v = vmain(env);
  cgCallHelper(v, env, CallSpec::direct(opFunc), callDest(env, inst),
               SyncOptions::Sync, args);
}

void implVecSet(IRLS& env, const IRInstruction* inst) {
  bool const setRef  = inst->op() == VecSetRef;
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

void implElemDict(IRLS& env, const IRInstruction* inst) {
  auto const key = inst->src(1);
  auto const flags = inst->op() == ElemDictW ? MOpFlags::Warn : MOpFlags::None;
  BUILD_OPTAB(ELEM_DICT_HELPER_TABLE, getKeyType(key), flags);

  auto args = argGroup(env, inst).ssa(0).ssa(1);

  auto& v = vmain(env);
  cgCallHelper(v, env, CallSpec::direct(opFunc), callDest(env, inst),
               SyncOptions::Sync, args);
}

void implDictGet(IRLS& env, const IRInstruction* inst) {
  auto const key = inst->src(1);
  auto const flags =
    (inst->op() == DictGetQuiet) ? MOpFlags::None : MOpFlags::Warn;
  BUILD_OPTAB(DICTGET_HELPER_TABLE, getKeyType(key), flags);

  auto args = argGroup(env, inst).ssa(0).ssa(1);

  auto& v = vmain(env);
  cgCallHelper(v, env, CallSpec::direct(opFunc), callDestTV(env, inst),
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

void implKeysetGet(IRLS& env, const IRInstruction* inst) {
  auto const key = inst->src(1);
  auto const flags =
    (inst->op() == KeysetGetQuiet) ? MOpFlags::None : MOpFlags::Warn;
  BUILD_OPTAB(KEYSETGET_HELPER_TABLE, getKeyType(key), flags);

  auto args = argGroup(env, inst).ssa(0).ssa(1);

  auto& v = vmain(env);
  cgCallHelper(v, env, CallSpec::direct(opFunc), callDestTV(env, inst),
               SyncOptions::Sync, args);
}

void implElemKeyset(IRLS& env, const IRInstruction* inst) {
  auto const key = inst->src(1);
  auto const flags = inst->op() == ElemKeysetW ? MOpFlags::Warn : MOpFlags::None;
  BUILD_OPTAB(ELEM_KEYSET_HELPER_TABLE, getKeyType(key), flags);

  auto args = argGroup(env, inst).ssa(0).ssa(1);

  auto& v = vmain(env);
  cgCallHelper(v, env, CallSpec::direct(opFunc), callDest(env, inst),
               SyncOptions::Sync, args);
}

void implKeysetIsset(IRLS& env, const IRInstruction* inst) {
  auto const key     = inst->src(1);
  auto const empty   = inst->op() == KeysetEmptyElem;
  BUILD_OPTAB(KEYSET_ISSET_EMPTY_ELEM_HELPER_TABLE, getKeyType(key), empty);

  auto args = argGroup(env, inst).ssa(0).ssa(1);

  auto& v = vmain(env);
  cgCallHelper(v, env, CallSpec::direct(opFunc), callDest(env, inst),
               SyncOptions::Sync, args);
}

}

///////////////////////////////////////////////////////////////////////////////

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

void cgCheckMixedArrayOffset(IRLS& env, const IRInstruction* inst) {
  auto const arr = srcLoc(env, inst, 0).reg();
  auto const key = srcLoc(env, inst, 1).reg();
  auto const branch = label(env, inst->taken());
  auto const pos = inst->extra<CheckMixedArrayOffset>()->index;
  auto& v = vmain(env);

  { // Also fail if our predicted position exceeds bounds.
    auto const sf = v.makeReg();
    v << cmplim{safe_cast<int32_t>(pos), arr[MixedArray::usedOff()], sf};
    ifThen(v, CC_LE, sf, branch);
  }
  { // Fail if the Elm key value doesn't match.
    auto const sf = v.makeReg();
    v << cmpqm{key, arr[MixedArray::elmOff(pos) + MixedArray::Elm::keyOff()], sf};
    ifThen(v, CC_NE, sf, branch);
  }
  auto const dataOff = MixedArray::elmOff(pos) + MixedArray::Elm::dataOff();

  { // Fail if the Elm key type doesn't match.
    auto const sf = v.makeReg();
    v << cmplim{0, arr[dataOff + TVOFF(m_aux)], sf};

    auto const key_info = checkStrictlyInteger(
      inst->src(0)->type(), // arr
      inst->src(1)->type()  // key
    );
    assertx(key_info.type != KeyType::Any);

    // Note that if `key' actually is an integer-ish string, we'd fail this
    // check (and most likely would have failed the previous check also), but
    // this false negative is allowed.
    auto const is_str_key = key_info.type == KeyType::Str;
    ifThen(v, is_str_key ? CC_L : CC_GE, sf, branch);
  }
  { // Fail if the Elm is a tombstone.  See MixedArray::isTombstone().
    auto const sf = v.makeReg();
    v << cmpbim{KindOfUninit, arr[dataOff + TVOFF(m_type)], sf};
    ifThen(v, CC_L, sf, branch);
  }
}

void cgCheckArrayCOW(IRLS& env, const IRInstruction* inst) {
  auto const arr = srcLoc(env, inst, 0).reg();
  auto& v = vmain(env);

  auto const sf = v.makeReg();
  v << cmplim{1, arr[FAST_REFCOUNT_OFFSET], sf};
  ifThen(v, CC_NE, sf, label(env, inst->taken()));
}

///////////////////////////////////////////////////////////////////////////////

void cgProfileDictOffset(IRLS& env, const IRInstruction* inst) {
  auto const key = inst->src(1);

  BUILD_OPTAB(PROFILE_DICT_OFFSET_HELPER_TABLE, getKeyType(key));
  auto& v = vmain(env);

  auto const rprof = v.makeReg();
  v << lea{rvmtl()[inst->extra<ProfileDictOffset>()->handle], rprof};

  auto args = argGroup(env, inst).ssa(0).ssa(1).reg(rprof);

  cgCallHelper(v, env, CallSpec::direct(opFunc), kVoidDest, SyncOptions::Sync,
               args);
}

void cgCheckDictOffset(IRLS& env, const IRInstruction* inst) {
  auto const dict = srcLoc(env, inst, 0).reg();
  auto const key = srcLoc(env, inst, 1).reg();
  auto const branch = label(env, inst->taken());
  auto const pos = inst->extra<CheckDictOffset>()->index;
  auto& v = vmain(env);

  { // Also fail if our predicted position exceeds bounds.
    auto const sf = v.makeReg();
    v << cmplim{safe_cast<int32_t>(pos), dict[MixedArray::usedOff()], sf};
    ifThen(v, CC_LE, sf, branch);
  }
  { // Fail if the Elm key value doesn't match.
    auto const sf = v.makeReg();
    v << cmpqm{key, dict[MixedArray::elmOff(pos) + MixedArray::Elm::keyOff()], sf};
    ifThen(v, CC_NE, sf, branch);
  }
  auto const dataOff = MixedArray::elmOff(pos) + MixedArray::Elm::dataOff();

  { // Fail if the Elm key type doesn't match.
    auto const sf = v.makeReg();
    v << cmplim{0, dict[dataOff + TVOFF(m_aux)], sf};

    auto const key_type = getKeyType(inst->src(1));
    assertx(key_type != KeyType::Any);

    // Note that if `key' actually is an integer-ish string, we'd fail this
    // check (and most likely would have failed the previous check also), but
    // this false negative is allowed.
    auto const is_str_key = key_type == KeyType::Str;
    ifThen(v, is_str_key ? CC_L : CC_GE, sf, branch);
  }
  { // Fail if the Elm is a tombstone.  See MixedArray::isTombstone().
    auto const sf = v.makeReg();
    v << cmpbim{KindOfUninit, dict[dataOff + TVOFF(m_type)], sf};
    ifThen(v, CC_L, sf, branch);
  }
}

void cgProfileKeysetOffset(IRLS& env, const IRInstruction* inst) {
  auto const key = inst->src(1);

  BUILD_OPTAB(PROFILE_KEYSET_OFFSET_HELPER_TABLE, getKeyType(key));
  auto& v = vmain(env);

  auto const rprof = v.makeReg();
  v << lea{rvmtl()[inst->extra<ProfileKeysetOffset>()->handle], rprof};

  auto args = argGroup(env, inst).ssa(0).ssa(1).reg(rprof);

  cgCallHelper(v, env, CallSpec::direct(opFunc), kVoidDest, SyncOptions::Sync,
               args);
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

///////////////////////////////////////////////////////////////////////////////

void cgElemArray(IRLS& env, const IRInstruction* i)  { implElemArray(env, i); }
void cgElemArrayW(IRLS& env, const IRInstruction* i) { implElemArray(env, i); }

void cgElemArrayD(IRLS& env, const IRInstruction* inst) {
  auto const key     = inst->src(1);
  auto const keyInfo = checkStrictlyInteger(inst->typeParam(), key->type());
  BUILD_OPTAB(ELEM_ARRAY_D_HELPER_TABLE, keyInfo.type);

  auto& v = vmain(env);
  cgCallHelper(v, env, CallSpec::direct(opFunc), callDest(env, inst),
               SyncOptions::Sync, arrArgs(env, inst, keyInfo));
}

void cgElemArrayU(IRLS& env, const IRInstruction* inst) {
  auto const key     = inst->src(1);
  auto const keyInfo = checkStrictlyInteger(inst->typeParam(), key->type());
  BUILD_OPTAB(ELEM_ARRAY_U_HELPER_TABLE, keyInfo.type);

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
  BUILD_OPTAB(ARRAYGET_HELPER_TABLE, keyInfo.type, keyInfo.checkForInt);

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

void cgArrayIsset(IRLS& env, const IRInstruction* inst) {
  auto const arr     = inst->src(0);
  auto const key     = inst->src(1);
  auto const keyInfo = checkStrictlyInteger(arr->type(), key->type());
  BUILD_OPTAB(ARRAY_ISSET_HELPER_TABLE, keyInfo.type, keyInfo.checkForInt);

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
      return CallSpec::direct(arrayIdxSi);
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

/////////////////////////////////////////////////////////////////////////////

void cgVecSet(IRLS& env, const IRInstruction* i)    { implVecSet(env, i); }
void cgVecSetRef(IRLS& env, const IRInstruction* i) { implVecSet(env, i); }

//////////////////////////////////////////////////////////////////////////////

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

////////////////////////////////////////////////////////////////////////////////

void cgElemKeyset(IRLS& env, const IRInstruction* i)  { implElemKeyset(env, i); }
void cgElemKeysetW(IRLS& env, const IRInstruction* i) { implElemKeyset(env, i); }

void cgElemKeysetU(IRLS& env, const IRInstruction* inst) {
  auto const key     = inst->src(1);
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

}}}
