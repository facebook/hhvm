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

#include "hphp/runtime/base/bespoke/layout.h"
#include "hphp/runtime/base/bespoke/logging-profile.h"
#include "hphp/runtime/base/mixed-array.h"
#include "hphp/runtime/base/packed-array.h"
#include "hphp/runtime/base/set-array.h"

#include "hphp/runtime/vm/jit/irlower.h"
#include "hphp/runtime/vm/jit/irlower-internal.h"
#include "hphp/runtime/vm/jit/ir-opcode.h"
#include "hphp/runtime/vm/jit/minstr-helpers.h"

namespace HPHP { namespace jit { namespace irlower {

//////////////////////////////////////////////////////////////////////////////

void cgLogArrayReach(IRLS& env, const IRInstruction* inst) {
  auto const data = inst->extra<LogArrayReach>();

  auto& v = vmain(env);
  auto const args = argGroup(env, inst).imm(data->profile).ssa(0);
  auto const target = CallSpec::method(&bespoke::SinkProfile::update);
  cgCallHelper(v, env, target, callDest(env, inst), SyncOptions::Sync, args);
}

void cgNewLoggingArray(IRLS& env, const IRInstruction* inst) {
  auto const data = inst->extra<NewLoggingArray>();

  auto const target = [&] {
    using Fn = ArrayData*(*)(ArrayData*, bespoke::LoggingProfile*);
    return shouldTestBespokeArrayLikes()
      ? CallSpec::direct(static_cast<Fn>(bespoke::makeBespokeForTesting))
      : CallSpec::direct(static_cast<Fn>(bespoke::maybeMakeLoggingArray));
  }();

  cgCallHelper(vmain(env), env, target, callDest(env, inst), SyncOptions::Sync,
               argGroup(env, inst).ssa(0).immPtr(data->profile));
}

void cgProfileArrLikeProps(IRLS& env, const IRInstruction* inst) {
  auto const target = CallSpec::direct(bespoke::profileArrLikeProps);
  cgCallHelper(vmain(env), env, target, kVoidDest, SyncOptions::Sync,
               argGroup(env, inst).ssa(0));
}

//////////////////////////////////////////////////////////////////////////////

namespace {
const bespoke::ConcreteLayout* getConcreteLayout(Type type) {
  assertx(type <= TArrLike);
  auto const layout = type.arrSpec().bespokeLayout();
  return layout ? layout->concreteLayout() : nullptr;
}

static TypedValue getInt(const ArrayData* ad, int64_t key) {
  return ad->get(key);
}

static TypedValue getStr(const ArrayData* ad, const StringData* key) {
  return ad->get(key);
}
}

// This macro returns a CallSpec to one of several static functions:
//
//    - the one on a specific, concrete BespokeLayout;
//    - the generic one on BespokeArray;
//    - the ones on the vanilla arrays (Packed, Mixed, Set);
//    - failing all those options, the CallSpec Generic
//
#define CALL_TARGET(Type, Fn, Generic)                                    \
  [&]{                                                                    \
    auto const spec = Type.arrSpec();                                     \
    auto const layout = spec.bespokeLayout();                             \
    if (layout) {                                                         \
      if (auto const concrete = layout->concreteLayout()) {               \
        return CallSpec::direct(concrete->vtable()->fn##Fn);              \
      }                                                                   \
      return CallSpec::direct(BespokeArray::Fn);                          \
    }                                                                     \
    if (spec.vanilla()) {                                                 \
      if (arr <= (TVArr|TVec))  return CallSpec::direct(PackedArray::Fn); \
      if (arr <= (TDArr|TDict)) return CallSpec::direct(MixedArray::Fn);  \
      if (arr <= TKeyset)       return CallSpec::direct(SetArray::Fn);    \
    }                                                                     \
    return Generic;                                                       \
  }()

void cgBespokeGet(IRLS& env, const IRInstruction* inst) {
  using GetInt = TypedValue (ArrayData::*)(int64_t) const;
  using GetStr = TypedValue (ArrayData::*)(const StringData*) const;

  auto const arr = inst->src(0)->type();
  auto const key = inst->src(1)->type();
  auto const target = (key <= TInt)
    ? CALL_TARGET(arr, NvGetInt, CallSpec::direct(getInt))
    : CALL_TARGET(arr, NvGetStr, CallSpec::direct(getStr));

  auto& v = vmain(env);
  auto const args = argGroup(env, inst).ssa(0).ssa(1);
  cgCallHelper(v, env, target, callDestTV(env, inst), SyncOptions::Sync, args);
}

void cgBespokeSet(IRLS& env, const IRInstruction* inst) {
  using SetInt = ArrayData* (ArrayData::*)(int64_t, TypedValue);
  using SetStr = ArrayData* (ArrayData::*)(StringData*, TypedValue);

  auto const setIntMove =
    CallSpec::method(static_cast<SetInt>(&ArrayData::setMove));
  auto const setStrMove =
    CallSpec::method(static_cast<SetStr>(&ArrayData::setMove));

  auto const arr = inst->src(0)->type();
  auto const key = inst->src(1)->type();
  auto const target = (key <= TInt)
    ? CALL_TARGET(arr, SetIntMove, setIntMove)
    : CALL_TARGET(arr, SetStrMove, setStrMove);

  auto& v = vmain(env);
  auto const args = argGroup(env, inst).ssa(0).ssa(1).typedValue(2);
  cgCallHelper(v, env, target, callDest(env, inst), SyncOptions::Sync, args);
}

void cgBespokeAppend(IRLS& env, const IRInstruction* inst) {
  using Append = ArrayData* (ArrayData::*)(TypedValue);

  auto const appendMove =
    CallSpec::method(static_cast<Append>(&ArrayData::appendMove));

  auto const arr = inst->src(0)->type();
  auto const target = CALL_TARGET(arr, AppendMove, appendMove);

  auto& v = vmain(env);
  auto const args = argGroup(env, inst).ssa(0).typedValue(1);
  cgCallHelper(v, env, target, callDest(env, inst), SyncOptions::Sync, args);
}

void cgBespokeIterFirstPos(IRLS& env, const IRInstruction* inst) {
  auto const arr = inst->src(0)->type();
  auto const iterBegin = CallSpec::method(&ArrayData::iter_begin);
  auto const target = CALL_TARGET(arr, IterBegin, iterBegin);

  auto& v = vmain(env);
  auto const args = argGroup(env, inst).ssa(0);
  cgCallHelper(v, env, target, callDest(env, inst), SyncOptions::Sync, args);
}

void cgBespokeIterLastPos(IRLS& env, const IRInstruction* inst) {
  auto const arr = inst->src(0)->type();
  auto const iterLast = CallSpec::method(&ArrayData::iter_last);
  auto const target = CALL_TARGET(arr, IterLast, iterLast);

  auto& v = vmain(env);
  auto const args = argGroup(env, inst).ssa(0);
  cgCallHelper(v, env, target, callDest(env, inst), SyncOptions::Sync, args);
}

void cgBespokeIterAdvancePos(IRLS& env, const IRInstruction* inst) {
  auto const arr = inst->src(0)->type();
  auto const iterAdvance = CallSpec::method(&ArrayData::iter_advance);
  auto const target = CALL_TARGET(arr, IterAdvance, iterAdvance);

  auto& v = vmain(env);
  auto const args = argGroup(env, inst).ssa(0).ssa(1);
  cgCallHelper(v, env, target, callDest(env, inst), SyncOptions::Sync, args);
}

void cgBespokeIterGetKey(IRLS& env, const IRInstruction* inst) {
  auto const arr = inst->src(0)->type();
  auto const getPosKey = CallSpec::method(&ArrayData::nvGetKey);
  auto const target = CALL_TARGET(arr, GetPosKey, getPosKey);

  auto& v = vmain(env);
  auto const args = argGroup(env, inst).ssa(0).ssa(1);
  cgCallHelper(v, env, target, callDestTV(env, inst), SyncOptions::Sync, args);
}

void cgBespokeIterGetVal(IRLS& env, const IRInstruction* inst) {
  auto const arr = inst->src(0)->type();
  auto const getPosVal = CallSpec::method(&ArrayData::nvGetVal);
  auto const target = CALL_TARGET(arr, GetPosVal, getPosVal);

  auto& v = vmain(env);
  auto const args = argGroup(env, inst).ssa(0).ssa(1);
  cgCallHelper(v, env, target, callDestTV(env, inst), SyncOptions::Sync, args);
}

void cgBespokeEscalateToVanilla(IRLS& env, const IRInstruction* inst) {
  auto const target = [&] {
    auto const layout = inst->src(0)->type().arrSpec().bespokeLayout();
    assertx(layout);
    if (auto const concrete = layout->concreteLayout()) {
      return CallSpec::direct(concrete->vtable()->fnEscalateToVanilla);
    } else {
      return CallSpec::direct(BespokeArray::ToVanilla);
    }
  }();

  auto& v = vmain(env);
  auto const reason = inst->src(1)->strVal()->data();
  auto const args = argGroup(env, inst).ssa(0).imm(reason);
  cgCallHelper(v, env, target, callDest(env, inst), SyncOptions::Sync, args);
}

#undef CALL_TARGET

void cgBespokeElem(IRLS& env, const IRInstruction* inst) {
  auto& v = vmain(env);
  auto const dest = callDest(env, inst);
  auto const target = [&] {
    auto const arr = inst->src(0);
    auto const key = inst->src(1);
    auto const layout = arr->type().arrSpec().bespokeLayout();

    // Bespoke arrays always have specific Elem helper functions.
    if (layout) {
      if (auto const concrete = layout->concreteLayout()) {
        return key->isA(TStr) ? CallSpec::direct(concrete->vtable()->fnElemStr)
                              : CallSpec::direct(concrete->vtable()->fnElemInt);
      } else {
        return key->isA(TStr) ? CallSpec::direct(BespokeArray::ElemStr)
                              : CallSpec::direct(BespokeArray::ElemInt);
      }
    }

    // Aside from known-bespokes, we only specialize certain Elem cases -
    // the ones we already have symbols for in the MInstrHelpers namespace.
    using namespace MInstrHelpers;
    auto const throwOnMissing = inst->src(2)->boolVal();
    if (arr->isA(TVanillaArrLike)) {
      if (arr->isA(TDArr|TDict)) {
        return key->isA(TStr)
          ? CallSpec::direct(throwOnMissing ? elemDictSD : elemDictSU)
          : CallSpec::direct(throwOnMissing ? elemDictID : elemDictIU);
      }
      if (arr->isA(TKeyset) && !throwOnMissing) {
        return key->isA(TStr)
          ? CallSpec::direct(elemKeysetSU)
          : CallSpec::direct(elemKeysetIU);
      }
    }
    return key->isA(TStr)
      ? CallSpec::direct(throwOnMissing ? elemSD : elemSU)
      : CallSpec::direct(throwOnMissing ? elemID : elemIU);
  }();
  auto const args = argGroup(env, inst).ssa(0).ssa(1).ssa(2);
  cgCallHelper(v, env, target, dest, SyncOptions::Sync, args);
}

//////////////////////////////////////////////////////////////////////////////

}}}
