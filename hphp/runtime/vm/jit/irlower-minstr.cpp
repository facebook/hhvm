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
#include "hphp/runtime/base/packed-array-defs.h"
#include "hphp/runtime/base/rds.h"
#include "hphp/runtime/base/typed-value.h"
#include "hphp/runtime/vm/member-operations.h"
#include "hphp/runtime/vm/unit.h"

#include "hphp/runtime/vm/jit/abi.h"
#include "hphp/runtime/vm/jit/arg-group.h"
#include "hphp/runtime/vm/jit/array-access-profile.h"
#include "hphp/runtime/vm/jit/bc-marker.h"
#include "hphp/runtime/vm/jit/code-gen-cf.h"
#include "hphp/runtime/vm/jit/code-gen-helpers.h"
#include "hphp/runtime/vm/jit/ir-instruction.h"
#include "hphp/runtime/vm/jit/minstr-helpers.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/jit/translator-runtime.h"

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
  cgCallHelper(v, env, target, callDest(env, inst), SyncOptions::Sync, args);
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

  auto args = propArgs(env, inst)
    .memberKeyS(1)
    .ssa(2);

  auto const target = [&] {
    if (inst->is(PropDX)) {
      BUILD_OPTAB2(base->isA(TObj),
                   PROPD_OBJ_HELPER_TABLE,
                   PROPD_HELPER_TABLE,
                   keyType);
      args.ssa(3);
      return target;
    } else {
      BUILD_OPTAB2(base->isA(TObj),
                   PROP_OBJ_HELPER_TABLE,
                   PROP_HELPER_TABLE,
                   mode, keyType);
      return target;
    }
  }();

  auto& v = vmain(env);
  cgCallHelper(v, env, target, callDest(env, inst), SyncOptions::Sync, args);
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

  BUILD_OPTAB2(base->isA(TObj),
               CGET_OBJ_PROP_HELPER_TABLE,
               CGET_PROP_HELPER_TABLE,
               keyType, mode);

  auto const args = propArgs(env, inst).memberKeyS(1);

  auto& v = vmain(env);
  cgCallHelper(v, env, target, callDestTV(env, inst), SyncOptions::Sync, args);
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

void cgSetProp(IRLS& env, const IRInstruction* inst) {
  auto const base = inst->src(0);
  auto const key = inst->src(1);
  auto const keyType = getKeyTypeNoInt(key);

  auto args = propArgs(env, inst)
    .memberKeyS(1)
    .typedValue(2);

  auto const target = [&] {
    if (base->isA(TObj)) {
      BUILD_OPTAB(SETPROP_OBJ_HELPER_TABLE, keyType);
      return target;
    } else {
      BUILD_OPTAB(SETPROP_HELPER_TABLE, keyType);
      args.ssa(3);
      return target;
    }
  }();

  auto& v = vmain(env);
  cgCallHelper(v, env, target, kVoidDest, SyncOptions::Sync, args);
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

  auto args = propArgs(env, inst)
    .typedValue(1)
    .typedValue(2)
    .imm(static_cast<int32_t>(extra->op));
  if (!base->isA(TObj)) args.ssa(3);

  auto& v = vmain(env);
  cgCallHelper(v, env, helper, callDestTV(env, inst), SyncOptions::Sync, args);
}

void cgIncDecProp(IRLS& env, const IRInstruction* inst) {
  auto const base = inst->src(0);
  auto const extra = inst->extra<IncDecProp>();

  auto helper = base->isA(TObj)
    ? CallSpec::direct(MInstrHelpers::incDecPropCO)
    : CallSpec::direct(MInstrHelpers::incDecPropC);

  auto args = propArgs(env, inst)
    .typedValue(1)
    .imm(static_cast<int32_t>(extra->op));
  if (!base->isA(TObj)) args.ssa(2);

  auto& v = vmain(env);
  cgCallHelper(v, env, helper, callDestTV(env, inst),
               SyncOptions::Sync, args);
}

void cgIssetProp(IRLS& env, const IRInstruction* inst) {
  auto const base = inst->src(0);
  auto const key = inst->src(1);
  auto const keyType = getKeyTypeNoInt(key);

  BUILD_OPTAB2(base->isA(TObj),
               ISSET_OBJ_PROP_HELPER_TABLE,
               ISSET_PROP_HELPER_TABLE,
               keyType);

  auto const args = propArgs(env, inst).memberKeyS(1);

  auto& v = vmain(env);
  cgCallHelper(v, env, target, callDest(env, inst), SyncOptions::Sync, args);
}

