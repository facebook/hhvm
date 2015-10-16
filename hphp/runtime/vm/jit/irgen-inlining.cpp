/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2015 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/runtime/vm/jit/irgen-inlining.h"

#include "hphp/runtime/vm/jit/analysis.h"

#include "hphp/runtime/vm/jit/irgen-guards.h"
#include "hphp/runtime/vm/jit/irgen-exit.h"
#include "hphp/runtime/vm/jit/irgen-sprop-global.h"

#include "hphp/runtime/vm/hhbc-codec.h"

namespace HPHP { namespace jit { namespace irgen {

bool isInlining(const IRGS& env) {
  return env.inlineLevel > 0;
}

/*
 * Attempts to begin inlining, and returns whether or not it successed.
 *
 * When doing gen-time inlining, we set up a series of IR instructions
 * that looks like this:
 *
 *   fp0  = DefFP
 *   sp   = DefSP<offset>
 *
 *   // ... normal stuff happens ...
 *
 *   // FPI region:
 *     SpillFrame sp, ...
 *     // ... probably some StStks due to argument expressions
 *     fp2   = DefInlineFP<func,retBC,retSP,off> sp
 *
 *         // ... callee body ...
 *
 *     InlineReturn fp2
 *
 * In DCE we attempt to remove the InlineReturn and DefInlineFP instructions if
 * they aren't needed.
 */
bool beginInlining(IRGS& env,
                   unsigned numParams,
                   const Func* target,
                   Offset returnBcOffset,
                   Block* returnTarget,
                   bool multipleReturns) {
  auto const& fpiStack = env.irb->fpiStack();

  assertx(!fpiStack.empty() &&
    "Inlining does not support calls with the FPush* in a different Tracelet");
  assertx(returnBcOffset >= 0 && "returnBcOffset before beginning of caller");
  assertx(curFunc(env)->base() + returnBcOffset < curFunc(env)->past() &&
         "returnBcOffset past end of caller");

  FTRACE(1, "[[[ begin inlining: {}\n", target->fullName()->data());

  SSATmp** params = (SSATmp**)alloca(sizeof(SSATmp*) * numParams);
  for (unsigned i = 0; i < numParams; ++i) {
    params[numParams - i - 1] = popF(env);
  }

  auto const prevSP    = fpiStack.front().returnSP;
  auto const prevSPOff = fpiStack.front().returnSPOff;
  auto const calleeSP  = sp(env);

  always_assert_flog(
    prevSP == calleeSP,
    "FPI stack pointer and callee stack pointer didn't match in beginInlining"
  );

  auto const& info = fpiStack.front();
  always_assert(!isFPushCuf(info.fpushOpc) && !info.interp);

  auto ctx = [&] {
    if (info.ctx || isFPushFunc(info.fpushOpc)) {
      return info.ctx;
    }

    constexpr int32_t adjust = offsetof(ActRec, m_r) - offsetof(ActRec, m_this);
    IRSPOffset ctxOff{invSPOff(env) - info.returnSPOff - adjust};
    return gen(env, LdStk, TCtx, IRSPOffsetData{ctxOff}, sp(env));
  }();

  if (debug) {
    IRSPOffset arOff = offsetFromIRSP(env, BCSPOffset{0});
    auto arFunc = gen(env, LdARFuncPtr, IRSPOffsetData{arOff}, sp(env));
    gen(env, DbgAssertFunc, arFunc, cns(env, target));
  }

  auto fpiFunc = fpiStack.front().func;
  always_assert_flog(fpiFunc == nullptr || fpiFunc == target,
                     "fpiFunc = {}  ;  target = {}",
                     fpiFunc ? fpiFunc->fullName()->data() : "null",
                     target  ? target->fullName()->data()  : "null");

  DefInlineFPData data;
  data.target        = target;
  data.retBCOff      = returnBcOffset;
  data.fromFPushCtor = isFPushCtor(info.fpushOpc);
  data.ctx           = ctx;
  data.retSPOff      = prevSPOff;
  data.spOffset      = offsetFromIRSP(env, BCSPOffset{0});
  data.numNonDefault = numParams;

  // Push state and update the marker before emitting any instructions so
  // they're all given markers in the callee.
  auto const key = SrcKey {
    target,
    target->getEntryForNumArgs(numParams),
    false
  };
  env.bcStateStack.emplace_back(key);
  env.inlineReturnTarget.emplace_back(
    ReturnTarget { returnTarget, multipleReturns }
  );
  env.inlineLevel++;
  updateMarker(env);

  auto const calleeFP = gen(env, DefInlineFP, data, calleeSP, fp(env));

  for (unsigned i = 0; i < numParams; ++i) {
    stLocRaw(env, i, calleeFP, params[i]);
  }
  const bool hasVariadicArg = target->hasVariadicCaptureParam();
  for (unsigned i = numParams; i < target->numLocals() - hasVariadicArg; ++i) {
    /*
     * Here we need to be generating hopefully-dead stores to initialize
     * non-parameter locals to KindOfUninit in case we have to leave the trace.
     */
    stLocRaw(env, i, calleeFP, cns(env, TUninit));
  }
  if (hasVariadicArg) {
    auto argNum = target->numLocals() - 1;
    always_assert(numParams <= argNum);
    stLocRaw(env, argNum, calleeFP, cns(env, staticEmptyArray()));
  }

  return true;
}

void endInlinedCommon(IRGS& env) {
  assertx(!curFunc(env)->isPseudoMain());

  assertx(!resumed(env));

  decRefLocalsInline(env);
  decRefThis(env);

  gen(env, InlineReturn, fp(env));

  // Return to the caller function.  Careful between here and the
  // updateMarker() below, where the caller state isn't entirely set up.
  env.inlineLevel--;
  env.bcStateStack.pop_back();
  env.inlineReturnTarget.pop_back();
  always_assert(env.bcStateStack.size() > 0);

  updateMarker(env);

  FTRACE(1, "]]] end inlining: {}\n", curFunc(env)->fullName()->data());
}

void endInlining(IRGS& env) {
  auto const retVal = pop(env, DataTypeGeneric);
  endInlinedCommon(env);
  push(env, retVal);
}

void retFromInlined(IRGS& env) {
  gen(env, Jmp, env.inlineReturnTarget.back().target);
}

//////////////////////////////////////////////////////////////////////

void inlSingletonSLoc(IRGS& env, const Func* func, PC op) {
  assertx(peek_op(op) == Op::StaticLocInit);

  TransFlags trflags;
  trflags.noinlineSingleton = true;

  auto exit = makeExit(env, trflags);
  auto const name = func->unit()->lookupLitstrId(getImmPtr(op, 1)->u_SA);

  // Side exit if the static local is uninitialized.
  auto const box = gen(env, LdStaticLocCached, StaticLocName { func, name });
  gen(env, CheckStaticLocInit, exit, box);

  // Side exit if the static local is null.
  auto const value  = gen(env, LdRef, TInitCell, box);
  auto const isnull = gen(env, IsType, TInitNull, value);
  gen(env, JmpNZero, exit, isnull);

  // Return the singleton.
  pushIncRef(env, value);
}

void inlSingletonSProp(IRGS& env,
                       const Func* func,
                       PC clsOp,
                       PC propOp) {
  assertx(peek_op(clsOp) == Op::String);
  assertx(peek_op(propOp) == Op::String);

  TransFlags trflags;
  trflags.noinlineSingleton = true;

  auto exitBlock = makeExit(env, trflags);

  // Pull the class and property names.
  auto const unit = func->unit();
  auto const clsName  = unit->lookupLitstrId(getImmPtr(clsOp,  0)->u_SA);
  auto const propName = unit->lookupLitstrId(getImmPtr(propOp, 0)->u_SA);

  // Make sure we have a valid class.
  auto const cls = Unit::lookupClass(clsName);
  if (UNLIKELY(!classHasPersistentRDS(cls))) {
    PUNT(SingletonSProp-Persistent);
  }

  // Make sure the sprop is accessible from the singleton method's context.
  auto const lookup = cls->findSProp(func->cls(), propName);
  if (UNLIKELY(lookup.prop == kInvalidSlot || !lookup.accessible)) {
    PUNT(SingletonSProp-Accessibility);
  }

  // Look up the static property.
  auto const sprop   = ldClsPropAddrKnown(env, cls, propName);
  auto const unboxed = gen(env, UnboxPtr, sprop);
  auto const value   = gen(env, LdMem, unboxed->type().deref(), unboxed);

  // Side exit if the static property is null.
  auto isnull = gen(env, IsType, TNull, value);
  gen(env, JmpNZero, exitBlock, isnull);

  // Return the singleton.
  pushIncRef(env, value);
}

//////////////////////////////////////////////////////////////////////

}}}
