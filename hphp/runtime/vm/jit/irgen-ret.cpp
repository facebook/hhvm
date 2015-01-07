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
#include "hphp/runtime/vm/jit/irgen.h"

#include "hphp/runtime/vm/jit/mc-generator.h"

#include "hphp/runtime/vm/jit/irgen-ringbuffer.h"
#include "hphp/runtime/vm/jit/irgen-exit.h"
#include "hphp/runtime/vm/jit/irgen-inlining.h"

#include "hphp/runtime/vm/jit/irgen-internal.h"

namespace HPHP { namespace jit { namespace irgen {

namespace {

//////////////////////////////////////////////////////////////////////

void retSurpriseCheck(HTS& env, SSATmp* frame, SSATmp* retVal) {
  ringbuffer(env, Trace::RBTypeFuncExit, curFunc(env)->fullName());
  ifThen(
    env,
    [&] (Block* taken) {
      gen(env, CheckSurpriseFlags, taken);
    },
    [&] {
      hint(env, Block::Hint::Unlikely);
      gen(env, ReturnHook, frame, retVal);
    }
  );
}

void freeLocalsAndThis(HTS& env) {
  auto const localCount = curFunc(env)->numLocals();
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

  if (curFunc(env)->mayHaveThis()) gen(env, DecRefThis, fp(env));
}

void normalReturn(HTS& env, SSATmp* retval) {
  gen(env, StRetVal, fp(env), retval);
  gen(env, RetAdjustStk, fp(env));
  auto const retAddr = gen(env, LdRetAddr, fp(env));
  gen(env, FreeActRec, fp(env));
  gen(env, RetCtrl, RetCtrlData { 0, false }, sp(env), fp(env), retAddr);
}

void asyncFunctionReturn(HTS& env, SSATmp* retval) {
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

  auto const retAddr = gen(env, LdRetAddr, fp(env));
  gen(env, FreeActRec, fp(env));

  // Decref the AsyncFunctionWaitHandle.  The TakeRef informs refcount-opts
  // that we're going to consume the reference.
  gen(env, TakeRef, resumableObj);
  gen(env, DecRef, resumableObj);

  gen(
    env,
    RetCtrl,
    RetCtrlData { offsetFromSP(env, 0), false },
    sp(env),
    fp(env),
    retAddr
  );
}

void generatorReturn(HTS& env, SSATmp* retval) {
  // Clear generator's key and value.
  auto const oldKey = gen(env, LdContArKey, Type::Cell, fp(env));
  gen(env, StContArKey, fp(env), cns(env, Type::InitNull));
  gen(env, DecRef, oldKey);

  auto const oldValue = gen(env, LdContArValue, Type::Cell, fp(env));
  gen(env, StContArValue, fp(env), cns(env, Type::InitNull));
  gen(env, DecRef, oldValue);

  gen(env,
      StContArState,
      GeneratorState { BaseGenerator::State::Done },
      fp(env));

  // Push return value of next()/send()/raise().
  push(env, cns(env, Type::InitNull));

  spillStack(env);
  auto const retAddr = gen(env, LdRetAddr, fp(env));
  gen(env, FreeActRec, fp(env));

  gen(
    env,
    RetCtrl,
    RetCtrlData { offsetFromSP(env, 0), false },
    sp(env),
    fp(env),
    retAddr
  );
}

void implRet(HTS& env, Type type) {
  if (curFunc(env)->attrs() & AttrMayUseVV) {
    // Note: this has to be the first thing, because we cannot bail after
    //       we start decRefing locs because then there'll be no corresponding
    //       bytecode boundaries until the end of RetC
    gen(env, ReleaseVVOrExit, makeExitSlow(env), fp(env));
  }

  // Pop the return value. Since it will be teleported to its place in memory,
  // we don't care about the type.
  auto const retval = pop(env, type, DataTypeGeneric);
  freeLocalsAndThis(env);
  retSurpriseCheck(env, fp(env), retval);

  if (curFunc(env)->isAsyncFunction()) {
    return asyncFunctionReturn(env, retval);
  }
  if (resumed(env)) {
    assert(curFunc(env)->isNonAsyncGenerator());
    return generatorReturn(env, retval);
  }
  return normalReturn(env, retval);
}

//////////////////////////////////////////////////////////////////////

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
    retFromInlined(env, Type::BoxedInitCell);
  } else {
    implRet(env, Type::BoxedInitCell);
  }
}

//////////////////////////////////////////////////////////////////////

}}}
