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

#include "hphp/runtime/vm/jit/normalized-instruction.h"

#include "hphp/runtime/vm/jit/array-iter-profile.h"
#include "hphp/runtime/vm/jit/irgen-exit.h"
#include "hphp/runtime/vm/jit/irgen-control.h"
#include "hphp/runtime/vm/jit/irgen-internal.h"
#include "hphp/runtime/vm/jit/irgen-interpone.h"
#include "hphp/runtime/vm/jit/target-profile.h"

#include "hphp/util/struct-log.h"

namespace HPHP::jit::irgen {

//////////////////////////////////////////////////////////////////////

namespace {

//////////////////////////////////////////////////////////////////////

const StaticString s_ArrayIterProfile{"ArrayIterProfile"};

ArrayIterProfile::Result profileIterInit(IRGS& env, SSATmp* base) {
  auto const generic = ArrayIterProfile::Result{};
  if (base->isA(TObj)) return generic;
  assertx(base->type().subtypeOfAny(TVec, TDict, TKeyset));

  auto const profile = TargetProfile<ArrayIterProfile>(
    env.context,
    env.irb->curMarker(),
    s_ArrayIterProfile.get()
  );

  if (profile.profiling()) {
    gen(env, ProfileIterInit, RDSHandleData { profile.handle() }, base);
  }

  if (!profile.optimizing()) return generic;
  auto const result = profile.data().result();
  return result.value_type == TBottom ? generic : result;
}

// `doneOffset` is the absolute offset to jump to if the base has no elements.
// `result` is a TBool that is true if the iterator has more items.
void implIterInitJmp(IRGS& env, Offset doneOffset, SSATmp* result,
                     uint32_t iterId) {
  ifThen(
    env,
    [&] (Block* taken) {
      gen(env, JmpZero, taken, result);
    },
    [&] {
      // Empty iteration- the iterator is dead
      gen(env, KillIter, IterId{iterId}, fp(env));
      gen(env, Jmp, getBlock(env, doneOffset));
    }
  );
}

// `bodyOffset` is the absolute offset to jump to if the base has more elements.
// `result` is a TBool that is true if the iterator has more items.
void implIterNextJmp(IRGS& env, Offset bodyOffset, SSATmp* result,
                     uint32_t iterId) {
  if (bodyOffset <= bcOff(env)) {
    ifThen(env,
      [&](Block* taken) {
        gen(env, JmpNZero, taken, result);
      },
      [&]{
        surpriseCheckWithTarget(env, bodyOffset);
        gen(env, Jmp, getBlock(env, bodyOffset));
      }
    );
  } else {
    gen(env, JmpNZero, getBlock(env, bodyOffset), result);
  }

  // Fallthrough to next block the iterator is dead
  gen(env, KillIter, IterId{iterId}, fp(env));
}

// If the iterator base is an empty array-like, this method will generate
// trivial IR for the loop (just jump directly to done) and return true.
bool iterInitEmptyBase(IRGS& env, Offset doneOffset, SSATmp* base) {
  auto const empty = base->hasConstVal(TArrLike) && base->arrLikeVal()->empty();
  if (!empty) return false;

  gen(env, Jmp, getBlock(env, doneOffset));
  return true;
}

//////////////////////////////////////////////////////////////////////

}  // namespace

//////////////////////////////////////////////////////////////////////

void emitIterBase(IRGS& env) {
  auto const base = topC(env);
  if (base->type().subtypeOfAny(TArrLike)) return;
  if (!base->type().subtypeOfAny(TObj)) return interpOne(env);

  if (auto cls = base->type().clsSpec().cls()) {
    if (!env.irb->constrainValue(base, GuardConstraint(cls).setWeak())) {
      using CT = CollectionType;

      if (cls->classof(SystemLib::getHH_IteratorClass())) {
        // nothing to do
        return;
      } else if (collections::isType(cls,
                                     CT::Map, CT::ImmMap,
                                     CT::Set, CT::ImmSet)) {
        popC(env);
        pushIncRef(env, gen(env, LdColDict, base));
        decRef(env, base);
        return;
      } else if (collections::isType(cls, CT::Vector, CT::ImmVector)) {
        // Can't use ConstVector, as that includes Pair.
        popC(env);
        pushIncRef(env, gen(env, LdColVec, base));
        decRef(env, base);
        return;
      }
    }
  }

  auto const ctx = curClass(env) ? cns(env, curClass(env)) : cns(env, nullptr);
  auto const extracted = gen(env, IterExtractBase, base, ctx);
  popC(env);
  push(env, extracted);
  decRef(env, base);
}

namespace {

const StaticString s_current("current");
const StaticString s_key("key");

void emitIterObjGet(IRGS& env, SSATmp* base, const StringData* methodName) {
  assertx(base->isA(TObj));

  pushIncRef(env, base);
  push(env, cns(env, TUninit));
  updateStackOffset(env);

  auto const fca = FCallArgs(
    FCallArgsFlags::SkipRepack | FCallArgsFlags::EnforceMutableReturn,
    0, 1, nullptr, nullptr, kInvalidOffset, nullptr
  );
  auto const subop = ObjMethodOp::NullThrows;
  emitFCallObjMethodD(env, fca, staticEmptyString(), subop, methodName);
}

}

void emitIterGetKey(IRGS& env, IterArgs ita, int32_t baseLocalId) {
  auto const base = ldLoc(env, baseLocalId, DataTypeSpecific);
  assertx(base->type().subtypeOfAny(TVec, TDict, TKeyset, TObj));
  if (base->isA(TObj)) return emitIterObjGet(env, base, s_key.get());

  auto const pos = gen(env, LdIterPos, IterId(ita.iterId), fp(env));
  auto const baseConst = has_flag(ita.flags, IterArgs::Flags::BaseConst);

  if (!allowBespokeArrayLikes() || base->type().arrSpec().vanilla()) {
    if (base->isA(TVec)) {
      push(env, pos);
    } else if (base->isA(TDict)) {
      auto const elm = baseConst
        ? gen(env, IntAsPtrToElem, pos)
        : gen(env, GetDictPtrIter, base, pos);
      pushIncRef(env, gen(env, LdPtrIterKey, TInt | TStr, base, elm));
    } else {
      assertx(base->isA(TKeyset));
      auto const elm = baseConst
        ? gen(env, IntAsPtrToElem, pos)
        : gen(env, GetKeysetPtrIter, base, pos);
      pushIncRef(env, gen(env, LdPtrIterVal, TInt | TStr, base, elm));
    }
    return;
  }

  if (base->type().arrSpec().bespoke()) {
    pushIncRef(env, gen(env, BespokeIterGetKey, base, pos));
    return;
  }

  pushIncRef(env, gen(env, IterGetKeyArr, IterData(ita), base, pos));
}

void emitIterGetValue(IRGS& env, IterArgs ita, int32_t baseLocalId) {
  auto const base = ldLoc(env, baseLocalId, DataTypeSpecific);
  assertx(base->type().subtypeOfAny(TVec, TDict, TKeyset, TObj));
  if (base->isA(TObj)) return emitIterObjGet(env, base, s_current.get());

  auto const pos = gen(env, LdIterPos, IterId(ita.iterId), fp(env));
  auto const baseConst = has_flag(ita.flags, IterArgs::Flags::BaseConst);
  auto const withKeys = has_flag(ita.flags, IterArgs::Flags::WithKeys);

  if (!allowBespokeArrayLikes() || base->type().arrSpec().vanilla()) {
    if (base->isA(TVec)) {
      if (baseConst && !withKeys && VanillaVec::stores_unaligned_typed_values) {
        auto const elm = gen(env, IntAsPtrToElem, pos);
        pushIncRef(env, gen(env, LdPtrIterVal, TInitCell, base, elm));
      } else {
        pushIncRef(env, gen(env, LdVecElem, base, pos));
      }
    } else if (base->isA(TDict)) {
      auto const elm = baseConst
        ? gen(env, IntAsPtrToElem, pos)
        : gen(env, GetDictPtrIter, base, pos);
      pushIncRef(env, gen(env, LdPtrIterVal, TInitCell, base, elm));
    } else {
      assertx(base->isA(TKeyset));
      auto const elm = baseConst
        ? gen(env, IntAsPtrToElem, pos)
        : gen(env, GetKeysetPtrIter, base, pos);
      pushIncRef(env, gen(env, LdPtrIterVal, TInt | TStr, base, elm));
    }
    return;
  }

  if (base->type().arrSpec().bespoke()) {
    pushIncRef(env, gen(env, BespokeIterGetVal, base, pos));
    return;
  }

  pushIncRef(env, gen(env, IterGetValArr, IterData(ita), base, pos));
}

void emitIterInit(IRGS& env, IterArgs ita,
                   int32_t baseLocalId, Offset doneRelOffset) {
  auto const doneOffset = bcOff(env) + doneRelOffset;
  auto const base = ldLoc(env, baseLocalId, DataTypeSpecific);
  if (!base->type().subtypeOfAny(TVec, TDict, TKeyset, TObj)) PUNT(IterInit);
  if (iterInitEmptyBase(env, doneOffset, base)) return;
  auto const profiledResult = profileIterInit(env, base);
  auto const done =
    specializeIterInit(env, doneOffset, ita, base, baseLocalId, profiledResult);
  if (done) return;

  auto const op = base->isA(TArrLike) ? IterInitArr : IterInitObj;
  auto const result = gen(env, op, IterData(ita), base, fp(env));
  implIterInitJmp(env, doneOffset, result, ita.iterId);
}

void emitIterNext(IRGS& env, IterArgs ita,
                   int32_t baseLocalId, Offset bodyRelOffset) {
  auto const bodyOffset = bcOff(env) + bodyRelOffset;
  auto const base = ldLoc(env, baseLocalId, DataTypeSpecific);
  if (!base->type().subtypeOfAny(TVec, TDict, TKeyset, TObj)) PUNT(IterNext);

  if (specializeIterNext(env, bodyOffset, ita, base, baseLocalId)) return;

  auto const op = base->isA(TArrLike) ? IterNextArr : IterNextObj;
  auto const result = gen(env, op, IterData(ita), base, fp(env));
  implIterNextJmp(env, bodyOffset, result, ita.iterId);
}

void emitIterFree(IRGS& env, int32_t iterId) {
  gen(env, KillIter, IterId(iterId), fp(env));
}

//////////////////////////////////////////////////////////////////////

}