IMPL_OPCODE_CALL(ProfileProp);

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

  auto args = elemArgs(env, inst).ssa(2);

  auto const target = [&] {
    if (inst->is(ElemDX)) {
      assertx(mode == MOpMode::Define);
      BUILD_OPTAB(ELEMD_HELPER_TABLE,
                  getKeyType(key),
                  RuntimeOption::EvalArrayProvenance);
      args.ssa(3);
      return target;
    } else {
      BUILD_OPTAB(ELEM_HELPER_TABLE, getKeyType(key), mode);
      return target;
    }
  }();

  auto& v = vmain(env);
  cgCallHelper(v, env, target, callDest(env, inst), SyncOptions::Sync, args);
}

}

///////////////////////////////////////////////////////////////////////////////

void cgElemX(IRLS& env, const IRInstruction* i)  { implElem(env, i); }
void cgElemDX(IRLS& env, const IRInstruction* i) { implElem(env, i); }
void cgElemUX(IRLS& env, const IRInstruction* i) { implElem(env, i); }

void cgCGetElem(IRLS& env, const IRInstruction* inst) {
  auto const mode  = inst->extra<MOpModeData>()->mode;
  auto const key   = inst->src(1);
  BUILD_OPTAB(CGETELEM_HELPER_TABLE, getKeyType(key), mode);

  auto& v = vmain(env);
  cgCallHelper(v, env, target, callDestTV(env, inst),
               SyncOptions::Sync, elemArgs(env, inst));
}

void cgSetElem(IRLS& env, const IRInstruction* inst) {
  auto const key = inst->src(1);
  BUILD_OPTAB(SETELEM_HELPER_TABLE,
              getKeyType(key),
              RuntimeOption::EvalArrayProvenance);

  auto& v = vmain(env);
  cgCallHelper(v, env, target, callDest(env, inst),
               SyncOptions::Sync, elemArgs(env, inst).typedValue(2).ssa(3));
}

void cgSetRange(IRLS& env, const IRInstruction* inst) {
  auto const target = inst->is(SetRangeRev) ?
    CallSpec::direct(HPHP::SetRange<true>) :
    CallSpec::direct(HPHP::SetRange<false>);
  cgCallHelper(
    vmain(env), env, target, callDest(env, inst),
    SyncOptions::Sync,
    argGroup(env, inst).ssa(0).ssa(1).typedValue(2).ssa(3).ssa(4)
  );
}

void cgSetRangeRev(IRLS& env, const IRInstruction* inst) {
  cgSetRange(env, inst);
}

void cgSetNewElem(IRLS& env, const IRInstruction* inst) {
  auto const target = RuntimeOption::EvalArrayProvenance
    ? CallSpec::direct(MInstrHelpers::setNewElem<true>)
    : CallSpec::direct(MInstrHelpers::setNewElem<false>);

  auto args = argGroup(env, inst)
    .ssa(0)
    .typedValue(1)
    .ssa(2);

  auto& v = vmain(env);
  cgCallHelper(v, env, target, callDest(env, inst), SyncOptions::Sync, args);
}

void cgSetOpElem(IRLS& env, const IRInstruction* inst) {
  auto& v = vmain(env);

  auto const target = CallSpec::direct(MInstrHelpers::setOpElem);

  auto const args = argGroup(env, inst)
    .ssa(0)
    .typedValue(1)
    .typedValue(2)
    .imm(uint32_t(inst->extra<SetOpElem>()->op))
    .ssa(3);

  cgCallHelper(v, env, target, callDestTV(env, inst),
               SyncOptions::Sync, args);
}

void cgIncDecElem(IRLS& env, const IRInstruction* inst) {
  auto& v = vmain(env);

  auto const target = CallSpec::direct(MInstrHelpers::incDecElem);

  auto const args = argGroup(env, inst)
    .ssa(0)
    .typedValue(1)
    .imm(uint32_t(inst->extra<IncDecElem>()->op))
    .ssa(2);

  cgCallHelper(v, env, target, callDestTV(env, inst),
               SyncOptions::Sync, args);
}

void cgUnsetElem(IRLS& env, const IRInstruction* inst) {
  auto const key = inst->src(1);
  BUILD_OPTAB(UNSET_ELEM_HELPER_TABLE, getKeyType(key));

  auto& v = vmain(env);
  cgCallHelper(v, env, target, kVoidDest,
               SyncOptions::Sync, elemArgs(env, inst));
}

void cgIssetElem(IRLS& env, const IRInstruction* inst) {
  auto const key     = inst->src(1);
  BUILD_OPTAB(ISSET_ELEM_HELPER_TABLE, getKeyType(key));

  auto& v = vmain(env);
  cgCallHelper(v, env, target, callDest(env, inst),
               SyncOptions::Sync, elemArgs(env, inst));
}

///////////////////////////////////////////////////////////////////////////////
// Offset profiling.

