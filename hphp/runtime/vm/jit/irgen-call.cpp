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
#include "hphp/runtime/vm/jit/irgen-call.h"

#include "hphp/runtime/base/stats.h"
#include "hphp/runtime/vm/method-lookup.h"
#include "hphp/runtime/vm/reified-generics.h"
#include "hphp/runtime/vm/runtime.h"

#include "hphp/runtime/vm/jit/call-target-profile.h"
#include "hphp/runtime/vm/jit/guard-constraint.h"
#include "hphp/runtime/vm/jit/meth-profile.h"
#include "hphp/runtime/vm/jit/normalized-instruction.h"
#include "hphp/runtime/vm/jit/target-profile.h"
#include "hphp/runtime/vm/jit/translate-region.h"
#include "hphp/runtime/vm/jit/type.h"
#include "hphp/runtime/vm/jit/type-array-elem.h"

#include "hphp/runtime/vm/jit/irgen-basic.h"
#include "hphp/runtime/vm/jit/irgen-builtin.h"
#include "hphp/runtime/vm/jit/irgen-control.h"
#include "hphp/runtime/vm/jit/irgen-create.h"
#include "hphp/runtime/vm/jit/irgen-exit.h"
#include "hphp/runtime/vm/jit/irgen-internal.h"
#include "hphp/runtime/vm/jit/irgen-interpone.h"
#include "hphp/runtime/vm/jit/irgen-types.h"

