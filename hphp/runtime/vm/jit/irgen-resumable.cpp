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

#include "hphp/runtime/ext/asio/wait_handle.h"
#include "hphp/runtime/ext/asio/async_function_wait_handle.h"
#include "hphp/runtime/ext/asio/async_generator.h"
#include "hphp/runtime/ext/ext_generator.h"

#include "hphp/runtime/vm/jit/irgen-exit.h"
#include "hphp/runtime/vm/jit/irgen-ringbuffer.h"

#include "hphp/runtime/vm/jit/irgen-internal.h"

namespace HPHP { namespace jit { namespace irgen {

namespace {

//////////////////////////////////////////////////////////////////////

void suspendHookImpl(HTS& env, SSATmp* frame, SSATmp* other, bool eager) {
  ringbuffer(env, Trace::RBTypeFuncExit, curFunc(env)->fullName());
  env.irb->ifThen(
    [&] (Block* taken) {
      gen(env, CheckSurpriseFlags, taken);
    },
    [&] {
      env.irb->hint(Block::Hint::Unlikely);
      gen(env, eager ? SuspendHookE : SuspendHookR, frame, other);
    }
  );
}

void suspendHookE(HTS& env, SSATmp* frame, SSATmp* resumableAR) {
  return suspendHookImpl(env, frame, resumableAR, true);
}

void suspendHookR(HTS& env, SSATmp* frame, SSATmp* objOrNullptr) {
  return suspendHookImpl(env, frame, objOrNullptr, false);
}

void implAwaitE(HTS& env, SSATmp* child, Offset resumeOffset, int numIters) {
  assert(curFunc(env)->isAsync());
  assert(!resumed(env));
  assert(child->type() <= Type::Obj);

  // Create the AsyncFunctionWaitHandle object. CreateAFWH takes care of
  // copying local variables and iterators.
  auto const func = curFunc(env);
  auto const resumeSk = SrcKey(func, resumeOffset, true);
  auto const resumeAddr = gen(env, LdBindAddr, LdBindAddrData(resumeSk));
  auto const waitHandle =
    gen(env,
        CreateAFWH,
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
  push(env, cns(env, Type::InitNull));
  env.irb->exceptionStackBoundary();
  suspendHookE(env, fp(env), asyncAR);
  discard(env, 1);

  // Grab caller info from ActRec, free ActRec, store the return value
  // and return control to the caller.
  gen(env, StRetVal, fp(env), waitHandle);
  auto const retAddr = gen(env, LdRetAddr, fp(env));
  auto const stack = gen(env, RetAdjustStack, fp(env));
  auto const frame = gen(env, FreeActRec, fp(env));
  gen(env, RetCtrl, RetCtrlData(false), stack, frame, retAddr);
}

void implAwaitR(HTS& env, SSATmp* child, Offset resumeOffset) {
  assert(curFunc(env)->isAsync());
  assert(resumed(env));
  assert(child->isA(Type::Obj));

  // We must do this before we do anything, because it can throw, and we can't
  // start tearing down the AFWH before that or the unwinder won't be able to
  // react.
  suspendHookR(env, fp(env), child);

  // Prepare child for establishing dependency.
  gen(env, AFWHPrepareChild, fp(env), child);

  // Suspend the async function.
  auto const resumeSk = SrcKey(curFunc(env), resumeOffset, true);
  auto const resumeAddr = gen(env, LdBindAddr, LdBindAddrData(resumeSk));
  gen(env, StAsyncArResume, ResumeOffset { resumeOffset }, fp(env),
    resumeAddr);

  // Set up the dependency.
  gen(env, AFWHBlockOn, fp(env), child);

  auto const stack = spillStack(env);
  auto const retAddr = gen(env, LdRetAddr, fp(env));
  auto const frame = gen(env, FreeActRec, fp(env));

  gen(env, RetCtrl, RetCtrlData(true), stack, frame, retAddr);
}

void yieldReturnControl(HTS& env) {
  // Push return value of next()/send()/raise().
  push(env, cns(env, Type::InitNull));

  auto const stack   = spillStack(env);
  auto const retAddr = gen(env, LdRetAddr, fp(env));
  auto const frame   = gen(env, FreeActRec, fp(env));
  gen(env, RetCtrl, RetCtrlData(true), stack, frame, retAddr);
}

void yieldImpl(HTS& env, Offset resumeOffset) {
  suspendHookR(env, fp(env), cns(env, Type::Nullptr));

  // Resumable::setResumeAddr(resumeAddr, resumeOffset)
  auto const resumeSk = SrcKey(curFunc(env), resumeOffset, true);
  auto const resumeAddr = gen(env, LdBindAddr, LdBindAddrData(resumeSk));
  gen(env, StContArResume, ResumeOffset { resumeOffset }, fp(env), resumeAddr);

  // Set yielded value.
  auto const oldValue = gen(env, LdContArValue, Type::Cell, fp(env));
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

void emitAwait(HTS& env, int32_t numIters) {
  auto const resumeOffset = nextBcOff(env);
  assert(curFunc(env)->isAsync());

  if (curFunc(env)->isAsyncGenerator()) PUNT(Await-AsyncGenerator);

  auto const exitSlow   = makeExitSlow(env);

  if (!topC(env)->isA(Type::Obj)) PUNT(Await-NonObject);

  auto const child = popC(env);
  gen(env, JmpZero, exitSlow, gen(env, IsWaitHandle, child));

  // cns() would ODR-use these
  auto const kSucceeded = c_WaitHandle::STATE_SUCCEEDED;
  auto const kFailed    = c_WaitHandle::STATE_FAILED;

  auto const state = gen(env, LdWHState, child);
  gen(env, JmpEqInt, exitSlow, state, cns(env, kFailed));

  env.irb->ifThenElse(
    [&] (Block* taken) {
      gen(env, JmpEqInt, taken, state, cns(env, kSucceeded));
    },
    [&] { // Next: the wait handle is not finished, we need to suspend
      if (resumed(env)) {
        implAwaitR(env, child, resumeOffset);
      } else {
        implAwaitE(env, child, resumeOffset, numIters);
      }
    },
    [&] { // Taken: retrieve the result from the wait handle
      auto const res = gen(env, LdWHResult, child);
      gen(env, IncRef, res);
      gen(env, DecRef, child);
      push(env, res);
    }
  );
}

//////////////////////////////////////////////////////////////////////

void emitCreateCont(HTS& env) {
  auto const resumeOffset = nextBcOff(env);
  assert(!resumed(env));
  assert(curFunc(env)->isGenerator());

  if (curFunc(env)->isAsyncGenerator()) PUNT(CreateCont-AsyncGenerator);

  // Create the Generator object. CreateCont takes care of copying local
  // variables and iterators.
  auto const func = curFunc(env);
  auto const resumeSk = SrcKey(func, resumeOffset, true);
  auto const resumeAddr = gen(env, LdBindAddr, LdBindAddrData(resumeSk));
  auto const cont =
    gen(env,
        CreateCont,
        fp(env),
        cns(env, func->numSlotsInFrame()),
        resumeAddr,
        cns(env, resumeOffset));

  // The suspend hook will decref the newly created generator if it throws.
  auto const contAR = gen(env, LdContActRec, cont);
  suspendHookE(env, fp(env), contAR);

  // Grab caller info from ActRec, free ActRec, store the return value
  // and return control to the caller.
  gen(env, StRetVal, fp(env), cont);
  auto const retAddr = gen(env, LdRetAddr, fp(env));
  auto const stack = gen(env, RetAdjustStack, fp(env));
  auto const frame = gen(env, FreeActRec, fp(env));
  gen(env, RetCtrl, RetCtrlData(false), stack, frame, retAddr);
}

void emitContEnter(HTS& env) {
  auto const returnOffset = nextBcOff(env);
  assert(curClass(env));
  assert(curClass(env)->classof(c_AsyncGenerator::classof()) ||
         curClass(env)->classof(c_Generator::classof()));
  assert(curFunc(env)->contains(returnOffset));

  // Load generator's FP and resume address.
  auto const genObj = ldThis(env);
  auto const genFp  = gen(env, LdContActRec, genObj);
  auto resumeAddr   = gen(env, LdContResumeAddr, genObj);

  // Make sure function enter hook is called if needed.
  auto const exitSlow = makeExitSlow(env);
  gen(env, CheckSurpriseFlags, exitSlow);

  // Exit to interpreter if resume address is not known.
  resumeAddr = gen(env, CheckNonNull, exitSlow, resumeAddr);

  // Sync stack.
  auto const stack = spillStack(env);

  // Enter generator.
  auto returnBcOffset = returnOffset - curFunc(env)->base();
  gen(env, ContEnter, stack, fp(env), genFp, resumeAddr,
    cns(env, returnBcOffset));
}

void emitContRaise(HTS& env) { PUNT(ContRaise); }

void emitYield(HTS& env) {
  auto const resumeOffset = nextBcOff(env);
  assert(resumed(env));
  assert(curFunc(env)->isGenerator());

  if (curFunc(env)->isAsyncGenerator()) PUNT(Yield-AsyncGenerator);

  yieldImpl(env, resumeOffset);

  // take a fast path if this generator has no yield k => v;
  if (curFunc(env)->isPairGenerator()) {
    auto const newIdx = gen(env, ContArIncIdx, fp(env));
    auto const oldKey = gen(env, LdContArKey, Type::Cell, fp(env));
    gen(env, StContArKey, fp(env), newIdx);
    gen(env, DecRef, oldKey);
  } else {
    // we're guaranteed that the key is an int
    gen(env, ContArIncKey, fp(env));
  }

  yieldReturnControl(env);
}

void emitYieldK(HTS& env) {
  auto const resumeOffset = nextBcOff(env);
  assert(resumed(env));
  assert(curFunc(env)->isGenerator());

  if (curFunc(env)->isAsyncGenerator()) PUNT(YieldK-AsyncGenerator);

  yieldImpl(env, resumeOffset);

  auto const newKey = popC(env);
  auto const oldKey = gen(env, LdContArKey, Type::Cell, fp(env));
  gen(env, StContArKey, fp(env), newKey);
  gen(env, DecRef, oldKey);

  auto const keyType = newKey->type();
  if (keyType <= Type::Int) {
    gen(env, ContArUpdateIdx, fp(env), newKey);
  }

  yieldReturnControl(env);
}

void emitContCheck(HTS& env, int32_t checkStarted) {
  assert(curClass(env));
  assert(curClass(env)->classof(c_AsyncGenerator::classof()) ||
         curClass(env)->classof(c_Generator::classof()));
  auto const cont = ldThis(env);
  gen(env, ContPreNext, makeExitSlow(env), cont,
    cns(env, static_cast<bool>(checkStarted)));
}

void emitContValid(HTS& env) {
  assert(curClass(env));
  auto const cont = ldThis(env);
  push(env, gen(env, ContValid, cont));
}

void emitContKey(HTS& env) {
  assert(curClass(env));
  auto const cont = ldThis(env);
  gen(env, ContStartedCheck, makeExitSlow(env), cont);
  auto const offset = cns(env, offsetof(c_Generator, m_key));
  auto const value = gen(env, LdContField, Type::Cell, cont, offset);
  pushIncRef(env, value);
}

void emitContCurrent(HTS& env) {
  assert(curClass(env));
  auto const cont = ldThis(env);
  gen(env, ContStartedCheck, makeExitSlow(env), cont);
  auto const offset = cns(env, offsetof(c_Generator, m_value));
  auto const value = gen(env, LdContField, Type::Cell, cont, offset);
  pushIncRef(env, value);
}

//////////////////////////////////////////////////////////////////////

}}}

