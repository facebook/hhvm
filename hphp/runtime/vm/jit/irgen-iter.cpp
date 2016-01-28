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
  assertx(instrJumpOffset(i.pc()) != nullptr);
  return i.offset() + i.imm[1].u_BA;
}

template<class Lambda>
void implMIterInit(IRGS& env, Offset relOffset, Lambda genFunc) {
  // TODO MIterInit doesn't check iterBranchTarget; this might be bug ...

  auto const exit  = makeExit(env);
  auto const pred  = env.irb->predictedStackInnerType(
    offsetFromIRSP(env, BCSPOffset{0}));
  auto const src   = topV(env);

  if (!pred.subtypeOfAny(TArr, TObj)) {
    PUNT(MIterInit-unsupportedSrcType);
  }

  // Guard the inner type before we call the helper.
  gen(env, CheckRefInner, pred, exit, src);

  auto const res = genFunc(src, pred);
  auto const out = popV(env);
  decRef(env, out);
  implCondJmp(env, bcOff(env) + relOffset, true, res);
}

//////////////////////////////////////////////////////////////////////

}

void emitIterInit(IRGS& env,
                  int32_t iterId,
                  Offset relOffset,
                  int32_t valLocalId) {
  auto const targetOffset = iterBranchTarget(*env.currentNormalizedInstruction);
  auto const src = popC(env);
  if (!src->type().subtypeOfAny(TArr, TObj)) PUNT(IterInit);
  auto const res = gen(
    env,
    IterInit,
    TBool,
    IterData(iterId, -1, valLocalId),
    src,
    fp(env)
  );
  implCondJmp(env, targetOffset, true, res);
}

void emitIterInitK(IRGS& env,
                   int32_t iterId,
                   Offset relOffset,
                   int32_t valLocalId,
                   int32_t keyLocalId) {
  auto const targetOffset = iterBranchTarget(*env.currentNormalizedInstruction);
  auto const src = popC(env);
  if (!src->type().subtypeOfAny(TArr, TObj)) PUNT(IterInitK);
  auto const res = gen(
    env,
    IterInitK,
    TBool,
    IterData(iterId, keyLocalId, valLocalId),
    src,
    fp(env)
  );
  implCondJmp(env, targetOffset, true, res);
}

void emitIterNext(IRGS& env,
                  int32_t iterId,
                  Offset relOffset,
                  int32_t valLocalId) {
  surpriseCheck(env, relOffset);
  auto const targetOffset = iterBranchTarget(*env.currentNormalizedInstruction);
  auto const res = gen(
    env,
    IterNext,
    TBool,
    IterData(iterId, -1, valLocalId),
    fp(env)
  );
  implCondJmp(env, targetOffset, false, res);
}

void emitIterNextK(IRGS& env,
                   int32_t iterId,
                   Offset relOffset,
                   int32_t valLocalId,
                   int32_t keyLocalId) {
  surpriseCheck(env, relOffset);
  auto const targetOffset = iterBranchTarget(*env.currentNormalizedInstruction);
  auto const res = gen(
    env,
    IterNextK,
    TBool,
    IterData(iterId, keyLocalId, valLocalId),
    fp(env)
  );
  implCondJmp(env, targetOffset, false, res);
}

void emitWIterInit(IRGS& env,
                   int32_t iterId,
                   Offset relOffset,
                   int32_t valLocalId) {
  auto const targetOffset = iterBranchTarget(*env.currentNormalizedInstruction);
  auto const src = popC(env);
  if (!src->type().subtypeOfAny(TArr, TObj)) PUNT(WIterInit);
  auto const res = gen(
    env,
    WIterInit,
    TBool,
    IterData(iterId, -1, valLocalId),
    src,
    fp(env)
  );
  implCondJmp(env, targetOffset, true, res);
}

void emitWIterInitK(IRGS& env,
                    int32_t iterId,
                    Offset relOffset,
                    int32_t valLocalId,
                    int32_t keyLocalId) {
  auto const targetOffset = iterBranchTarget(*env.currentNormalizedInstruction);
  auto const src = popC(env);
  if (!src->type().subtypeOfAny(TArr, TObj)) PUNT(WIterInitK);
  auto const res = gen(
    env,
    WIterInitK,
    TBool,
    IterData(iterId, keyLocalId, valLocalId),
    src,
    fp(env)
  );
  implCondJmp(env, targetOffset, true, res);
}