namespace HPHP { namespace jit { namespace irgen {

//////////////////////////////////////////////////////////////////////

namespace {

bool emitCallerReffinessChecksKnown(IRGS& env, const Func* callee,
                                    const FCallArgs& fca) {
  if (!fca.enforceReffiness()) return true;

  for (auto i = 0; i < fca.numArgs; ++i) {
    if (callee->byRef(i) != fca.byRef(i)) {
      auto const func = cns(env, callee);
      gen(env, ThrowParamRefMismatch, ParamData { i }, func);
      return false;
    }
  }
  return true;
}

void emitCallerReffinessChecksUnknown(IRGS& env, SSATmp* callee,
                                      const FCallArgs& fca) {
  if (!fca.enforceReffiness()) return;

  SSATmp* numParams = nullptr;
  for (uint32_t i = 0; i * 8 < fca.numArgs; i += 8) {
    uint64_t vals = 0;
    for (uint32_t j = 0; j < 8 && (i + j) * 8 < fca.numArgs; ++j) {
      vals |= ((uint64_t)fca.byRefs[i + j]) << (8 * j);
    }

    uint64_t bits = fca.numArgs - i * 8;
    uint64_t mask = bits >= 64
      ? std::numeric_limits<uint64_t>::max()
      : (1UL << bits) - 1;

    // CheckRefs only needs to know the number of parameters when there are more
    // than 64 args.
    if (i == 0) {
      numParams = cns(env, 64);
    } else if (!numParams || numParams->hasConstVal()) {
      numParams = gen(env, LdFuncNumParams, callee);
    }

    auto const crData = CheckRefsData { i * 8, mask, vals };
    ifThen(
      env,
      [&] (Block* taken) {
        gen(env, CheckRefs, taken, crData, callee, numParams);
      },
      [&] {
        hint(env, Block::Hint::Unlikely);
        gen(env, ThrowParamRefMismatchRange, crData, callee);
      }
    );
  }
}

void emitCallerDynamicCallChecksKnown(IRGS& env, const Func* callee) {
  assertx(callee);
  int dynCallErrorLevel = callee->isMethod() ?
    (
      callee->isStatic() ?
        RuntimeOption::EvalForbidDynamicCallsToClsMeth :
        RuntimeOption::EvalForbidDynamicCallsToInstMeth
    ) :
    RuntimeOption::EvalForbidDynamicCallsToFunc;
  if (dynCallErrorLevel <= 0) return;
  if (callee->isDynamicallyCallable()) return;
  gen(env, RaiseForbiddenDynCall, cns(env, callee));
}

void emitCallerDynamicCallChecksUnknown(IRGS& env, SSATmp* callee,
                                        bool mightCareAboutDynCall) {
  assertx(!callee->hasConstVal());
  if (!mightCareAboutDynCall) return;

  ifElse(
    env,
    [&] (Block* skip) {
      auto const dynCallable = gen(
        env,
        FuncHasAttr,
        AttrData {static_cast<int32_t>(AttrDynamicallyCallable)},
        callee);
      gen(env, JmpNZero, skip, dynCallable);
    },
    [&] {
      hint(env, Block::Hint::Unlikely);
      gen(env, RaiseForbiddenDynCall, callee);
    }
  );
}

} // namespace

void emitCallerRxChecksKnown(IRGS& env, const Func* callee) {
  assertx(callee);
  if (RuntimeOption::EvalRxEnforceCalls <= 0) return;
  auto const callerLevel = curRxLevel(env);
  if (!rxEnforceCallsInLevel(callerLevel)) return;

  auto const minReqCalleeLevel = rxRequiredCalleeLevel(callerLevel);
  if (callee->rxLevel() >= minReqCalleeLevel) return;
  gen(env, RaiseRxCallViolation, fp(env), cns(env, callee));
}

namespace {

void emitCallerRxChecksUnknown(IRGS& env, SSATmp* callee) {
  assertx(!callee->hasConstVal());
  if (RuntimeOption::EvalRxEnforceCalls <= 0) return;
  auto const callerLevel = curRxLevel(env);
  if (!rxEnforceCallsInLevel(callerLevel)) return;

  ifThen(
    env,
    [&] (Block* taken) {
      auto const minReqCalleeLevel = rxRequiredCalleeLevel(callerLevel);
      auto const calleeLevel = gen(env, LdFuncRxLevel, callee);
      auto const lt = gen(env, LtInt, calleeLevel, cns(env, minReqCalleeLevel));
      gen(env, JmpNZero, taken, lt);
    },
    [&] {
      hint(env, Block::Hint::Unlikely);
      gen(env, RaiseRxCallViolation, fp(env), callee);
    }
  );
}

//////////////////////////////////////////////////////////////////////

IRSPRelOffset fsetActRec(
  IRGS& env,
  SSATmp* func,
  SSATmp* objOrClass,
  uint32_t numArgs,
  SSATmp* invName,
  bool dynamicCall,
  SSATmp* tsList
) {
  auto const arOffset =
    offsetFromIRSP(env, BCSPRelOffset{static_cast<int32_t>(numArgs)});

  gen(
    env,
    SpillFrame,
    ActRecInfo { arOffset, numArgs },
    sp(env),
    func,
    objOrClass ? objOrClass : cns(env, TNullptr),
    invName ? invName : cns(env, TNullptr),
    cns(env, dynamicCall),
    tsList ? tsList : cns(env, TNullptr)
  );

  return arOffset;
}

//////////////////////////////////////////////////////////////////////

void callUnpack(IRGS& env, const Func* callee, const FCallArgs& fca,
                bool unlikely) {
  auto const data = CallUnpackData {
    spOffBCFromIRSP(env),
    fca.numArgs + 1,
    fca.numRets - 1,
    bcOff(env),
    callee,
  };
  push(env, gen(env, CallUnpack, data, sp(env), fp(env)));
  if (unlikely) gen(env, Jmp, makeExit(env, nextBcOff(env)));
}

SSATmp* callImpl(IRGS& env, const Func* callee, const FCallArgs& fca,
                 bool asyncEagerReturn) {
  auto const data = CallData {
    spOffBCFromIRSP(env),
    fca.numArgs,
    fca.numRets - 1,
    bcOff(env) - curFunc(env)->base(),
    callee,
    asyncEagerReturn,
  };
  return gen(env, Call, data, sp(env), fp(env));
}

void callRegular(IRGS& env, const Func* callee, const FCallArgs& fca,
                 bool unlikely) {
  push(env, callImpl(env, callee, fca, false));
  if (unlikely) gen(env, Jmp, makeExit(env, nextBcOff(env)));
}

void callWithAsyncEagerReturn(IRGS& env, const Func* callee,
                              const FCallArgs& fca, bool unlikely) {
  auto const retVal = callImpl(env, callee, fca, true);

  ifThenElse(
    env,
    [&] (Block* taken) {
      auto const aux = gen(env, LdTVAux, LdTVAuxData {}, retVal);
      auto const tst = gen(env, AndInt, aux, cns(env, 1u << 31));
      gen(env, JmpNZero, taken, tst);
    },
    [&] {
      auto const ty = callee ? awaitedCallReturnType(callee) : TInitCell;
      push(env, gen(env, AssertType, ty, retVal));
      auto const asyncEagerOffset = bcOff(env) + fca.asyncEagerOffset;
      if (unlikely) {
        gen(env, Jmp, makeExit(env, asyncEagerOffset));
      } else {
        jmpImpl(env, asyncEagerOffset);
      }
    },
    [&] {
      hint(env, Block::Hint::Unlikely);
      auto const ty = callee ? callReturnType(callee) : TInitCell;
      push(env, gen(env, AssertType, ty, retVal));
      if (unlikely) gen(env, Jmp, makeExit(env, nextBcOff(env)));
    }
  );
}

void callKnown(IRGS& env, const Func* callee, const FCallArgs& fca) {
  assertx(callee);
  if (fca.hasUnpack()) {
    return callUnpack(env, callee, fca, false /* unlikely */);
  }

  if (fca.asyncEagerOffset != kInvalidOffset &&
      callee->supportsAsyncEagerReturn()) {
    return callWithAsyncEagerReturn(env, callee, fca, false /* unlikely */);
  }

  return callRegular(env, callee, fca, false /* unlikely */);
}

void callUnknown(IRGS& env, SSATmp* callee, const FCallArgs& fca, bool unlikely,
                 bool noAsyncEagerReturn) {
  assertx(!callee->hasConstVal() || env.formingRegion);
  if (fca.hasUnpack()) return callUnpack(env, nullptr, fca, unlikely);

  if (noAsyncEagerReturn || fca.asyncEagerOffset == kInvalidOffset) {
    return callRegular(env, nullptr, fca, unlikely);
  }

  if (fca.supportsAsyncEagerReturn()) {
    return callWithAsyncEagerReturn(env, nullptr, fca, unlikely);
  }

  ifThenElse(
    env,
    [&] (Block* taken) {
      auto const supportsAER = gen(
        env,
        FuncHasAttr,
        AttrData {static_cast<int32_t>(AttrSupportsAsyncEagerReturn)},
        callee);
      gen(env, JmpNZero, taken, supportsAER);
    },
    [&] {
      hint(env, Block::Hint::Unlikely);
      callRegular(env, nullptr, fca, unlikely);
    },
    [&] {
      callWithAsyncEagerReturn(env, nullptr, fca, unlikely);
    }
  );
}

//////////////////////////////////////////////////////////////////////

/*
 * In PGO mode, we use profiling to try to determine the most likely target
 * function at each call site.  profiledCalledFunc() returns the most likely
 * called function based on profiling, as long as it was seen at least
 * Eval.JitPGOCalledFuncCheckThreshold percent of the times during profiling.
 * When a callee satisfies this condition, profiledCalledFunc() returns such
 * callee and it also returns the probability of seeing that callee.
 */
const Func* profiledCalledFunc(IRGS& env, double& probability) {
  probability = 0;
  if (!RuntimeOption::RepoAuthoritative) return nullptr;

  auto profile = TargetProfile<CallTargetProfile>(
    env.unit.context(), env.irb->curMarker(), callTargetProfileKey());

  // NB: the profiling used here is shared and done in getCallTarget() in
  // irlower-call.cpp, so we only handle the optimization phase here.
  if (!profile.optimizing()) return nullptr;

  auto const data = profile.data();
  auto profiledFunc = data.choose(probability);

  if (profiledFunc == nullptr) return nullptr;

  // Don't emit the check if the probability of it succeeding is below the
  // threshold.
  if (probability * 100 < RuntimeOption::EvalJitPGOCalledFuncCheckThreshold) {
    return nullptr;
  }

  return profiledFunc;
}

template<class TKnown, class TUnknown>
void callProfiledFunc(IRGS& env, SSATmp* callee,
                      TKnown callKnown, TUnknown callUnknown) {
  double profiledFuncBias{0};
  auto const profiledFunc = profiledCalledFunc(env, profiledFuncBias);
  if (!profiledFunc) return callUnknown(false);

  ifThenElse(
    env,
    [&] (Block* taken) {
      auto const equal = gen(env, EqFunc, callee, cns(env, profiledFunc));
      gen(env, JmpZero, taken, equal);
    },
    [&] {
      callKnown(profiledFunc);
    },
    [&] {
      // Current marker's SP points to the wrong place after callKnown() above.
      updateMarker(env);
      env.irb->exceptionStackBoundary();

      auto const unlikely = profiledFuncBias * 100 >=
        RuntimeOption::EvalJitPGOCalledFuncExitThreshold;
      if (unlikely) {
        hint(env, Block::Hint::Unlikely);
        IRUnit::Hinter h(env.irb->unit(), Block::Hint::Unlikely);
        callUnknown(true);
      } else {
        callUnknown(false);
      }
    }
  );
}

//////////////////////////////////////////////////////////////////////

void prepareToCallKnown(IRGS& env, const Func* callee, SSATmp* objOrClass,
                        uint32_t numArgs, const StringData* invName,
                        bool dynamicCall, SSATmp* tsList) {
  assertx(callee);

  // Caller checks
  if (dynamicCall) emitCallerDynamicCallChecksKnown(env, callee);
  emitCallerRxChecksKnown(env, callee);

  auto const func = cns(env, callee);
  fsetActRec(env, func, objOrClass, numArgs,
             invName ? cns(env, invName) : nullptr,
             dynamicCall, tsList);
}

void prepareToCallUnknown(IRGS& env, SSATmp* callee, SSATmp* objOrClass,
                          uint32_t numArgs, SSATmp* invName,
                          bool dynamicCall, bool mightCareAboutDynCall,
                          SSATmp* tsList) {
  assertx(callee->isA(TFunc));
  if (callee->hasConstVal() && (invName == nullptr || invName->hasConstVal())) {
    return prepareToCallKnown(env, callee->funcVal(), objOrClass, numArgs,
                              invName ? invName->strVal() : nullptr,
                              dynamicCall, tsList);
  }

  // Caller checks
  if (dynamicCall) {
    emitCallerDynamicCallChecksUnknown(env, callee, mightCareAboutDynCall);
  }
  emitCallerRxChecksUnknown(env, callee);

  fsetActRec(env, callee, objOrClass, numArgs, invName, dynamicCall, tsList);
}

template<class Fn>
SSATmp* prepareToCallCustom(IRGS& env, SSATmp* objOrClass, uint32_t numArgs,
                            bool dynamicCall, SSATmp* tsList, Fn prepare) {
  auto const arOffset = fsetActRec(
    env, cns(env, TNullptr), objOrClass, numArgs, nullptr, dynamicCall, tsList);

  // This is special. We need to sync SP in case prepare() reenters. Otherwise
  // it would clobber the ActRec we just pushed.
  updateMarker(env);
  env.irb->exceptionStackBoundary();

  // Responsible for:
  // - performing caller checks
  // - populating missing ActRec fields (m_func at minimum)
  // - returning SSATmp* of the resolved func pointer
  return prepare(arOffset);
}

//////////////////////////////////////////////////////////////////////

void prepareAndCallKnown(IRGS& env, const Func* callee, const FCallArgs& fca,
                         SSATmp* objOrClass, const StringData* invName,
                         bool dynamicCall, SSATmp* tsList) {
  assertx(callee);
  if (!emitCallerReffinessChecksKnown(env, callee, fca)) return;
  prepareToCallKnown(env, callee, objOrClass, fca.numArgsInclUnpack(), invName,
                     dynamicCall, tsList);
  if (invName == nullptr) {
    auto const inlined = irGenTryInlineFCall(
      env, callee, fca, objOrClass, objOrClass->type(), curSrcKey(env).op());
    if (inlined) return;
  }

  // We just wrote to the stack, make sure Call opcode can set up its Catch.
  updateMarker(env);
  env.irb->exceptionStackBoundary();

  if (!env.formingRegion) {
    callKnown(env, callee, fca);
  } else {
    // Do not use the inferred Func* if we are forming a region. We may have
    // inferred the target of the call based on specialized type information
    // that won't be available when the region is translated. If we allow the
    // FCall to specialize using this information, we may infer narrower type
    // for the return value, erroneously preventing the region from breaking
    // on unknown type.
    callUnknown(env, cns(env, callee), fca, false, false);
  }
}

void prepareAndCallUnknown(IRGS& env, SSATmp* callee, const FCallArgs& fca,
                           SSATmp* objOrClass, SSATmp* invName,
                           bool dynamicCall, bool mightCareAboutDynCall,
                           SSATmp* tsList, bool unlikely) {
  assertx(callee->isA(TFunc));
  if (callee->hasConstVal() && (invName == nullptr || invName->hasConstVal())) {
    return prepareAndCallKnown(env, callee->funcVal(), fca, objOrClass,
                               invName ? invName->strVal() : nullptr,
                               dynamicCall, tsList);
  }

  emitCallerReffinessChecksUnknown(env, callee, fca);
  prepareToCallUnknown(env, callee, objOrClass, fca.numArgsInclUnpack(),
                       invName, dynamicCall, mightCareAboutDynCall, tsList);

  // We just wrote to the stack, make sure Call opcode can set up its Catch.
  updateMarker(env);
  env.irb->exceptionStackBoundary();

  callUnknown(env, callee, fca, unlikely, invName != nullptr /* no AER */);
}

void prepareAndCallProfiled(IRGS& env, SSATmp* callee, const FCallArgs& fca,
                            SSATmp* objOrClass, bool dynamicCall,
                            bool mightCareAboutDynCall, SSATmp* tsList) {
  assertx(callee->isA(TFunc));
  auto const handleKnown = [&] (const Func* knownCallee) {
    prepareAndCallKnown(env, knownCallee, fca, objOrClass, nullptr, dynamicCall,
                        tsList);
  };
  if (callee->hasConstVal()) return handleKnown(callee->funcVal());

  auto const handleUnknown = [&] (bool unlikely) {
    prepareAndCallUnknown(env, callee, fca, objOrClass, nullptr, dynamicCall,
                          mightCareAboutDynCall, tsList, unlikely);
  };
  callProfiledFunc(env, callee, handleKnown, handleUnknown);
}

//////////////////////////////////////////////////////////////////////

// Calling object method when we don't know the Func* statically.
template<class TProfile>
void fcallObjMethodUnknown(
  IRGS& env,
  const FCallArgs& fca,
  SSATmp* obj,
  SSATmp* methodName,
  bool dynamic,
  SSATmp* ts,
  TProfile profileMethod,
  bool noCallProfiling
) {
  implIncStat(env, Stats::ObjMethod_cached);

  updateMarker(env);
  env.irb->exceptionStackBoundary();

  auto const magicCallBlock = defBlock(env, Block::Hint::Unlikely);
  auto const doneBlock = defBlock(env, Block::Hint::Likely);

  SSATmp* funcMM;
  SSATmp* funcWoM;

  auto const cls = gen(env, LdObjClass, obj);

  if (methodName->hasConstVal()) {
    auto const regularCallBlock = defBlock(env, Block::Hint::Likely);
    auto const slowCheckBlock = defBlock(env, Block::Hint::Unlikely);

    // check for TC cache hit; go to slow check on miss
    auto const tcCache = gen(env, LdSmashable);
    gen(env, CheckSmashableClass, slowCheckBlock, tcCache, cls);

    // fast path: load func from TC cache and proceed to regular call
    auto const funcS = gen(env, LdSmashableFunc, tcCache);
    gen(env, Jmp, regularCallBlock, funcS);

    // slow path: run C++ helper to determine Func*, then check for magic call
    env.irb->appendBlock(slowCheckBlock);
    auto const fnData = FuncNameData { methodName->strVal() };
    funcMM = gen(env, LdObjMethodS, fnData, cls, tcCache);
    auto const funcNM = gen(env, CheckFuncMMNonMagic, magicCallBlock, funcMM);
    gen(env, Jmp, regularCallBlock, funcNM);

    // set up dst for a regular (non-magic) call
    env.irb->appendBlock(regularCallBlock);
    auto const label = env.unit.defLabel(1, env.irb->nextBCContext());
    regularCallBlock->push_back(label);
    funcWoM = label->dst(0);
    funcWoM->setType(TFunc);
  } else {
    funcMM = gen(env, LdObjMethodD, cls, methodName);
    funcWoM = gen(env, CheckFuncMMNonMagic, magicCallBlock, funcMM);
  }

  // prepare to do a regular (non-magic) call
  auto const mightCareAboutDynCall =
    RuntimeOption::EvalForbidDynamicCallsToInstMeth > 0
    || RuntimeOption::EvalForbidDynamicCallsToClsMeth > 0;
  profileMethod(funcWoM);
  if (noCallProfiling) {
    prepareAndCallUnknown(env, funcWoM, fca, obj, nullptr, dynamic,
                          mightCareAboutDynCall, ts, true);
  } else {
    prepareAndCallProfiled(env, funcWoM, fca, obj, dynamic,
                           mightCareAboutDynCall, ts);
  }
  gen(env, Jmp, doneBlock);

  // prepare to do a magic call (no profiling, as inlining is not supported)
  env.irb->appendBlock(magicCallBlock);
  auto const funcM = gen(env, AssertType, TFuncM, funcMM);
  auto const funcWM = gen(env, LdFuncMFunc, funcM);
  profileMethod(funcWM);
  prepareAndCallUnknown(env, funcWM, fca, obj, methodName, dynamic,
                        mightCareAboutDynCall, ts, true);
  assertx(env.irb->curBlock()->back().isTerminal());

  // done
  env.irb->appendBlock(doneBlock);
}

void lookupObjMethodExactFunc(IRGS& env, SSATmp* obj, const Func* func) {
  /*
   * Static function: throw an exception; obj will be decref'd via stack.
   *
   * Static closure body: we still need to pass the object instance for the
   * closure prologue to properly do its dispatch (and extract use vars). It
   * will decref it and set up the alternative class pointer before entering the
   * "real" cloned closure body.
   */
  implIncStat(env, Stats::ObjMethod_known);
  if (func->isStaticInPrologue()) {
    gen(env, ThrowHasThisNeedStatic, cns(env, func));
  }
}

SSATmp* lookupObjMethodInterfaceFunc(IRGS& env, SSATmp* obj,
                                     const Func* ifaceFunc) {
  implIncStat(env, Stats::ObjMethod_ifaceslot);
  auto const cls = gen(env, LdObjClass, obj);
  auto const vtableSlot = ifaceFunc->cls()->preClass()->ifaceVtableSlot();
  auto const imData = IfaceMethodData { vtableSlot, ifaceFunc->methodSlot() };
  auto const func = gen(env, LdIfaceMethod, imData, cls);
  if (ifaceFunc->attrs() & AttrStatic) {
    gen(env, ThrowHasThisNeedStatic, func);
  }
  return func;
}

SSATmp* lookupObjMethodNonExactFunc(IRGS& env, SSATmp* obj,
                                    const Func* superFunc) {
  implIncStat(env, Stats::ObjMethod_methodslot);
  auto const cls = gen(env, LdObjClass, obj);
  auto const methSlot = -(superFunc->methodSlot() + 1);
  auto const func = gen(env, LdClsMethod, cls, cns(env, methSlot));
  if (superFunc->isStaticInPrologue()) {
    gen(env, ThrowHasThisNeedStatic, func);
  }
  return func;
}

const StaticString methProfileKey{ "MethProfile-FCallObjMethod" };

inline SSATmp* ldCtxForClsMethod(IRGS& env,
                                 const Func* callee,
                                 SSATmp* callCtx,
                                 const Class* cls,
                                 bool exact) {

  assertx(callCtx->isA(TCls));

  auto gen_missing_this = [&] {
    if (!callee->isStaticInPrologue()) {
      gen(env, ThrowMissingThis, cns(env, callee));
    }
    return callCtx;
  };

  if (callee->isStaticInPrologue()) return callCtx;
  if (!hasThis(env)) {
    return gen_missing_this();
  }

  auto const maybeUseThis = curClass(env)->classof(cls);
  if (!maybeUseThis && !cls->classof(curClass(env))) {
    return gen_missing_this();
  }

  auto skipAT = [] (SSATmp* val) {
    while (val->inst()->is(AssertType, CheckType, CheckCtxThis)) {
      val = val->inst()->src(0);
    }
    return val;
  };

  auto const canUseThis = [&] () -> bool {
    // A static::foo() call can always pass through a $this
    // from the caller (if it has one). Match the common patterns
    auto cc = skipAT(callCtx);
    if (cc->inst()->is(LdObjClass, LdClsCtx, LdClsCctx)) {
      cc = skipAT(cc->inst()->src(0));
      if (cc->inst()->is(LdCtx, LdCctx)) return true;
    }
    return maybeUseThis && (exact || cls->attrs() & AttrNoOverride);
  }();

  auto const ctx = gen(env, LdCtx, fp(env));
  auto thiz = castCtxThis(env, ctx);

  if (canUseThis) {
    gen(env, IncRef, thiz);
    return thiz;
  }

  return cond(
    env,
    [&] (Block* taken) {
      auto thizCls = gen(env, LdObjClass, thiz);
      auto flag = exact ?
        gen(env, ExtendsClass, ExtendsClassData{ cls, true }, thizCls) :
        gen(env, InstanceOf, thizCls, callCtx);
      gen(env, JmpZero, taken, flag);
    },
    [&] {
      gen(env, IncRef, thiz);
      return thiz;
    },
    [&] {
      hint(env, Block::Hint::Unlikely);
      gen_missing_this();
      return gen(env, ConvClsToCctx, callCtx);
    });
}

template<class Fn>
void optimizeProfiledCallMethod(IRGS& env,
                                const FCallArgs& fca,
                                SSATmp* objOrCls,
                                bool knownIfaceFunc,
                                const StringData* methodName,
                                bool dynamic,
                                uint32_t numExtraInputs,
                                Fn emitFCall) {
  always_assert(objOrCls->type().subtypeOfAny(TObj, TCls));
  auto const isStaticCall = objOrCls->type() <= TCls;
  auto profile = TargetProfile<MethProfile>(env.context, env.irb->curMarker(),
                                            methProfileKey.get());
  if (!profile.optimizing()) {
    emitFCall(&profile);
    return;
  }

  auto getCtx = [&](const Func* callee,
                    SSATmp* ctx,
                    const Class* cls) -> SSATmp* {
    if (isStaticCall) {
      return ldCtxForClsMethod(env, callee, ctx,
                               cls ? cls : callee->cls(), cls != nullptr);
    }
    if (!callee->isStaticInPrologue()) return ctx;
    assertx(ctx->type() <= TObj);
    auto ret = cls ? cns(env, cls) : gen(env, LdObjClass, ctx);
    decRef(env, ctx);
    return ret;
  };

  auto const fallback = [&] {
    hint(env, Block::Hint::Unlikely);
    IRUnit::Hinter h(env.irb->unit(), Block::Hint::Unlikely);

    // Current marker's SP points to the wrong place after the hot path.
    updateMarker(env);
    env.irb->exceptionStackBoundary();

    emitFCall(nullptr, true /* no call profiling */);
    gen(env, Jmp, makeExit(env, nextBcOff(env)));
  };

  MethProfile data = profile.data();

  if (auto const uniqueMeth = data.uniqueMeth()) {
    bool isMagic = !uniqueMeth->name()->isame(methodName);
    if (auto const uniqueClass = data.uniqueClass()) {
      // Profiling saw a unique class.
      // Check for it, then burn in the func
      ifThen(
        env,
        [&] (Block* sideExit) {
          auto const ty = isStaticCall
            ? Type::ExactCls(uniqueClass) : Type::ExactObj(uniqueClass);
          auto const refined = gen(env, CheckType, ty, sideExit, objOrCls);
          env.irb->constrainValue(refined, GuardConstraint(uniqueClass));
          auto const ctx = getCtx(uniqueMeth, refined, uniqueClass);
          discard(env, numExtraInputs);
          prepareAndCallKnown(env, uniqueMeth, fca, ctx,
                              isMagic ? methodName : nullptr,
                              dynamic, nullptr);
        },
        fallback
      );
      return;
    }

    if (isMagic) {
      emitFCall();
      return;
    }

    // Although there were multiple classes, the method was unique
    // (this comes up eg for a final method in a base class).  But
    // note that we can't allow a magic call here since it's possible
    // that an as-yet-unseen derived class defines a method named
    // methodName.
    ifThen(
      env,
      [&] (Block* sideExit) {
        auto const slot = cns(env, uniqueMeth->methodSlot());
        auto const negSlot = cns(env, -(uniqueMeth->methodSlot() + 1));
        auto const cls = isStaticCall
          ? objOrCls : gen(env, LdObjClass, objOrCls);
        auto const len = gen(env, LdFuncVecLen, cls);
        auto const cmp = gen(env, LteInt, len, slot);
        gen(env, JmpNZero, sideExit, cmp);
        auto const meth = gen(env, LdClsMethod, cls, negSlot);
        auto const same = gen(env, EqFunc, meth, cns(env, uniqueMeth));
        gen(env, JmpZero, sideExit, same);
        auto const ctx = getCtx(uniqueMeth, objOrCls, nullptr);
        discard(env, numExtraInputs);
        prepareAndCallKnown(env, uniqueMeth, fca, ctx, nullptr, dynamic,
                            nullptr);
      },
      fallback
    );
    return;
  }

  if (auto const baseMeth = data.baseMeth()) {
    if (!baseMeth->name()->isame(methodName)) {
      emitFCall();
      return;
    }

    // The method was defined in a common base class.  We just need to check for
    // an instance of the class, and then use the method from the right slot.
    ifThen(
      env,
      [&] (Block* sideExit) {
        auto const cls = isStaticCall
          ? objOrCls : gen(env, LdObjClass, objOrCls);
        auto const ecData = ExtendsClassData{baseMeth->cls(), true};
        auto const flag = gen(env, ExtendsClass, ecData, cls);
        gen(env, JmpZero, sideExit, flag);
        auto const negSlot = cns(env, -(baseMeth->methodSlot() + 1));
        auto const meth = gen(env, LdClsMethod, cls, negSlot);
        auto const ctx = getCtx(baseMeth, objOrCls, nullptr);
        auto const mightCareAboutDynCall =
          RuntimeOption::EvalForbidDynamicCallsToClsMeth > 0
          || RuntimeOption::EvalForbidDynamicCallsToInstMeth > 0;
        discard(env, numExtraInputs);
        prepareAndCallProfiled(env, meth, fca, ctx, dynamic,
                               mightCareAboutDynCall, nullptr);
      },
      fallback
    );
    return;
  }

  // If we know the object implements a known interface that defines the called
  // method, the other cases below are not worth the extra checks they insert.
  if (knownIfaceFunc) {
    emitFCall();
    return;
  }

  if (auto const intfMeth = data.interfaceMeth()) {
    if (!intfMeth->name()->isame(methodName)) {
      emitFCall();
      return;
    }

    auto const vtableSlot = intfMeth->cls()->preClass()->ifaceVtableSlot();
    if (vtableSlot == kInvalidSlot) {
      emitFCall();
      return;
    }

    // The method was defined in a common interface, so check for that and use
    // LdIfaceMethod.
    ifThen(
      env,
      [&] (Block* sideExit) {
        auto const cls = isStaticCall
          ? objOrCls : gen(env, LdObjClass, objOrCls);
        auto const cData = InstanceOfIfaceVtableData{
          intfMeth->cls(), !isStaticCall
        };
        auto const flag = gen(env, InstanceOfIfaceVtable, cData, cls);
        gen(env, JmpZero, sideExit, flag);
        auto const imData = IfaceMethodData{vtableSlot, intfMeth->methodSlot()};
        auto const meth = gen(env, LdIfaceMethod, imData, cls);
        auto const ctx = getCtx(intfMeth, objOrCls, nullptr);
        auto const mightCareAboutDynCall =
          RuntimeOption::EvalForbidDynamicCallsToClsMeth > 0
          || RuntimeOption::EvalForbidDynamicCallsToInstMeth > 0;
        discard(env, numExtraInputs);
        prepareAndCallProfiled(env, meth, fca, ctx, dynamic,
                               mightCareAboutDynCall, nullptr);
      },
      fallback
    );
    return;
  }

  emitFCall();
}

void fcallObjMethodObj(IRGS& env, const FCallArgs& fca, SSATmp* obj,
                       const StringData* clsHint, SSATmp* methodName,
                       bool dynamic, SSATmp* tsList) {
  assertx(obj->isA(TObj));
  assertx(methodName->isA(TStr));

  implIncStat(env, Stats::ObjMethod_total);

  auto const lookup = [&] {
    auto constexpr notFound = ImmutableObjMethodLookup {
      ImmutableObjMethodLookup::Type::NotFound,
      nullptr
    };

    if (!methodName->hasConstVal()) return notFound;

    if (!clsHint->empty()) {
      auto const cls = Unit::lookupUniqueClassInContext(clsHint, curClass(env));
      if (cls) {
        assertx(!isInterface(cls));
        obj = gen(env, AssertType, Type::SubObj(cls), obj);
        return lookupImmutableObjMethod(cls, methodName->strVal(), curFunc(env),
                                        true);
      }
    }

    if (auto cls = obj->type().clsSpec().cls()) {
      if (!env.irb->constrainValue(obj, GuardConstraint(cls).setWeak())) {
        // We know the class without having to specialize a guard any further.
        // We may still want to use MethProfile to gather more information in
        // case the class isn't known exactly.
        auto const exactClass =
          obj->type().clsSpec().exact() || cls->attrs() & AttrNoOverride;
        return lookupImmutableObjMethod(cls, methodName->strVal(), curFunc(env),
                                        exactClass);
      }
    }

    return notFound;
  }();

  const Func* knownIfaceFunc = nullptr;

  // If we know which exact or overridden method to call, we don't need PGO.
  switch (lookup.type) {
    case ImmutableObjMethodLookup::Type::MagicFunc:
      lookupObjMethodExactFunc(env, obj, lookup.func);
      prepareAndCallKnown(env, lookup.func, fca, obj, methodName->strVal(),
                          dynamic, tsList);
      return;
    case ImmutableObjMethodLookup::Type::Func:
      lookupObjMethodExactFunc(env, obj, lookup.func);
      prepareAndCallKnown(env, lookup.func, fca, obj, nullptr, dynamic, tsList);
      return;
    case ImmutableObjMethodLookup::Type::Class: {
      auto const func = lookupObjMethodNonExactFunc(env, obj, lookup.func);
      auto const mightCareAboutDynCall =
        RuntimeOption::EvalForbidDynamicCallsToInstMeth > 0
        || RuntimeOption::EvalForbidDynamicCallsToClsMeth > 0;
      prepareAndCallProfiled(env, func, fca, obj, dynamic,
                             mightCareAboutDynCall, tsList);
      return;
    }
    case ImmutableObjMethodLookup::Type::NotFound:
      break;
    case ImmutableObjMethodLookup::Type::Interface:
      knownIfaceFunc = lookup.func;
      break;
  }

  auto const emitFCall = [&] (TargetProfile<MethProfile>* profile = nullptr,
                              bool noCallProfiling = false) {
    auto const profileMethod = [&] (SSATmp* callee) {
      if (!profile || !profile->profiling()) return;
      auto const cls = gen(env, LdObjClass, obj);
      auto const pctData = ProfileCallTargetData { profile->handle() };
      gen(env, ProfileMethod, pctData, cls, callee);
    };


    if (knownIfaceFunc == nullptr) {
      fcallObjMethodUnknown(env, fca, obj, methodName, dynamic, tsList,
                            profileMethod, noCallProfiling);
    } else {
      auto const func = lookupObjMethodInterfaceFunc(env, obj, knownIfaceFunc);
      auto const mightCareAboutDynCall =
        RuntimeOption::EvalForbidDynamicCallsToInstMeth > 0
        || RuntimeOption::EvalForbidDynamicCallsToClsMeth > 0;
      profileMethod(func);
      if (noCallProfiling) {
        prepareAndCallUnknown(env, func, fca, obj, nullptr, dynamic,
                              mightCareAboutDynCall, tsList, true);
      } else {
        prepareAndCallProfiled(env, func, fca, obj, dynamic,
                               mightCareAboutDynCall, tsList);
      }
    }
  };

  // If the method has reified generics, we can't burn the value in the JIT
  if (!RuntimeOption::RepoAuthoritative || !methodName->hasConstVal() ||
      tsList) {
    return emitFCall();
  }

  // If we don't know anything about the object's class, or all we know is an
  // interface that it implements, then enable PGO.
  optimizeProfiledCallMethod(env, fca, obj, knownIfaceFunc != nullptr,
                             methodName->strVal(), dynamic, 0, emitFCall);
}

void fpushFuncObj(IRGS& env, uint32_t numParams) {
  auto const slowExit = makeExitSlow(env);
  auto const obj      = popC(env);
  auto const cls      = gen(env, LdObjClass, obj);
  auto const func     = gen(env, LdObjInvoke, slowExit, cls);
  prepareToCallUnknown(env, func, obj, numParams, nullptr, false, false,
                       nullptr);
}

void fpushFuncArr(IRGS& env, uint32_t numParams) {
  auto const arr = popC(env);
  auto const prepare = [&] (IRSPRelOffset arOffset) {
    auto const func = gen(env, LdArrFuncCtx, IRSPRelOffsetData { arOffset },
                          arr, sp(env), fp(env));
    decRef(env, arr);
    return func;
  };
  prepareToCallCustom(env, nullptr, numParams, true, nullptr, prepare);
}

void fpushFuncClsMeth(IRGS& env, uint32_t numParams) {
  auto const clsMeth = popC(env);
  auto const cls = gen(env, LdClsFromClsMeth, clsMeth);
  auto const func = gen(env, LdFuncFromClsMeth, clsMeth);
  prepareToCallUnknown(env, func, cls, numParams, nullptr, false, false,
                       nullptr);
}

void fPushFuncDImpl(IRGS& env, uint32_t numParams, const StringData* name,
                    SSATmp* tsList) {
  auto const lookup = lookupImmutableFunc(curUnit(env), name);
  if (lookup.func) {
    // We know the function, but we have to ensure its unit is loaded. Use
    // LdFuncCached, ignoring the result to ensure this.
    if (lookup.needsUnitLoad) gen(env, LdFuncCached, FuncNameData { name });
    prepareToCallKnown(env, lookup.func, nullptr, numParams, nullptr, false,
                       tsList);
    return;
  }

  auto const func = gen(env, LdFuncCached, FuncNameData { name });
  prepareToCallUnknown(env, func, nullptr, numParams, nullptr, false, false,
                       tsList);
}

} // namespace

void emitFPushFuncD(IRGS& env, uint32_t numParams, const StringData* name) {
  fPushFuncDImpl(env, numParams, name, nullptr);
}

void emitFPushFuncRD(IRGS& env, uint32_t numParams, const StringData* name) {
  fPushFuncDImpl(env, numParams, name, popC(env));
}

namespace {

//////////////////////////////////////////////////////////////////////

SSATmp* specialClsRefToCls(IRGS& env, SpecialClsRef ref) {
  switch (ref) {
    case SpecialClsRef::Static:
      if (!curClass(env)) return nullptr;
      return gen(env, LdClsCtx, ldCtx(env));
    case SpecialClsRef::Self:
      if (auto const clss = curClass(env)) return cns(env, clss);
      return nullptr;
    case SpecialClsRef::Parent:
      if (auto const clss = curClass(env)) {
        if (auto const parent = clss->parent()) return cns(env, parent);
      }
      return nullptr;
  }
  always_assert(false);
}

SSATmp* getReifiedGenerics(IRGS& env, SSATmp* funName) {
  if (funName->hasConstVal(TStr)) {
    auto const name = funName->strVal();
    if (!isReifiedName(name)) return nullptr;
    return cns(env, getReifiedTypeList(stripClsOrFnNameFromReifiedName(name)));
  }
  return cond(
    env,
    [&] (Block* not_reified_block) {
      // Lets do a quick check before calling IsReifiedName since that's an
      // expensive native call
      // Reified names always start with a $
      // It is also safe to read the 0th char without checking the length since
      // if it is an empty string, then we'll be reading the null terminator
      auto const first_char = gen(env, OrdStrIdx, funName, cns(env, 0));
      auto const issame = gen(env, EqInt, cns(env, (uint64_t)'$'), first_char);
      gen(env, JmpZero, not_reified_block, issame);
      auto const isreified = gen(env, IsReifiedName, funName);
      gen(env, JmpZero, not_reified_block, isreified);
    },
    [&] {
      hint(env, Block::Hint::Unlikely);
      return gen(env, LdReifiedGeneric, funName);
    },
    [&] {
      return cns(env, TNullptr);
    }
  );
}


folly::Optional<int> specialClsReifiedPropSlot(IRGS& env, SpecialClsRef ref) {
  auto const cls = curClass(env);
  if (!cls) return folly::none;
  auto result = [&] (const Class* cls) -> folly::Optional<int> {
    if (!cls->hasReifiedGenerics()) return folly::none;
    auto const slot = cls->lookupReifiedInitProp();
    assertx(slot != kInvalidSlot);
    return slot;
  };
  switch (ref) {
    case SpecialClsRef::Static:
      // Currently we disallow new static on reified classes
      return folly::none;
    case SpecialClsRef::Self:
      return result(cls);
    case SpecialClsRef::Parent:
      if (!cls->parent()) return folly::none;
      return result(cls->parent());
  }
  always_assert(false);
}

void emitDynamicConstructChecks(IRGS& env, SSATmp* cls) {
  if (RuntimeOption::EvalForbidDynamicConstructs <= 0) return;
  if (cls->hasConstVal()) {
    if (cls->clsVal()->isDynamicallyConstructible()) return;
    gen(env, RaiseForbiddenDynConstruct, cls);
    return;
  }

  ifElse(
    env,
    [&] (Block* skip) {
      auto const dynConstructible = gen(env, IsClsDynConstructible, cls);
      gen(env, JmpNZero, skip, dynConstructible);
    },
    [&] {
      hint(env, Block::Hint::Unlikely);
      gen(env, RaiseForbiddenDynConstruct, cls);
    }
  );
}

} // namespace

void emitNewObj(IRGS& env) {
  auto const cls = popC(env);
  if (!cls->isA(TCls)) PUNT(NewObj-NotClass);
  emitDynamicConstructChecks(env, cls);
  push(env, gen(env, AllocObj, cls));
}

void emitNewObjR(IRGS& env) {
  auto const generics = popC(env);
  auto const cls      = popC(env);
  if (!cls->isA(TCls))     PUNT(NewObjR-NotClass);

  emitDynamicConstructChecks(env, cls);
  auto const obj = [&] {
    if (generics->isA(RuntimeOption::EvalHackArrDVArrs ? TVec : TArr)) {
      return gen(env, AllocObjReified, cls, generics);
    } else if (generics->isA(TInitNull)) {
      return gen(env, AllocObj, cls);
    } else {
      PUNT(NewObjR-BadReified);
    }
  }();
  push(env, obj);
}

namespace {

void emitNewObjDImpl(IRGS& env, const StringData* className,
                     SSATmp* tsList) {
  auto const cls = Unit::lookupUniqueClassInContext(className, curClass(env));
  bool const persistentCls = classIsPersistentOrCtxParent(env, cls);
  bool const canInstantiate = cls && isNormalClass(cls) && !isAbstract(cls);
  if (persistentCls && canInstantiate && !cls->hasNativePropHandler() &&
      !cls->hasReifiedGenerics() && !cls->hasReifiedParent()) {
    push(env, allocObjFast(env, cls));
    return;
  }

  auto const finishWithKnownCls = [&] {
    if (cls->hasReifiedGenerics()) {
      if (!tsList) PUNT(NewObjD-ReifiedCls);
      push(env, gen(env, AllocObjReified, cns(env, cls), tsList));
      return;
    }
    push(env, gen(env, AllocObj, cns(env, cls)));
  };

  if (persistentCls) return finishWithKnownCls();
  auto const cachedCls = gen(env, LdClsCached, cns(env, className));
  if (cls) return finishWithKnownCls();
  if (tsList) {
    push(env, gen(env, AllocObjReified, cachedCls, tsList));
    return;
  }
  push(env, gen(env, AllocObj, cachedCls));
}

} // namespace

void emitNewObjD(IRGS& env, const StringData* className) {
  emitNewObjDImpl(env, className, nullptr);
}

void emitNewObjRD(IRGS& env, const StringData* className) {
  auto const cell = popC(env);
  auto const tsList = [&] () -> SSATmp* {
    if (cell->isA(RuntimeOption::EvalHackArrDVArrs ? TVec : TArr)) {
      return cell;
    } else if (cell->isA(TInitNull)) {
      return nullptr;
    } else {
      PUNT(NewObjRD-BadReified);
    }
  }();
  emitNewObjDImpl(env, className, tsList);
  decRef(env, cell);
}

void emitNewObjS(IRGS& env, SpecialClsRef ref) {
  auto const cls = specialClsRefToCls(env, ref);
  if (!cls) return interpOne(env);
  auto const slot = specialClsReifiedPropSlot(env, ref);
  if (slot == folly::none) {
    push(env, gen(env, AllocObj, cls));
    return;
  }

  auto const this_ = checkAndLoadThis(env);
  auto const ty = RuntimeOption::EvalHackArrDVArrs ? TVec : TArr;
  auto const addr = gen(
    env,
    LdPropAddr,
    ByteOffsetData { (ptrdiff_t)curClass(env)->declPropOffset(*slot) },
    ty.lval(Ptr::Prop),
    this_
  );
  auto const reified_generic = gen(env, LdMem, ty, addr);
  push(env, gen(env, AllocObjReified, cls, reified_generic));
}

void emitFCallCtor(IRGS& env, FCallArgs fca, const StringData* clsHint) {
  assertx(fca.numRets == 1);
  assertx(fca.asyncEagerOffset == kInvalidOffset);
  auto const numArgs = fca.numArgsInclUnpack();
  auto const objPos = static_cast<int32_t>(numArgs + 2);
  auto const obj = topC(env, BCSPRelOffset{objPos});
  if (!obj->isA(TObj)) PUNT(FCallCtor-NonObj);

  auto const exactCls = [&] {
    if (!clsHint->empty()) {
      auto const cls = Unit::lookupUniqueClassInContext(clsHint, curClass(env));
      if (cls) return cls;
    }
    return obj->type().clsSpec().exactCls();
  }();
  if (exactCls) {
    if (auto const ctor = lookupImmutableCtor(exactCls, curClass(env))) {
      return prepareAndCallKnown(env, ctor, fca, obj, nullptr, false, nullptr);
    }
  }

  auto const cls = exactCls ? cns(env, exactCls) : gen(env, LdObjClass, obj);
  auto const callee = gen(env, LdClsCtor, cls, fp(env));
  prepareAndCallProfiled(env, callee, fca, obj, false, false, nullptr);
}

void emitLockObj(IRGS& env) {
  auto obj = topC(env);
  if (!obj->isA(TObj)) PUNT(LockObj-NonObj);
  gen(env, LockObj, obj);
}

void emitFPushFunc(IRGS& env, uint32_t numParams, const ImmVector& v) {
  if (v.size() != 0) PUNT(InOut-FPushFunc);
  auto const callee = topC(env);

  if (callee->isA(TObj)) return fpushFuncObj(env, numParams);
  if (callee->isA(TArr) || callee->isA(TVec)) {
    return fpushFuncArr(env, numParams);
  }
  if (callee->isA(TFunc)) {
    popC(env);
    ifElse(
      env,
      [&] (Block* taken) {
        gen(env, CheckNonNull, taken, gen(env, LdFuncCls, callee));
        auto const attr = AttrData {static_cast<int32_t>(AttrIsMethCaller)};
        gen(env, JmpNZero, taken, gen(env, FuncHasAttr, attr, callee));
      },
      [&] { // next, attrs & IsMethCaller == 0 && Func has Cls
        hint(env, Block::Hint::Unlikely);
        gen(
          env,
          RaiseError,
          cns(env, makeStaticString(Strings::CALL_ILLFORMED_FUNC))
        );
      }
    );
    prepareToCallUnknown(env, callee, nullptr, numParams, nullptr, false, false,
                         nullptr);
    return;
  }
  if (topC(env)->isA(TClsMeth)) {
    return fpushFuncClsMeth(env, numParams);
  }

  if (!callee->isA(TStr)) {
    PUNT(FPushFunc_not_Str);
  }

  popC(env);

  auto const prepare = [&] (IRSPRelOffset arOffset) {
    auto const func = gen(env, LdFunc, IRSPRelOffsetData { arOffset }, callee,
                          sp(env), fp(env));
    decRef(env, callee);
    return func;
  };
  auto const tsList = getReifiedGenerics(env, callee);
  prepareToCallCustom(env, nullptr, numParams, true, tsList, prepare);
}

void emitResolveFunc(IRGS& env, const StringData* name) {
  auto const lookup = lookupImmutableFunc(curUnit(env), name);
  auto func = lookup.func;
  if (!func) {
    push(env, gen(env, LookupFuncCached, FuncNameData { name }));
    return;
  }
  if (lookup.needsUnitLoad) gen(env, LookupFuncCached, FuncNameData { name });
  push(env, cns(env, func));
}

namespace {

void fcallObjMethod(IRGS& env, const FCallArgs& fca, const StringData* clsHint,
                     ObjMethodOp subop, SSATmp* methodName, bool dynamic,
                     SSATmp* tsList, bool extraInput) {
  assertx(methodName->isA(TStr));
  auto const objPos = fca.numArgsInclUnpack() + (extraInput ? 3 : 2);
  auto const obj = topC(env, BCSPRelOffset { static_cast<int32_t>(objPos) });

  if (obj->type() <= TObj) {
    if (extraInput) popC(env);
    fcallObjMethodObj(env, fca, obj, clsHint, methodName, dynamic, tsList);
    return;
  }

  // null?->method(...), pop extra stack input, all arguments and two uninits,
  // the null "object" and all uninits for inout returns, then push null.
  if (obj->type() <= TInitNull && subop == ObjMethodOp::NullSafe) {
    if (extraInput) popDecRef(env, DataTypeGeneric);
    if (fca.hasUnpack()) popDecRef(env, DataTypeGeneric);
    for (uint32_t i = 0; i < fca.numArgs; ++i) popDecRef(env, DataTypeGeneric);
    popU(env);
    popU(env);
    popDecRef(env, DataTypeGeneric);
    for (uint32_t i = 0; i < fca.numRets - 1; ++i) popU(env);
    push(env, cns(env, TInitNull));
    return;
  }

  interpOne(env);
}

} // namespace

void emitFCallObjMethod(IRGS& env, FCallArgs fca, const StringData* clsHint,
                        ObjMethodOp subop, const ImmVector& v) {
  auto const methodName = topC(env);
  if (v.size() != 0 || !methodName->isA(TStr)) return interpOne(env);
  fcallObjMethod(env, fca, clsHint, subop, methodName, true, nullptr, true);
}

void emitFCallObjMethodD(IRGS& env, FCallArgs fca, const StringData* clsHint,
                         ObjMethodOp subop, const StringData* methodName) {
  fcallObjMethod(env, fca, clsHint, subop, cns(env, methodName), false,
                 nullptr, false);
}


void emitFCallObjMethodRD(IRGS& env, FCallArgs fca, const StringData* clsHint,
                          ObjMethodOp subop, const StringData* methodName) {
  auto const tsList = topC(env);
  fcallObjMethod(env, fca, clsHint, subop, cns(env, methodName), false, tsList,
                 true);
}

namespace {

SSATmp* loadClsMethodUnknown(IRGS& env,
                             const ClsMethodData& data,
                             Block* onFail) {
  // Look up the Func* in the targetcache. If it's not there, try the slow
  // path. If that fails, slow exit.
  return cond(
    env,
    [&] (Block* taken) {
      return gen(env, LdClsMethodCacheFunc, data, taken);
    },
    [&] (SSATmp* func) { // next
      implIncStat(env, Stats::TgtCache_StaticMethodHit);
      return func;
    },
    [&] { // taken
      hint(env, Block::Hint::Unlikely);
      auto const result = gen(env, LookupClsMethodCache, data, fp(env));
      return gen(env, CheckNonNull, onFail, result);
    }
  );
}

void fcallClsMethodD(IRGS& env,
                     const FCallArgs& fca,
                     const StringData* classHint,
                     const StringData* className,
                     const StringData* methodName,
                     bool isRFlavor) {
  // TODO: take advantage of classHint if it is unique, but className is not
  auto const cls = Unit::lookupUniqueClassInContext(className, curClass(env));
  if (cls) {
    auto const func = lookupImmutableClsMethod(cls, methodName, curFunc(env),
                                               true);
    if (func) {
      if (!classIsPersistentOrCtxParent(env, cls)) {
        gen(env, LdClsCached, cns(env, className));
      }
      auto const ctx = ldCtxForClsMethod(env, func, cns(env, cls), cls, true);
      auto const tsList = isRFlavor ? popC(env) : nullptr;
      return prepareAndCallKnown(env, func, fca, ctx, nullptr, false, tsList);
    }
  }

  auto const slowExit = makeExitSlow(env);
  auto const ne = NamedEntity::get(className);
  auto const data = ClsMethodData { className, methodName, ne };
  auto const func = loadClsMethodUnknown(env, data, slowExit);
  auto const ctx = gen(env, LdClsMethodCacheCls, data);
  auto const tsList = isRFlavor ? popC(env) : nullptr;
  prepareAndCallProfiled(env, func, fca, ctx, false, false, tsList);
}

}

void emitFCallClsMethodD(IRGS& env,
                         FCallArgs fca,
                         const StringData* classHint,
                         const StringData* className,
                         const StringData* methodName) {
  fcallClsMethodD(env, fca, classHint, className, methodName, false);
}

void emitFCallClsMethodRD(IRGS& env,
                          FCallArgs fca,
                          const StringData* classHint,
                          const StringData* className,
                          const StringData* methodName) {
  fcallClsMethodD(env, fca, classHint, className, methodName, true);
}

const StaticString s_resolveMagicCall(
  "Unable to resolve magic call for inst_meth()");

void emitResolveObjMethod(IRGS& env) {
  auto const name = topC(env, BCSPRelOffset { 0 });
  auto const obj = topC(env, BCSPRelOffset { 1 });
  if (!(obj->type() <= TObj) || !(name->type() <= TStr)) {
    PUNT(ResolveObjMethod-nonObjStr);
  }
  if (!name->hasConstVal()) PUNT(ResolveObjMethod-nonConstStr);
  auto cls = obj->type().clsSpec().cls();
  if (!cls || env.irb->constrainValue(obj, GuardConstraint(cls).setWeak())) {
    PUNT(ResolveObjMethod-unknownClass);
  }
  auto const exactClass = obj->type().clsSpec().exact() ||
                    cls->attrs() & AttrNoOverride;
  auto const methodName = name->strVal();

  SSATmp* func = nullptr;
  auto const lookup = lookupImmutableObjMethod(
    cls, methodName, curFunc(env), exactClass);
  switch (lookup.type) {
    case ImmutableObjMethodLookup::Type::NotFound:
      PUNT(ResolveObjMethod-unknownObjMethod);
    case ImmutableObjMethodLookup::Type::MagicFunc:
      gen(env, ThrowInvalidOperation, cns(env, s_resolveMagicCall.get()));
      return;
    case ImmutableObjMethodLookup::Type::Func:
      lookupObjMethodExactFunc(env, obj, lookup.func);
      func = cns(env, lookup.func);
      break;
    case ImmutableObjMethodLookup::Type::Class:
      func = lookupObjMethodNonExactFunc(env, obj, lookup.func);
      break;
    case ImmutableObjMethodLookup::Type::Interface:
      func = lookupObjMethodInterfaceFunc(env, obj, lookup.func);
      break;
  }

  assertx(func);
  auto methPair = gen(env, AllocVArray, PackedArrayData { 2 });
  gen(env, InitPackedLayoutArray, IndexData { 0 }, methPair, obj);
  gen(env, InitPackedLayoutArray, IndexData { 1 }, methPair, func);
  decRef(env, name);
  popC(env);
  popC(env);
  push(env, methPair);
}


const StaticString s_resolveClsMagicCall(
  "Unable to resolve magic call for class_meth()");

void emitResolveClsMethod(IRGS& env) {
  auto const classNameTmp = topC(env, BCSPRelOffset { 1 });
  auto const methodNameTmp = topC(env, BCSPRelOffset { 0 });
  if (!classNameTmp->hasConstVal(TStr) || !methodNameTmp->hasConstVal(TStr)) {
    return interpOne(env);
  }
  auto className = classNameTmp->strVal();
  auto methodName = methodNameTmp->strVal();

  auto const clsMeth = [&] {
    auto const cls = Unit::lookupUniqueClassInContext(className, curClass(env));
    if (cls) {
      auto const func = lookupImmutableClsMethod(cls, methodName, curFunc(env),
                                                 true);
      if (func) {
        if (!classIsPersistentOrCtxParent(env, cls)) {
          gen(env, LdClsCached, classNameTmp);
        }
        ldCtxForClsMethod(env, func, cns(env, cls), cls, true);

        // For clsmeth, we want to return the class user gave,
        // not the class where func is associated with.
        return gen(env, NewClsMeth, cns(env, cls), cns(env, func));
      }
    }

    auto const slowExit = makeExitSlow(env);
    auto const ne = NamedEntity::get(className);
    auto const data = ClsMethodData { className, methodName, ne };
    auto const funcTmp = loadClsMethodUnknown(env, data, slowExit);
    auto const clsTmp = gen(env, LdClsCached, classNameTmp);
    return gen(env, NewClsMeth, clsTmp, funcTmp);
  }();

  decRef(env, methodNameTmp);
  decRef(env, classNameTmp);
  popC(env);
  popC(env);
  push(env, clsMeth);
}

namespace {

SSATmp* forwardCtx(IRGS& env, const Func* parentFunc, SSATmp* funcTmp) {
  assertx(!parentFunc->isClosureBody());
  assertx(funcTmp->type() <= TFunc);

  if (parentFunc->isStatic()) {
    return gen(env, FwdCtxStaticCall, ldCtx(env));
  }

  if (!hasThis(env)) {
    assertx(!parentFunc->isStaticInPrologue());
    gen(env, ThrowMissingThis, funcTmp);
    return ldCtx(env);
  }

  auto const obj = castCtxThis(env, ldCtx(env));
  gen(env, IncRef, obj);
  return obj;
}

SSATmp* lookupClsMethodKnown(IRGS& env,
                             const StringData* methodName,
                             SSATmp* callerCtx,
                             const Class *baseClass,
                             bool exact,
                             bool forward,
                             SSATmp*& calleeCtx) {
  auto const func = lookupImmutableClsMethod(
    baseClass, methodName, curFunc(env), exact);
  if (!func) return nullptr;

  auto funcTmp = exact || func->isImmutableFrom(baseClass) ?
    cns(env, func) :
    gen(env, LdClsMethod, callerCtx, cns(env, -(func->methodSlot() + 1)));

  calleeCtx = forward
    ? forwardCtx(env, func, funcTmp)
    : ldCtxForClsMethod(env, func, callerCtx, baseClass, exact);
  return funcTmp;
}

void fcallClsMethodCommon(IRGS& env,
                          const FCallArgs& fca,
                          const StringData* clsHint,
                          SSATmp* clsVal,
                          SSATmp* methVal,
                          bool forward,
                          bool dynamic,
                          uint32_t numExtraInputs,
                          SSATmp* tsList) {
  assertx(clsVal->isA(TCls));
  assertx(methVal->isA(TStr));

  auto const emitFCall = [&] (TargetProfile<MethProfile>* profile = nullptr,
                              bool noCallProfiling = false) {
    auto const curCls = curClass(env);
    auto const thiz = curCls && hasThis(env) ? ldThis(env) : cns(env, nullptr);
    auto const lctx = curCls ? cns(env, curCls) : cns(env, nullptr);
    auto const funcN = gen(env, LookupClsMethod, clsVal, methVal, thiz, lctx);
    auto const func = gen(env, CheckNonNull, makeExitSlow(env), funcN);

    if (profile && profile->profiling()) {
      auto const pctData = ProfileCallTargetData { profile->handle() };
      gen(env, ProfileMethod, pctData, clsVal, func);
    }

    auto const ctx = forward ? gen(env, FwdCtxStaticCall, ldCtx(env)) : clsVal;
    auto const mightCareAboutDynCall =
      RuntimeOption::EvalForbidDynamicCallsToClsMeth > 0;
    decRef(env, methVal);
    discard(env, numExtraInputs);
    if (noCallProfiling) {
      prepareAndCallUnknown(env, func, fca, ctx, nullptr, dynamic,
                            mightCareAboutDynCall, tsList, true);
    } else {
      prepareAndCallProfiled(env, func, fca, ctx, dynamic,
                             mightCareAboutDynCall, tsList);
    }
  };

  if (!methVal->hasConstVal()) {
    emitFCall();
    return;
  }

  auto const methodName = methVal->strVal();
  auto const knownClass = [&] () -> std::pair<const Class*, bool> {
    if (!clsHint->empty()) {
      auto const cls = Unit::lookupUniqueClassInContext(clsHint, curClass(env));
      if (cls) return std::make_pair(cls, true);
    }

    if (auto const cs = clsVal->type().clsSpec()) {
      return std::make_pair(cs.cls(), cs.exact());
    }

    return std::make_pair(nullptr, false);
  }();

  if (knownClass.first) {
    SSATmp* ctx;
    auto const func = lookupClsMethodKnown(env, methodName, clsVal,
                                           knownClass.first, knownClass.second,
                                           forward, ctx);
    if (func) {
      auto const mightCareAboutDynCall =
        RuntimeOption::EvalForbidDynamicCallsToInstMeth > 0
        || RuntimeOption::EvalForbidDynamicCallsToClsMeth > 0;
      discard(env, numExtraInputs);
      return prepareAndCallProfiled(env, func, fca, ctx, dynamic,
                                    mightCareAboutDynCall, tsList);
    }
  }

  // If the method has reified generics, we can't burn the value in the JIT
  if (!RuntimeOption::RepoAuthoritative || clsVal->hasConstVal() || forward ||
      tsList) {
    emitFCall();
    return;
  }

  optimizeProfiledCallMethod(env, fca, clsVal, false, methodName, dynamic,
                             numExtraInputs, emitFCall);
}

}

void emitFCallClsMethod(IRGS& env, FCallArgs fca, const StringData* clsHint,
                        const ImmVector& v) {
  auto const cls = topC(env);
  auto const methName = topC(env, BCSPRelOffset { 1 });
  if (v.size() != 0 || !cls->isA(TCls) || !methName->isA(TStr)) {
    return interpOne(env);
  }

  fcallClsMethodCommon(env, fca, clsHint, cls, methName, false, true, 2,
                       nullptr);
}

void emitFCallClsMethodS(IRGS& env, FCallArgs fca, const StringData* clsHint,
                         SpecialClsRef ref, const ImmVector& v) {
  auto const cls = specialClsRefToCls(env, ref);
  auto const methName = topC(env);
  if (v.size() != 0 || !cls || !methName->isA(TStr)) return interpOne(env);

  auto const fwd = ref == SpecialClsRef::Self || ref == SpecialClsRef::Parent;
  fcallClsMethodCommon(env, fca, clsHint, cls, methName, fwd, true, 1, nullptr);
}

void emitFCallClsMethodSD(IRGS& env, FCallArgs fca, const StringData* clsHint,
                          SpecialClsRef ref, const StringData* methName) {
  auto const cls = specialClsRefToCls(env, ref);
  if (!cls) return interpOne(env);

  auto const fwd = ref == SpecialClsRef::Self || ref == SpecialClsRef::Parent;
  fcallClsMethodCommon(env, fca, clsHint, cls, cns(env, methName), fwd, false,
                       0, nullptr);
}

void emitFCallClsMethodSRD(IRGS& env, FCallArgs fca, const StringData* clsHint,
                           SpecialClsRef ref, const StringData* methName) {
  auto const cls = specialClsRefToCls(env, ref);
  auto const tsList = topC(env);
  if (!cls) return interpOne(env);

  auto const fwd = ref == SpecialClsRef::Self || ref == SpecialClsRef::Parent;
  fcallClsMethodCommon(env, fca, clsHint, cls, cns(env, methName), fwd, false,
                       1, tsList);
}

//////////////////////////////////////////////////////////////////////

namespace {

SSATmp* ldPreLiveFunc(IRGS& env, IRSPRelOffset actRecOff) {
  if (env.currentNormalizedInstruction->funcd) {
    return cns(env, env.currentNormalizedInstruction->funcd);
  }

  // Try to load Func* from fpiStack, but only if we are not forming a region.
  // Otherwise we may have inferred the target of the call based on specialized
  // type information that won't be available when the region is translated.
  // If we allow the FCall to specialize using this information, we may infer
  // narrower type for the return value, erroneously preventing the region from
  // breaking on unknown type.
  if (!env.formingRegion) {
    auto const& fpiStack = env.irb->fs().fpiStack();
    if (!fpiStack.empty() && fpiStack.back().func) {
      return cns(env, fpiStack.back().func);
    }
  }

  return gen(env, LdARFuncPtr, TFunc, IRSPRelOffsetData { actRecOff }, sp(env));
}

} // namespace

void emitFCall(IRGS& env, FCallArgs fca, const StringData*, const StringData*) {
  auto const numStackInputs = fca.numArgsInclUnpack();
  auto const actRecOff = spOffBCFromIRSP(env) + numStackInputs;
  auto const callee = ldPreLiveFunc(env, actRecOff);

  auto const tryInline = [&] (const Func* knownCallee) {
    // Make sure the FPushOp was in the region.
    auto const& fpiStack = env.irb->fs().fpiStack();
    if (fpiStack.empty()) return false;

    // Make sure the FPushOp wasn't interpreted, based on a spanned another
    // call, or marked as not eligible for inlining by frame-state.
    auto const& info = fpiStack.back();
    if (!info.inlineEligible || info.spansCall) return false;

    // Its possible that we have an "FCall T2 meth" guarded by eg an
    // InstanceOfD T2, and that we know the object has type T1, and we
    // also know that T1::meth exists. The FCall is actually
    // unreachable, but we might not have figured that out yet - so we
    // could be trying to inline T1::meth while the fpiStack has
    // T2::meth.
    if (info.func && info.func != knownCallee) return false;

    return irGenTryInlineFCall(env, knownCallee, fca, info.ctx, info.ctxType,
                               info.fpushOpc);
  };

  auto const doCallKnown = [&] (const Func* knownCallee) {
    if (!callee->hasConstVal()) {
      auto const data = IRSPRelOffsetData{ actRecOff };
      gen(env, AssertARFunc, data, sp(env), cns(env, knownCallee));
    }
    if (!emitCallerReffinessChecksKnown(env, knownCallee, fca)) return;

    if (tryInline(knownCallee)) return;
    callKnown(env, knownCallee, fca);
  };

  if (callee->hasConstVal()) return doCallKnown(callee->funcVal());

  auto const doCallUnknown = [&] (bool unlikely) {
    emitCallerReffinessChecksUnknown(env, callee, fca);
    callUnknown(env, callee, fca, unlikely, false);
  };

  callProfiledFunc(env, callee, doCallKnown, doCallUnknown);
}

void emitDirectCall(IRGS& env, Func* callee, uint32_t numParams,
                    SSATmp* const* const args) {
  auto const callBcOffset = bcOff(env) - curFunc(env)->base();

  allocActRec(env);
  for (int32_t i = 0; i < numParams; i++) {
    push(env, args[i]);
  }

  env.irb->fs().setFPushOverride(Op::FPushFuncD);
  prepareToCallKnown(env, callee, nullptr, numParams, nullptr, false, nullptr);
  assertx(!env.irb->fs().hasFPushOverride());

  updateMarker(env);
  env.irb->exceptionStackBoundary();

  auto const retVal = gen(
    env,
    Call,
    CallData {
      spOffBCFromIRSP(env),
      static_cast<uint32_t>(numParams),
      0,
      callBcOffset,
      callee,
      false
    },
    sp(env),
    fp(env)
  );

  push(env, retVal);
}

//////////////////////////////////////////////////////////////////////

Type callReturnType(const Func* callee) {
  // Don't make any assumptions about functions which can be intercepted. The
  // interception functions can return arbitrary types.
  if (RuntimeOption::EvalJitEnableRenameFunction ||
      callee->attrs() & AttrInterceptable) {
    return TInitCell;
  }

  if (callee->isCPPBuiltin()) {
    // If the function is builtin, use the builtin's return type, then take into
    // account coercion failures.
    return builtinReturnType(callee);
  }

  if (callee->takesInOutParams()) {
    auto const ty = typeFromRAT(callee->repoReturnType(), callee->cls());
    if (ty <= TVec) return vecElemType(ty, Type::cns(0), callee->cls()).first;
    return TInitCell;
  }

  // Otherwise use HHBBC's analysis if present
  return typeFromRAT(callee->repoReturnType(), callee->cls());
}

Type callOutType(const Func* callee, uint32_t index) {
  assertx(callee->takesInOutParams());
  assertx(index < callee->numInOutParams());

  // Don't make any assumptions about functions which can be intercepted. The
  // interception functions can return arbitrary types.
  if (RuntimeOption::EvalJitEnableRenameFunction ||
      callee->attrs() & AttrInterceptable) {
    return TInitCell;
  }

  if (callee->isCPPBuiltin()) {
    uint32_t param_idx = 0;
    for (; param_idx < callee->numParams(); param_idx++) {
      if (!callee->params()[param_idx].inout) continue;
      if (!index) break;
      index--;
    }
    assertx(!index);
    // If the function is builtin, use the builtin's return type, then take into
    // account coercion failures.
    return builtinOutType(callee, param_idx);
  }

  auto const ty = typeFromRAT(callee->repoReturnType(), callee->cls());
  if (ty <= TVec) {
    auto const off = callee->numInOutParams() - index - 1;
    return vecElemType(ty, Type::cns(off + 1), callee->cls()).first;
  }
  return TInitCell;
}

Type awaitedCallReturnType(const Func* callee) {
  // Don't make any assumptions about functions which can be intercepted. The
  // interception functions can return arbitrary types.
  if (RuntimeOption::EvalJitEnableRenameFunction ||
      callee->attrs() & AttrInterceptable) {
    return TInitCell;
  }

  return typeFromRAT(callee->repoAwaitedReturnType(), callee->cls());
}

//////////////////////////////////////////////////////////////////////

}}}
