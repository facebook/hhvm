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

#include "hphp/runtime/vm/jit/irlower-bespoke.h"

#include <hphp/util/assertions.h>
#include "hphp/runtime/base/bespoke/escalation-logging.h"
#include "hphp/runtime/base/bespoke/layout.h"
#include "hphp/runtime/base/bespoke/logging-array.h"
#include "hphp/runtime/base/bespoke/logging-profile.h"
#include "hphp/runtime/base/bespoke/monotype-dict.h"
#include "hphp/runtime/base/bespoke/monotype-vec.h"
#include "hphp/runtime/base/bespoke/struct-data-layout.h"
#include "hphp/runtime/base/bespoke/struct-dict.h"
#include "hphp/runtime/base/bespoke/type-structure.h"
#include "hphp/runtime/base/vanilla-dict.h"
#include "hphp/runtime/base/vanilla-keyset.h"
#include "hphp/runtime/base/vanilla-vec.h"

#include "hphp/runtime/vm/jit/code-gen-cf.h"
#include "hphp/runtime/vm/jit/code-gen-helpers.h"
#include "hphp/runtime/vm/jit/irlower.h"
#include "hphp/runtime/vm/jit/irlower-internal.h"
#include "hphp/runtime/vm/jit/ir-opcode.h"
#include "hphp/runtime/vm/jit/minstr-helpers.h"
#include "hphp/runtime/vm/jit/translator-inline.h"

