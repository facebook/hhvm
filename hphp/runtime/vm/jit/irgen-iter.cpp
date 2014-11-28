/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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
 * This function returns the offset of instruction i's branch target.
 * This is normally the offset corresponding to the branch being
 * taken.  However, if i does not break a trace and it's followed in
 * the trace by the instruction in the taken branch, then this
 * function returns the offset of the i's fall-through instruction.
 * In that case, the invertCond output argument is set to true;
 * otherwise it's set to false.
 */
Offset iterBranchTarget(const NormalizedInstruction& i, bool& invertCond) {
  assert(instrJumpOffset(reinterpret_cast<const Op*>(i.pc())) != nullptr);
  auto targetOffset = i.offset() + i.imm[1].u_BA;
  invertCond = false;
  if (!i.endsRegion && i.nextOffset == targetOffset) {
    invertCond = true;
    targetOffset = i.offset() + instrLen((Op*)i.pc());
  }
  return targetOffset;
}

template<class Lambda>
void implMIterInit(HTS& env, Offset relOffset, Lambda genFunc) {
  // TODO MIterInit doesn't check iterBranchTarget; this might be bug ...

  auto const exit  = makeExit(env);
  auto const stack = spillStack(env);
  env.irb->exceptionStackBoundary();
  auto const pred  = getStackInnerTypePrediction(stack, 0);
  auto const src   = topV(env);

  if (!pred.subtypeOfAny(Type::Arr, Type::Obj)) {
    PUNT(MIterInit-unsupportedSrcType);
  }

  // Guard the inner type before we call the helper.
  gen(env, CheckRefInner, pred, exit, src);

  auto const res = genFunc(src, pred);
  auto const out = popV(env);
  gen(env, DecRef, out);
  implCondJmp(env, bcOff(env) + relOffset, true, res);
}

//////////////////////////////////////////////////////////////////////

}

void emitIterInit(HTS& env,
                  int32_t iterId,
                  Offset relOffset,
                  int32_t valLocalId) {
  bool invertCond = false;
  auto const targetOffset = iterBranchTarget(*env.currentNormalizedInstruction,
                                             invertCond);
  auto const src = popC(env);
  if (!src->type().subtypeOfAny(Type::Arr, Type::Obj)) PUNT(IterInit);
  auto const res = gen(
    env,
    IterInit,
    Type::Bool,
    IterData(iterId, -1, valLocalId),
    src,
    fp(env)
  );
  implCondJmp(env, targetOffset, !invertCond, res);
}

void emitIterInitK(HTS& env,
                   int32_t iterId,
                   Offset relOffset,
                   int32_t valLocalId,
                   int32_t keyLocalId) {
  bool invertCond = false;
  auto const targetOffset = iterBranchTarget(*env.currentNormalizedInstruction,
                                             invertCond);

  auto const src = popC(env);
  if (!src->type().subtypeOfAny(Type::Arr, Type::Obj)) PUNT(IterInitK);
  auto const res = gen(
    env,
    IterInitK,
    Type::Bool,
    IterData(iterId, keyLocalId, valLocalId),
    src,
    fp(env)
  );
  implCondJmp(env, targetOffset, !invertCond, res);
}

void emitIterNext(HTS& env,
                  int32_t iterId,
                  Offset relOffset,
                  int32_t valLocalId) {
  bool invertCond = false;
  auto const targetOffset = iterBranchTarget(*env.currentNormalizedInstruction,
                                             invertCond);
  auto const res = gen(
    env,
    IterNext,
    Type::Bool,
    IterData(iterId, -1, valLocalId),
    fp(env)
  );
  implCondJmp(env, targetOffset, invertCond, res);
}

void emitIterNextK(HTS& env,
                   int32_t iterId,
                   Offset relOffset,
                   int32_t valLocalId,
                   int32_t keyLocalId) {
  bool invertCond = false;
  auto const targetOffset = iterBranchTarget(*env.currentNormalizedInstruction,
                                             invertCond);
  auto const res = gen(
    env,
    IterNextK,
    Type::Bool,
    IterData(iterId, keyLocalId, valLocalId),
    fp(env)
  );
  implCondJmp(env, targetOffset, invertCond, res);
}

void emitWIterInit(HTS& env,
                   int32_t iterId,
                   Offset relOffset,
                   int32_t valLocalId) {
  bool invertCond = false;
  auto const targetOffset = iterBranchTarget(*env.currentNormalizedInstruction,
                                             invertCond);
  auto const src = popC(env);
  if (!src->type().subtypeOfAny(Type::Arr, Type::Obj)) PUNT(WIterInit);
  auto const res = gen(
    env,
    WIterInit,
    Type::Bool,
    IterData(iterId, -1, valLocalId),
    src,
    fp(env)
  );
  implCondJmp(env, targetOffset, !invertCond, res);
}