namespace {

void implProfileHackArrayAccess(IRLS& env, const IRInstruction* inst,
                                const CallSpec& target) {
  auto& v = vmain(env);

  auto const extra = inst->extra<ArrayAccessProfileData>();
  auto args = argGroup(env, inst).ssa(0).ssa(1)
              .addr(rvmtl(), safe_cast<int32_t>(extra->handle))
              .imm(extra->cowCheck);

  cgCallHelper(v, env, target, kVoidDest, SyncOptions::Sync, args);
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
  auto const is_str_key = key_type == KeyType::Str;
  { // Fail if the Elm key type doesn't match.
    auto const sf = v.makeReg();
    v << cmplim{0, arr[elmOff + Elm::dataOff() + TVOFF(m_aux)], sf};

    assertx(key_type != KeyType::Any);

    // Note that if `key' actually is an integer-ish string, we'd fail this
    // check (and most likely would have failed the previous check also), but
    // this false negative is allowed.
    ifThen(v, is_str_key ? CC_L : CC_GE, sf, branch);
  }
  { // Fail if the Elm is a tombstone.  See MixedArray::isTombstone().
    // We set the key type to look like an int when setting it be a tombstone.
    // This is originally to simplify the type scan, however we can use this
    // to elide this check when looking for string keys.
    if (!is_str_key) {
      auto const sf = v.makeReg();
      v << cmpbim{static_cast<data_type_t>(kInvalidDataType),
                  arr[elmOff + Elm::dataOff() + TVOFF(m_type)], sf};
      ifThen(v, CC_E, sf, branch);
    }
  }
}

}

void cgProfileMixedArrayAccess(IRLS& env, const IRInstruction* inst) {
  BUILD_OPTAB(PROFILE_MIXED_ARRAY_ACCESS_HELPER_TABLE,
              getKeyType(inst->src(1)));
  implProfileHackArrayAccess(env, inst, target);
}

void cgProfileDictAccess(IRLS& env, const IRInstruction* inst) {
  BUILD_OPTAB(PROFILE_DICT_ACCESS_HELPER_TABLE, getKeyType(inst->src(1)));
  implProfileHackArrayAccess(env, inst, target);
}

void cgProfileKeysetAccess(IRLS& env, const IRInstruction* inst) {
  BUILD_OPTAB(PROFILE_KEYSET_ACCESS_HELPER_TABLE, getKeyType(inst->src(1)));
  implProfileHackArrayAccess(env, inst, target);
}

void cgCheckMixedArrayOffset(IRLS& env, const IRInstruction* inst) {
  implCheckMixedArrayLikeOffset(env, inst, getKeyType(inst->src(1)));
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
    v << cmpbim{static_cast<data_type_t>(kInvalidDataType),
                keyset[tvOff + TVOFF(m_type)], sf};
    ifThen(v, CC_E, sf, branch);
  }
}

void cgCheckArrayCOW(IRLS& env, const IRInstruction* inst) {
  auto const arr = srcLoc(env, inst, 0).reg();
  auto& v = vmain(env);

  auto const sf = emitCmpRefCount(v, OneReference, arr);
  ifThen(v, CC_NE, sf, label(env, inst->taken()));
}

void cgCheckMixedArrayKeys(IRLS& env, const IRInstruction* inst) {
  auto const src = srcLoc(env, inst, 0).reg();
  auto const mask = MixedArrayKeys::getMask(inst->typeParam());
  always_assert_flog(mask, "Invalid MixedArray key check: {}",
                     inst->typeParam().toString());

  auto& v = vmain(env);
  auto const sf = v.makeReg();
  v << testbim{int8_t(*mask), src[MixedArray::kKeyTypesOffset], sf};
  v << jcc{CC_NZ, sf, {label(env, inst->next()), label(env, inst->taken())}};
}

///////////////////////////////////////////////////////////////////////////////
// Array.

namespace {

void implArrayGet(IRLS& env, const IRInstruction* inst, MOpMode mode) {
  if (inst->src(0)->isA(TMixedArr)) {
    if (mode == MOpMode::None) return cgDictGetQuiet(env, inst);
    if (mode == MOpMode::Warn) return cgDictGet(env, inst);
  }
  BUILD_OPTAB(ARRAYGET_HELPER_TABLE, getKeyType(inst->src(1)), mode);

  auto& v = vmain(env);
  cgCallHelper(v, env, target, callDestTV(env, inst),
               SyncOptions::Sync, argGroup(env, inst).ssa(0).ssa(1));
}

void implArraySet(IRLS& env, const IRInstruction* inst) {
  auto const& arr_type = inst->src(0)->type();
  auto const& key_type = inst->src(1)->type();

  using SetIntMove = ArrayData* (ArrayData::*)(int64_t, TypedValue);
  using SetStrMove = ArrayData* (ArrayData::*)(StringData*, TypedValue);

  assertx(key_type.subtypeOfAny(TInt, TStr));
  auto const target = (key_type <= TInt)
    ? CallSpec::array(arr_type, &g_array_funcs.setIntMove,
                      static_cast<SetIntMove>(&ArrayData::setMove))
    : CallSpec::array(arr_type, &g_array_funcs.setStrMove,
                      static_cast<SetStrMove>(&ArrayData::setMove));

  auto& v = vmain(env);
  auto const args = argGroup(env, inst).ssa(0).ssa(1).typedValue(2);
  cgCallHelper(v, env, target, callDest(env, inst), SyncOptions::Sync, args);
}

}

