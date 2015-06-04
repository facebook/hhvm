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
#include "hphp/runtime/vm/jit/irgen-ret.h"

#include "hphp/runtime/vm/jit/mc-generator.h"

#include "hphp/runtime/vm/jit/irgen.h"
#include "hphp/runtime/vm/jit/irgen-exit.h"
#include "hphp/runtime/vm/jit/irgen-inlining.h"

#include "hphp/runtime/vm/jit/irgen-internal.h"

namespace HPHP { namespace jit { namespace irgen {

namespace {

//////////////////////////////////////////////////////////////////////

const StaticString s_returnHook("SurpriseReturnHook");

void retSurpriseCheck(IRGS& env, SSATmp* retVal) {
  /*
   * This is a weird situation for throwing: we've partially torn down the
   * ActRec (decref'd all the frame's locals), and we've popped the return
   * value.  If we throw, the unwinder needs to know the return value is not on
   * the eval stack, so we need to sync the marker after the pop---the return
   * value will be decref'd by the return hook while unwinding, in this case.
   */
  updateMarker(env);
  env.irb->exceptionStackBoundary();

  ifThen(
    env,
    [&] (Block* taken) {
      auto const ptr = resumed(env) ? sp(env) : fp(env);
      gen(env, CheckSurpriseFlags, taken, ptr);
    },
    [&] {
      hint(env, Block::Hint::Unlikely);
      ringbufferMsg(env, Trace::RBTypeMsg, s_returnHook.get());
      gen(env, ReturnHook, fp(env), retVal);
    }
  );
  ringbufferMsg(env, Trace::RBTypeFuncExit, curFunc(env)->fullName());
}

void freeLocalsAndThis(IRGS& env) {
  auto const localCount = curFunc(env)->numLocals();

  auto const shouldFreeInline = [&]() -> bool {
    // In a pseudomain, we have to do a non-inline DecRef, because we can't
    // side-exit in the middle of the sequence of LdLocPseudoMains.
    if (curFunc(env)->isPseudoMain()) return false;
    // We don't want to specialize on arg types for builtins
    if (curFunc(env)->builtinFuncPtr()) return false;

    auto const count = mcg->numTranslations(
      env.irb->unit().context().srcKey());
    constexpr int kTooPolyRet = 6;
    if (localCount > 0 && count > kTooPolyRet) return false;
    auto numRefCounted = int{0};
    for (auto i = uint32_t{0}; i < localCount; ++i) {
      if (env.irb->localType(i, DataTypeGeneric).maybe(TCounted)) {
        ++numRefCounted;
      }
    }
    return numRefCounted <= RuntimeOption::EvalHHIRInliningMaxReturnDecRefs;
  }();

  if (shouldFreeInline) {
    decRefLocalsInline(env);
    for (unsigned i = 0; i < localCount; ++i) {
      env.irb->constrainLocal(i, DataTypeCountness, "inlined RetC/V");
    }
  } else {
    gen(env, GenericRetDecRefs, fp(env));
  }

  decRefThis(env);
}

void normalReturn(IRGS& env, SSATmp* retval) {
  gen(env, StRetVal, fp(env), retval);
  auto const data = RetCtrlData { offsetToReturnSlot(env), false };
  gen(env, RetCtrl, data, sp(env), fp(env));
}

void asyncFunctionReturn(IRGS& env, SSATmp* retval) {
  if (!resumed(env)) {
    // Return from an eagerly-executed async function: wrap the return value in
    // a StaticWaitHandle object and return that normally.
    auto const wrapped = gen(env, CreateSSWH, retval);
    normalReturn(env, wrapped);
    return;
  }

  auto parentChain = gen(env, LdAsyncArParentChain, fp(env));
  gen(env, StAsyncArSucceeded, fp(env));
  gen(env, StAsyncArResult, fp(env), retval);
  gen(env, ABCUnblock, parentChain);

  spillStack(env);

  // Must load this before FreeActRec, which adjusts fp(env).
  auto const resumableObj = gen(env, LdResumableArObj, fp(env));

  gen(env, FreeActRec, fp(env));
  gen(env, DecRef, resumableObj);

  gen(
    env,
    AsyncRetCtrl,
    IRSPOffsetData { offsetFromIRSP(env, BCSPOffset{0}) },
    sp(env),
    fp(env)
  );
}

void generatorReturn(IRGS& env, SSATmp* retval) {
  // Clear generator's key and value.
  auto const oldKey = gen(env, LdContArKey, TCell, fp(env));
  gen(env, StContArKey, fp(env), cns(env, TInitNull));
  gen(env, DecRef, oldKey);

  auto const oldValue = gen(env, LdContArValue, TCell, fp(env));
  gen(env, StContArValue, fp(env), cns(env, TInitNull));
  gen(env, DecRef, oldValue);

  gen(env,
      StContArState,
      GeneratorState { BaseGenerator::State::Done },
      fp(env));

  // Push return value of next()/send()/raise().
  push(env, cns(env, TInitNull));

  spillStack(env);
  gen(
    env,
    RetCtrl,
    RetCtrlData { offsetFromIRSP(env, BCSPOffset{0}), true },
    sp(env),
    fp(env)
  );
}

void implRet(IRGS& env) {
  auto func = curFunc(env);

  // Pop the return value. Since it will be teleported to its place in memory,
  // we don't care about the type.
  auto const retval = pop(env, DataTypeGeneric);

  if (func->attrs() & AttrMayUseVV) {
    ifElse(
      env,
      [&] (Block* skip) {
        gen(env, ReleaseVVAndSkip, skip, fp(env));
      },
      [&] { freeLocalsAndThis(env); }
    );
  } else {
    freeLocalsAndThis(env);
  }

  retSurpriseCheck(env, retval);

  if (func->isAsyncFunction()) {
    return asyncFunctionReturn(env, retval);
  }
  if (resumed(env)) {
    assertx(curFunc(env)->isNonAsyncGenerator());
    return generatorReturn(env, retval);
  }
  return normalReturn(env, retval);
}

//////////////////////////////////////////////////////////////////////

}

IRSPOffset offsetToReturnSlot(IRGS& env) {
  return offsetFromIRSP(
    env,
    BCSPOffset{
      logicalStackDepth(env).offset +
        AROFF(m_r) / int32_t{sizeof(Cell)}
    }
  );
}

void emitRetC(IRGS& env) {
  if (curFunc(env)->isAsyncGenerator()) PUNT(RetC-AsyncGenerator);

  if (isInlining(env)) {
    assertx(!resumed(env));
    retFromInlined(env);
  } else {
    implRet(env);
  }
}

void emitRetV(IRGS& env) {
  assertx(!resumed(env));
  assertx(!curFunc(env)->isResumable());
  if (isInlining(env)) {
    retFromInlined(env);
  } else {
    implRet(env);
  }
}

//////////////////////////////////////////////////////////////////////

}}}