void emitWIterInitK(HTS& env,
                    int32_t iterId,
                    Offset relOffset,
                    int32_t valLocalId,
                    int32_t keyLocalId) {
  bool invertCond = false;
  auto const targetOffset = iterBranchTarget(*env.currentNormalizedInstruction,
                                             invertCond);
  auto const src = popC(env);
  if (!src->type().subtypeOfAny(Type::Arr, Type::Obj)) PUNT(WIterInitK);
  auto const res = gen(
    env,
    WIterInitK,
    Type::Bool,
    IterData(iterId, keyLocalId, valLocalId),
    src,
    fp(env)
  );
  implCondJmp(env, targetOffset, !invertCond, res);
}

void emitWIterNext(HTS& env,
                   int32_t iterId,
                   Offset relOffset,
                   int32_t valLocalId) {
  bool invertCond = false;
  auto const targetOffset = iterBranchTarget(*env.currentNormalizedInstruction,
                                             invertCond);
  auto const res = gen(
    env,
    WIterNext,
    Type::Bool,
    IterData(iterId, -1, valLocalId),
    fp(env)
  );
  implCondJmp(env, targetOffset, invertCond, res);
}

void emitWIterNextK(HTS& env,
                    int32_t iterId,
                    Offset relOffset,
                    int32_t valLocalId,
                    int32_t keyLocalId) {
  bool invertCond = false;
  auto const targetOffset = iterBranchTarget(*env.currentNormalizedInstruction,
                                             invertCond);
  auto const res = gen(
    env,
    WIterNextK,
    Type::Bool,
    IterData(iterId, keyLocalId, valLocalId),
    fp(env)
  );
  implCondJmp(env, targetOffset, invertCond, res);
}

void emitMIterInit(HTS& env,
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

void emitMIterInitK(HTS& env,
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

void emitMIterNext(HTS& env,
                   int32_t iterId,
                   Offset relOffset,
                   int32_t valLocalId) {
  auto const res = gen(
    env,
    MIterNext,
    Type::Bool,
    IterData(iterId, -1, valLocalId),
    fp(env)
  );
  implCondJmp(env, bcOff(env) + relOffset, false, res);
}

void emitMIterNextK(HTS& env,
                    int32_t iterId,
                    Offset relOffset,
                    int32_t valLocalId,
                    int32_t keyLocalId) {
  auto const res = gen(
    env,
    MIterNextK,
    Type::Bool,
    IterData(iterId, keyLocalId, valLocalId),
    fp(env)
  );
  implCondJmp(env, bcOff(env) + relOffset, false, res);
}

void emitIterFree(HTS& env, int32_t iterId) {
  gen(env, IterFree, IterId(iterId), fp(env));
}

void emitMIterFree(HTS& env, int32_t iterId) {
  gen(env, MIterFree, IterId(iterId), fp(env));
}

void emitIterBreak(HTS& env,
                   const ImmVector& iv,
                   Offset relOffset) {
  always_assert(env.currentNormalizedInstruction->endsRegion);

  for (int iterIndex = 0; iterIndex < iv.size(); iterIndex += 2) {
    IterKind iterKind = (IterKind)iv.vec32()[iterIndex];
    Id       iterId   = iv.vec32()[iterIndex + 1];
    switch (iterKind) {
    case KindOfIter:  gen(env, IterFree,  IterId(iterId), fp(env)); break;
    case KindOfMIter: gen(env, MIterFree, IterId(iterId), fp(env)); break;
    case KindOfCIter: gen(env, CIterFree, IterId(iterId), fp(env)); break;
    }
  }

  // Would need to change this if we support not ending regions on this:
  gen(env, Jmp, makeExit(env, bcOff(env) + relOffset));
}

void emitDecodeCufIter(HTS& env, int32_t iterId, Offset relOffset) {
  auto const src        = popC(env);
  auto const type       = src->type();
  if (type.subtypeOfAny(Type::Arr, Type::Str, Type::Obj)) {
    auto const res = gen(
      env,
      DecodeCufIter,
      Type::Bool,
      IterId(iterId),
      src,
      fp(env)
    );
    gen(env, DecRef, src);
    implCondJmp(env, bcOff(env) + relOffset, true, res);
  } else {
    gen(env, DecRef, src);
    jmpImpl(env,
            bcOff(env) + relOffset,
            instrJmpFlags(*env.currentNormalizedInstruction));
  }
}

void emitCIterFree(HTS& env, int32_t iterId) {
  gen(env, CIterFree, IterId(iterId), fp(env));
}

//////////////////////////////////////////////////////////////////////

}}}