void cgElemArrayX(IRLS& env, const IRInstruction* inst) {
  auto const mode = inst->extra<ElemArrayX>()->mode;
  BUILD_OPTAB(ELEM_ARRAY_HELPER_TABLE, getKeyType(inst->src(1)), mode);

  auto& v = vmain(env);
  cgCallHelper(v, env, target, callDest(env, inst),
               SyncOptions::Sync, argGroup(env, inst).ssa(0).ssa(1));
}

void cgElemArrayD(IRLS& env, const IRInstruction* inst) {
  BUILD_OPTAB(ELEM_ARRAY_D_HELPER_TABLE, getKeyType(inst->src(1)));

  auto& v = vmain(env);
  cgCallHelper(v, env, target, callDest(env, inst),
               SyncOptions::Sync, argGroup(env, inst).ssa(0).ssa(1));
}

void cgElemArrayU(IRLS& env, const IRInstruction* inst) {
  BUILD_OPTAB(ELEM_ARRAY_U_HELPER_TABLE, getKeyType(inst->src(1)));

  auto& v = vmain(env);
  cgCallHelper(v, env, target, callDest(env, inst),
               SyncOptions::Sync, argGroup(env, inst).ssa(0).ssa(1));
}

void cgElemMixedArrayK(IRLS& env, const IRInstruction* inst) {
  auto const arr = srcLoc(env, inst, 0).reg();
  auto const dst = dstLoc(env, inst, 0);
  auto const pos = inst->extra<ElemMixedArrayK>()->index;
  auto const off = MixedArray::elmOff(pos) + MixedArray::Elm::dataOff();

  auto& v = vmain(env);

  v << lea{arr[off], dst.reg(tv_lval::val_idx)};
  if (wide_tv_val) {
    static_assert(TVOFF(m_data) == 0, "");
    v << lea{arr[off + TVOFF(m_type)], dst.reg(tv_lval::type_idx)};
  }
}

void cgArrayGet(IRLS& env, const IRInstruction* inst) {
  return implArrayGet(env, inst, inst->extra<ArrayGet>()->mode);
}

///////////////////////////////////////////////////////////////////////////////

void cgGetMixedPtrIter(IRLS& env, const IRInstruction* inst) {
  auto const pos_tmp = inst->src(1);
  auto const arr = srcLoc(env, inst, 0).reg();
  auto const pos = srcLoc(env, inst, 1).reg();
  auto const dst = dstLoc(env, inst, 0).reg();

  auto& v = vmain(env);
  if (pos_tmp->hasConstVal(TInt)) {
    auto const offset = MixedArray::elmOff(pos_tmp->intVal());
    if (deltaFits(offset, sz::dword)) {
      v << addqi{safe_cast<int32_t>(offset), arr, dst, v.makeReg()};
      return;
    }
  }

  auto const px3 = v.makeReg();
  v << lea{pos[pos * 2], px3};
  v << lea{arr[px3 * 8 + MixedArray::dataOff()], dst};
}

void cgGetPackedPtrIter(IRLS& env, const IRInstruction* inst) {
  auto const pos_tmp = inst->src(1);
  auto const arr = srcLoc(env, inst, 0).reg();
  auto const pos = srcLoc(env, inst, 1).reg();
  auto const dst = dstLoc(env, inst, 0).reg();

  auto& v = vmain(env);
  if (pos_tmp->hasConstVal(TInt)) {
    auto const n = pos_tmp->intVal();
    auto const offset = PackedArray::entriesOffset() + n * sizeof(TypedValue);
    if (deltaFits(offset, sz::dword)) {
      v << addqi{safe_cast<int32_t>(offset), arr, dst, v.makeReg()};
      return;
    }
  }

  auto const px2 = v.makeReg();
  v << shlqi{1, pos, px2, v.makeReg()};
  v << lea{arr[px2 * 8 + PackedArray::entriesOffset()], dst};
}

