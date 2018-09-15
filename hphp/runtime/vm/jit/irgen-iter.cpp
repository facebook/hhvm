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
  assertx(instrJumpOffset(i.pc()) != kInvalidOffset);
  switch (i.op()) {
    case OpIterInit:
    case OpIterInitK:
    case OpIterNext:
    case OpIterNextK:
    case OpWIterInit:
    case OpWIterInitK:
    case OpWIterNext:
    case OpWIterNextK:
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

template<class Lambda>
void implMIterInit(IRGS& env, Offset relOffset, Lambda genFunc) {
  // TODO MIterInit doesn't check iterBranchTarget; this might be bug ...

  auto const exit  = makeExit(env);
  auto const pred  = env.irb->predictedStackInnerType(spOffBCFromIRSP(env));
  auto const src   = topV(env);

  if (!pred.subtypeOfAny(TArrLike, TObj)) {
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
    IterData(iterId, uint32_t(-1), valLocalId),
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

  surpriseCheck(env, relOffset);
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
  implCondJmp(env, targetOffset, false, res);
}

void emitLIterNextK(IRGS& env,
                    int32_t iterId,
                    int32_t baseLocalId,
                    Offset relOffset,
                    int32_t valLocalId,
                    int32_t keyLocalId) {
  if (curFunc(env)->isPseudoMain()) PUNT(LIterNextK-pseudomain);

  surpriseCheck(env, relOffset);
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
  implCondJmp(env, targetOffset, false, res);
}

void emitWIterInit(IRGS& env, int32_t iterId, Offset /*relOffset*/,
                   int32_t valLocalId) {
  auto const targetOffset = iterBranchTarget(*env.currentNormalizedInstruction);
  auto const src = popC(env);
  if (!src->type().subtypeOfAny(TArrLike, TObj)) PUNT(WIterInit);
  auto const res = gen(
    env,
    WIterInit,
    TBool,
    IterInitData(iterId, uint32_t(-1), valLocalId, true),
    src,
    fp(env)
  );
  implCondJmp(env, targetOffset, true, res);
}

void emitWIterInitK(IRGS& env, int32_t iterId, Offset /*relOffset*/,
                    int32_t valLocalId, int32_t keyLocalId) {
  auto const targetOffset = iterBranchTarget(*env.currentNormalizedInstruction);
  auto const src = popC(env);
  if (!src->type().subtypeOfAny(TArrLike, TObj)) PUNT(WIterInitK);
  auto const res = gen(
    env,
    WIterInitK,
    TBool,
    IterInitData(iterId, keyLocalId, valLocalId, true),
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
    IterData(iterId, uint32_t(-1), valLocalId),
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
      IterInitData(iterId, uint32_t(-1), valLocalId, true),
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
      IterInitData(iterId, keyLocalId, valLocalId, true),
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
    IterData(iterId, uint32_t(-1), valLocalId),
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
    case KindOfMIter: emitMIterFree(env, ent.id); break;
    case KindOfLIter: emitLIterFree(env, ent.id, ent.local); break;
    case KindOfCIter: emitCIterFree(env, ent.id); break;
    }
  }

  jmpImpl(env, bcOff(env) + relOffset);
}

void emitDecodeCufIter(IRGS& env, int32_t iterId, Offset relOffset) {
  auto const src        = topC(env);
  auto const type       = src->type();

  if (type <= TObj) {
    auto const slowExit = makeExitSlow(env);
    auto const cls      = gen(env, LdObjClass, src);
    auto const func     = gen(env, LdObjInvoke, slowExit, cls);
    gen(env, StCufIterFunc, IterId(iterId), fp(env), func);
    gen(env, StCufIterCtx, IterId(iterId), fp(env), src);
    gen(env, StCufIterInvName, IterId(iterId), fp(env), cns(env, TNullptr));
    gen(env, StCufIterDynamic, IterId(iterId), fp(env), cns(env, false));
    discard(env, 1);
    return;
  }

  if (type.subtypeOfAny(TArr, TVec, TStr, TFunc)) {
    // Do this first, because DecodeCufIter will do a sanity check on the flag.
    auto const isDynamic = !(type <= TFunc);
    gen(env, StCufIterDynamic, IterId(iterId), fp(env), cns(env, isDynamic));
    auto const res = gen(
      env,
      DecodeCufIter,
      TBool,
      IterId(iterId),
      src,
      fp(env)
    );
    discard(env, 1);
    decRef(env, src);
    implCondJmp(env, bcOff(env) + relOffset, true, res);
  } else {
    discard(env, 1);
    decRef(env, src);
    jmpImpl(env, bcOff(env) + relOffset);
  }
}

void emitCIterFree(IRGS& env, int32_t iterId) {
  auto const ctx = gen(
    env,
    LdCufIterCtx,
    TCtx | TNullptr,
    IterId(iterId),
    fp(env)
  );
  auto const invName = gen(
    env,
    LdCufIterInvName,
    TStr | TNullptr,
    IterId(iterId),
    fp(env)
  );
  ifNonNull(env, ctx, [&](SSATmp* t) { decRef(env, t); });
  ifNonNull(env, invName, [&](SSATmp* t) { decRef(env, t); });
  gen(env, KillCufIter, IterId(iterId), fp(env));
}

//////////////////////////////////////////////////////////////////////

}}}
