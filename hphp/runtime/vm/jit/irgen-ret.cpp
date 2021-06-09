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
#include "hphp/runtime/vm/jit/irgen-ret.h"


#include "hphp/runtime/vm/resumable.h"
#include "hphp/runtime/vm/jit/analysis.h"
#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/irgen.h"
#include "hphp/runtime/vm/jit/irgen-exit.h"
#include "hphp/runtime/vm/jit/irgen-inlining.h"

#include "hphp/runtime/vm/jit/irgen-internal.h"

namespace HPHP { namespace jit { namespace irgen {

namespace {

//////////////////////////////////////////////////////////////////////

const StaticString s_returnHook("SurpriseReturnHook");

template<class AH>
void retSurpriseCheck(IRGS& env, SSATmp* retVal, AH afterHook) {
  /*
   * This is a weird situation for throwing: we've partially torn down the
   * ActRec (decref'd all the frame's locals), and we've popped the return
   * value.  If we throw, the unwinder needs to know the return value is not on
   * the eval stack, so we need to sync the marker after the pop---the return
   * value will be decref'd by the return hook while unwinding, in this case.
   */
  updateMarker(env);
  env.irb->exceptionStackBoundary();

  ifThenElse(
    env,
    [&] (Block* taken) {
      auto const ptr = resumeMode(env) != ResumeMode::None ? sp(env) : fp(env);
      gen(env, CheckSurpriseFlags, taken, ptr);
    },
    [&] {
      ringbufferMsg(env, Trace::RBTypeFuncExit, curFunc(env)->fullName());
    },
    [&] {
      hint(env, Block::Hint::Unlikely);
      ringbufferMsg(env, Trace::RBTypeMsg, s_returnHook.get());
      gen(env, ReturnHook, fp(env), retVal);
      ringbufferMsg(env, Trace::RBTypeFuncExit, curFunc(env)->fullName());
      afterHook();
    }
  );
}

void freeLocalsAndThis(IRGS& env) {
  auto const localCount = curFunc(env)->numLocals();

  auto const shouldFreeInline = [&]() -> bool {
    // We don't want to specialize on arg types for builtins
    if (curFunc(env)->arFuncPtr()) return false;

    if (localCount > RuntimeOption::EvalHHIRInliningMaxReturnLocals) {
      return false;
    }
    auto numRefCounted = int{0};
    for (auto i = uint32_t{0}; i < localCount; ++i) {
      if (env.irb->local(i, DataTypeGeneric).type.maybe(TCounted)) {
        ++numRefCounted;
      }
    }
    return numRefCounted <= RuntimeOption::EvalHHIRInliningMaxReturnDecRefs;
  }();

  if (shouldFreeInline) {
    decRefLocalsInline(env);
  } else {
    gen(env, GenericRetDecRefs, fp(env));
  }

  decRefThis(env);
}

void normalReturn(IRGS& env, SSATmp* retval, bool suspended) {
  assertx(resumeMode(env) == ResumeMode::None);
  // If we're on the eager side of an async function, we have to zero-out the
  // TV aux of the return value, because it might be used as a flag if async
  // eager return was requested.
  auto const aux = [&] {
    if (suspended) return AuxUnion{0};
    if (curFunc(env)->isAsyncFunction()) {
      return AuxUnion{std::numeric_limits<uint32_t>::max()};
    }
    return AuxUnion{0};
  }();

  auto const data = RetCtrlData { offsetToReturnSlot(env), false, aux };
  gen(env, RetCtrl, data, sp(env), fp(env), retval);
}

void asyncFunctionReturn(IRGS& env, SSATmp* retVal, bool suspended) {
  if (resumeMode(env) == ResumeMode::None) {
    retSurpriseCheck(env, retVal, []{});

    if (suspended) return normalReturn(env, retVal, true);

    // Return from an eagerly-executed async function: wrap the return value in
    // a StaticWaitHandle object and return that normally, unless async eager
    // return was requested.
    auto const wrapped = cond(
      env,
      [&] (Block* taken) {
        auto flags = gen(env, LdARFlags, fp(env));
        auto test = gen(
          env, AndInt, flags,
          cns(env, static_cast<int32_t>(1 << ActRec::AsyncEagerRet)));
        gen(env, JmpNZero, taken, test);
      },
      [&] {
        return gen(env, CreateSSWH, retVal);
      },
      [&] {
        return retVal;
      });
    normalReturn(env, wrapped, false);
    return;
  }
  assertx(!suspended);

  auto const spAdjust = offsetFromIRSP(env, BCSPRelOffset{-1});

  // When surprise flag is set, the slow path is always used.
  retSurpriseCheck(env, retVal, [&] {
    gen(env, AsyncFuncRetSlow, IRSPRelOffsetData { spAdjust }, sp(env), fp(env),
        retVal);
  });

  // Call stub that will mark this AFWH as finished, unblock parents and
  // possibly take fast path to resume parent. Leave SP pointing to a single
  // uninitialized cell which will be filled by the stub.
  gen(env, AsyncFuncRet, IRSPRelOffsetData { spAdjust }, sp(env), fp(env),
      retVal);
}

void generatorReturn(IRGS& env, SSATmp* retval) {
  assertx(resumeMode(env) == ResumeMode::GenIter);
  retSurpriseCheck(env, retval, []{});

  if (!curFunc(env)->isAsync()) {
    // Clear generator's key.
    auto const oldKey = gen(env, LdContArKey, TInitCell, fp(env));
    gen(env, StContArKey, fp(env), cns(env, TInitNull));
    decRef(env, oldKey);

    // Populate the generator's value with retval to support `getReturn`
    auto const oldValue = gen(env, LdContArValue, TInitCell, fp(env));
    gen(env, StContArValue, fp(env), retval);
    decRef(env, oldValue);
    retval = cns(env, TInitNull);
  } else {
    assertx(retval->isA(TInitNull));
    retval = gen(env, CreateSSWH, cns(env, TInitNull));
  }

  gen(env,
      StContArState,
      GeneratorState { BaseGenerator::State::Done },
      fp(env));

  // Return control to the caller (Gen::next()).
  auto const spAdjust = offsetFromIRSP(env, BCSPRelOffset{-1});
  auto const retData = RetCtrlData { spAdjust, true, AuxUnion{0} };
  gen(env, RetCtrl, retData, sp(env), fp(env), retval);
}

void implRet(IRGS& env, bool suspended) {
  auto func = curFunc(env);
  assertx(!suspended || func->isAsyncFunction());
  assertx(!suspended || resumeMode(env) == ResumeMode::None);

  // Pop the return value. Since it will be teleported to its place in memory,
  // we don't care about the type.
  auto const retval = pop(env, DataTypeGeneric);

  freeLocalsAndThis(env);

  // Async function has its own surprise check.
  if (func->isAsyncFunction()) {
    return asyncFunctionReturn(env, retval, suspended);
  }

  if (func->isGenerator()) {
    return generatorReturn(env, retval);
  }

  assertx(resumeMode(env) == ResumeMode::None);
  retSurpriseCheck(env, retval, []{});
  return normalReturn(env, retval, false);
}

//////////////////////////////////////////////////////////////////////

}

IRSPRelOffset offsetToReturnSlot(IRGS& env) {
  assertx(resumeMode(env) == ResumeMode::None);
  auto const fpOff = offsetOfFrame(fp(env));
  assertx(fpOff);
  return *fpOff + kArRetOff / int32_t{sizeof(TypedValue)};
}

void emitRetC(IRGS& env) {
  if (curFunc(env)->isAsyncGenerator() &&
      resumeMode(env) == ResumeMode::Async) {
    PUNT(RetC-AsyncGenerator);
  }

  if (isInlining(env)) {
    assertx(resumeMode(env) == ResumeMode::None);
    retFromInlined(env);
  } else {
    implRet(env, false);
  }
}

void emitRetM(IRGS& env, uint32_t nvals) {
  assertx(resumeMode(env) == ResumeMode::None);
  assertx(!curFunc(env)->isResumable());
  assertx(nvals > 1);

  if (isInlining(env)) {
    retFromInlined(env);
    return;
  }

  // Pop the return values. Since they will be teleported to their places in
  // memory, we don't care about their types.
  for (int i = 0; i < nvals - 1; i++) {
    gen(env, StOutValue, IndexData(i), fp(env), pop(env, DataTypeGeneric));
  }

  implRet(env, false);
}

void emitRetCSuspended(IRGS& env) {
  assertx(curFunc(env)->isAsyncFunction());
  assertx(resumeMode(env) == ResumeMode::None);

  if (isInlining(env)) {
    suspendFromInlined(env, pop(env, DataTypeGeneric));
  } else {
    implRet(env, true);
  }
}

//////////////////////////////////////////////////////////////////////

}}}