void cgAdvanceMixedPtrIter(IRLS& env, const IRInstruction* inst) {
  auto const src = srcLoc(env, inst, 0).reg();
  auto const dst = dstLoc(env, inst, 0).reg();

  auto& v = vmain(env);
  auto const extra = inst->extra<AdvanceMixedPtrIter>();
  auto const delta = extra->offset * int32_t(sizeof(MixedArrayElm));
  v << addqi{delta, src, dst, v.makeReg()};
}

void cgAdvancePackedPtrIter(IRLS& env, const IRInstruction* inst) {
  auto const src = srcLoc(env, inst, 0).reg();
  auto const dst = dstLoc(env, inst, 0).reg();

  auto& v = vmain(env);
  auto const extra = inst->extra<AdvancePackedPtrIter>();
  auto const delta = extra->offset * int32_t(sizeof(TypedValue));
  v << addqi{delta, src, dst, v.makeReg()};
}

void cgLdPtrIterKey(IRLS& env, const IRInstruction* inst) {
  static_assert(sizeof(MixedArrayElm::hash_t) == 4, "");
  auto const elm = srcLoc(env, inst, 0).reg();
  auto const dst = dstLoc(env, inst, 0);

  auto& v = vmain(env);
  if (inst->dst(0)->type().needsReg()) {
    assertx(dst.hasReg(1));
    auto const sf = v.makeReg();
    v << cmplim{0, elm[MixedArrayElm::hashOff()], sf};
    v << cmovb{CC_L, sf, v.cns(KindOfString), v.cns(KindOfInt64), dst.reg(1)};
  }
  v << load{elm[MixedArrayElm::keyOff()], dst.reg(0)};
}

void cgLdPtrIterVal(IRLS& env, const IRInstruction* inst) {
  static_assert(MixedArrayElm::dataOff() == 0, "");
  auto const elm = srcLoc(env, inst, 0).reg();
  loadTV(vmain(env), inst->dst(0), dstLoc(env, inst, 0), elm[0]);
}

void cgEqPtrIter(IRLS& env, const IRInstruction* inst) {
  auto const s0 = srcLoc(env, inst, 0).reg();
  auto const s1 = srcLoc(env, inst, 1).reg();
  auto const d  = dstLoc(env, inst, 0).reg();

  auto& v = vmain(env);
  auto const sf = v.makeReg();
  v << cmpq{s1, s0, sf};
  v << setcc{CC_E, sf, d};
}

///////////////////////////////////////////////////////////////////////////////

void cgMixedArrayGetK(IRLS& env, const IRInstruction* inst) {
  auto const arr = srcLoc(env, inst, 0).reg();
  auto const pos = inst->extra<MixedArrayGetK>()->index;
  auto const off = MixedArray::elmOff(pos) + MixedArray::Elm::dataOff();

  auto& v = vmain(env);
  loadTV(v, inst->dst(0), dstLoc(env, inst, 0), arr[off]);
}

void cgArraySet(IRLS& env, const IRInstruction* i)    { implArraySet(env, i); }

IMPL_OPCODE_CALL(SetNewElemArray);
IMPL_OPCODE_CALL(AddNewElem);

static ArrayData* addNewElemKeysetImpl(ArrayData* keyset, TypedValue v) {
  assertx(keyset->isKeysetKind());
  auto out = SetArray::Append(keyset, v);
  if (keyset != out) decRefArr(keyset);
  return out;
}

