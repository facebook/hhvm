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

#include "hphp/runtime/base/bespoke/escalation-logging.h"
#include "hphp/runtime/base/bespoke/layout.h"
#include "hphp/runtime/base/bespoke/logging-profile.h"
#include "hphp/runtime/base/bespoke/monotype-dict.h"
#include "hphp/runtime/base/bespoke/monotype-vec.h"
#include "hphp/runtime/base/bespoke/struct-dict.h"
#include "hphp/runtime/base/mixed-array.h"
#include "hphp/runtime/base/packed-array.h"
#include "hphp/runtime/base/set-array.h"

#include "hphp/runtime/vm/jit/code-gen-cf.h"
#include "hphp/runtime/vm/jit/code-gen-helpers.h"
#include "hphp/runtime/vm/jit/irlower.h"
#include "hphp/runtime/vm/jit/irlower-internal.h"
#include "hphp/runtime/vm/jit/ir-opcode.h"
#include "hphp/runtime/vm/jit/minstr-helpers.h"
#include "hphp/runtime/vm/jit/translator-inline.h"

namespace HPHP { namespace jit { namespace irlower {

//////////////////////////////////////////////////////////////////////////////
// Generic BespokeArrays

namespace {
static void logGuardFailure(TypedValue tv, uint16_t layout, uint64_t sk) {
  assertx(tvIsArrayLike(tv));
  auto const al = ArrayLayout::FromUint16(layout);
  bespoke::logGuardFailure(val(tv).parr, al, SrcKey(sk));
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
    if (layout.vanilla()) {                                         \
      if (arr <= TVec)    return CallSpec::direct(PackedArray::Fn); \
      if (arr <= TDict)   return CallSpec::direct(MixedArray::Fn);  \
      if (arr <= TKeyset) return CallSpec::direct(SetArray::Fn);    \
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
    if (layout.vanilla()) {                                                  \
      if (arr <= TVec) {                                                     \
        return CallSpec::direct(SynthesizedArrayFunctions<PackedArray>::Fn); \
      }                                                                      \
      if (arr <= TDict) {                                                    \
        return CallSpec::direct(SynthesizedArrayFunctions<MixedArray>::Fn);  \
      }                                                                      \
      if (arr <= TKeyset) {                                                  \
        return CallSpec::direct(SynthesizedArrayFunctions<SetArray>::Fn);    \
      }                                                                      \
    }                                                                        \
    return Generic;                                                          \
  }()


CallSpec destructorForArrayLike(Type arr) {
  assertx(arr <= TArrLike);
  assertx(allowBespokeArrayLikes());
  return CALL_TARGET(arr, Release, CallSpec::method(&ArrayData::release));
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

  auto& v = vmain(env);
  auto const args = argGroup(env, inst).ssa(0).ssa(1);
  cgCallHelper(v, env, target, callDestTV(env, inst), SyncOptions::Sync, args);
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

void cgBespokeIterEnd(IRLS& env, const IRInstruction* inst) {
  auto const arr = inst->src(0)->type();
  auto const iterEnd = CallSpec::method(&ArrayData::iter_end);
  auto const target = CALL_TARGET(arr, IterEnd, iterEnd);

  auto& v = vmain(env);
  auto const args = argGroup(env, inst).ssa(0);
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
    auto const layout = inst->src(0)->type().arrSpec().layout();
    auto const vtable = layout.bespokeLayout()->vtable();
    if (vtable->fnEscalateToVanilla) {
      return CallSpec::direct(vtable->fnEscalateToVanilla);
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
    args.ssa(3);
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

  auto const slayout = bespoke::StructLayout::As(layout.bespokeLayout());
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
    .imm(layout->extraInitializer());

  cgCallHelper(v, env, target, callDest(env, inst), SyncOptions::None, args);
}

void cgInitStructPositions(IRLS& env, const IRInstruction* inst) {
  auto const arr = inst->src(0);
  auto const rarr = srcLoc(env, inst, 0).reg();
  auto const extra = inst->extra<InitStructPositions>();
  auto& v = vmain(env);

  v << storel{v.cns(extra->numSlots), rarr[ArrayData::offsetofSize()]};

  auto const layout = arr->type().arrSpec().layout();
  auto const slayout = bespoke::StructLayout::As(layout.bespokeLayout());

  auto constexpr kSlotsPerStore = 8;
  auto const size = extra->numSlots;
  auto const padBytes = slayout->positionOffset() & 0x7;
  for (auto i = 0; i < size + padBytes; i += kSlotsPerStore) {
    uint64_t slots = 0;
    for (auto j = 0; j < kSlotsPerStore; j++) {
      // The type array comes before the positions array, and the types should
      // stay initialized with KindOfUninit. Later positions can be zeroed.
      auto const index = static_cast<int32_t>(i + j - padBytes);
      auto const slot = [&]{
        if (index < 0) return static_cast<uint8_t>(KindOfUninit);
        if (index < size) return safe_cast<uint8_t>(extra->slots[index]);
        return static_cast<uint8_t>(0);
      }();
      slots = slots | (safe_cast<uint64_t>(slot) << (j * 8));
    }
    auto const offset = slayout->positionOffset() + i - padBytes;
    assertx((offset % kSlotsPerStore) == 0);
    v << store{v.cns(slots), rarr[offset]};
  }
}

void cgInitStructElem(IRLS& env, const IRInstruction* inst) {
  auto const arr = inst->src(0);
  auto const layout = arr->type().arrSpec().layout();
  auto const slayout = bespoke::StructLayout::As(layout.bespokeLayout());
  auto const rarr = srcLoc(env, inst, 0).reg();
  auto const slot = inst->extra<InitStructElem>()->index;
  auto const type = rarr[slayout->typeOffsetForSlot(slot)];
  auto const data = rarr[slayout->valueOffsetForSlot(slot)];
  storeTV(vmain(env), inst->src(1)->type(), srcLoc(env, inst, 1), type, data);
}

void cgNewBespokeStructDict(IRLS& env, const IRInstruction* inst) {
  auto const sp = srcLoc(env, inst, 0).reg();
  auto const extra = inst->extra<NewBespokeStructDict>();
  auto& v = vmain(env);

  auto const n = static_cast<size_t>((extra->numSlots + 7) & ~7);
  auto const slots = reinterpret_cast<uint8_t*>(v.allocData<uint64_t>(n / 8));
  for (auto i = 0; i < extra->numSlots; i++) {
    slots[i] = safe_cast<uint8_t>(extra->slots[i]);
  }
  for (auto i = extra->numSlots; i < n; i++) {
    slots[i] = static_cast<uint8_t>(KindOfUninit);
  }

  auto const layout = StructLayout::As(extra->layout.bespokeLayout());
  auto const target = CallSpec::direct(StructDict::MakeStructDict);
  auto const args = argGroup(env, inst)
    .imm(layout->sizeIndex())
    .imm(layout->extraInitializer())
    .imm(extra->numSlots)
    .dataPtr(slots)
    .addr(sp, cellsToBytes(extra->offset.offset));

  cgCallHelper(v, env, target, callDest(env, inst), SyncOptions::None, args);
}

void cgLdStructDictElem(IRLS& env, const IRInstruction* inst) {
  auto const arr = inst->src(0);
  auto const key = inst->src(1);
  auto const slot = getStructSlot(arr, key);

  if (!slot) return cgBespokeGet(env, inst);

  if (*slot == kInvalidSlot) {
    always_assert(inst->dst()->isA(TUninit));
    return;
  }

  auto const layout = arr->type().arrSpec().layout();
  auto const slayout = bespoke::StructLayout::As(layout.bespokeLayout());

  auto const rarr = srcLoc(env, inst, 0).reg();
  auto const type = rarr[slayout->typeOffsetForSlot(*slot)];
  auto const data = rarr[slayout->valueOffsetForSlot(*slot)];
  loadTV(vmain(env), inst->dst()->type(), dstLoc(env, inst, 0), type, data);
}

void cgStructDictGetWithColor(IRLS& env, const IRInstruction* inst) {
  auto const arr = inst->src(0);
  auto const key = inst->src(1);
  auto const dst = dstLoc(env, inst, 0);

  auto const rarr = srcLoc(env, inst, 0).reg();
  auto const rkey = srcLoc(env, inst, 1).reg();

  auto& v = vmain(env);

  auto const makeNonstaticCall =
    [&](Vout& v, const Vreg& value, const Vreg& type) {
      auto const target =
        CallSpec::direct(bespoke::StructDict::NvGetStrNonStatic);
      auto const args = argGroup(env, inst).ssa(0).ssa(1);
      cgCallHelper(v, env, target, callDest(value, type), SyncOptions::Sync,
                   args);
    };

  // Is the key certain to be non-static? If so, invoke the non-static helper.
  if (!key->type().maybe(TStaticStr)) {
    makeNonstaticCall(v, dst.reg(0), dst.reg(1));
    return;
  }

  // 1) Read the string's color. This may be junk if the string is non-static.
  auto constexpr layoutMask = StructLayout::kMaxColor;
  auto constexpr hashEntrySize =
    sizeof(bespoke::StructLayout::PerfectHashEntry);
  auto const getColor = [&] {
    auto const colorMasked = [&] {
      static_assert(folly::isPowTwo(layoutMask + 1));
      static_assert(layoutMask <= std::numeric_limits<uint8_t>::max());
      auto const colorPremask = v.makeReg();
      v << loadzbq{rkey[StringData::colorOffset()], colorPremask};
      if (layoutMask == std::numeric_limits<uint8_t>::max()) {
        return colorPremask;
      } else {
        auto const colorMasked = v.makeReg();
        v << andqi{uint8_t(layoutMask), colorPremask, colorMasked, v.makeReg()};
        return colorMasked;
      }
    }();

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

  auto constexpr strHashOffset =
          offsetof(bespoke::StructLayout::PerfectHashEntry, str);
  auto constexpr valHashOffset =
          offsetof(bespoke::StructLayout::PerfectHashEntry, valueOffset);
  auto constexpr typeHashOffset =
          offsetof(bespoke::StructLayout::PerfectHashEntry, typeOffset);

  // 2) Obtain the addresses of the string, value offset, and type offset in
  // the perfect hash table.
  auto const tuple = [&] {
    auto const layout = arr->type().arrSpec().layout();
    assertx(layout.is_struct());

    // 2a) If the layout is known, we can obtain its perfect hash table address
    // statically.
    if (layout.bespokeLayout()->isConcrete()) {
      auto const hashTable = (uintptr_t)
        bespoke::StructLayout::hashTableForLayout(layout.bespokeLayout());
      auto const base = v.cns(hashTable);

      auto const entryWithOffset = [&]() -> std::function<Vptr(size_t)> {
        if (key->hasConstVal(TStr)) {
          return [&](size_t off) {
            auto const offWithColor =
              (key->strVal()->color() & layoutMask) * hashEntrySize + off;
            return base[offWithColor];
          };
        }

        auto const color = getColor();
        return [=](size_t off) { return base[color * 8 + off]; };
      }();

      return std::make_tuple(
        entryWithOffset(strHashOffset),
        entryWithOffset(valHashOffset),
        entryWithOffset(typeHashOffset)
      );
    }

    // 2b) Otherwise, we'll have to index into the table set to find the
    // layout's perfect hash table.
    auto const hashTableSet = (uintptr_t) bespoke::StructLayout::hashTableSet();
    auto const layoutIndex = v.makeReg();
    v << loadzwq{rarr[ArrayData::offsetOfBespokeIndex()], layoutIndex};

    auto constexpr hashTableSize =
      sizeof(bespoke::StructLayout::PerfectHashTable);
    static_assert(folly::isPowTwo(hashTableSize));
    auto const layoutShift =
      safe_cast<uint8_t>(folly::findLastSet(hashTableSize) - 1);
    auto const hashTableOffset = v.makeReg();

    v << shlqi{layoutShift, layoutIndex, hashTableOffset, v.makeReg()};

    auto const entryWithOffset = [&]() -> std::function<Vptr(size_t)> {
      if (key->hasConstVal(TStr)) {
        return [&](size_t off) {
          auto const offWithColor =
            (key->strVal()->color() & layoutMask) * hashEntrySize + off;
          return hashTableOffset[v.cns(hashTableSet) + offWithColor];
        };
      }

      auto const base = v.makeReg();
      v << lea{hashTableOffset[v.cns(hashTableSet)], base};
      auto const color = getColor();
      return [=](size_t off) { return base[color * 8 + off]; };
    }();

    return std::make_tuple(
      entryWithOffset(strHashOffset),
      entryWithOffset(valHashOffset),
      entryWithOffset(typeHashOffset)
    );
  }();

  auto const strPtr = std::get<0>(tuple);
  auto const valPtr = std::get<1>(tuple);
  auto const typePtr = std::get<2>(tuple);

  // 3) Retrieve the string pointer from the perfect hash table.
  auto const hashStr = v.makeReg();
  emitLdLowPtr(v, strPtr, hashStr, sizeof(LowStringPtr));

  // 4) Check if the string pointer matches.
  auto const sf = v.makeReg();
  v << cmpq{rkey, hashStr, sf};
  cond(v, v, CC_E, sf, v.makeTuple({dst.reg(0), dst.reg(1)}),
    [&] (Vout& v) {
      // 4a) The string matched! Load the value and type offsets out of the
      // perfect hash table, and use them to index into the struct.
      auto const valueOff = v.makeReg();
      auto const typeOff = v.makeReg();
      v << loadzwq{valPtr, valueOff};
      v << loadzwq{typePtr, typeOff};
      auto const value = v.makeReg();
      auto const type = v.makeReg();
      v << load{rarr[valueOff], value};
      v << loadb{rarr[typeOff], type};
      return v.makeTuple({value, type});
    },
    [&] (Vout& v) {
      // 4b) The string did not match. If the key is static, then we know the
      // field is absent. If the key is non-static, we fall back to the
      // non-static lookup routine.
      if (key->type() <= TStaticStr) {
        return v.makeTuple({v.cns(0), v.cns(KindOfUninit)});
      }

      auto const value = v.makeReg();
      auto const type = v.makeReg();
      auto const sf = emitCmpRefCount(v, StaticValue, rkey);
      cond(v, v, CC_E, sf, v.makeTuple({value, type}),
        [&] (Vout& v) {
          return v.makeTuple({v.cns(0), v.cns(KindOfUninit)});
        },
        [&] (Vout& v) {
          auto const callValue = v.makeReg();
          auto const callType = v.makeReg();
          makeNonstaticCall(v, callValue, callType);
          return v.makeTuple({callValue, callType});
        },
        StringTag{}
      );
      return v.makeTuple({value, type});
    },
    StringTag{}
  );
}

void cgStructDictSet(IRLS& env, const IRInstruction* inst) {
  auto const arr = inst->src(0);
  auto const key = inst->src(1);
  auto const slot = getStructSlot(arr, key);

  if (!slot || (*slot == kInvalidSlot)) return cgBespokeSet(env, inst);

  auto& v = vmain(env);
  auto const target = CallSpec::direct(bespoke::StructDict::SetStrInSlot);
  auto const args = argGroup(env, inst).ssa(0).imm(*slot).typedValue(2);
  cgCallHelper(v, env, target, callDest(env, inst), SyncOptions::Sync, args);
}

//////////////////////////////////////////////////////////////////////////////

}}}
