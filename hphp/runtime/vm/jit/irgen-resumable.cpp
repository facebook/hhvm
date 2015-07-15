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

#include "hphp/runtime/ext/asio/ext_wait-handle.h"
#include "hphp/runtime/ext/asio/ext_async-function-wait-handle.h"
#include "hphp/runtime/ext/asio/ext_async-generator.h"
#include "hphp/runtime/ext/generator/ext_generator.h"
#include "hphp/runtime/base/repo-auth-type-codec.h"

#include "hphp/runtime/vm/jit/irgen-exit.h"
#include "hphp/runtime/vm/jit/irgen-ret.h"
#include "hphp/runtime/vm/jit/irgen-types.h"

#include "hphp/runtime/vm/jit/irgen-internal.h"

namespace HPHP { namespace jit { namespace irgen {

namespace {

//////////////////////////////////////////////////////////////////////

void suspendHookE(IRGS& env,
                  SSATmp* frame,
                  SSATmp* resumableAR,
                  SSATmp* resumable) {
  ringbufferMsg(env, Trace::RBTypeFuncExit, curFunc(env)->fullName());
  ifThen(
    env,
    [&] (Block* taken) {
      gen(env, CheckSurpriseFlags, taken, fp(env));
    },
    [&] {
      hint(env, Block::Hint::Unlikely);
      gen(env, SuspendHookE, frame, resumableAR, resumable);
    }
  );
}

void suspendHookR(IRGS& env, SSATmp* frame, SSATmp* objOrNullptr) {
  ringbufferMsg(env, Trace::RBTypeFuncExit, curFunc(env)->fullName());
  ifThen(
    env,
    [&] (Block* taken) {
      // Check using sp(env) in the -R version---remember that fp(env) does not
      // point into the eval stack.
      gen(env, CheckSurpriseFlags, taken, sp(env));
    },
    [&] {
      hint(env, Block::Hint::Unlikely);
      gen(env, SuspendHookR, frame, objOrNullptr);
    }
  );
}

void implAwaitE(IRGS& env, SSATmp* child, Offset resumeOffset, int numIters) {
  assertx(curFunc(env)->isAsync());
  assertx(!resumed(env));
  assertx(child->type() <= TObj);

  // Create the AsyncFunctionWaitHandle object. CreateAFWH takes care of
  // copying local variables and iterators.
  auto const func = curFunc(env);
  auto const resumeSk = SrcKey(func, resumeOffset, true);
  auto const bind_data = LdBindAddrData { resumeSk, invSPOff(env) };
  auto const resumeAddr = gen(env, LdBindAddr, bind_data);
  auto const waitHandle =
    gen(env,
        func->attrs() & AttrMayUseVV ? CreateAFWH : CreateAFWHNoVV,
        fp(env),
        cns(env, func->numSlotsInFrame()),
        resumeAddr,
        cns(env, resumeOffset),
        child);

  auto const asyncAR = gen(env, LdAFWHActRec, waitHandle);

  // Call the FunctionSuspend hook.  We need put to a null on the stack in the
  // catch trace in place of our input, since we've already shuffled that value
  // into the heap to be owned by the waitHandle, so the unwinder can't decref
  // it.
  push(env, cns(env, TInitNull));
  env.irb->exceptionStackBoundary();
  suspendHookE(env, fp(env), asyncAR, waitHandle);
  discard(env, 1);

  // store the return value and return control to the caller.
  gen(env, StRetVal, fp(env), waitHandle);
  auto const ret_data = RetCtrlData { offsetToReturnSlot(env), false };
  gen(env, RetCtrl, ret_data, sp(env), fp(env));
}

void implAwaitR(IRGS& env, SSATmp* child, Offset resumeOffset) {
  assertx(curFunc(env)->isAsync());
  assertx(resumed(env));
  assertx(child->isA(TObj));

  // We must do this before we do anything, because it can throw, and we can't
  // start tearing down the AFWH before that or the unwinder won't be able to
  // react.
  suspendHookR(env, fp(env), child);

  // Prepare child for establishing dependency.
  gen(env, AFWHPrepareChild, fp(env), child);

  // Suspend the async function.
  auto const resumeSk = SrcKey(curFunc(env), resumeOffset, true);
  auto const data = LdBindAddrData { resumeSk, invSPOff(env) };
  auto const resumeAddr = gen(env, LdBindAddr, data);
  gen(env, StAsyncArResume, ResumeOffset { resumeOffset }, fp(env),
    resumeAddr);

  // Set up the dependency.
  gen(env, AFWHBlockOn, fp(env), child);

  spillStack(env);
  auto const stack    = sp(env);
  auto const frame    = fp(env);
  auto const spAdjust = offsetFromIRSP(env, BCSPOffset{0});
  gen(env, RetCtrl, RetCtrlData(spAdjust, true), stack, frame);
}

void yieldReturnControl(IRGS& env) {
  // Push return value of next()/send()/raise().
  push(env, cns(env, TInitNull));

  spillStack(env);
  auto const stack    = sp(env);
  auto const frame    = fp(env);
  auto const spAdjust = offsetFromIRSP(env, BCSPOffset{0});
  gen(env, RetCtrl, RetCtrlData { spAdjust, true }, stack, frame);
}

void yieldImpl(IRGS& env, Offset resumeOffset) {
  suspendHookR(env, fp(env), cns(env, TNullptr));

  // Resumable::setResumeAddr(resumeAddr, resumeOffset)
  auto const resumeSk = SrcKey(curFunc(env), resumeOffset, true);
  auto const data = LdBindAddrData { resumeSk, invSPOff(env) };
  auto const resumeAddr = gen(env, LdBindAddr, data);
  gen(env, StContArResume, ResumeOffset { resumeOffset }, fp(env), resumeAddr);

  // Set yielded value.
  auto const oldValue = gen(env, LdContArValue, TCell, fp(env));
  gen(env, StContArValue, fp(env),
    popC(env, DataTypeGeneric)); // teleporting value
  gen(env, DecRef, oldValue);

  // Set state from Running to Started.
  gen(env, StContArState,
      GeneratorState { BaseGenerator::State::Started },
      fp(env));
}

//////////////////////////////////////////////////////////////////////

}

void emitWHResult(IRGS& env) {
  assertx(topC(env)->isA(TObj));
  auto const exitSlow = makeExitSlow(env);
  auto const child = popC(env);
  // In most conditions, this will be optimized out by the simplifier.
  // We already need to setup a side-exit for the !succeeded case.
  gen(env, JmpZero, exitSlow, gen(env, IsWaitHandle, child));
  static_assert(
    c_WaitHandle::STATE_SUCCEEDED == 0,
    "we test state for non-zero, success must be zero"
  );
  gen(env, JmpNZero, exitSlow, gen(env, LdWHState, child));
  auto const res = gen(env, LdWHResult, TInitCell, child);
  gen(env, IncRef, res);
  gen(env, DecRef, child);
  push(env, res);
}

void emitAwait(IRGS& env, int32_t numIters) {
  auto const resumeOffset = nextBcOff(env);
  assertx(curFunc(env)->isAsync());

  if (curFunc(env)->isAsyncGenerator()) PUNT(Await-AsyncGenerator);

  auto const exitSlow   = makeExitSlow(env);

  if (!topC(env)->isA(TObj)) PUNT(Await-NonObject);

  auto const child = popC(env);
  gen(env, JmpZero, exitSlow, gen(env, IsWaitHandle, child));

  // cns() would ODR-use these
  auto const kSucceeded = c_WaitHandle::STATE_SUCCEEDED;
  auto const kFailed    = c_WaitHandle::STATE_FAILED;

  auto const state = gen(env, LdWHState, child);

  /*
   * HHBBC may have proven something about the inner type of this wait handle.
   *
   * So, we may have an assertion on the type of the top of the stack after
   * this instruction.  We know the next bytecode instruction is reachable from
   * fallthrough on the Await, so if it is an AssertRATStk 0, anything coming
   * out of the wait handle must be a subtype of that type, so this is a safe
   * and conservative way to do this optimization (even if our successor
   * bytecode offset is a jump target from things we aren't thinking about
   * here).
   */
  auto const knownTy = [&] {
    auto pc = curUnit(env)->at(resumeOffset);
    if (*reinterpret_cast<const Op*>(pc) != Op::AssertRATStk) return TInitCell;
    ++pc;
    auto const stkLoc = decodeVariableSizeImm(&pc);
    if (stkLoc != 0) return TInitCell;
    auto const rat = decodeRAT(curUnit(env), pc);
    auto const ty = ratToAssertType(env, rat);
    return ty ? *ty : TInitCell;
  }();

  ifThenElse(
    env,
    [&] (Block* taken) {
      auto const succeeded = gen(env, EqInt, state, cns(env, kSucceeded));
      gen(env, JmpNZero, taken, succeeded);
    },
    [&] { // Next: the wait handle is not finished, we need to suspend
      auto const failed = gen(env, EqInt, state, cns(env, kFailed));
      gen(env, JmpNZero, exitSlow, failed);
      if (resumed(env)) {
        implAwaitR(env, child, resumeOffset);
      } else {
        implAwaitE(env, child, resumeOffset, numIters);
      }
    },
    [&] { // Taken: retrieve the result from the wait handle
      auto const res = gen(env, LdWHResult, knownTy, child);
      gen(env, IncRef, res);
      gen(env, DecRef, child);
      push(env, res);
    }
  );
}

//////////////////////////////////////////////////////////////////////

void emitCreateCont(IRGS& env) {
  auto const resumeOffset = nextBcOff(env);
  assertx(!resumed(env));
  assertx(curFunc(env)->isGenerator());

  if (curFunc(env)->isAsync()) PUNT(CreateCont-AsyncGenerator);

  // Create the Generator object. CreateCont takes care of copying local
  // variables and iterators.
  auto const func = curFunc(env);
  auto const resumeSk = SrcKey(func, resumeOffset, true);
  auto const bind_data = LdBindAddrData { resumeSk, invSPOff(env) + 1 };
  auto const resumeAddr = gen(env, LdBindAddr, bind_data);
  auto const cont =
    gen(env,
        CreateCont,
        fp(env),
        cns(env, func->numSlotsInFrame()),
        resumeAddr,
        cns(env, resumeOffset));

  // The suspend hook will decref the newly created generator if it throws.
  auto const contAR =
    gen(env,
        LdContActRec,
        IsAsyncData(curFunc(env)->isAsync()),
        cont);
  suspendHookE(env, fp(env), contAR, cont);

  // Grab caller info from ActRec, free ActRec, store the return value
  // and return control to the caller.
  gen(env, StRetVal, fp(env), cont);
  auto const ret_data = RetCtrlData { offsetToReturnSlot(env), false };
  gen(env, RetCtrl, ret_data, sp(env), fp(env));
}

void emitContEnter(IRGS& env) {
  auto const returnOffset = nextBcOff(env);
  assertx(curClass(env));
  assertx(curClass(env)->classof(AsyncGenerator::getClass()) ||
          curClass(env)->classof(Generator::getClass()));
  assertx(curFunc(env)->contains(returnOffset));

  auto isAsync = curClass(env)->classof(AsyncGenerator::getClass());
  // Load generator's FP and resume address.
  auto const genObj = ldThis(env);
  auto const genFp  = gen(env, LdContActRec, IsAsyncData(isAsync), genObj);
  auto resumeAddr   = gen(env, LdContResumeAddr, IsAsyncData(isAsync), genObj);

  // Make sure function enter hook is called if needed.
  auto const exitSlow = makeExitSlow(env);
  gen(env, CheckSurpriseFlags, exitSlow, fp(env));

  // Exit to interpreter if resume address is not known.
  resumeAddr = gen(env, CheckNonNull, exitSlow, resumeAddr);

  spillStack(env);
  env.irb->exceptionStackBoundary();
  auto returnBcOffset = returnOffset - curFunc(env)->base();
  gen(
    env,
    ContEnter,
    ContEnterData { offsetFromIRSP(env, BCSPOffset{0}), returnBcOffset },
    sp(env),
    fp(env),
    genFp,
    resumeAddr
  );
}

void emitContRaise(IRGS& env) { PUNT(ContRaise); }

void emitYield(IRGS& env) {
  auto const resumeOffset = nextBcOff(env);
  assertx(resumed(env));
  assertx(curFunc(env)->isGenerator());

  if (curFunc(env)->isAsyncGenerator()) PUNT(Yield-AsyncGenerator);

  yieldImpl(env, resumeOffset);

  // take a fast path if this generator has no yield k => v;
  if (curFunc(env)->isPairGenerator()) {
    auto const newIdx = gen(env, ContArIncIdx, fp(env));
    auto const oldKey = gen(env, LdContArKey, TCell, fp(env));
    gen(env, StContArKey, fp(env), newIdx);
    gen(env, DecRef, oldKey);
  } else {
    // we're guaranteed that the key is an int
    gen(env, ContArIncKey, fp(env));
  }

  yieldReturnControl(env);
}

void emitYieldK(IRGS& env) {
  auto const resumeOffset = nextBcOff(env);
  assertx(resumed(env));
  assertx(curFunc(env)->isGenerator());

  if (curFunc(env)->isAsync()) PUNT(YieldK-AsyncGenerator);

  yieldImpl(env, resumeOffset);

  auto const newKey = popC(env);
  auto const oldKey = gen(env, LdContArKey, TCell, fp(env));
  gen(env, StContArKey, fp(env), newKey);
  gen(env, DecRef, oldKey);

  auto const keyType = newKey->type();
  if (keyType <= TInt) {
    gen(env, ContArUpdateIdx, fp(env), newKey);
  }

  yieldReturnControl(env);
}

void emitContCheck(IRGS& env, int32_t checkStarted) {
  assertx(curClass(env));
  assertx(curClass(env)->classof(AsyncGenerator::getClass()) ||
          curClass(env)->classof(Generator::getClass()));
  auto const cont = ldThis(env);
  gen(env, ContPreNext,
    IsAsyncData(curClass(env)->classof(AsyncGenerator::getClass())),
    makeExitSlow(env), cont, cns(env, static_cast<bool>(checkStarted)));
}

void emitContValid(IRGS& env) {
  assertx(curClass(env));
  assertx(curClass(env)->classof(AsyncGenerator::getClass()) ||
          curClass(env)->classof(Generator::getClass()));
  auto const cont = ldThis(env);
  push(env, gen(env, ContValid,
    IsAsyncData(curClass(env)->classof(AsyncGenerator::getClass())), cont));
}

void emitContKey(IRGS& env) {
  assertx(curClass(env));
  auto const cont = ldThis(env);
  gen(env, ContStartedCheck, IsAsyncData(false), makeExitSlow(env), cont);
  auto const offset = cns(env,
    offsetof(Generator, m_key) - Generator::objectOff());
  auto const value = gen(env, LdContField, TCell, cont, offset);
  pushIncRef(env, value);
}

void emitContCurrent(IRGS& env) {
  assertx(curClass(env));
  auto const cont = ldThis(env);
  gen(env, ContStartedCheck, IsAsyncData(false), makeExitSlow(env), cont);
  auto const offset = cns(env,
    offsetof(Generator, m_value) - Generator::objectOff());
  auto const value = gen(env, LdContField, TCell, cont, offset);
  pushIncRef(env, value);
}

//////////////////////////////////////////////////////////////////////

}}}

