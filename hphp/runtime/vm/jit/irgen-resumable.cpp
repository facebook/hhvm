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

#include "hphp/runtime/ext/asio/ext_wait-handle.h"
#include "hphp/runtime/ext/asio/ext_async-function-wait-handle.h"
#include "hphp/runtime/ext/asio/ext_async-generator.h"
#include "hphp/runtime/ext/asio/ext_static-wait-handle.h"
#include "hphp/runtime/ext/generator/ext_generator.h"
#include "hphp/runtime/base/repo-auth-type-codec.h"

#include "hphp/runtime/vm/hhbc-codec.h"
#include "hphp/runtime/vm/resumable.h"
#include "hphp/runtime/vm/unwind.h"

#include "hphp/runtime/vm/jit/analysis.h"
#include "hphp/runtime/vm/jit/irgen-call.h"
#include "hphp/runtime/vm/jit/irgen-control.h"
#include "hphp/runtime/vm/jit/irgen-exit.h"
#include "hphp/runtime/vm/jit/irgen-inlining.h"
#include "hphp/runtime/vm/jit/irgen-ret.h"
#include "hphp/runtime/vm/jit/irgen-types.h"

#include "hphp/runtime/vm/jit/irgen-internal.h"

namespace HPHP { namespace jit { namespace irgen {

namespace {

//////////////////////////////////////////////////////////////////////

template<class Hook>
void suspendHook(IRGS& env, Hook hook) {
  // Sync the marker to let the unwinder know that the consumed input is
  // no longer on the eval stack.
  env.irb->setCurMarker(makeMarker(env, bcOff(env)));
  env.irb->exceptionStackBoundary();

  ringbufferMsg(env, Trace::RBTypeFuncExit, curFunc(env)->fullName());
  ifThen(
    env,
    [&] (Block* taken) {
      auto const ptr = resumeMode(env) != ResumeMode::None ? sp(env) : fp(env);
      gen(env, CheckSurpriseFlags, taken, ptr);
    },
    [&] {
      hint(env, Block::Hint::Unlikely);
      hook();
    }
  );
}

void implAwaitE(IRGS& env, SSATmp* child, Offset resumeOffset) {
  assertx(curFunc(env)->isAsync());
  assertx(resumeMode(env) != ResumeMode::Async);
  assertx(child->type() <= TObj);

  // Bind address at which the execution should resume after awaiting.
  auto const func = curFunc(env);
  auto const resumeSk = SrcKey(func, resumeOffset, ResumeMode::Async,
                               hasThis(env));
  auto const bindData = LdBindAddrData { resumeSk, spOffBCFromFP(env) + 1 };
  auto const resumeAddr = gen(env, LdBindAddr, bindData);

  if (!curFunc(env)->isGenerator()) {
    // Create the AsyncFunctionWaitHandle object. CreateAFWH takes care of
    // copying local variables and iterators.
    auto const waitHandle =
      gen(env,
          func->attrs() & AttrMayUseVV ? CreateAFWH : CreateAFWHNoVV,
          fp(env),
          cns(env, func->numSlotsInFrame()),
          resumeAddr,
          cns(env, resumeOffset),
          child);

    auto const asyncAR = gen(env, LdAFWHActRec, waitHandle);

    // Call the suspend hook.
    suspendHook(env, [&] {
      gen(env, SuspendHookAwaitEF, fp(env), asyncAR, waitHandle);
    });

    if (RuntimeOption::EvalHHIRGenerateAsserts) {
      gen(env, DbgTrashRetVal, fp(env));
    }

    if (isInlining(env)) {
      suspendFromInlined(env, waitHandle);
      return;
    }

    // Return control to the caller.
    auto const spAdjust = offsetToReturnSlot(env);
    auto const retData = RetCtrlData { spAdjust, false, AuxUnion{0} };
    gen(env, RetCtrl, retData, sp(env), fp(env), waitHandle);
  } else {
    assertx(!isInlining(env));

    // Create the AsyncGeneratorWaitHandle object.
    auto const waitHandle =
      gen(env, CreateAGWH, fp(env), resumeAddr, cns(env, resumeOffset), child);

    // Call the suspend hook.
    suspendHook(env, [&] {
      gen(env, SuspendHookAwaitEG, fp(env), waitHandle);
    });

    // Return control to the caller (AG::next()).
    auto const spAdjust = offsetFromIRSP(env, BCSPRelOffset{-1});
    auto const retData = RetCtrlData { spAdjust, true, AuxUnion{0} };
    gen(env, RetCtrl, retData, sp(env), fp(env), waitHandle);
  }
}

void implAwaitR(IRGS& env, SSATmp* child, Offset resumeOffset) {
  assertx(curFunc(env)->isAsync());
  assertx(resumeMode(env) == ResumeMode::Async);
  assertx(child->isA(TObj));
  assertx(!isInlining(env));

  // We must do this before we do anything, because it can throw, and we can't
  // start tearing down the AFWH before that or the unwinder won't be able to
  // react.
  suspendHook(env, [&] {
    gen(env, SuspendHookAwaitR, fp(env), child);
  });

  // Prepare child for establishing dependency.
  gen(env, AFWHPrepareChild, fp(env), child);

  // Suspend the async function.
  auto const resumeSk = SrcKey(curFunc(env), resumeOffset, ResumeMode::Async,
                               hasThis(env));
  auto const data = LdBindAddrData { resumeSk, spOffBCFromFP(env) + 1 };
  auto const resumeAddr = gen(env, LdBindAddr, data);
  gen(env, StArResumeAddr, ResumeOffset { resumeOffset }, fp(env),
      resumeAddr);

  // Set up the dependency.
  gen(env, AFWHBlockOn, fp(env), child);

  // Call stub that will either transfer control to another ResumableWaitHandle,
  // or return control back to the scheduler. Leave SP pointing to a single
  // uninitialized cell which will be filled by the stub.
  auto const spAdjust = offsetFromIRSP(env, BCSPRelOffset{-1});
  gen(env, AsyncSwitchFast, IRSPRelOffsetData { spAdjust }, sp(env), fp(env));
}

SSATmp* implYieldGen(IRGS& env, SSATmp* key, SSATmp* value) {
  if (key != nullptr) {
    // Teleport yielded key.
    auto const oldKey = gen(env, LdContArKey, TCell, fp(env));
    gen(env, StContArKey, fp(env), key);
    decRef(env, oldKey);

    if (key->type() <= TInt) {
      gen(env, ContArUpdateIdx, fp(env), key);
    }
  } else {
    // Increment key.
    if (curFunc(env)->isPairGenerator()) {
      auto const newIdx = gen(env, ContArIncIdx, fp(env));
      auto const oldKey = gen(env, LdContArKey, TCell, fp(env));
      gen(env, StContArKey, fp(env), newIdx);
      decRef(env, oldKey);
    } else {
      // Fast path: if this generator has no yield k => v, it is
      // guaranteed that the key is an int.
      gen(env, ContArIncKey, fp(env));
    }
  }

  // Teleport yielded value.
  auto const oldValue = gen(env, LdContArValue, TCell, fp(env));
  gen(env, StContArValue, fp(env), value);
  decRef(env, oldValue);

  // Return value of iteration.
  return cns(env, TInitNull);
}

SSATmp* implYieldAGen(IRGS& env, SSATmp* key, SSATmp* value) {
  key = key ? key : cns(env, TInitNull);

  // Wrap the key and value into a tuple.
  auto const keyValueTuple = gen(env, AllocVArray, PackedArrayData { 2 });
  gen(env, InitPackedLayoutArray, IndexData { 0 }, keyValueTuple, key);
  gen(env, InitPackedLayoutArray, IndexData { 1 }, keyValueTuple, value);

  // Wrap the tuple into a StaticWaitHandle.
  return gen(env, CreateSSWH, keyValueTuple);
}

void implYield(IRGS& env, bool withKey) {
  assertx(resumeMode(env) != ResumeMode::None);
  assertx(curFunc(env)->isGenerator());

  if (resumeMode(env) == ResumeMode::Async) PUNT(Yield-AsyncGenerator);

  suspendHook(env, [&] {
    gen(env, SuspendHookYield, fp(env));
  });

  // Resumable::setResumeAddr(resumeAddr, resumeOffset)
  auto const resumeOffset = nextBcOff(env);
  auto const resumeSk = SrcKey(curFunc(env), resumeOffset, ResumeMode::GenIter,
                               hasThis(env));
  auto const data = LdBindAddrData { resumeSk, spOffBCFromFP(env) };
  auto const resumeAddr = gen(env, LdBindAddr, data);
  gen(env, StArResumeAddr, ResumeOffset { resumeOffset }, fp(env), resumeAddr);

  // No inc/dec-ref as keys and values are teleported.
  auto const value = popC(env, DataTypeGeneric);
  auto const key = withKey ? popC(env) : nullptr;

  auto const retVal = !curFunc(env)->isAsync()
    ? implYieldGen(env, key, value)
    : implYieldAGen(env, key, value);

  // Set state from Running to Started.
  gen(env, StContArState,
      GeneratorState { BaseGenerator::State::Started },
      fp(env));

  // Return control to the caller (Gen::next()).
  auto const spAdjust = offsetFromIRSP(env, BCSPRelOffset{-1});
  auto const retData = RetCtrlData { spAdjust, true, AuxUnion{0} };
  gen(env, RetCtrl, retData, sp(env), fp(env), retVal);
}

/*
 * HHBBC may have proven something about the inner type of this awaitable.
 *
 * So, we may have an assertion on the type of the top of the stack after
 * this instruction.  We know the next bytecode instruction is reachable from
 * fallthrough on the Await, so if it is an AssertRATStk 0, anything coming
 * out of the awaitable must be a subtype of that type, so this is a safe
 * and conservative way to do this optimization (even if our successor
 * bytecode offset is a jump target from things we aren't thinking about
 * here).
 */
Type awaitedTypeFromHHBBC(IRGS& env, Offset nextBcOff) {
  auto pc = curUnit(env)->at(nextBcOff);
  if (decode_op(pc) != Op::AssertRATStk) return TInitCell;
  auto const stkLoc = decode_iva(pc);
  if (stkLoc != 0) return TInitCell;
  auto const rat = decodeRAT(curUnit(env), pc);
  return typeFromRAT(rat, curClass(env));
}

/*
 * Try to determine the inner awaitable type from the source of SSATmp.
 */
Type awaitedTypeFromSSATmp(const SSATmp* awaitable) {
  awaitable = canonical(awaitable);

  auto const inst = awaitable->inst();
  if (inst->is(Call)) {
    auto const callee = inst->extra<Call>()->callee;
    return callee ? awaitedCallReturnType(callee) : TInitCell;
  }
  if (inst->is(CreateAFWH)) {
    return awaitedCallReturnType(inst->func());
  }
  if (inst->is(DefLabel)) {
    auto ty = TBottom;
    auto const dsts = inst->dsts();
    inst->block()->forEachSrc(
      std::find(dsts.begin(), dsts.end(), awaitable) - dsts.begin(),
      [&] (const IRInstruction*, const SSATmp* src) {
        ty = ty | awaitedTypeFromSSATmp(src);
      }
    );
    return ty;
  }

  return TInitCell;
}

Type awaitedType(IRGS& env, SSATmp* awaitable, Offset nextBcOff) {
  return awaitedTypeFromHHBBC(env, nextBcOff) &
         awaitedTypeFromSSATmp(awaitable);
}

bool likelySuspended(const SSATmp* awaitable) {
  awaitable = canonical(awaitable);
  auto const inst = awaitable->inst();
  if (inst->is(Call) && inst->extra<Call>()->asyncEagerReturn) return true;
  if (inst->is(CreateAFWH)) return true;
  if (inst->is(DefLabel)) {
    auto likely = true;
    auto const dsts = inst->dsts();
    inst->block()->forEachSrc(
      std::find(dsts.begin(), dsts.end(), awaitable) - dsts.begin(),
      [&] (const IRInstruction*, const SSATmp* src) {
        likely = likely && likelySuspended(src);
      }
    );
    return likely;
  }

  return false;
}

//////////////////////////////////////////////////////////////////////

}

void emitWHResult(IRGS& env) {
  if (!topC(env)->isA(TObj)) PUNT(WHResult-NonObject);

  auto const exitSlow = makeExitSlow(env);
  auto const child = popC(env);
  // In most conditions, this will be optimized out by the simplifier.
  // We already need to setup a side-exit for the !succeeded case.
  gen(env, JmpZero, exitSlow, gen(env, IsWaitHandle, child));
  static_assert(
    c_Awaitable::STATE_SUCCEEDED == 0,
    "we test state for non-zero, success must be zero"
  );
  gen(env, JmpNZero, exitSlow, gen(env, LdWHState, child));
  auto const awaitedTy = awaitedType(env, child, nextBcOff(env));
  auto const res = gen(env, LdWHResult, awaitedTy, child);
  gen(env, IncRef, res);
  decRef(env, child);
  push(env, res);
}

void emitAwait(IRGS& env) {
  auto const resumeOffset = nextBcOff(env);
  assertx(curFunc(env)->isAsync());
  assertx(spOffBCFromFP(env) == spOffEmpty(env) + 1);

  if (curFunc(env)->isAsyncGenerator() &&
      resumeMode(env) == ResumeMode::Async) {
    PUNT(Await-AsyncGenerator);
  }

  auto const exitSlow = makeExitSlow(env);

  if (!topC(env)->isA(TObj)) PUNT(Await-NonObject);

  auto const child = popC(env);
  auto const childIsSWH =
    child->type() <= Type::SubObj(c_StaticWaitHandle::classof());
  gen(env, JmpZero, exitSlow, gen(env, IsWaitHandle, child));

  auto const handleSucceeded = [&] {
    auto const awaitedTy = awaitedType(env, child, resumeOffset);
    auto const res = gen(env, LdWHResult, awaitedTy, child);
    gen(env, IncRef, res);
    decRef(env, child);
    push(env, res);
  };
  auto const handleFailed = [&] {
    auto const offset = findCatchHandler(curFunc(env), bcOff(env));
    if (offset != InvalidAbsoluteOffset) {
      auto const exception = gen(env, LdWHResult, TObj, child);
      gen(env, IncRef, exception);
      decRef(env, child);
      push(env, exception);
      jmpImpl(env, offset);
    } else {
      gen(env, Jmp, exitSlow);
    }
  };
  auto const handleNotFinished = [&] {
    if (childIsSWH) {
      gen(env, Unreachable, ASSERT_REASON);
    } else if (resumeMode(env) == ResumeMode::Async) {
      implAwaitR(env, child, resumeOffset);
    } else {
      implAwaitE(env, child, resumeOffset);
    }
  };

  auto const state = gen(env, LdWHState, child);
  assertx(c_Awaitable::STATE_SUCCEEDED == 0);
  assertx(c_Awaitable::STATE_FAILED == 1);

  if (childIsSWH || !likelySuspended(child)) {
    ifThenElse(env,
      [&] (Block* taken) { gen(env, JmpNZero, taken, state); },
      [&] { handleSucceeded(); },
      [&] {
        ifThenElse(env,
          [&] (Block* taken) {
            if (childIsSWH) return;
            gen(env, JmpZero, taken, gen(env, EqInt, state, cns(env, 1)));
          },
          [&] { handleFailed(); },
          [&] { handleNotFinished(); }
        );
      }
    );
  } else {
    ifThenElse(env,
      [&] (Block* taken) {
        gen(env, JmpNZero, taken, gen(env, LteInt, state, cns(env, 1)));
      },
      [&] { handleNotFinished(); },
      [&] {
        // Coming from a call with request for async eager return that did
        // not return eagerly.
        hint(env, Block::Hint::Unlikely);
        IRUnit::Hinter h(env.irb->unit(), Block::Hint::Unlikely);

        ifThenElse(env,
          [&] (Block* taken) { gen(env, JmpNZero, taken, state); },
          [&] {
            handleSucceeded();
            gen(env, Jmp, makeExit(env, resumeOffset));
          },
          [&] { handleFailed(); }
        );
      }
    );
  }
}

void emitAwaitAll(IRGS& env, LocalRange locals) {
  auto const resumeOffset = nextBcOff(env);
  assertx(curFunc(env)->isAsync());
  assertx(spOffBCFromFP(env) == spOffEmpty(env));

  if (curFunc(env)->isAsyncGenerator() &&
      resumeMode(env) == ResumeMode::Async) {
    PUNT(Await-AsyncGenerator);
  }

  auto const exitSlow = makeExitSlow(env);

  auto const cnt = [&] {
    if (locals.count > RuntimeOption::EvalJitMaxAwaitAllUnroll) {
      return gen(
        env,
        CountWHNotDone,
        CountWHNotDoneData { locals.first, locals.count },
        exitSlow,
        fp(env)
      );
    }
    auto cnt = cns(env, 0);
    for (int i = 0; i < locals.count; ++i) {
      auto const loc = ldLoc(env, locals.first + i, nullptr, DataTypeSpecific);
      if (loc->isA(TNull)) continue;
      if (!loc->isA(TObj)) PUNT(Await-NonObject);
      gen(env, JmpZero, exitSlow, gen(env, IsWaitHandle, loc));
      auto const not_done = gen(env, LdWHNotDone, loc);
      cnt = gen(env, AddInt, cnt, not_done);
    }
    return cnt;
  }();

  ifThenElse(
    env,
    [&] (Block* taken) {
      gen(env, JmpNZero, taken, cnt);
    },
    [&] { // Next: all of the wait handles are finished
      push(env, cns(env, TInitNull));
    },
    [&] { // Taken: some of the wait handles have not yet completed
      hint(env, Block::Hint::Unlikely);
      IRUnit::Hinter h(env.irb->unit(), Block::Hint::Unlikely);

      auto const wh = gen(
        env,
        CreateAAWH,
        CreateAAWHData { locals.first, locals.count },
        fp(env),
        cnt
      );

      if (resumeMode(env) == ResumeMode::Async) {
        implAwaitR(env, wh, resumeOffset);
      } else {
        implAwaitE(env, wh, resumeOffset);
      }
    }
  );
}

//////////////////////////////////////////////////////////////////////

void emitCreateCont(IRGS& env) {
  auto const resumeOffset = nextBcOff(env);
  assertx(resumeMode(env) == ResumeMode::None);
  assertx(curFunc(env)->isGenerator());

  // Create the Generator object. CreateCont takes care of copying local
  // variables and iterators.
  auto const func = curFunc(env);
  auto const resumeSk = SrcKey(func, resumeOffset, ResumeMode::GenIter,
                               hasThis(env));
  auto const bind_data = LdBindAddrData { resumeSk, spOffBCFromFP(env) + 1 };
  auto const resumeAddr = gen(env, LdBindAddr, bind_data);
  auto const cont =
    gen(env,
        curFunc(env)->isAsync() ? CreateAGen : CreateGen,
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

  suspendHook(env, [&] {
    gen(env, SuspendHookCreateCont, fp(env), contAR, cont);
  });

  // Grab caller info from the ActRec, free the ActRec, and return control to
  // the caller.
  if (RuntimeOption::EvalHHIRGenerateAsserts) {
    gen(env, DbgTrashRetVal, fp(env));
  }
  auto const spAdjust = offsetToReturnSlot(env);
  auto const retData = RetCtrlData { spAdjust, false, AuxUnion{0} };
  gen(env, RetCtrl, retData, sp(env), fp(env), cont);
}

void emitContEnter(IRGS& env) {
  assertx(curClass(env));
  assertx(curClass(env)->classof(AsyncGenerator::getClass()) ||
          curClass(env)->classof(Generator::getClass()));

  auto const callBCOffset = bcOff(env) - curFunc(env)->base();
  auto const isAsync = curClass(env)->classof(AsyncGenerator::getClass());
  // Load generator's FP and resume address.
  auto const genObj = ldThis(env);
  auto const genFp  = gen(env, LdContActRec, IsAsyncData(isAsync), genObj);
  auto resumeAddr   = gen(env, LdContResumeAddr, IsAsyncData(isAsync), genObj);

  // Make sure function enter hook is called if needed.
  auto const exitSlow = makeExitSlow(env);
  gen(env, CheckSurpriseFlags, exitSlow, fp(env));

  // Exit to interpreter if resume address is not known.
  resumeAddr = gen(env, CheckNonNull, exitSlow, resumeAddr);

  auto const retVal = gen(
    env,
    ContEnter,
    ContEnterData { spOffBCFromIRSP(env), callBCOffset, isAsync },
    sp(env),
    fp(env),
    genFp,
    resumeAddr
  );

  push(env, retVal);
}

void emitContRaise(IRGS& /*env*/) {
  PUNT(ContRaise);
}

void emitYield(IRGS& env) {
  implYield(env, false);
}

void emitYieldK(IRGS& env) {
  implYield(env, true);
}

void emitContCheck(IRGS& env, ContCheckOp subop) {
  assertx(curClass(env));
  assertx(curClass(env)->classof(AsyncGenerator::getClass()) ||
          curClass(env)->classof(Generator::getClass()));
  auto const cont = ldThis(env);
  auto const checkStarted = subop == ContCheckOp::CheckStarted;
  gen(env, ContPreNext,
    IsAsyncData(curClass(env)->classof(AsyncGenerator::getClass())),
    makeExitSlow(env), cont, cns(env, checkStarted));
}

void emitContValid(IRGS& env) {
  assertx(curClass(env));
  assertx(curClass(env)->classof(AsyncGenerator::getClass()) ||
          curClass(env)->classof(Generator::getClass()));
  auto const cont = ldThis(env);
  push(env, gen(env, ContValid,
    IsAsyncData(curClass(env)->classof(AsyncGenerator::getClass())), cont));
}

// Delegate generators aren't currently supported in the IR, so just use the
// interpreter if we get into a situation where we need to use the delegate
void interpIfHasDelegate(IRGS& env, SSATmp *cont) {
  auto const delegateOffset = cns(env,
      offsetof(Generator, m_delegate) - Generator::objectOff());
  auto const delegate = gen(env, LdContField, TObj, cont, delegateOffset);
  // Check if delegate is non-null. If it is, go to the interpreter
  gen(env, CheckType, TNull, makeExitSlow(env), delegate);
}

void emitContKey(IRGS& env) {
  assertx(curClass(env));
  auto const cont = ldThis(env);
  gen(env, ContStartedCheck, IsAsyncData(false), makeExitSlow(env), cont);

  interpIfHasDelegate(env, cont);

  auto const offset = cns(env,
    offsetof(Generator, m_key) - Generator::objectOff());
  auto const value = gen(env, LdContField, TCell, cont, offset);
  pushIncRef(env, value);
}

void emitContCurrent(IRGS& env) {
  assertx(curClass(env));
  auto const cont = ldThis(env);
  gen(env, ContStartedCheck, IsAsyncData(false), makeExitSlow(env), cont);

  interpIfHasDelegate(env, cont);

  // We reuse the same storage for the return value as the yield (`m_value`),
  // so doing a blind read will cause `current` to return the wrong value on a
  // finished generator. We should return NULL instead
  ifThenElse(
    env,
    [&] (Block *taken) {
      auto const done = gen(env, ContValid, IsAsyncData(false), cont);
      gen(env, JmpZero, taken, done);
    },
    [&] {
      auto const offset = cns(env,
        offsetof(Generator, m_value) - Generator::objectOff());
      auto const value = gen(env, LdContField, TCell, cont, offset);
      pushIncRef(env, value);
    },
    [&] {
      hint(env, Block::Hint::Unlikely);
      emitNull(env);
    }
  );
}

//////////////////////////////////////////////////////////////////////

}}}
