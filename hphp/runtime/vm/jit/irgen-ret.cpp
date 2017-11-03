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
#include "hphp/runtime/vm/jit/types.h"
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
      auto const ptr = resumeMode(env) != ResumeMode::None ? sp(env) : fp(env);
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
    for (unsigned i = 0; i < localCount; ++i) {
      env.irb->constrainLocal(i, DataTypeCountness, "inlined RetC/V");
    }
  } else {
    gen(env, GenericRetDecRefs, fp(env));
  }

  decRefThis(env);
}

void normalReturn(IRGS& env, SSATmp* retval) {
  if (RuntimeOption::EvalHHIRGenerateAsserts) {
    gen(env, DbgTrashRetVal, fp(env));
  }
  // If we're on the eager side of an async function, we have to zero-out the
  // TV aux of the return value, because it might be used as a flag if we were
  // called with FCallAwait.
  auto const aux =
    (curFunc(env)->isAsyncFunction() && resumeMode(env) == ResumeMode::None)
      ? folly::make_optional(AuxUnion{0})
      : folly::none;

  auto const data = RetCtrlData { offsetToReturnSlot(env), false, aux };
  gen(env, RetCtrl, data, sp(env), fp(env), retval);
}

void emitAsyncRetSlow(IRGS& env, SSATmp* retVal) {
  // Slow path: unblock all parents, then return.
  auto parentChain = gen(env, LdAsyncArParentChain, fp(env));
  gen(env, StAsyncArSucceeded, fp(env));
  gen(env, StAsyncArResult, fp(env), retVal);
  gen(env, ABCUnblock, parentChain);

  // Must load this before FreeActRec, which adjusts fp(env).
  auto const resumableObj = gen(env, LdResumableArObj, fp(env));
  gen(env, FreeActRec, fp(env));
  decRef(env, resumableObj);

  // Transfer control back to the asio scheduler. Make uninitialized space
  // on the stack for null "return value" to be written by the enterTCExit stub.
  auto const spAdjust = offsetFromIRSP(env, BCSPRelOffset{-1});
  gen(env, AsyncRetCtrl, IRSPRelOffsetData { spAdjust }, sp(env), fp(env));
}

void asyncRetSurpriseCheck(IRGS& env, SSATmp* retVal) {
  // The AsyncRet unique stub may or may not be able to do fast return (jump to
  // parent directly).  So we don't know for sure if return hook should be
  // called.  When profiling is enabled, we call the return hook, and follow the
  // slow path return (uncommon case).

  updateMarker(env);
  env.irb->exceptionStackBoundary();

  ifThen(
    env,
    [&] (Block* taken) {
      gen(env, CheckSurpriseFlags, taken, sp(env));
    },
    [&] {
      hint(env, Block::Hint::Unlikely);

      ringbufferMsg(env, Trace::RBTypeMsg, s_returnHook.get());
      gen(env, ReturnHook, fp(env), retVal);
      ringbufferMsg(env, Trace::RBTypeFuncExit, curFunc(env)->fullName());

      // Uncommon case: after calling the return hook, follow the slow path.
      // Next opcode is unreachable on this path.
      emitAsyncRetSlow(env, retVal);
    }
  );
  ringbufferMsg(env, Trace::RBTypeFuncExit, curFunc(env)->fullName());
}

void asyncFunctionReturn(IRGS& env, SSATmp* retVal) {
  if (resumeMode(env) == ResumeMode::None) {
    retSurpriseCheck(env, retVal);

    // Return from an eagerly-executed async function: wrap the return value in
    // a StaticWaitHandle object and return that normally, unless we were called
    // via FCallAwait
    auto const wrapped = cond(
      env,
      [&] (Block* taken) {
        auto flags = gen(env, LdARNumArgsAndFlags, fp(env));
        auto test = gen(
          env, AndInt, flags,
          cns(env, static_cast<int32_t>(ActRec::Flags::IsFCallAwait)));
        gen(env, JmpNZero, taken, test);
      },
      [&] {
        return gen(env, CreateSSWH, retVal);
      },
      [&] {
        return retVal;
      });
    normalReturn(env, wrapped);
    return;
  }

  // When surprise flag is set, the slow path is always used.  So fast path is
  // never reached in that case (e.g. when debugging).  Consider disabling this
  // when debugging the fast return path.
  asyncRetSurpriseCheck(env, retVal);

  // Call stub that will mark this AFWH as finished, unblock parents and
  // possibly take fast path to resume parent. Leave SP pointing to a single
  // uninitialized cell which will be filled by the stub.
  auto const spAdjust = offsetFromIRSP(env, BCSPRelOffset{-1});
  gen(env, AsyncRetFast, IRSPRelOffsetData { spAdjust }, sp(env), fp(env),
      retVal);
}

void generatorReturn(IRGS& env, SSATmp* retval) {
  // Clear generator's key.
  auto const oldKey = gen(env, LdContArKey, TCell, fp(env));
  gen(env, StContArKey, fp(env), cns(env, TInitNull));
  decRef(env, oldKey);

  // Populate the generator's value with retval to support `getReturn`
  auto const oldValue = gen(env, LdContArValue, TCell, fp(env));
  gen(env, StContArValue, fp(env), retval);
  decRef(env, oldValue);

  gen(env,
      StContArState,
      GeneratorState { BaseGenerator::State::Done },
      fp(env));

  auto const spAdjust = offsetFromIRSP(env, BCSPRelOffset{-1});
  gen(
    env,
    RetCtrl,
    RetCtrlData { spAdjust, true },
    sp(env),
    fp(env),
    cns(env, TInitNull)
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

  // Async function has its own surprise check.
  if (func->isAsyncFunction()) {
    return asyncFunctionReturn(env, retval);
  }

  retSurpriseCheck(env, retval);

  if (resumeMode(env) == ResumeMode::GenIter) {
    assertx(curFunc(env)->isNonAsyncGenerator());
    return generatorReturn(env, retval);
  }
  assertx(resumeMode(env) == ResumeMode::None);
  return normalReturn(env, retval);
}

//////////////////////////////////////////////////////////////////////

}

IRSPRelOffset offsetToReturnSlot(IRGS& env) {
  auto const retOff = FPRelOffset { kArRetOff / int32_t{sizeof(Cell)} };
  return retOff.to<IRSPRelOffset>(env.irb->fs().irSPOff());
}

void emitRetC(IRGS& env) {
  if (curFunc(env)->isAsyncGenerator()) PUNT(RetC-AsyncGenerator);

  if (isInlining(env)) {
    assertx(resumeMode(env) == ResumeMode::None);
    retFromInlined(env);
  } else {
    implRet(env);
  }
}

void emitRetV(IRGS& env) {
  assertx(resumeMode(env) == ResumeMode::None);
  assertx(!curFunc(env)->isResumable());
  if (isInlining(env)) {
    retFromInlined(env);
  } else {
    implRet(env);
  }
}

//////////////////////////////////////////////////////////////////////

}}}
