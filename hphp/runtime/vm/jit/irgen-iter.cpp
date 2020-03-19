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
#include "hphp/runtime/vm/jit/target-profile.h"

#include "hphp/util/struct-log.h"

namespace HPHP { namespace jit { namespace irgen {

//////////////////////////////////////////////////////////////////////

namespace {

//////////////////////////////////////////////////////////////////////

// `doneOffset` is the relative offset to jump to if the base has no elements.
// `result` is a TBool that is true if the iterator has more items.
void implIterInitJmp(IRGS& env, Offset doneOffset, SSATmp* result) {
  auto const targetOffset = bcOff(env) + doneOffset;
  auto const target = getBlock(env, targetOffset);
  assertx(target != nullptr);
  gen(env, JmpZero, target, result);
}

// `loopOffset` is the relative offset to jump to if the base has more elements.
// `result` is a TBool that is true if the iterator has more items.
void implIterNextJmp(IRGS& env, Offset loopOffset, SSATmp* result) {
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
}

// If the iterator base is an empty array-like, this method will generate
// trivial IR for the loop (just jump directly to done) and return true.
bool iterInitEmptyBase(IRGS& env, Offset doneOffset, SSATmp* base, bool local) {
  auto const empty = base->hasConstVal(TArrLike) && base->arrLikeVal()->empty();
  if (!empty) return false;

  // NOTE: `base` is static, so we can skip the dec-ref for non-local iters.
  if (!local) discard(env, 1);
  auto const targetOffset = bcOff(env) + doneOffset;
  auto const target = getBlock(env, targetOffset);
  assertx(target != nullptr);
  gen(env, Jmp, target);
  return true;
}

//////////////////////////////////////////////////////////////////////

}  // namespace

//////////////////////////////////////////////////////////////////////

void emitIterInit(IRGS& env, IterArgs ita, Offset doneOffset) {
  auto const base = topC(env);
  if (!base->type().subtypeOfAny(TArrLike, TObj)) PUNT(IterInit);
  if (iterInitEmptyBase(env, doneOffset, base, false)) return;
  specializeIterInit(env, doneOffset, ita, kInvalidId);

  discard(env, 1);
  updateMarker(env);
  env.irb->exceptionStackBoundary();

  auto const op = ita.hasKey() ? IterInitK : IterInit;
  auto const data = IterData(ita);
  auto const result = gen(env, op, data, base, fp(env));
  implIterInitJmp(env, doneOffset, result);
}

void emitIterNext(IRGS& env, IterArgs ita, Offset loopOffset) {
  if (specializeIterNext(env, loopOffset, ita, kInvalidId)) return;

  auto const op = ita.hasKey() ? IterNextK : IterNext;
  auto const result = gen(env, op, IterData(ita), fp(env));
  implIterNextJmp(env, loopOffset, result);
}

void emitLIterInit(IRGS& env, IterArgs ita,
                   int32_t baseLocalId, Offset doneOffset) {
  if (curFunc(env)->isPseudoMain()) PUNT(LIterInit-pseudomain);
  auto const base = ldLoc(env, baseLocalId, nullptr, DataTypeSpecific);
  if (!base->type().subtypeOfAny(TArrLike, TObj)) PUNT(LIterInit);
  if (iterInitEmptyBase(env, doneOffset, base, true)) return;
  specializeIterInit(env, doneOffset, ita, baseLocalId);

  if (base->isA(TObj)) gen(env, IncRef, base);
  auto const op = base->isA(TArrLike)
    ? (ita.hasKey() ? LIterInitK : LIterInit)
    : (ita.hasKey() ? IterInitK : IterInit);
  auto const data = IterData(ita);
  auto const result = gen(env, op, data, base, fp(env));
  implIterInitJmp(env, doneOffset, result);
}

void emitLIterNext(IRGS& env, IterArgs ita,
                   int32_t baseLocalId, Offset loopOffset) {
  if (curFunc(env)->isPseudoMain()) PUNT(LIterNext-pseudomain);
  if (specializeIterNext(env, loopOffset, ita, baseLocalId)) return;

  auto const base = ldLoc(env, baseLocalId, nullptr, DataTypeSpecific);
  auto const result = [&]{
    if (base->isA(TArrLike)) {
      auto const op = ita.hasKey() ? LIterNextK : LIterNext;
      return gen(env, op, IterData(ita), base, fp(env));
    }
    auto const op = ita.hasKey() ? IterNextK : IterNext;
    return gen(env, op, IterData(ita), fp(env));
  }();
  implIterNextJmp(env, loopOffset, result);
}

void emitIterFree(IRGS& env, int32_t iterId) {
  gen(env, IterFree, IterId(iterId), fp(env));
  gen(env, KillIter, IterId(iterId), fp(env));
}

void emitLIterFree(IRGS& env, int32_t iterId, int32_t baseLocalId) {
  auto const baseType = env.irb->local(baseLocalId, DataTypeSpecific).type;
  if (!(baseType <= TArrLike)) gen(env, IterFree, IterId(iterId), fp(env));
  gen(env, KillIter, IterId(iterId), fp(env));
}

//////////////////////////////////////////////////////////////////////

}}}