static ArrayData* addNewElemVecImpl(ArrayData* vec, TypedValue v) {
  assertx(vec->isVecArrayKind());
  auto out = PackedArray::AppendVec(vec, v);
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

void cgArrayIsset(IRLS& env, const IRInstruction* inst) {
  BUILD_OPTAB(ARRAY_ISSET_HELPER_TABLE, getKeyType(inst->src(1)));

  auto& v = vmain(env);
  cgCallHelper(v, env, target, callDest(env, inst),
               SyncOptions::Sync, argGroup(env, inst).ssa(0).ssa(1));
}

void cgArrayIdx(IRLS& env, const IRInstruction* inst) {
  auto const arr = inst->src(0);
  auto const def = inst->src(2);
  if (arr->isA(TMixedArr)) return cgDictIdx(env, inst);
  if (def->isA(TInitNull)) return implArrayGet(env, inst, MOpMode::None);
  auto const keyType = getKeyType(inst->src(1));

  auto const target = [&] () -> CallSpec {
    if (keyType == KeyType::Int) return CallSpec::direct(arrayIdxI);
    assertx(keyType == KeyType::Str);
    auto const scan =
      inst->extra<SizeHintData>()->hint == SizeHintData::SmallStatic &&
      inst->src(1)->isA(TStaticStr);
    return CallSpec::direct(scan ? arrayIdxScan : arrayIdxS);
  }();

  auto& v = vmain(env);
  cgCallHelper(v, env, target, callDestTV(env, inst), SyncOptions::Sync,
               argGroup(env, inst).ssa(0).ssa(1).typedValue(2));
}

template <TypedValue (*f)(ArrayData*)>
void containerFirstLastHelper(IRLS& env, const IRInstruction* inst) {
  auto& v = vmain(env);
  cgCallHelper(v, env, CallSpec::direct(f),
              callDestTV(env, inst), SyncOptions::None,
              argGroup(env, inst).ssa(0));
}

void cgVecFirst(IRLS& env, const IRInstruction* inst) {
  containerFirstLastHelper<vecFirstLast<true>>(env, inst);
}

void cgVecLast(IRLS& env, const IRInstruction* inst) {
  containerFirstLastHelper<vecFirstLast<false>>(env, inst);
}

void cgDictFirst(IRLS& env, const IRInstruction* inst) {
  containerFirstLastHelper<arrFirstLast<true, false>>(env, inst);
}

void cgDictLast(IRLS& env, const IRInstruction* inst) {
  containerFirstLastHelper<arrFirstLast<false, false>>(env, inst);
}

void cgDictFirstKey(IRLS& env, const IRInstruction* inst) {
  containerFirstLastHelper<arrFirstLast<true, true>>(env, inst);
}

void cgDictLastKey(IRLS& env, const IRInstruction* inst) {
  containerFirstLastHelper<arrFirstLast<false, true>>(env, inst);
}

void cgKeysetFirst(IRLS& env, const IRInstruction* inst) {
  containerFirstLastHelper<arrFirstLast<true, false>>(env, inst);
}

void cgKeysetLast(IRLS& env, const IRInstruction* inst) {
  containerFirstLastHelper<arrFirstLast<false, false>>(env, inst);
}

///////////////////////////////////////////////////////////////////////////////
// Vec.

namespace {

struct LvalPtrs {
  Vptr type, val;
};
LvalPtrs implPackedLayoutElemAddr(IRLS& env, Vloc arrLoc,
                                  Vloc idxLoc, const SSATmp* idx) {
  auto const rarr = arrLoc.reg();
  auto const ridx = idxLoc.reg();
  auto& v = vmain(env);

  static_assert(sizeof(TypedValue) == 16, "");
  static_assert(TVOFF(m_data) == 0, "");

  if (idx->hasConstVal()) {
    auto const offset = PackedArray::entriesOffset() +
                        idx->intVal() * sizeof(TypedValue);
    if (deltaFits(offset, sz::dword)) {
      return {rarr[offset + TVOFF(m_type)], rarr[offset]};
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

  auto const valPtr = rarr[
    scaled_idx * int(sizeof(TypedValue) / 2) + PackedArray::entriesOffset()
  ];
  return {valPtr + TVOFF(m_type), valPtr};
}

void implVecSet(IRLS& env, const IRInstruction* inst) {
  BUILD_OPTAB(VECSET_HELPER_TABLE, RuntimeOption::EvalArrayProvenance);

  auto args = argGroup(env, inst).
    ssa(0).
    ssa(1).
    typedValue(2);

  auto& v = vmain(env);
  cgCallHelper(v, env, target, callDest(env, inst), SyncOptions::Sync, args);
}

/*
 * Thread-local RDS packed array access sampling counter.
 */
rds::Link<uint32_t, rds::Mode::Local> s_counter;

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

    if (arrTy.maybe(TPackedArr)) {
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

      if (arrTy <= TPackedArr) {
        profile(v);
      } else {
        auto const sf = v.makeReg();
        v << cmpbim{ArrayData::kPackedKind, arrLoc.reg()[HeaderKindOffset], sf};
        ifThen(v, CC_E, sf, profile);
      }
    }
  }

  auto const dstLoc = irlower::dstLoc(env, inst, 0);
  auto const addr = implPackedLayoutElemAddr(env, arrLoc, idxLoc, inst->src(1));
  vmain(env) << lea{addr.val, dstLoc.reg(tv_lval::val_idx)};
  if (wide_tv_val) {
    vmain(env) << lea{addr.type, dstLoc.reg(tv_lval::type_idx)};
  }
}

namespace {

void packedLayoutLoadImpl(IRLS& env, const IRInstruction* inst) {
  auto const arrLoc = srcLoc(env, inst, 0);
  auto const idxLoc = srcLoc(env, inst, 1);
  auto const addr = implPackedLayoutElemAddr(env, arrLoc, idxLoc, inst->src(1));

  loadTV(vmain(env), inst->dst()->type(), dstLoc(env, inst, 0),
         addr.type, addr.val);
}

}

void cgLdVecElem(IRLS& env, const IRInstruction* inst) {
  packedLayoutLoadImpl(env, inst);
}

void cgLdPackedElem(IRLS& env, const IRInstruction* inst) {
  packedLayoutLoadImpl(env, inst);
}

void cgElemVecD(IRLS& env, const IRInstruction* inst) {
  BUILD_OPTAB(ELEM_VEC_D_HELPER_TABLE, RuntimeOption::EvalArrayProvenance);

  auto args = argGroup(env, inst).ssa(0).ssa(1);

  auto& v = vmain(env);
  cgCallHelper(v, env, target, callDest(env, inst), SyncOptions::Sync, args);
}

IMPL_OPCODE_CALL(ElemVecU)

void cgVecSet(IRLS& env, const IRInstruction* i)    { implVecSet(env, i); }

void cgSetNewElemVec(IRLS& env, const IRInstruction* inst) {
  auto const target = RuntimeOption::EvalArrayProvenance
    ? CallSpec::direct(MInstrHelpers::setNewElemVec<true>)
    : CallSpec::direct(MInstrHelpers::setNewElemVec<false>);

  auto args = argGroup(env, inst)
    .ssa(0)
    .typedValue(1);


  auto& v = vmain(env);
  cgCallHelper(v, env, target, callDest(env, inst), SyncOptions::Sync, args);
}

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

void implDictGet(IRLS& env, const IRInstruction* inst, MOpMode mode) {
  assertx(mode == MOpMode::None || mode == MOpMode::Warn);
  auto const key = inst->src(1);
  BUILD_OPTAB(DICTGET_HELPER_TABLE, getKeyType(key), mode);

  auto args = argGroup(env, inst).ssa(0).ssa(1);

  auto& v = vmain(env);
  cgCallHelper(v, env, target, callDestTV(env, inst), SyncOptions::Sync, args);
}

void implDictSet(IRLS& env, const IRInstruction* inst) {
  BUILD_OPTAB(DICTSET_HELPER_TABLE,
              getKeyType(inst->src(1)),
              RuntimeOption::EvalArrayProvenance);

  auto args = argGroup(env, inst).
    ssa(0).
    ssa(1).
    typedValue(2);

  auto& v = vmain(env);
  cgCallHelper(v, env, target, callDest(env, inst), SyncOptions::Sync, args);
}

}

void cgElemDictX(IRLS& env, const IRInstruction* inst) {
  auto const key = inst->src(1);
  auto const mode = inst->extra<ElemDictX>()->mode;
  BUILD_OPTAB(ELEM_DICT_HELPER_TABLE, getKeyType(key), mode);

  auto args = argGroup(env, inst).ssa(0).ssa(1);

  auto& v = vmain(env);
  cgCallHelper(v, env, target, callDest(env, inst), SyncOptions::Sync, args);
}

void cgElemDictD(IRLS& env, const IRInstruction* inst) {
  auto const key     = inst->src(1);
  BUILD_OPTAB(ELEM_DICT_D_HELPER_TABLE,
              getKeyType(key),
              RuntimeOption::EvalArrayProvenance);

  auto args = argGroup(env, inst).ssa(0).ssa(1);

  auto& v = vmain(env);
  cgCallHelper(v, env, target, callDest(env, inst), SyncOptions::Sync, args);
}

void cgElemDictU(IRLS& env, const IRInstruction* inst) {
  auto const key     = inst->src(1);
  BUILD_OPTAB(ELEM_DICT_U_HELPER_TABLE, getKeyType(key));

  auto args = argGroup(env, inst).ssa(0).ssa(1);

  auto& v = vmain(env);
  cgCallHelper(v, env, target, callDest(env, inst), SyncOptions::Sync, args);
}

void cgElemDictK(IRLS& env, const IRInstruction* inst) {
  auto const dict = srcLoc(env, inst, 0).reg();
  auto const dst = dstLoc(env, inst, 0);
  auto const pos = inst->extra<ElemDictK>()->index;
  auto const off = MixedArray::elmOff(pos) + MixedArray::Elm::dataOff();

  auto& v = vmain(env);

  v << lea{dict[off], dst.reg(tv_lval::val_idx)};
  if (wide_tv_val) {
    static_assert(TVOFF(m_data) == 0, "");
    v << lea{dict[off + TVOFF(m_type)], dst.reg(tv_lval::type_idx)};
  }
}

void cgDictGet(IRLS& env, const IRInstruction* inst) {
  implDictGet(env, inst, MOpMode::Warn);
}
void cgDictGetQuiet(IRLS& env, const IRInstruction* inst) {
  implDictGet(env, inst, MOpMode::None);
}

void cgDictGetK(IRLS& env, const IRInstruction* inst) {
  auto const dict = srcLoc(env, inst, 0).reg();
  auto const pos = inst->extra<DictGetK>()->index;
  auto const off = MixedArray::elmOff(pos) + MixedArray::Elm::dataOff();

  auto& v = vmain(env);
  loadTV(v, inst->dst(0), dstLoc(env, inst, 0), dict[off]);
}

void cgDictSet(IRLS& env, const IRInstruction* i)    { implDictSet(env, i); }

void cgDictIsset(IRLS& env, const IRInstruction* inst) {
  auto const key     = inst->src(1);
  BUILD_OPTAB(DICT_ISSET_ELEM_HELPER_TABLE, getKeyType(key));

  auto args = argGroup(env, inst).ssa(0).ssa(1);

  auto& v = vmain(env);
  cgCallHelper(v, env, target, callDest(env, inst), SyncOptions::Sync, args);
}

void cgDictIdx(IRLS& env, const IRInstruction* inst) {
  auto const def = inst->src(2);
  if (def->isA(TInitNull)) return implDictGet(env, inst, MOpMode::None);
  auto const keyType = getKeyType(inst->src(1));

  auto const target = [&] () -> CallSpec {
    if (keyType == KeyType::Int) return CallSpec::direct(dictIdxI);
    assertx(keyType == KeyType::Str);
    auto const scan =
      inst->extra<SizeHintData>()->hint == SizeHintData::SmallStatic &&
      inst->src(1)->isA(TStaticStr);
    return CallSpec::direct(scan ? dictIdxScan : dictIdxS);
  }();

  auto args = argGroup(env, inst).ssa(0).ssa(1).typedValue(2);
  auto& v = vmain(env);
  cgCallHelper(v, env, target, callDestTV(env, inst),
               SyncOptions::Sync, args);
}

///////////////////////////////////////////////////////////////////////////////
// Keyset.

namespace {

void implKeysetGet(IRLS& env, const IRInstruction* inst) {
  auto const key = inst->src(1);
  auto const mode = inst->op() == KeysetGetQuiet
    ? MOpMode::None
    : MOpMode::Warn;
  BUILD_OPTAB(KEYSETGET_HELPER_TABLE, getKeyType(key), mode);

  auto args = argGroup(env, inst).ssa(0).ssa(1);

  auto& v = vmain(env);
  cgCallHelper(v, env, target, callDestTV(env, inst), SyncOptions::Sync, args);
}

}

void cgElemKeysetX(IRLS& env, const IRInstruction* inst) {
  auto const key = inst->src(1);
  auto const mode = inst->extra<ElemKeysetX>()->mode;
  BUILD_OPTAB(ELEM_KEYSET_HELPER_TABLE, getKeyType(key), mode);

  auto args = argGroup(env, inst).ssa(0).ssa(1);

  auto& v = vmain(env);
  cgCallHelper(v, env, target, callDest(env, inst), SyncOptions::Sync, args);
}

void cgElemKeysetU(IRLS& env, const IRInstruction* inst) {
  auto const key = inst->src(1);
  BUILD_OPTAB(ELEM_KEYSET_U_HELPER_TABLE, getKeyType(key));

  auto args = argGroup(env, inst).ssa(0).ssa(1);

  auto& v = vmain(env);
  cgCallHelper(v, env, target, callDest(env, inst), SyncOptions::Sync, args);
}

void cgElemKeysetK(IRLS& env, const IRInstruction* inst) {
  auto const keyset = srcLoc(env, inst, 0).reg();
  auto const dst = dstLoc(env, inst, 0);
  auto const pos = inst->extra<ElemKeysetK>()->index;
  auto const off = SetArray::tvOff(pos);

  auto& v = vmain(env);
  v << lea{keyset[off], dst.reg(tv_lval::val_idx)};
  if (wide_tv_val) {
    static_assert(TVOFF(m_data) == 0, "");
    v << lea{keyset[off + TVOFF(m_type)], dst.reg(tv_lval::type_idx)};
  }
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
  cgCallHelper(v, env, target, callDest(env, inst), SyncOptions::Sync, args);
}

void cgKeysetIsset(IRLS& env, const IRInstruction* inst) {
  auto const key = inst->src(1);
  BUILD_OPTAB(KEYSET_ISSET_ELEM_HELPER_TABLE, getKeyType(key));

  auto args = argGroup(env, inst).ssa(0).ssa(1);

  auto& v = vmain(env);
  cgCallHelper(v, env, target, callDest(env, inst), SyncOptions::Sync, args);
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

void cgVectorSet(IRLS& env, const IRInstruction* inst) {
  auto const target = inst->src(1)->isA(TInt)
    ? CallSpec::direct(MInstrHelpers::vectorSetImplI)
    : CallSpec::direct(MInstrHelpers::vectorSetImplS);

  auto const args = argGroup(env, inst)
    .ssa(0)
    .ssa(1)
    .typedValue(2);

  auto& v = vmain(env);
  cgCallHelper(v, env, target, kVoidDest, SyncOptions::Sync, args);
}

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
