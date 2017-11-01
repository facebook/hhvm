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
#include "hphp/runtime/vm/jit/irgen-inlining.h"

#include "hphp/runtime/vm/jit/analysis.h"

#include "hphp/runtime/vm/jit/irgen-call.h"
#include "hphp/runtime/vm/jit/irgen-exit.h"
#include "hphp/runtime/vm/jit/irgen-func-prologue.h"
#include "hphp/runtime/vm/jit/irgen-sprop-global.h"

#include "hphp/runtime/vm/hhbc-codec.h"
#include "hphp/runtime/vm/resumable.h"

namespace HPHP { namespace jit { namespace irgen {

bool isInlining(const IRGS& env) {
  return env.inlineLevel > 0;
}

bool beginInlining(IRGS& env,
                   unsigned numParams,
                   const Func* target,
                   SrcKey startSk,
                   Offset returnBcOffset,
                   ReturnTarget returnTarget,
                   int cost) {
  auto const& fpiStack = env.irb->fs().fpiStack();

  assertx(!fpiStack.empty() &&
    "Inlining does not support calls with the FPush* in a different Tracelet");
  assertx(returnBcOffset >= 0 && "returnBcOffset before beginning of caller");
  // curFunc is null when called from conjureBeginInlining
  assertx((!curFunc(env) ||
          curFunc(env)->base() + returnBcOffset < curFunc(env)->past()) &&
         "returnBcOffset past end of caller");

  FTRACE(1, "[[[ begin inlining: {}\n", target->fullName()->data());

  auto const& info = fpiStack.back();
  if (info.func && info.func != target) {
    // Its possible that we have an "FCallD T2 meth" guarded by eg an
    // InstanceOfD T2, and that we know the object has type T1, and we
    // also know that T1::meth exists. The FCallD is actually
    // unreachable, but we might not have figured that out yet - so we
    // could be trying to inline T1::meth while the fpiStack has
    // T2::meth.
    return false;
  }

  always_assert(isFPush(info.fpushOpc) &&
                !isFPushCuf(info.fpushOpc) &&
                info.inlineEligible);

  auto const prevSP = fpiStack.back().returnSP;
  auto const prevBCSPOff = fpiStack.back().returnSPOff;
  auto const calleeSP = sp(env);

  always_assert_flog(
    prevSP == calleeSP,
    "FPI stack pointer and callee stack pointer didn't match in beginInlining"
  );

  // The VM stack-pointer is conceptually pointing to the last
  // parameter, so we need to add numParams to get to the ActRec
  IRSPRelOffset calleeAROff = spOffBCFromIRSP(env) + numParams;

  auto ctx = [&] () -> SSATmp* {
    if (!target->implCls()) {
      return nullptr;
    }
    auto ty = info.ctxType;
    if (!target->isClosureBody()) {
      if (target->isStaticInPrologue() ||
          (!hasThis(env) &&
           isFPushClsMethod(info.fpushOpc))) {
        assertx(!ty.maybe(TObj));
        if (ty.hasConstVal(TCctx)) {
          ty = Type::ExactCls(ty.cctxVal().cls());
        } else if (!ty.hasConstVal(TCls)) {
          if (!ty.maybe(TCls)) ty = TCls;
          ty &= Type::SubCls(target->cls());
        }
      } else {
        if (target->attrs() & AttrRequiresThis ||
            isFPushObjMethod(info.fpushOpc) ||
            ty <= TObj) {
          ty &= Type::SubObj(target->cls());
        }
      }
    }
    if (info.ctx && !info.ctx->isA(TNullptr)) {
      if (info.ctx->type() <= ty) {
        return info.ctx;
      }
      if (info.ctx->type().maybe(ty)) {
        return gen(env, AssertType, ty, info.ctx);
      }
      if (info.ctx->type() <= TCctx && ty <= TCls) {
        return gen(env, AssertType, ty, gen(env, LdClsCctx, info.ctx));
      }
    }
    if (ty <= TObj) {
      return gen(env, LdARCtx, ty, IRSPRelOffsetData{calleeAROff}, sp(env));
    }
    if (ty <= TCls) {
      auto const cctx =
        gen(env, LdARCtx, TCctx, IRSPRelOffsetData{calleeAROff}, sp(env));
      return gen(env, AssertType, ty, gen(env, LdClsCctx, cctx));
    }
    return nullptr;
  }();

  // If the ctx was extracted from SpillFrame it may be a TCls, otherwise it
  // will be a TCtx (= TObj | TCctx) read from the stack
  assertx(!ctx || (ctx->type() <= (TCtx | TCls) && target->implCls()));

  jit::vector<SSATmp*> params{numParams};
  for (unsigned i = 0; i < numParams; ++i) {
    params[numParams - i - 1] = popF(env);
  }

  // NB: Now that we've popped the callee's arguments off the stack
  // and thus modified the caller's frame state, we're committed to
  // inlining. If we bail out from now on, the caller's frame state
  // will be as if the arguments don't exist on the stack (even though
  // they do).

  if (RuntimeOption::EvalHHIRGenerateAsserts) {
    gen(env, DbgAssertARFunc, IRSPRelOffsetData{calleeAROff},
        sp(env), cns(env, target));
  }

  gen(
    env,
    BeginInlining,
    BeginInliningData{calleeAROff, target, cost},
    sp(env)
  );

  DefInlineFPData data;
  data.target        = target;
  data.retBCOff      = returnBcOffset;
  data.ctx           = target->isClosureBody() ? nullptr : ctx;
  data.retSPOff      = prevBCSPOff;
  data.spOffset      = calleeAROff;
  data.numNonDefault = numParams;

  assertx(startSk.func() == target &&
          startSk.offset() == target->getEntryForNumArgs(numParams) &&
          startSk.resumeMode() == ResumeMode::None);

  env.bcStateStack.emplace_back(startSk);
  env.inlineReturnTarget.emplace_back(returnTarget);
  env.inlineLevel++;
  updateMarker(env);

  auto const calleeFP = gen(env, DefInlineFP, data, calleeSP, fp(env));

  for (unsigned i = 0; i < numParams; ++i) {
    stLocRaw(env, i, calleeFP, params[i]);
  }
  emitPrologueLocals(env, numParams, target, ctx);

  // "Kill" all the class-ref slots initially. This normally won't do anything
  // (the class-ref slots should be unoccupied at this point), but in debugging
  // builds it will write poison values to them.
  for (uint32_t slot = 0; slot < target->numClsRefSlots(); ++slot) {
    killClsRef(env, slot);
  }

  if (data.ctx && data.ctx->isA(TObj)) {
    assertx(startSk.hasThis());
  } else if (data.ctx && !data.ctx->type().maybe(TObj)) {
    assertx(!startSk.hasThis());
  } else if (target->cls()) {
    auto const psk =
      SrcKey{startSk.func(), startSk.offset(), SrcKey::PrologueTag{}};
    env.bcStateStack.back() = psk;
    updateMarker(env);

    auto sideExit = [&] (bool hasThis) {
      hint(env, Block::Hint::Unlikely);
      auto const sk =
        SrcKey { startSk.func(), startSk.offset(), ResumeMode::None, hasThis };
      gen(
        env,
        ReqBindJmp,
        ReqBindJmpData {
          sk,
          FPInvOffset { startSk.func()->numSlotsInFrame() },
          spOffBCFromIRSP(env),
          TransFlags{}
        },
        sp(env),
        fp(env)
      );
    };

    ifThenElse(
      env,
      [&] (Block* taken) {
        auto const maybeThis = gen(env, LdCtx, fp(env));
        gen(env, CheckCtxThis, taken, maybeThis);
      },
      [&] {
        if (!startSk.hasThis()) {
          sideExit(true);
        }
      },
      [&] {
        if (startSk.hasThis()) {
          sideExit(false);
        }
      }
    );

    env.bcStateStack.back() = startSk;
    updateMarker(env);
  }

  return true;
}

bool conjureBeginInlining(IRGS& env,
                          const Func* func,
                          SrcKey startSk,
                          Type thisType,
                          const std::vector<Type>& args,
                          ReturnTarget returnTarget) {
  auto conjure = [&](Type t) {
    return t.admitsSingleVal() ? cns(env, t) : gen(env, Conjure, t);
  };

  always_assert(isFPush(env.context.callerFPushOp));
  auto const numParams = args.size();
  env.irb->fs().setFPushOverride(env.context.callerFPushOp);
  fpushActRec(
    env,
    cns(env, func),
    thisType != TBottom ? conjure(thisType) : nullptr,
    numParams,
    nullptr /* invName */
  );
  assertx(!env.irb->fs().hasFPushOverride());

  for (auto const argType : args) {
    push(env, conjure(argType));
  }

  return beginInlining(
    env,
    numParams,
    func,
    startSk,
    0 /* returnBcOffset */,
    returnTarget,
    9 /* cost */
  );
}

void implInlineReturn(IRGS& env) {
  assertx(!curFunc(env)->isPseudoMain());
  assertx(resumeMode(env) == ResumeMode::None);

  auto const& fs = env.irb->fs();

  // The offset of our caller's FP relative to our own.
  auto const callerFPOff =
    // Offset of the (unchanged) vmsp relative to our fp...
    - fs.irSPOff()
    // ...plus the offset of our parent's fp relative to vmsp.
    + FPInvOffset{0}.to<IRSPRelOffset>(fs.callerIRSPOff()).offset;

  // Return to the caller function.
  gen(env, InlineReturn, FPRelOffsetData { callerFPOff }, fp(env));

  // Pop the inlined frame in our IRGS.  Be careful between here and the
  // updateMarker() below, where the caller state isn't entirely set up.
  env.inlineLevel--;
  env.bcStateStack.pop_back();
  env.inlineReturnTarget.pop_back();
  always_assert(env.bcStateStack.size() > 0);

  updateMarker(env);

  FTRACE(1, "]]] end inlining: {}\n", curFunc(env)->fullName()->data());
}

void endInlining(IRGS& env) {
  // The IR instructions should be associated with one of the return bytecodes,
  // which should be one of the predecessors of this block.
  auto const curBlock = env.irb->curBlock();
  always_assert(curBlock && !curBlock->preds().empty());
  auto const bcContext = curBlock->preds().front().inst()->bcctx();
  env.bcStateStack.back().setOffset(bcContext.marker.sk().offset());
  updateMarker(env);
  env.irb->resetCurIROff(bcContext.iroff + 1);

  decRefLocalsInline(env);
  decRefThis(env);

  auto const retTy = callReturnType(curFunc(env));
  auto const retVal = pop(env, DataTypeGeneric);

  implInlineReturn(env);
  push(env, gen(env, AssertType, retTy, retVal));
}

void conjureEndInlining(IRGS& env, bool builtin) {
  if (!builtin) {
    endInlining(env);
  }
  gen(env, ConjureUse, pop(env));
  gen(env, EndBlock);
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
  gen(env, CheckStaticLoc, StaticLocName { func, name }, exit);
  auto const box = gen(env, LdStaticLoc, StaticLocName { func, name });

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