void emitWIterNext(IRGS& env,
                   int32_t iterId,
                   Offset relOffset,
                   int32_t valLocalId) {
  surpriseCheck(env, relOffset);
  auto const targetOffset = iterBranchTarget(*env.currentNormalizedInstruction);
  auto const res = gen(
    env,
    WIterNext,
    TBool,
    IterData(iterId, -1, valLocalId),
    fp(env)
  );
  implCondJmp(env, targetOffset, false, res);
}

void emitWIterNextK(IRGS& env,
                    int32_t iterId,
                    Offset relOffset,
                    int32_t valLocalId,
                    int32_t keyLocalId) {
  surpriseCheck(env, relOffset);
  auto const targetOffset = iterBranchTarget(*env.currentNormalizedInstruction);
  auto const res = gen(
    env,
    WIterNextK,
    TBool,
    IterData(iterId, keyLocalId, valLocalId),
    fp(env)
  );
  implCondJmp(env, targetOffset, false, res);
}

void emitMIterInit(IRGS& env,
                   int32_t iterId,
                   Offset relOffset,
                   int32_t valLocalId) {
  implMIterInit(env, relOffset, [&] (SSATmp* src, Type type) {
    return gen(
      env,
      MIterInit,
      type,
      IterData(iterId, -1, valLocalId),
      src,
      fp(env)
    );
  });
}

void emitMIterInitK(IRGS& env,
                    int32_t iterId,
                    Offset relOffset,
                    int32_t valLocalId,
                    int32_t keyLocalId) {
  implMIterInit(env, relOffset, [&] (SSATmp* src, Type type) {
    return gen(
      env,
      MIterInitK,
      type,
      IterData(iterId, keyLocalId, valLocalId),
      src,
      fp(env)
    );
  });
}

void emitMIterNext(IRGS& env,
                   int32_t iterId,
                   Offset relOffset,
                   int32_t valLocalId) {
  surpriseCheck(env, relOffset);
  auto const res = gen(
    env,
    MIterNext,
    TBool,
    IterData(iterId, -1, valLocalId),
    fp(env)
  );
  implCondJmp(env, bcOff(env) + relOffset, false, res);
}

void emitMIterNextK(IRGS& env,
                    int32_t iterId,
                    Offset relOffset,
                    int32_t valLocalId,
                    int32_t keyLocalId) {
  surpriseCheck(env, relOffset);
  auto const res = gen(
    env,
    MIterNextK,
    TBool,
    IterData(iterId, keyLocalId, valLocalId),
    fp(env)
  );
  implCondJmp(env, bcOff(env) + relOffset, false, res);
}

void emitIterFree(IRGS& env, int32_t iterId) {
  gen(env, IterFree, IterId(iterId), fp(env));
}

void emitMIterFree(IRGS& env, int32_t iterId) {
  gen(env, MIterFree, IterId(iterId), fp(env));
}

void emitIterBreak(IRGS& env,
                   const ImmVector& iv,
                   Offset relOffset) {
  for (int iterIndex = 0; iterIndex < iv.size(); iterIndex += 2) {
    IterKind iterKind = (IterKind)iv.vec32()[iterIndex];
    Id       iterId   = iv.vec32()[iterIndex + 1];
    switch (iterKind) {
    case KindOfIter:  gen(env, IterFree,  IterId(iterId), fp(env)); break;
    case KindOfMIter: gen(env, MIterFree, IterId(iterId), fp(env)); break;
    case KindOfCIter: gen(env, CIterFree, IterId(iterId), fp(env)); break;
    }
  }

  jmpImpl(env, bcOff(env) + relOffset);
}

void emitDecodeCufIter(IRGS& env, int32_t iterId, Offset relOffset) {
  auto const src        = popC(env);
  auto const type       = src->type();
  if (type.subtypeOfAny(TArr, TStr, TObj)) {
    auto const res = gen(
      env,
      DecodeCufIter,
      TBool,
      IterId(iterId),
      src,
      fp(env)
    );
    decRef(env, src);
    implCondJmp(env, bcOff(env) + relOffset, true, res);
  } else {
    decRef(env, src);
    jmpImpl(env, bcOff(env) + relOffset);
  }
}

void emitCIterFree(IRGS& env, int32_t iterId) {
  gen(env, CIterFree, IterId(iterId), fp(env));
}

//////////////////////////////////////////////////////////////////////

}}}
