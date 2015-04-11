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
#include "hphp/runtime/vm/jit/irgen-inlining.h"

#include "hphp/runtime/vm/jit/analysis.h"

#include "hphp/runtime/vm/jit/irgen-guards.h"
#include "hphp/runtime/vm/jit/irgen-exit.h"
#include "hphp/runtime/vm/jit/irgen-sprop-global.h"

namespace HPHP { namespace jit { namespace irgen {

bool isInlining(const IRGS& env) {
  return env.inlineLevel > 0;
}

/*
 * When doing gen-time inlining, we set up a series of IR instructions
 * that looks like this:
 *
 *   fp0  = DefFP
 *   sp0  = DefSP<offset>
 *
 *   // ... normal stuff happens ...
 *   // sp_pre = some SpillStack, or maybe the DefSP
 *
 *   // FPI region:
 *     sp1   = SpillStack sp_pre, ...
 *     sp2   = SpillFrame sp1, ...
 *     // ... possibly more spillstacks due to argument expressions
 *     sp3   = SpillStack sp2, -argCount
 *     fp2   = DefInlineFP<func,retBC,retSP,off> sp2 sp1
 *     sp4   = ReDefSP<spOffset,spansCall> sp1 fp2
 *
 *         // ... callee body ...
 *
 *           = InlineReturn fp2
 *
 * [ sp5  = ResetSP<spOffset> fp0 ]
 *
 * The rest of the code then depends on sp5, and not any of the StkPtr
 * tree going through the callee body.  The sp5 tmp has the same view
 * of the stack as sp1 did, which represents what the stack looks like
 * before the return address is pushed but after the activation record
 * is popped.
 *
 * In DCE we attempt to remove the SpillFrame, InlineReturn, and
 * DefInlineFP instructions if they aren't needed.
 *
 * ReDefSP takes sp1, the stack pointer from before the inlined frame.
 * This SSATmp may be used for determining stack types in the
 * simplifier, or stack values if the inlined body doesn't contain a
 * call---these instructions both take an extradata `spansCall' which
 * is true iff a Call occured anywhere between the the definition of
 * its first argument and itself.
 */
void beginInlining(IRGS& env,
                   unsigned numParams,
                   const Func* target,
                   Offset returnBcOffset) {
  assertx(!env.fpiStack.empty() &&
    "Inlining does not support calls with the FPush* in a different Tracelet");
  assertx(returnBcOffset >= 0 && "returnBcOffset before beginning of caller");
  assertx(curFunc(env)->base() + returnBcOffset < curFunc(env)->past() &&
         "returnBcOffset past end of caller");

  FTRACE(1, "[[[ begin inlining: {}\n", target->fullName()->data());

  SSATmp* params[numParams];
  for (unsigned i = 0; i < numParams; ++i) {
    params[numParams - i - 1] = popF(env);
  }

  auto const prevSP    = env.fpiStack.top().returnSP;
  auto const prevSPOff = env.fpiStack.top().returnSPOff;
  spillStack(env);
  auto const calleeSP  = sp(env);

  always_assert_flog(
    env.fpiStack.top().spillFrame != nullptr,
    "Couldn't find SpillFrame for inlined call on sp {}."
    " Was the FPush instruction interpreted?",
    *calleeSP->inst()
  );

  auto const sframe = env.fpiStack.top().spillFrame;

  DefInlineFPData data;
  data.target        = target;
  data.retBCOff      = returnBcOffset;
  data.fromFPushCtor = sframe->extra<ActRecInfo>()->isFromFPushCtor();
  data.ctx           = sframe->src(2);
  data.retSPOff      = prevSPOff;
  data.spOffset      = offsetFromIRSP(env, BCSPOffset{0}).offset;

  // Push state and update the marker before emitting any instructions so
  // they're all given markers in the callee.
  auto const key = SrcKey {
    target,
    target->getEntryForNumArgs(numParams),
    false
  };
  env.bcStateStack.emplace_back(key);
  env.inlineLevel++;
  updateMarker(env);

  auto const calleeFP = gen(env, DefInlineFP, data, calleeSP, prevSP, fp(env));
  gen(env, ReDefSP, StackOffset{target->numLocals()}, fp(env));

  for (unsigned i = 0; i < numParams; ++i) {
    stLocRaw(env, i, calleeFP, params[i]);
  }
  for (unsigned i = numParams; i < target->numLocals(); ++i) {
    /*
     * Here we need to be generating hopefully-dead stores to initialize
     * non-parameter locals to KindOfUninit in case we have to leave the trace.
     */
    stLocRaw(env, i, calleeFP, cns(env, TUninit));
  }

  env.fpiActiveStack.push(std::make_pair(env.fpiStack.top().returnSP,
                                         env.fpiStack.top().returnSPOff));
  env.fpiStack.pop();
}

void endInlinedCommon(IRGS& env) {
  assertx(!env.fpiActiveStack.empty());
  assertx(!curFunc(env)->isPseudoMain());

  assertx(!resumed(env));

  decRefLocalsInline(env);
  if (curFunc(env)->mayHaveThis()) {
    gen(env, DecRefThis, fp(env));
  }

  /*
   * Pop the ActRec and restore the stack and frame pointers.  It's
   * important that this does endInlining before pushing the return
   * value so stack offsets are properly tracked.
   */
  gen(env, InlineReturn, fp(env));

  // Return to the caller function.  Careful between here and the
  // updateMarker() below, where the caller state isn't entirely set up.
  env.inlineLevel--;
  env.bcStateStack.pop_back();
  always_assert(env.bcStateStack.size() > 0);

  env.fpiActiveStack.pop();

  updateMarker(env);
  gen(env, ResetSP, StackOffset{env.irb->spOffset().offset}, fp(env));

  /*
   * After the end of inlining, we are restoring to a previously defined stack
   * that we know is entirely materialized (i.e. in memory), so stackDeficit
   * needs to be slammed to zero.
   *
   * The push of the return value in the caller of this function is not yet
   * materialized.
   */
  assertx(env.irb->evalStack().empty());
  env.irb->clearStackDeficit();

  FTRACE(1, "]]] end inlining: {}\n", curFunc(env)->fullName()->data());
}

void retFromInlined(IRGS& env, Type type) {
  auto const retVal = pop(env, type, DataTypeGeneric);
  endInlinedCommon(env);
  push(env, retVal);
}

//////////////////////////////////////////////////////////////////////

void inlSingletonSLoc(IRGS& env, const Func* func, const Op* op) {
  assertx(*op == Op::StaticLocInit);

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
                       const Op* clsOp,
                       const Op* propOp) {
  assertx(*clsOp == Op::String);
  assertx(*propOp == Op::String);

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
