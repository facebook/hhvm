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

#include "hphp/runtime/vm/jit/irgen-ringbuffer.h"
#include "hphp/runtime/vm/jit/irgen-exit.h"
#include "hphp/runtime/vm/jit/irgen-inlining.h"

#include "hphp/runtime/vm/jit/irgen-internal.h"

namespace HPHP { namespace jit { namespace irgen {

namespace {

//////////////////////////////////////////////////////////////////////

void implRet(HTS& env, Type type) {
  auto const func = curFunc(env);
  if (func->attrs() & AttrMayUseVV) {
    // Note: this has to be the first thing, because we cannot bail after
    //       we start decRefing locs because then there'll be no corresponding
    //       bytecode boundaries until the end of RetC
    gen(env, ReleaseVVOrExit, makeExitSlow(env), fp(env));
  }

  // Pop the return value. Since it will be teleported to its place in memory,
  // we don't care about the type.
  auto retVal = pop(env, type, func->isGenerator() ? DataTypeSpecific
                                                   : DataTypeGeneric);

  // Free local variables.  We do the decrefs inline if there are less
  // refcounted locals than a threshold.
  auto const localCount = func->numLocals();
  auto const shouldFreeInline = mcg->useLLVM() || [&]() -> bool {
    auto const count = mcg->numTranslations(
      env.irb->unit().context().srcKey());
    constexpr int kTooPolyRet = 6;
    if (localCount > 0 && count > kTooPolyRet) return false;
    auto numRefCounted = int{0};
    for (auto i = uint32_t{0}; i < localCount; ++i) {
      if (env.irb->localType(i, DataTypeGeneric).maybeCounted()) {
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

  // Free $this.
  if (func->mayHaveThis()) {
    gen(env, DecRefThis, fp(env));
  }

  retSurpriseCheck(env, fp(env), retVal, false);

  // In async function, wrap the return value into succeeded StaticWaitHandle.
  if (!resumed(env) && func->isAsyncFunction()) {
    retVal = gen(env, CreateSSWH, retVal);
  }

  SSATmp* stack;
  SSATmp* resumableObj = nullptr;
  if (!resumed(env)) {
    // Store the return value.
    gen(env, StRetVal, fp(env), retVal);

    // Free ActRec.
    stack = gen(env, RetAdjustStack, fp(env));
  } else if (func->isAsyncFunction()) {
    // Load the parent chain.
    auto parentChain = gen(env, LdAsyncArParentChain, fp(env));

    // Mark the async function as succeeded.
    gen(env, StAsyncArSucceeded, fp(env));

    // Store the return value.
    gen(env, StAsyncArResult, fp(env), retVal);

    // Unblock parents.
    gen(env, ABCUnblock, parentChain);

    // Sync SP.
    stack = spillStack(env);

    // Get the AsyncFunctionWaitHandle.
    resumableObj = gen(env, LdResumableArObj, fp(env));
  } else if (func->isNonAsyncGenerator()) {
    // Clear generator's key and value.
    auto const oldKey = gen(env, LdContArKey, Type::Cell, fp(env));
    gen(env, StContArKey, fp(env), cns(env, Type::InitNull));
    gen(env, DecRef, oldKey);

    auto const oldValue = gen(env, LdContArValue, Type::Cell, fp(env));
    gen(env, StContArValue, fp(env), cns(env, Type::InitNull));
    gen(env, DecRef, oldValue);

    // Mark generator as finished.
    gen(env,
        StContArState,
        GeneratorState { BaseGenerator::State::Done },
        fp(env));

    // Push return value of next()/send()/raise().
    push(env, cns(env, Type::InitNull));

    // Sync SP.
    stack = spillStack(env);
  } else {
    not_reached();
  }

  // Grab caller info from ActRec.
  auto const retAddr = gen(env, LdRetAddr, fp(env));
  auto const frame = gen(env, FreeActRec, fp(env));

  // Drop reference to this resumable. The reference to the object storing
  // the frame is implicitly owned by the execution. TakeRef is used to inform
  // the refcount optimizer about this fact.
  if (resumableObj != nullptr) {
    gen(env, TakeRef, resumableObj);
    gen(env, DecRef, resumableObj);
  }

  // Return control to the caller.
  gen(env, RetCtrl, RetCtrlData(false), stack, frame, retAddr);
}

//////////////////////////////////////////////////////////////////////

}

void retSurpriseCheck(HTS& env,
                      SSATmp* frame,
                      SSATmp* retVal,
                      bool suspendingResumed) {
  ringbuffer(env, Trace::RBTypeFuncExit, curFunc(env)->fullName());
  env.irb->ifThen(
    [&](Block* taken) {
      gen(env, CheckSurpriseFlags, taken);
    },
    [&] {
      env.irb->hint(Block::Hint::Unlikely);
      if (retVal != nullptr) {
        gen(env,
            FunctionReturnHook,
            RetCtrlData(suspendingResumed),
            frame,
            retVal);
      } else {
        gen(env,
            FunctionSuspendHook,
            RetCtrlData(suspendingResumed),
            frame,
            cns(env, suspendingResumed));
      }
    }
  );
}

void emitRetC(HTS& env) {
  if (curFunc(env)->isAsyncGenerator()) PUNT(RetC-AsyncGenerator);

  if (isInlining(env)) {
    assert(!resumed(env));
    retFromInlined(env, Type::Cell);
  } else {
    implRet(env, Type::Cell);
  }
}

void emitRetV(HTS& env) {
  assert(!resumed(env));
  assert(!curFunc(env)->isResumable());
  if (isInlining(env)) {
    retFromInlined(env, Type::BoxedCell);
  } else {
    implRet(env, Type::BoxedCell);
  }
}

//////////////////////////////////////////////////////////////////////

}}}