namespace HPHP::jit::irlower {

//////////////////////////////////////////////////////////////////////////////
// Generic BespokeArrays

namespace {
static void logGuardFailure(TypedValue tv, uint16_t layout, uint64_t sk) {
  assertx(tvIsArrayLike(tv));
  auto const al = ArrayLayout::FromUint16(layout);
  bespoke::logGuardFailure(val(tv).parr, al, SrcKey(sk));
}

bool maybeLogging(Type t) {
  auto const loggingLayout =
    ArrayLayout(bespoke::LoggingArray::GetLayoutIndex());
  auto const loggingArray = TArrLike.narrowToLayout(loggingLayout);
  return t.maybe(loggingArray);
}

bool arrayLikeIsVanilla(const Type& type) {
  if (!allowBespokeArrayLikes()) return true;
  if (type.arrSpec().layout().vanilla()) return true;
  if (type.isKnownDataType() && !arrayTypeCouldBeBespoke(type.toDataType())) {
    return true;
  }
  return false;
}
}

void cgLogArrayReach(IRLS& env, const IRInstruction* inst) {
  auto const data = inst->extra<LogArrayReach>();

  auto& v = vmain(env);
  auto const args = argGroup(env, inst).imm(data->profile).ssa(0);
  auto const target = CallSpec::method(&bespoke::SinkProfile::update);
  cgCallHelper(v, env, target, callDest(env, inst), SyncOptions::Sync, args);
}

void cgLogGuardFailure(IRLS& env, const IRInstruction* inst) {
  auto const layout = inst->typeParam().arrSpec().layout().toUint16();
  auto const sk = inst->marker().sk().toAtomicInt();

  auto& v = vmain(env);
  auto const args = argGroup(env, inst).typedValue(0).imm(layout).imm(sk);
  auto const target = CallSpec::direct(logGuardFailure);
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

// This macro returns a CallSpec to one of several static functions:
//
//    - the one on a specific, concrete bespoke layout;
//    - the generic one on BespokeArray;
//    - the ones on the vanilla arrays (Packed, Mixed, Set);
//    - failing all those options, the CallSpec Generic
//
#define CALL_TARGET(Type, Fn, Generic)                              \
  [&]{                                                              \
    auto const layout = Type.arrSpec().layout();                    \
    if (layout.bespoke()) {                                         \
      auto const vtable = layout.bespokeLayout()->vtable();         \
      if (vtable->fn##Fn) {                                         \
        return CallSpec::direct(vtable->fn##Fn);                    \
      } else {                                                      \
        return CallSpec::direct(BespokeArray::Fn);                  \
      }                                                             \
    }                                                               \
    if (arrayLikeIsVanilla(arr)) {                                  \
      if (arr <= TVec)    return CallSpec::direct(VanillaVec::Fn);  \
      if (arr <= TDict)   return CallSpec::direct(VanillaDict::Fn); \
      if (arr <= TKeyset) return CallSpec::direct(VanillaKeyset::Fn);\
    }                                                               \
    return Generic;                                                 \
  }()

#define CALL_TARGET_SYNTH(Type, Fn, Generic)                                 \
  [&]{                                                                       \
    auto const layout = Type.arrSpec().layout();                             \
    if (layout.bespoke()) {                                                  \
      auto const vtable = layout.bespokeLayout()->vtable();                  \
      if (vtable->fn##Fn) {                                                  \
        return CallSpec::direct(vtable->fn##Fn);                             \
      } else {                                                               \
        return CallSpec::direct(BespokeArray::Fn);                           \
      }                                                                      \
    }                                                                        \
    if (arrayLikeIsVanilla(arr)) {                                           \
      if (arr <= TVec) {                                                     \
        return CallSpec::direct(SynthesizedArrayFunctions<VanillaVec>::Fn);  \
      }                                                                      \
      if (arr <= TDict) {                                                    \
        return CallSpec::direct(SynthesizedArrayFunctions<VanillaDict>::Fn); \
      }                                                                      \
      if (arr <= TKeyset) {                                                  \
        return CallSpec::direct(SynthesizedArrayFunctions<VanillaKeyset>::Fn);\
      }                                                                      \
    }                                                                        \
    return Generic;                                                          \
  }()

CallSpec destructorForArrayLike(Type arr) {
  assertx(arr <= TArrLike);
  return CALL_TARGET(arr, Release, CallSpec::method(&ArrayData::release));
}

CallSpec copyFuncForArrayLike(Type arr) {
  assertx(arr <= TArrLike);
  return CALL_TARGET(arr, Copy, CallSpec::method(&ArrayData::copy));
}

void cgBespokeGet(IRLS& env, const IRInstruction* inst) {
  using GetInt = TypedValue (ArrayData::*)(int64_t) const;
  using GetStr = TypedValue (ArrayData::*)(const StringData*) const;

  auto const getInt =
    CallSpec::method(static_cast<GetInt>(&ArrayData::get));
  auto const getStr =
    CallSpec::method(static_cast<GetStr>(&ArrayData::get));

  auto const arr = inst->src(0)->type();
  auto const key = inst->src(1)->type();
  auto const target = (key <= TInt)
    ? CALL_TARGET(arr, NvGetInt, getInt)
    : CALL_TARGET(arr, NvGetStr, getStr);

  auto const syncMode = maybeLogging(inst->src(0)->type())
    ? SyncOptions::Sync
    : SyncOptions::None;

  auto& v = vmain(env);
  auto const args = argGroup(env, inst).ssa(0).ssa(1);
  cgCallHelper(v, env, target, callDestTV(env, inst), syncMode, args);
}

void cgBespokeGetThrow(IRLS& env, const IRInstruction* inst) {
  using GetInt = TypedValue (ArrayData::*)(int64_t) const;
  using GetStr = TypedValue (ArrayData::*)(const StringData*) const;

  auto const getInt =
    CallSpec::method(static_cast<GetInt>(&ArrayData::getThrow));
  auto const getStr =
    CallSpec::method(static_cast<GetStr>(&ArrayData::getThrow));

  auto const arr = inst->src(0)->type();
  auto const key = inst->src(1)->type();
  auto const target = (key <= TInt)
    ? CALL_TARGET_SYNTH(arr, NvGetIntThrow, getInt)
    : CALL_TARGET_SYNTH(arr, NvGetStrThrow, getStr);

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

void cgBespokeUnset(IRLS &env, const IRInstruction *inst) {
  using UnsetInt = ArrayData* (ArrayData::*)(int64_t);
  using UnsetStr = ArrayData* (ArrayData::*)(const StringData*);

  auto const unsetIntMove =
    CallSpec::method(static_cast<UnsetInt>(&ArrayData::removeMove));
  auto const unsetStrMove =
    CallSpec::method(static_cast<UnsetStr>(&ArrayData::removeMove));

  auto const arr = inst->src(0)->type();
  auto const key = inst->src(1)->type();
  auto const target = (key <= TInt)
    ? CALL_TARGET(arr, RemoveIntMove, unsetIntMove)
    : CALL_TARGET(arr, RemoveStrMove, unsetStrMove);

  auto& v = vmain(env);
  auto const args = argGroup(env, inst).ssa(0).ssa(1);
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
  cgCallHelper(v, env, target, callDest(env, inst), SyncOptions::None, args);
}

void cgBespokeIterLastPos(IRLS& env, const IRInstruction* inst) {
  auto const arr = inst->src(0)->type();
  auto const iterLast = CallSpec::method(&ArrayData::iter_last);
  auto const target = CALL_TARGET(arr, IterLast, iterLast);

  auto& v = vmain(env);
  auto const args = argGroup(env, inst).ssa(0);
  cgCallHelper(v, env, target, callDest(env, inst), SyncOptions::None, args);
}

void cgBespokeIterEnd(IRLS& env, const IRInstruction* inst) {
  auto const arr = inst->src(0)->type();
  auto const iterEnd = CallSpec::method(&ArrayData::iter_end);
  auto const target = CALL_TARGET(arr, IterEnd, iterEnd);

  auto& v = vmain(env);
  auto const args = argGroup(env, inst).ssa(0);
  cgCallHelper(v, env, target, callDest(env, inst), SyncOptions::None, args);
}

void cgBespokeIterGetKey(IRLS& env, const IRInstruction* inst) {
  auto const arr = inst->src(0)->type();
  auto const getPosKey = CallSpec::method(&ArrayData::nvGetKey);
  auto const target = CALL_TARGET(arr, GetPosKey, getPosKey);

  auto& v = vmain(env);
  auto const args = argGroup(env, inst).ssa(0).ssa(1);
  cgCallHelper(v, env, target, callDestTV(env, inst), SyncOptions::None, args);
}

void cgBespokeIterGetVal(IRLS& env, const IRInstruction* inst) {
  auto const arr = inst->src(0)->type();
  auto const getPosVal = CallSpec::method(&ArrayData::nvGetVal);
  auto const target = CALL_TARGET(arr, GetPosVal, getPosVal);

  auto& v = vmain(env);
  auto const args = argGroup(env, inst).ssa(0).ssa(1);
  cgCallHelper(v, env, target, callDestTV(env, inst), SyncOptions::None, args);
}

void cgBespokeEscalateToVanilla(IRLS& env, const IRInstruction* inst) {
  auto const target = [&] {
    auto const layout = inst->src(0)->type().arrSpec().layout();
    auto const vtable = layout.bespokeLayout()->vtable();
    if (vtable->fnEscalateToVanilla) {
      return CallSpec::direct(vtable->fnEscalateToVanilla);
    } else {
      return CallSpec::direct(BespokeArray::ToVanilla);
    }
  }();

  auto const syncMode = maybeLogging(inst->src(0)->type())
    ? SyncOptions::Sync
    : SyncOptions::None;

  auto& v = vmain(env);
  auto const reason = inst->src(1)->strVal()->data();
  auto const args = argGroup(env, inst).ssa(0).imm(reason);
  cgCallHelper(v, env, target, callDest(env, inst), syncMode, args);
}

void cgBespokeElem(IRLS& env, const IRInstruction* inst) {
  auto& v = vmain(env);
  auto const dest = callDest(env, inst);

  auto args = argGroup(env, inst).ssa(0).ssa(1);

  auto const target = [&] {
    auto const arr = inst->src(0);
    auto const key = inst->src(1);
    auto const layout = arr->type().arrSpec().layout();

    // Bespoke arrays always have specific Elem helper functions.
    if (layout.bespoke()) {
      args.ssa(2);
      auto const vtable = layout.bespokeLayout()->vtable();
      if (key->isA(TStr) && vtable->fnElemStr) {
        return CallSpec::direct(vtable->fnElemStr);
      } else if (key->isA(TInt) && vtable->fnElemInt) {
        return CallSpec::direct(vtable->fnElemInt);
      } else {
        return key->isA(TStr) ? CallSpec::direct(BespokeArray::ElemStr)
                              : CallSpec::direct(BespokeArray::ElemInt);
      }
    }

    // Aside from known-bespokes, we only specialize certain Elem cases -
    // the ones we already have symbols for in the MInstrHelpers namespace.
    using namespace MInstrHelpers;
    auto const throwOnMissing = inst->src(2)->boolVal();
    if (layout.vanilla()) {
      if (arr->isA(TDict)) {
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
  cgCallHelper(v, env, target, dest, SyncOptions::Sync, args);
}

//////////////////////////////////////////////////////////////////////////////
// MonotypeVec and MonotypeDict

namespace {
using MonotypeDict = bespoke::MonotypeDict<int64_t>;

// Returns a pointer to a value `off` bytes into the MonotypeDict element at
// the iterator position `pos` in the dict pointed to by `rarr`.
Vptr ptrToMonotypeDictElm(Vout& v, Vreg rarr, Vreg rpos, Type pos, size_t off) {
  auto const base = MonotypeDict::entriesOffset() + off;

  if (pos.hasConstVal()) {
    auto const offset = pos.intVal() * MonotypeDict::elmSize() + base;
    if (deltaFits(offset, sz::dword)) return rarr[offset];
  }

  static_assert(MonotypeDict::elmSize() == 16);
  auto posl = v.makeReg();
  auto scaled_posl = v.makeReg();
  auto scaled_pos = v.makeReg();
  v << movtql{rpos, posl};
  v << shlli{1, posl, scaled_posl, v.makeReg()};
  v << movzlq{scaled_posl, scaled_pos};
  return rarr[scaled_pos * int(MonotypeDict::elmSize() / 2) + base];
}
}

void cgLdMonotypeDictTombstones(IRLS& env, const IRInstruction* inst) {
  static_assert(MonotypeDict::tombstonesSize() == 2);
  auto const rarr = srcLoc(env, inst, 0).reg();
  auto const used = dstLoc(env, inst, 0).reg();
  vmain(env) << loadzwq{rarr[MonotypeDict::tombstonesOffset()], used};
}

void cgLdMonotypeDictKey(IRLS& env, const IRInstruction* inst) {
  auto const rarr = srcLoc(env, inst, 0).reg();
  auto const rpos = srcLoc(env, inst, 1).reg();
  auto const pos = inst->src(1)->type();
  auto const dst = dstLoc(env, inst, 0);

  auto& v = vmain(env);
  auto const off = MonotypeDict::elmKeyOffset();
  auto const ptr = ptrToMonotypeDictElm(v, rarr, rpos, pos, off);
  v << load{ptr, dst.reg(0)};

  if (dst.hasReg(1)) {
    auto const sf = v.makeReg();
    auto const intb = v.cns(KindOfInt64);
    auto const strb = v.cns(KindOfString);
    auto const mask = safe_cast<int32_t>(MonotypeDict::intKeyMask().raw);
    auto const layout = rarr[ArrayData::offsetOfBespokeIndex()];
    v << testwim{mask, layout, sf};
    v << cmovb{CC_Z, sf, intb, strb, dst.reg(1)};
  }
}

void cgLdMonotypeDictVal(IRLS& env, const IRInstruction* inst) {
  auto const rarr = srcLoc(env, inst, 0).reg();
  auto const rpos = srcLoc(env, inst, 1).reg();
  auto const pos = inst->src(1)->type();
  auto const dst = dstLoc(env, inst, 0);

  auto& v = vmain(env);
  auto const off = MonotypeDict::elmValOffset();
  auto const ptr = ptrToMonotypeDictElm(v, rarr, rpos, pos, off);
  v << load{ptr, dst.reg(0)};

  if (dst.hasReg(1)) {
    v << loadb{rarr[MonotypeDict::typeOffset()], dst.reg(1)};
  }
}

void cgLdMonotypeVecElem(IRLS& env, const IRInstruction* inst) {
  auto const rarr = srcLoc(env, inst, 0).reg();
  auto const ridx = srcLoc(env, inst, 1).reg();
  auto const idx = inst->src(1);

  auto const type = rarr[bespoke::MonotypeVec::typeOffset()];
  auto const value = [&] {
    auto const base = bespoke::MonotypeVec::entriesOffset();
    if (idx->hasConstVal()) {
      auto const offset = idx->intVal() * sizeof(Value) + base;
      if (deltaFits(offset, sz::dword)) return rarr[offset];
    }
    return rarr[ridx * int(sizeof(Value)) + base];
  }();

  loadTV(vmain(env), inst->dst()->type(), dstLoc(env, inst, 0),
         type, value);
}

//////////////////////////////////////////////////////////////////////////////
// TypeStructure

void callBespokeGetStr(IRLS& env, const IRInstruction* inst, CallDest dest) {
  using GetStr = TypedValue (ArrayData::*)(const StringData*) const;
  auto const getStr =
    CallSpec::method(static_cast<GetStr>(&ArrayData::get));

  auto const arr = inst->src(0)->type();
  auto const target = CALL_TARGET(arr, NvGetStr, getStr);

  auto const syncMode = maybeLogging(arr)
    ? SyncOptions::Sync
    : SyncOptions::None;

  auto& v = vmain(env);
  auto const args = argGroup(env, inst).ssa(0).ssa(1);
  cgCallHelper(v, env, target, dest, syncMode, args);
}

void cgLdTypeStructureVal(IRLS &env, const IRInstruction *inst) {
  auto& v = vmain(env);

  auto const sf = v.makeReg();
  auto const data = v.makeReg();
  auto const dt = v.makeReg();
  auto next = v.makeBlock();
  auto const dst = dstLoc(env, inst, 0);

  callBespokeGetStr(env, inst, callDest(data, dt));
  emitCmpTVType(v, sf, KindOfUninit, dt);
  v << jcc{CC_E, sf, {next, label(env, inst->taken())}};
  v = next;
  v << copy{data, dst.reg(0)};
  // if return type is known (ie. key is constant), dst uses only one register
  if (dst.hasReg(1)) v << copy{dt, dst.reg(1)};
}

void cgLdTypeStructureValCns(IRLS &env, const IRInstruction *inst) {
  auto const rarr = srcLoc(env, inst, 0).reg();
  auto const key = inst->extra<KeyedData>()->key;
  assertx(!key->empty());

  auto& v = vmain(env);
  auto const dst = dstLoc(env, inst, 0).reg();
  auto const fieldSF = v.makeReg();

  auto const offset = bespoke::TypeStructure::getFieldOffset(key);
  auto const fieldPair = bespoke::TypeStructure::getFieldPair(key);
  auto const dt = fieldPair.first;
  assertx(!isNullType(dt));

  auto const fieldsByteOffset = bespoke::TypeStructure::fieldsByteOffset();
  auto const fieldBitOffset = fieldPair.second;
  assertx(0 <= fieldBitOffset && fieldBitOffset < 8);
  auto const fieldBitMask = static_cast<uint8_t>(1 << fieldBitOffset);
  v << testbim{fieldBitMask, rarr[fieldsByteOffset], fieldSF};

  // load value at offset if the field exists on this kind of type structure
  // otherwise load falsy values
  cond(
    v, CC_NE, fieldSF, dst,
    [&] (Vout& v) {
      auto value = v.makeReg();
      if (dt == KindOfBoolean) {
        auto const sf = v.makeReg();
        auto const bitOffset = bespoke::TypeStructure::getBooleanBitOffset(key);
        v << testbim{static_cast<uint8_t>(1 << bitOffset), rarr[offset], sf};
        v << setcc{CC_NE, sf, value};
      } else if (dt == KindOfInt64) {
        v << loadzbq{rarr[offset], value};
      } else if (isArrayLikeType(dt) || isStringType(dt)) {
        v << load{rarr[offset], value};
      } else {
        always_assert(false);
      }
      return value;
    },
    [&] (Vout& v) {
      if (dt == KindOfBoolean) {
        return v.cns(false);
      } else if (dt == KindOfInt64) {
        return v.cns(0);
      }
      assertx(isArrayLikeType(dt) || isStringType(dt));
      return v.cns(nullptr);
    }
  );
}

#undef CALL_TARGET

//////////////////////////////////////////////////////////////////////////////
// StructDict

namespace {

using bespoke::StructDict;
using bespoke::StructLayout;

// Returns none if the layout is an abstract struct layout.
Optional<Slot> getStructSlot(const SSATmp* arr, const SSATmp* key) {
  assertx(key->hasConstVal(TStr));
  auto const layout = arr->type().arrSpec().layout();
  assertx(layout.is_struct());

  if (!layout.bespokeLayout()->isConcrete()) return std::nullopt;

  auto const slayout = StructLayout::As(layout.bespokeLayout());
  return slayout->keySlot(key->strVal());
}

}

void cgAllocBespokeStructDict(IRLS& env, const IRInstruction* inst) {
  auto const extra = inst->extra<AllocBespokeStructDict>();
  auto& v = vmain(env);

  auto const layout = StructLayout::As(extra->layout.bespokeLayout());
  auto const target = CallSpec::direct(StructDict::AllocStructDict);
  auto const args = argGroup(env, inst)
    .imm(layout->sizeIndex())
    .imm(layout->extraInitializer())
    .imm(layout->mayContainCounted());

  cgCallHelper(v, env, target, callDest(env, inst), SyncOptions::None, args);
}

void cgInitStructPositions(IRLS& env, const IRInstruction* inst) {
  auto const arr = inst->src(0);
  auto const rarr = srcLoc(env, inst, 0).reg();
  auto const extra = inst->extra<InitStructPositions>();
  auto& v = vmain(env);

  v << storel{v.cns(extra->numSlots), rarr[ArrayData::offsetofSize()]};

  auto const layout = arr->type().arrSpec().layout();
  auto const slayout = StructLayout::As(layout.bespokeLayout());

  auto const posSize = slayout->isBigStruct() ? 2 : 1;
  auto constexpr kStoreSize = 8;
  auto const slotsPerStore = kStoreSize / posSize;
  auto const padBytes = slayout->positionOffset() & 0x7;
  assertx((padBytes % posSize) == 0);
  auto const padSlots = padBytes / posSize;
  auto const numSlots = extra->numSlots + padSlots;

  for (auto i = 0; i < numSlots; i += slotsPerStore) {
    uint64_t slots = 0;
    for (auto j = 0; j < slotsPerStore; j++) {
      // We store 8 bytes at a time, starting from an 8 byte aligned address,
      // which can be before the poition offset. The gap between the start
      // address and the position offset may contain UnalignedTypedValue or
      // DataType and therefore should be initialized with KindOfUninit.
      // Later positions can be zeroed.
      auto const index = static_cast<int32_t>(i + j - padSlots);
      auto const slot = [&]{
        if (index < 0) {
          if (slayout->isBigStruct()) {
            auto const u = static_cast<uint8_t>(KindOfUninit);
            return static_cast<uint16_t>(u << 8 | u);
          } else {
            return static_cast<uint16_t>(KindOfUninit);
          }
        }
        if (index < extra->numSlots) {
          return safe_cast<uint16_t>(extra->slots[index]);
        }
        return uint16_t{0};
      }();
      auto const shiftedSlot =
        static_cast<uint64_t>(Slot(slot)) << (j * posSize * 8);
      slots = slots | shiftedSlot;
    }
    auto const offset = slayout->positionOffset() + i * posSize - padBytes;
    assertx((offset % slotsPerStore) == 0);
    v << store{v.cns(slots), rarr[offset]};
  }
}

void cgInitStructElem(IRLS& env, const IRInstruction* inst) {
  auto const arr = inst->src(0);
  auto const val = inst->src(1);
  auto const layout = arr->type().arrSpec().layout();
  auto const slayout = StructLayout::As(layout.bespokeLayout());
  auto const slot = inst->extra<InitStructElem>()->index;
  always_assert(val->isA(slayout->getTypeBound(slot)));

  auto const rarr = srcLoc(env, inst, 0).reg();
  auto const type = rarr[slayout->typeOffsetForSlot(slot)];
  auto const data = rarr[slayout->valueOffsetForSlot(slot)];
  storeTV(vmain(env), val->type(), srcLoc(env, inst, 1), type, data);
}

namespace {
template<typename PosType>
void newBespokeStructDictImpl(IRLS& env, const IRInstruction* inst) {
  auto const sp = srcLoc(env, inst, 0).reg();

  auto& v = vmain(env);

  auto const extra = inst->extra<NewBespokeStructDict>();
  auto const layout = StructLayout::As(extra->layout.bespokeLayout());
  auto const n = static_cast<size_t>((extra->numSlots + 7) & ~7);
  auto constexpr slotsPerAlloc = sizeof(uint64_t) / sizeof(PosType);
  auto const slots = reinterpret_cast<PosType*>(
    v.allocData<uint64_t>(n / slotsPerAlloc)
  );
  for (auto i = 0; i < extra->numSlots; i++) {
    slots[i] = safe_cast<PosType>(extra->slots[i]);
  }
  constexpr uint16_t initVal =
    (uint8_t)KindOfUninit << 8 | (uint8_t)KindOfUninit;
  for (auto i = extra->numSlots; i < n; i++) {
    slots[i] = static_cast<PosType>(initVal);
  }

  auto const target = std::is_same_v<PosType, uint8_t> ?
    CallSpec::direct(StructDict::MakeStructDictSmall) :
    CallSpec::direct(StructDict::MakeStructDictBig);

  auto const args = argGroup(env, inst)
    .imm(layout->sizeIndex())
    .imm(layout->extraInitializer())
    .imm(extra->numSlots)
    .imm(layout->mayContainCounted())
    .dataPtr(slots)
    .addr(sp, cellsToBytes(extra->offset.offset));

  cgCallHelper(v, env, target, callDest(env, inst), SyncOptions::None, args);
}
}

void cgNewBespokeStructDict(IRLS& env, const IRInstruction* inst) {
  auto const extra = inst->extra<NewBespokeStructDict>();
  auto const layout = StructLayout::As(extra->layout.bespokeLayout());
  layout->isBigStruct() ?
    newBespokeStructDictImpl<uint16_t>(env, inst) :
    newBespokeStructDictImpl<uint8_t>(env, inst);
}

void cgStructDictSlot(IRLS& env, const IRInstruction* inst) {
  auto const arr = inst->src(0);
  auto const key = inst->src(1);

  auto const rarr = srcLoc(env, inst, 0).reg();
  auto const rkey = srcLoc(env, inst, 1).reg();
  auto const dst = dstLoc(env, inst, 0).reg();

  auto& v = vmain(env);

  auto const& layout = arr->type().arrSpec().layout();
  assertx(layout.is_struct());

  // If we know both the layout and the key, we can calculate this as
  // a constant.
  if (key->hasConstVal(TStr)) {
    if (auto const slot = getStructSlot(arr, key)) {
      if (*slot == kInvalidSlot) {
        v << jmp{label(env, inst->taken())};
      } else {
        v << copy{v.cns(*slot), dst};
      }
      return;
    }
  }

  // Handle the case where the key definitely isn't a static
  // string. In this case, we rely on C++ to map the key to slot.
  auto const nonStaticCase = [&] (Vout& v) {
    auto const slot = v.makeReg();
    if (layout.bespokeLayout()->isConcrete()) {
      auto const slayout = StructLayout::As(layout.bespokeLayout());
      Slot(StructLayout::*fn)(const StringData*) const =
        &StructLayout::keySlotNonStatic;
      auto const target = CallSpec::method(fn);
      auto const args = argGroup(env, inst).immPtr(slayout).ssa(1);
      cgCallHelper(v, env, target, callDest(slot), SyncOptions::None, args);
    } else {
      auto const layoutIndex = v.makeReg();
      v << loadzwq{rarr[ArrayData::offsetOfBespokeIndex()], layoutIndex};
      Slot(*fn)(bespoke::LayoutIndex, const StringData*) =
        &StructLayout::keySlotNonStatic;
      auto const target = CallSpec::direct(fn);
      auto const args = argGroup(env, inst).reg(layoutIndex).ssa(1);
      cgCallHelper(v, env, target, callDest(slot), SyncOptions::None, args);
    }

    static_assert(sizeof(Slot) == 4);
    auto const sf = v.makeReg();
    v << cmpli{int32_t(kInvalidSlot), slot, sf};
    fwdJcc(v, env, CC_E, sf, inst->taken());
    return slot;
  };

  // Is the key certain to be non-static? If so, invoke the non-static
  // helper.
  if (!key->type().maybe(TStaticStr)) {
    auto const slot = nonStaticCase(v);
    v << copy{slot, dst};
    return;
  }

  // This register will be assigned iff the string is not constant.
  auto const colorMasked = v.makeReg();
  auto constexpr layoutMask = StructLayout::kMaxColor;
  static_assert(folly::isPowTwo(layoutMask + 1));
  static_assert(layoutMask <= std::numeric_limits<uint16_t>::max());

  // Calculate pointers to the hash table string and slot value, based
  // on what we know about the layout and key.
  auto const tuple = [&] {
    using PerfectHashEntry = StructLayout::PerfectHashEntry;
    auto constexpr hashEntrySize = sizeof(PerfectHashEntry);

    auto constexpr strHashOffset  = offsetof(PerfectHashEntry, str);
    auto constexpr slotHashOffset = offsetof(PerfectHashEntry, slot);
    auto constexpr dupFlagOffset = offsetof(PerfectHashEntry, maybeDup);

    // Read the string's color. This may be junk if the string is
    // non-static.
    auto const color = [&] {
      auto const colorPremask = v.makeReg();
      v << loadzwq{rkey[StringData::colorOffset()], colorPremask};
      v << andqi{
        uint16_t(layoutMask), colorPremask, colorMasked, v.makeReg()
      };

      static_assert(hashEntrySize == 8 || hashEntrySize == 16);
      if constexpr (hashEntrySize == 16) {
        // Without lowptr enabled, we have to use a 16-byte stride.
        auto const colorFinal = v.makeReg();
        v << lea{colorMasked[colorMasked], colorFinal};
        return colorFinal;
      } else {
        return colorMasked;
      }
    };

    if (layout.bespokeLayout()->isConcrete()) {
      auto const hashTable = StructLayout::hashTable(layout.bespokeLayout());
      auto const base = v.cns(uintptr_t(hashTable));
      auto const c = color();
      return std::make_tuple(
        base[c * 8 + slotHashOffset],
        base[c * 8 + strHashOffset],
        base[c * 8 + dupFlagOffset]
      );
    }

    auto const hashTableSet = (uintptr_t) StructLayout::hashTableSet();
    auto const layoutIndex = v.makeReg();
    v << loadzwq{rarr[ArrayData::offsetOfBespokeIndex()], layoutIndex};

    auto constexpr hashTableSize = sizeof(StructLayout::PerfectHashTable);
    static_assert(folly::isPowTwo(hashTableSize));
    auto const layoutShift =
      safe_cast<uint8_t>(folly::findLastSet(hashTableSize) - 1);

    auto const hashTableOffset = v.makeReg();
    v << shlqi{layoutShift, layoutIndex, hashTableOffset, v.makeReg()};

    if (key->hasConstVal(TStr)) {
      auto const offWithColor =
        (key->strVal()->color() & layoutMask) * hashEntrySize;
      return std::make_tuple(
        hashTableOffset[v.cns(hashTableSet) + (offWithColor + slotHashOffset)],
        hashTableOffset[v.cns(hashTableSet) + (offWithColor + strHashOffset)],
        hashTableOffset[v.cns(hashTableSet) + (offWithColor + dupFlagOffset)]
      );
    }

    auto const base = v.makeReg();
    v << lea{hashTableOffset[v.cns(hashTableSet)], base};
    auto const c = color();
    return std::make_tuple(
      base[c * 8 + slotHashOffset],
      base[c * 8 + strHashOffset],
      base[c * 8 + dupFlagOffset]
    );
  }();
  auto const slotPtr = std::get<0>(tuple);
  auto const strPtr = std::get<1>(tuple);
  auto const dupFlagPtr = std::get<2>(tuple);

  auto const hashStr = v.makeReg();
  auto const hashSF = v.makeReg();

  // Load the string in the hash table and check if it matches the
  // key.
  emitLdLowPtr(v, strPtr, hashStr, sizeof(LowStringPtr));
  v << cmpq{rkey, hashStr, hashSF};

  // If the key is definitely static and the strings don't match and the color
  // may not be duplicate, we're done. Otherwise, lookup using a C++ helper.
  if (key->isA(TStaticStr)) {
    cond(
      v, vcold(env), CC_E, hashSF, dst,
      [&] (Vout& v) {
        auto const slot = v.makeReg();
        v << loadzwq{slotPtr, slot};
        return slot;
      },
      [&] (Vout& v) {
        auto const dupSF = v.makeReg();
        v << cmpbim{static_cast<uint8_t>(true), dupFlagPtr, dupSF};
        fwdJcc(v, env, CC_NE, dupSF, inst->taken());
        return nonStaticCase(v);
      },
      StringTag {}
    );
    return;
  }

  // Otherwise the the key could be non-static. If they match, we're
  // good. Otherwise, check if the string is static
  // and the color may not be duplicate. If it is, then we definitely
  // don't have a match. Otherwise, evoke a C++ helper to do the lookup.
  cond(
    v, vcold(env), CC_E, hashSF, dst,
    [&] (Vout& v) {
      auto const slot = v.makeReg();
      v << loadzwq{slotPtr, slot};
      return slot;
    },
    [&] (Vout& v) {
      auto const refSF = emitCmpRefCount(v, StaticValue, rkey);
      auto const slot = v.makeReg();
      return cond(v, vcold(env), CC_NE, refSF, slot,
        [&] (Vout& v) {
          return nonStaticCase(v);
        },
        [&] (Vout& v) {
          auto const dupSF = v.makeReg();
          v << cmpbim{static_cast<uint8_t>(true), dupFlagPtr, dupSF};
          fwdJcc(v, env, CC_NE, dupSF, inst->taken());
          return nonStaticCase(v);
        },
        StringTag{}
      );
    },
    StringTag{}
  );
}

void cgStructDictElemAddr(IRLS& env, const IRInstruction* inst) {
  auto const arr   = inst->src(0);
  auto const slot  = inst->src(2);
  auto const rarr  = srcLoc(env, inst, 0).reg();
  auto const rslot = srcLoc(env, inst, 2).reg();
  auto const dst   = dstLoc(env, inst, 0);
  auto& v = vmain(env);

  auto const& layout = arr->type().arrSpec().layout();
  assertx(layout.is_struct());

  if (layout.bespokeLayout()->isConcrete() && slot->hasConstVal(TInt)) {
    auto const slayout = StructLayout::As(layout.bespokeLayout());
    v << lea{
      rarr[slayout->valueOffsetForSlot(slot->intVal())],
      dst.reg(tv_lval::val_idx)
    };
    v << lea{
      rarr[slayout->typeOffsetForSlot(slot->intVal())],
      dst.reg(tv_lval::type_idx)
    };
    return;
  }

  if constexpr (bespoke::detail_struct_data_layout::stores_utv) {
    static_assert(sizeof(UnalignedTypedValue) == 9);
    auto const rslot_times_9 = v.makeReg();
    v << lea{rslot[rslot * 8], rslot_times_9};
    auto constexpr val_offset =
      sizeof(StructDict) + offsetof(UnalignedTypedValue, m_data);
    auto constexpr type_offset =
      sizeof(StructDict) + offsetof(UnalignedTypedValue, m_type);
    static_assert(type_offset == val_offset + 8);
    v << lea{
      rarr[rslot_times_9 + val_offset], dst.reg(tv_lval::val_idx)
    };
    v << addqi{
      8, dst.reg(tv_lval::val_idx), dst.reg(tv_lval::type_idx), v.makeReg()
    };
  } else {
    using DataLayout = bespoke::detail_struct_data_layout::TypePosValLayout;
    if (layout.bespokeLayout()->isConcrete()) {
      auto const slayout = StructLayout::As(layout.bespokeLayout());
      auto const valBegin = DataLayout::valueOffsetForSlot(slayout, 0);
      v << lea{
        rarr[rslot * safe_cast<int>(sizeof(Value)) + valBegin],
        dst.reg(tv_lval::val_idx)
      };
    } else {
      static_assert(DataLayout::valueOffsetSize() == 1);
      auto const valBegin = v.makeReg();
      auto const valIdx = v.makeReg();
      v << loadzbq{rarr[DataLayout::valueOffsetOffset()], valBegin};
      v << addq{valBegin, rslot, valIdx, v.makeReg()};
      v << lea{
        rarr[valIdx * safe_cast<int>(sizeof(Value))],
        dst.reg(tv_lval::val_idx)
      };
    }
    v << lea{
      rarr[rslot * safe_cast<int>(sizeof(DataType))
        + DataLayout::staticTypeOffset()],
      dst.reg(tv_lval::type_idx)
    };
  }
}

void cgStructDictTypeBoundCheck(IRLS& env, const IRInstruction* inst) {
  auto const val = inst->src(0);
  auto const arr = inst->src(1);

  auto const rarr  = srcLoc(env, inst, 1).reg();
  auto const rslot = srcLoc(env, inst, 2).reg();

  auto const valLoc = srcLoc(env, inst, 0);
  auto const dst    = dstLoc(env, inst, 0);

  auto& v = vmain(env);

  auto const layout = [&] {
    auto const& layout = arr->type().arrSpec().layout();
    assertx(layout.is_struct());
    if (layout.bespokeLayout()->isConcrete()) {
      auto const slayout = StructLayout::As(layout.bespokeLayout());
      return v.cns((uintptr_t)slayout);
    } else {
      auto const layoutIndex = v.makeReg();
      auto const layout = v.makeReg();
      auto const layouts = v.cns((uintptr_t)bespoke::layoutsForJIT());
      v << loadzwq{rarr[ArrayData::offsetOfBespokeIndex()], layoutIndex};
      v << load{
        layouts[layoutIndex * safe_cast<int>(sizeof(bespoke::Layout*))],
        layout
      };
      return layout;
    }
  }();

  static_assert(StructLayout::Field::typeMaskSize() == 1);
  static_assert(sizeof(StructLayout::Field) == 8 ||
                sizeof(StructLayout::Field) == 16);
  auto const adjustedSlot = [&] {
    if (sizeof(StructLayout::Field) == 16) {
      auto const doubled = v.makeReg();
      v << addq{rslot, rslot, doubled, v.makeReg()};
      return doubled;
    }
    return rslot;
  }();

  auto const typeMask = v.makeReg();
  v << loadb{
    layout[adjustedSlot * 8 +
           StructLayout::Field::typeMaskOffset() +
           StructLayout::fieldsOffset()],
    typeMask
  };

  auto const sf = v.makeReg();
  if (val->type().isKnownDataType()) {
    auto const dataType = val->type().toDataType();
    v << testbi{static_cast<int8_t>(dataType), typeMask, sf};
  } else {
    auto const rtype = valLoc.reg(1);
    v << testb{rtype, typeMask, sf};
  }

  fwdJcc(v, env, CC_NE, sf, inst->taken());
  copyTV(v, valLoc, dst, inst->dst()->type());
}

void cgStructDictAddNextSlot(IRLS& env, const IRInstruction* inst) {
  auto const arr   = inst->src(0);
  auto const rarr  = srcLoc(env, inst, 0).reg();
  auto const rslot = srcLoc(env, inst, 1).reg();
  auto& v = vmain(env);

  auto const& layout = arr->type().arrSpec().layout();
  assertx(layout.is_struct());

  static_assert(ArrayData::sizeofSize() == 4);

  auto const size = v.makeReg();
  v << loadzlq{rarr[ArrayData::offsetofSize()], size};

  if (layout.bespokeLayout()->isConcrete()) {
    auto const slayout = StructLayout::As(layout.bespokeLayout());
    auto const smallSlot = v.makeReg();
    if (slayout->isBigStruct()) {
      v << movtqw{rslot, smallSlot};
      v << storew{smallSlot, rarr[size * 2 + slayout->positionOffset()]};
    } else {
      v << movtqb{rslot, smallSlot};
      v << storeb{smallSlot, rarr[size + slayout->positionOffset()]};
    }
  } else {
    if constexpr (bespoke::detail_struct_data_layout::stores_utv) {
      assertx(StructDict::numFieldsSize() == 2);
      auto const numFields = v.makeReg();
      v << loadzwq{rarr[StructDict::numFieldsOffset()], numFields};
      static_assert(sizeof(UnalignedTypedValue) == 9);
      auto const sf = v.makeReg();
      v << testqi{0xff00, numFields, sf};
      ifThenElse(v, CC_NZ, sf,
        [&](Vout& v) {
          auto const num_fields_times_9 = v.makeReg();
          v << lea{numFields[numFields * 8], num_fields_times_9};
          auto const num_fields_times_9_plus_1 = v.makeReg();
          v << incq {num_fields_times_9, num_fields_times_9_plus_1, v.makeReg()};
          auto const aligned_num_fields = v.makeReg();
          v << andqi {
            ~0x01, num_fields_times_9_plus_1, aligned_num_fields, v.makeReg()
          };
          auto const scaledSize = v.makeReg();
          v << shlqi {1, size, scaledSize, v.makeReg()};
          auto const positionsOffset = v.makeReg();
          v << addq{scaledSize, aligned_num_fields, positionsOffset, v.makeReg()};
          auto const smallSlot = v.makeReg();
          v << movtqw{rslot, smallSlot};
          v << storew{smallSlot, rarr[positionsOffset + sizeof(StructDict)]};
        },
        [&](Vout& v) {
          auto const smallSlot = v.makeReg();
          v << movtqb{rslot, smallSlot};
          auto const posBegin = v.makeReg();
          v << lea{numFields[numFields * 8 + sizeof(StructDict)], posBegin};
          auto const posOffset = v.makeReg();
          v << lea{posBegin[size], posOffset};
          v << storeb{smallSlot, rarr[posOffset]};
        }
      );
    } else {
      assertx(StructDict::numFieldsSize() == 1);
      auto const numFields = v.makeReg();
      v << loadzbq{rarr[StructDict::numFieldsOffset()], numFields};
      auto const positionsOffset = v.makeReg();
      v << addq{size, numFields, positionsOffset, v.makeReg()};
      auto const smallSlot = v.makeReg();
      v << movtqb{rslot, smallSlot};
      v << storeb{smallSlot, rarr[positionsOffset + sizeof(StructDict)]};
    }
  }

  auto const newSize = v.makeReg();
  auto const truncatedSize = v.makeReg();
  v << incq{size, newSize, v.makeReg()};
  v << movtql{newSize, truncatedSize};
  v << storel{truncatedSize, rarr[ArrayData::offsetofSize()]};
}

void cgStructDictUnset(IRLS& env, const IRInstruction* inst) {
  auto const arr = inst->src(0);
  auto const key = inst->src(1);
  auto const slot = getStructSlot(arr, key);

  if (!slot) return cgBespokeUnset(env, inst);

  auto& v = vmain(env);
  if (*slot == kInvalidSlot) {
    v << copy{srcLoc(env, inst, 0).reg(), dstLoc(env, inst, 0).reg()};
    return;
  }

  // Removing a required field escalates, so we don't JIT a fast path for it.
  auto const layout = arr->type().arrSpec().layout();
  auto const slayout = StructLayout::As(layout.bespokeLayout());
  if (slayout->field(*slot).required) return cgBespokeUnset(env, inst);

  auto const target = CallSpec::direct(StructDict::RemoveStrInSlot);
  auto const args = argGroup(env, inst).ssa(0).imm(*slot);
  cgCallHelper(v, env, target, callDest(env, inst), SyncOptions::None, args);
}

void cgStructDictSlotInPos(IRLS& env, const IRInstruction* inst) {
  auto const rarr = srcLoc(env, inst, 0).reg();
  auto const rpos = srcLoc(env, inst, 1).reg();
  auto const rdst = dstLoc(env, inst, 0).reg();

  auto& v = vmain(env);
  auto const arr = inst->src(0);
  auto const& layout = arr->type().arrSpec().layout();
  assertx(layout.is_struct());
  if (layout.bespokeLayout()->isConcrete()) {
    auto const slayout = StructLayout::As(layout.bespokeLayout());
    if (slayout->isBigStruct()) {
      v << loadzwq{rarr[rpos * 2 + slayout->positionOffset()], rdst};
    } else {
      v << loadzbq{rarr[rpos + slayout->positionOffset()], rdst};
    }
  } else {
    if constexpr (bespoke::detail_struct_data_layout::stores_utv) {
      assertx(StructDict::numFieldsSize() == 2);
      auto const numFields = v.makeReg();
      v << loadzwq{rarr[StructDict::numFieldsOffset()], numFields};
      static_assert(sizeof(UnalignedTypedValue) == 9);
      auto const sf = v.makeReg();
      v << testqi{0xff00, numFields, sf};
      cond(v, CC_NZ, sf, rdst,
        [&](Vout& v) {
          // iterator positions are aligned to 2 bytes, starting after typed values.
          // offset_past_tvs_plus_1 = numFields * 9 + sizeof(StructDict) + 1
          // aligned_pos_begin = offset_past_tvs_plus_1 & ~0x01;
          auto const offset_past_tvs_plus_1 = v.makeReg();
          v << lea{
            numFields[numFields * 8 + sizeof(StructDict) + 1],
            offset_past_tvs_plus_1
          };
          auto const aligned_pos_begin = v.makeReg();
          v << andqi {
            ~0x01, offset_past_tvs_plus_1, aligned_pos_begin, v.makeReg()
          };
          auto const pos_offset = v.makeReg();
          v << lea{aligned_pos_begin[rpos * 2], pos_offset};
          auto const slot = v.makeReg();
          v << loadzwq{rarr[pos_offset], slot};
          return slot;
        },
        [&](Vout& v) {
          auto const pos_begin = v.makeReg();
          v << lea{numFields[numFields * 8 + sizeof(StructDict)], pos_begin};
          auto const pos_offset = v.makeReg();
          v << lea{pos_begin[rpos], pos_offset};
          auto const slot = v.makeReg();
          v << loadzbq{rarr[pos_offset], slot};
          return slot;
        }
      );
    } else {
      assertx(StructDict::numFieldsSize() == 1);
      auto const numFields = v.makeReg();
      v << loadzbq{rarr[StructDict::numFieldsOffset()], numFields};
      auto const pos_offset = v.makeReg();
      v << lea{rpos[numFields + sizeof(StructDict)], pos_offset};
      v << loadzbq{rarr[pos_offset], rdst};
    }
  }
}

void cgLdStructDictKey(IRLS& env, const IRInstruction* inst) {
  auto& v = vmain(env);
  auto const rarr = srcLoc(env, inst, 0).reg();
  auto const rslot = srcLoc(env, inst, 1).reg();
  auto const rdst = dstLoc(env, inst, 0).reg();

  // Get a pointer to the bespoke layout for this array into a register.
  auto const layout = [&]{
    auto const layout = inst->src(0)->type().arrSpec().layout().bespokeLayout();
    if (layout->isConcrete()) return v.cns(uintptr_t(layout));

    auto const value = v.makeReg();
    auto const index = v.makeReg();
    auto const array = v.cns(bespoke::layoutsForJIT());
    v << loadzwq{rarr[ArrayData::offsetOfBespokeIndex()], index};
    v << load{array[index * safe_cast<int>(sizeof(bespoke::Layout*))], value};
    return value;
  }();

  auto constexpr offset = StructLayout::fieldsOffset() +
                          offsetof(StructLayout::Field, key);
  auto constexpr size = sizeof(StructLayout::Field);
  static_assert(!use_lowptr || size == 8);
  static_assert(use_lowptr || size == 16);
  if constexpr (use_lowptr) {
    v << loadzlq{layout[rslot * 8 + offset], rdst};
  } else {
    auto const rslot_scaled = v.makeReg();
    v << shlqi {4, rslot, rslot_scaled, v.makeReg()};
    v << load{layout[rslot_scaled + offset], rdst};
  }
}

void cgLdStructDictVal(IRLS& env, const IRInstruction* inst) {
  auto& v = vmain(env);
  auto const rarr = srcLoc(env, inst, 0).reg();
  auto const rslot = srcLoc(env, inst, 1).reg();
  auto const dst = dstLoc(env, inst, 0);

	if constexpr (bespoke::detail_struct_data_layout::stores_utv) {
    static_assert(sizeof(UnalignedTypedValue) == 9);
    auto const rslot_times_9 = v.makeReg();
    v << lea{rslot[rslot * 8], rslot_times_9};
    auto constexpr val_offset =
      sizeof(StructDict) + offsetof(UnalignedTypedValue, m_data);
    auto constexpr type_offset =
      sizeof(StructDict) + offsetof(UnalignedTypedValue, m_type);
    static_assert(type_offset == val_offset + 8);
    auto const val = rarr[rslot_times_9 + val_offset];
    auto const type = rarr[rslot_times_9 + type_offset];
    loadTV(v, inst->dst()->type(), dst, type, val);
  } else {
    auto const layout = inst->src(0)->type().arrSpec().layout().bespokeLayout();
    using DataLayout = bespoke::detail_struct_data_layout::TypePosValLayout;
    auto const val = [&]() -> Vptr {
      if (layout->isConcrete()) {
        auto const slayout = StructLayout::As(layout);
        auto const val_begin = slayout->valueOffsetForSlot(0);
        return rarr[rslot * safe_cast<int>(sizeof(Value)) + val_begin];
      } else {
        static_assert(DataLayout::valueOffsetSize() == 1);
        auto const val_begin = v.makeReg();
        v << loadzbq{rarr[DataLayout::valueOffsetOffset()], val_begin};
        auto const val_offset = v.makeReg();
        v << addq{val_begin, rslot, val_offset, v.makeReg()};
        return rarr[val_offset * safe_cast<int>(sizeof(Value))];
      }
    }();
    auto const type = rarr[rslot + DataLayout::staticTypeOffset()];
    loadTV(v, inst->dst()->type(), dst, type, val);
  }
}

//////////////////////////////////////////////////////////////////////////////

}
