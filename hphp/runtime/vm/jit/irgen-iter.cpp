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

#include "hphp/runtime/vm/jit/irgen-exit.h"
#include "hphp/runtime/vm/jit/irgen-control.h"
#include "hphp/runtime/vm/jit/irgen-internal.h"

namespace HPHP { namespace jit { namespace irgen {

namespace {

//////////////////////////////////////////////////////////////////////

/*
 * This function returns the offset of instruction i's branch target,
 * which is the offset corresponding to the branch being taken.
 */
Offset iterBranchTarget(const NormalizedInstruction& i) {
  assertx(instrJumpOffsets(i.pc()).size() == 1);
  switch (i.op()) {
    case OpIterInit:
    case OpIterInitK:
    case OpIterNext:
    case OpIterNextK:
      return i.offset() + i.imm[1].u_BA;
    case OpLIterInit:
    case OpLIterInitK:
    case OpLIterNext:
    case OpLIterNextK:
      return i.offset() + i.imm[2].u_BA;
    default:
      always_assert(false);
  }
}

//////////////////////////////////////////////////////////////////////

}

void emitIterInit(IRGS& env, int32_t iterId, Offset /*relOffset*/,
                  int32_t valLocalId) {
  auto const targetOffset = iterBranchTarget(*env.currentNormalizedInstruction);
  auto const src = popC(env);
  if (!src->type().subtypeOfAny(TArrLike, TObj)) PUNT(IterInit);
  auto const res = gen(
    env,
    IterInit,
    TBool,
    IterInitData(iterId, uint32_t(-1), valLocalId, true),
    src,
    fp(env)
  );
  implCondJmp(env, targetOffset, true, res);
}

void emitIterInitK(IRGS& env, int32_t iterId, Offset /*relOffset*/,
                   int32_t valLocalId, int32_t keyLocalId) {
  auto const targetOffset = iterBranchTarget(*env.currentNormalizedInstruction);
  auto const src = popC(env);
  if (!src->type().subtypeOfAny(TArrLike, TObj)) PUNT(IterInitK);
  auto const res = gen(
    env,
    IterInitK,
    TBool,
    IterInitData(iterId, keyLocalId, valLocalId, true),
    src,
    fp(env)
  );
  implCondJmp(env, targetOffset, true, res);
}

static void implIterJmp(
  IRGS& env, Offset relOffset, Offset targetOffset, SSATmp* src) {
  auto const target = getBlock(env, targetOffset);
  assertx(target != nullptr);
  auto const boolSrc = gen(env, ConvCellToBool, src);
  decRef(env, src);

  if (relOffset <= 0) {
    ifThen(env,
      [&] (Block* taken) {
        gen(env, JmpNZero, taken, boolSrc);
      },
      [&] {
        surpriseCheckWithTarget(env, targetOffset);
        gen(env, Jmp, target);
      }
    );
  } else {
    gen(env, JmpNZero, target, boolSrc);
    return;
  }
}

void emitIterNext(IRGS& env,
                  int32_t iterId,
                  Offset relOffset,
                  int32_t valLocalId) {
  auto const targetOffset = iterBranchTarget(*env.currentNormalizedInstruction);
  auto const res = gen(
    env,
    IterNext,
    TBool,
    IterData(iterId, uint32_t(-1), valLocalId),
    fp(env)
  );
  implIterJmp(env, relOffset, targetOffset, res);
}

void emitIterNextK(IRGS& env,
                   int32_t iterId,
                   Offset relOffset,
                   int32_t valLocalId,
                   int32_t keyLocalId) {
  auto const targetOffset = iterBranchTarget(*env.currentNormalizedInstruction);
  auto const res = gen(
    env,
    IterNextK,
    TBool,
    IterData(iterId, keyLocalId, valLocalId),
    fp(env)
  );
  implIterJmp(env, relOffset, targetOffset, res);
}

void emitLIterInit(IRGS& env,
                   int32_t iterId,
                   int32_t baseLocalId,
                   Offset relOffset,
                   int32_t valLocalId) {
  if (curFunc(env)->isPseudoMain()) PUNT(LIterInit-pseudomain);

  auto const targetOffset = iterBranchTarget(*env.currentNormalizedInstruction);

  auto const base = ldLoc(env, baseLocalId, nullptr, DataTypeSpecific);
  if (!base->type().subtypeOfAny(TArrLike, TObj)) PUNT(LIterInit);

  if (base->isA(TObj)) gen(env, IncRef, base);
  auto const res = gen(
    env,
    base->isA(TArrLike) ? LIterInit : IterInit,
    TBool,
    IterInitData(iterId, uint32_t(-1), valLocalId, false),
    base,
    fp(env)
  );
  implCondJmp(env, targetOffset, true, res);
}

void emitLIterInitK(IRGS& env,
                    int32_t iterId,
                    int32_t baseLocalId,
                    Offset relOffset,
                    int32_t valLocalId,
                    int32_t keyLocalId) {
  if (curFunc(env)->isPseudoMain()) PUNT(LIterInitK-pseudomain);

  auto const targetOffset = iterBranchTarget(*env.currentNormalizedInstruction);

  auto const base = ldLoc(env, baseLocalId, nullptr, DataTypeSpecific);
  if (!base->type().subtypeOfAny(TArrLike, TObj)) PUNT(LIterInitK);

  if (base->isA(TObj)) gen(env, IncRef, base);
  auto const res = gen(
    env,
    base->isA(TArrLike) ? LIterInitK : IterInitK,
    TBool,
    IterInitData(iterId, keyLocalId, valLocalId, false),
    base,
    fp(env)
  );
  implCondJmp(env, targetOffset, true, res);
}

void emitLIterNext(IRGS& env,
                   int32_t iterId,
                   int32_t baseLocalId,
                   Offset relOffset,
                   int32_t valLocalId) {
  if (curFunc(env)->isPseudoMain()) PUNT(LIterNext-pseudomain);

  auto const targetOffset = iterBranchTarget(*env.currentNormalizedInstruction);

  auto const base = ldLoc(env, baseLocalId, nullptr, DataTypeSpecific);
  auto const res = [&]{
    if (base->isA(TArrLike)) {
      return gen(
        env,
        LIterNext,
        TBool,
        IterData(iterId, uint32_t(-1), valLocalId),
        base,
        fp(env)
      );
    } else {
      return gen(
        env,
        IterNext,
        TBool,
        IterData(iterId, uint32_t(-1), valLocalId),
        fp(env)
      );
    };
  }();
  implIterJmp(env, relOffset, targetOffset, res);
}

void emitLIterNextK(IRGS& env,
                    int32_t iterId,
                    int32_t baseLocalId,
                    Offset relOffset,
                    int32_t valLocalId,
                    int32_t keyLocalId) {
  if (curFunc(env)->isPseudoMain()) PUNT(LIterNextK-pseudomain);

  auto const targetOffset = iterBranchTarget(*env.currentNormalizedInstruction);

  auto const base = ldLoc(env, baseLocalId, nullptr, DataTypeSpecific);
  auto const res = [&]{
    if (base->isA(TArrLike)) {
      return gen(
        env,
        LIterNextK,
        TBool,
        IterData(iterId, keyLocalId, valLocalId),
        base,
        fp(env)
      );
    } else {
      return gen(
        env,
        IterNextK,
        TBool,
        IterData(iterId, keyLocalId, valLocalId),
        fp(env)
      );
    };
  }();
  implIterJmp(env, relOffset, targetOffset, res);
}

void emitIterFree(IRGS& env, int32_t iterId) {
  gen(env, IterFree, IterId(iterId), fp(env));
}

void emitLIterFree(IRGS& env, int32_t iterId, int32_t baseLocalId) {
  auto const baseType = env.irb->local(baseLocalId, DataTypeSpecific).type;
  // If the base is a known array, there's nothing to actually free in the
  // iterator. However, in debug builds we get some extra sanity checking if we
  // call the free function anyways.
  if (baseType <= TArrLike && !debug) return;
  gen(env, IterFree, IterId(iterId), fp(env));
}

void emitIterBreak(IRGS& env, Offset relOffset, const IterTable& it) {
  for (auto const& ent : it) {
    switch (ent.kind) {
    case KindOfIter:  emitIterFree(env, ent.id);  break;
    case KindOfLIter: emitLIterFree(env, ent.id, ent.local); break;
    }
  }

  jmpImpl(env, bcOff(env) + relOffset);
}

//////////////////////////////////////////////////////////////////////

}}}
