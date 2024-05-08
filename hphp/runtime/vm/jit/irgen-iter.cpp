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

// `doneOffset` is the relative offset to jump to if the base has no elements.
// `result` is a TBool that is true if the iterator has more items.
void implIterInitJmp(IRGS& env, Offset doneOffset, SSATmp* result,
                     uint32_t iterId) {
  auto const targetOffset = bcOff(env) + doneOffset;
  auto const target = getBlock(env, targetOffset);
  assertx(target != nullptr);
  ifThen(
    env,
    [&] (Block* taken) {
      gen(env, JmpZero, taken, result);
    },
    [&] {
      // Empty iteration- the iterator is dead
      gen(env, KillIter, IterId{iterId}, fp(env));
      gen(env, Jmp, target);
    }
  );
}

// `loopOffset` is the relative offset to jump to if the base has more elements.
// `result` is a TBool that is true if the iterator has more items.
void implIterNextJmp(IRGS& env, Offset loopOffset, SSATmp* result,
                     uint32_t iterId) {
  auto const targetOffset = bcOff(env) + loopOffset;
  auto const target = getBlock(env, targetOffset);
  assertx(target != nullptr);

  if (loopOffset <= 0) {
    ifThen(env,
      [&](Block* taken) {
        gen(env, JmpNZero, taken, result);
      },
      [&]{
        surpriseCheckWithTarget(env, targetOffset);
        gen(env, Jmp, target);
      }
    );
  } else {
    gen(env, JmpNZero, target, result);
  }

  // Fallthrough to next block the iterator is dead
  gen(env, KillIter, IterId{iterId}, fp(env));
}

// If the iterator base is an empty array-like, this method will generate
// trivial IR for the loop (just jump directly to done) and return true.
bool iterInitEmptyBase(IRGS& env, Offset doneOffset, SSATmp* base) {
  auto const empty = base->hasConstVal(TArrLike) && base->arrLikeVal()->empty();
  if (!empty) return false;

  auto const targetOffset = bcOff(env) + doneOffset;
  auto const target = getBlock(env, targetOffset);
  assertx(target != nullptr);
  gen(env, Jmp, target);
  return true;
}

// For profiling tracelets, we deliberately widen the types of locals used
// as a local iterator base to make it easier to merge tracelets later.
//
// This pessimization only affects region formation. We still track types
// precisely when we optimize the IRUnit.
void widenLocalIterBase(IRGS& env, int baseLocalId) {
  if (env.context.kind != TransKind::Profile) return;
  env.irb->fs().clearForUnprocessedPred();
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

void emitLIterInit(IRGS& env, IterArgs ita,
                   int32_t baseLocalId, Offset doneOffset) {
  auto const base = ldLoc(env, baseLocalId, DataTypeSpecific);
  if (!base->type().subtypeOfAny(TVec, TDict, TKeyset, TObj)) PUNT(LIterInit);
  if (iterInitEmptyBase(env, doneOffset, base)) return;
  auto const profiledResult = profileIterInit(env, base);
  specializeIterInit(env, doneOffset, ita, base, baseLocalId, profiledResult);

  auto const op = base->isA(TArrLike)
    ? (ita.hasKey() ? LIterInitArrK : LIterInitArr)
    : (ita.hasKey() ? LIterInitObjK : LIterInitObj);
  auto const data = IterData(ita);
  auto const result = gen(env, op, data, base, fp(env));
  widenLocalIterBase(env, baseLocalId);
  implIterInitJmp(env, doneOffset, result, ita.iterId);
}

void emitLIterNext(IRGS& env, IterArgs ita,
                   int32_t baseLocalId, Offset loopOffset) {
  auto const base = ldLoc(env, baseLocalId, DataTypeSpecific);
  if (!base->type().subtypeOfAny(TVec, TDict, TKeyset, TObj)) PUNT(LIterNext);

  if (specializeIterNext(env, loopOffset, ita, base, baseLocalId)) return;

  auto const op = base->isA(TArrLike)
    ? ita.hasKey() ? LIterNextArrK : LIterNextArr
    : ita.hasKey() ? LIterNextObjK : LIterNextObj;
  auto const result = gen(env, op, IterData(ita), base, fp(env));
  widenLocalIterBase(env, baseLocalId);
  implIterNextJmp(env, loopOffset, result, ita.iterId);
}

void emitLIterFree(IRGS& env, int32_t iterId) {
  gen(env, KillIter, IterId(iterId), fp(env));
}

//////////////////////////////////////////////////////////////////////

}
