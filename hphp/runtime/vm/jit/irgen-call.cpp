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

const Class* callContext(IRGS& env, const FCallArgs& fca) {
  if (!fca.context) return curClass(env);
  return lookupUniqueClass(env, fca.context, true /* trustUnit */);
}

bool emitCallerInOutChecksKnown(IRGS& env, const Func* callee,
                                    const FCallArgs& fca) {
  if (!fca.enforceInOut()) return true;

  for (auto i = 0; i < fca.numArgs; ++i) {
    if (callee->isInOut(i) != fca.isInOut(i)) {
      auto const func = cns(env, callee);
      gen(env, ThrowParamInOutMismatch, ParamData { i }, func);
      return false;
    }
  }
  return true;
}

void emitCallerInOutChecksUnknown(IRGS& env, SSATmp* callee,
                                      const FCallArgs& fca) {
  if (!fca.enforceInOut()) return;

  SSATmp* numParams = nullptr;
  for (uint32_t i = 0; i * 8 < fca.numArgs; i += 8) {
    uint64_t vals = 0;
    for (uint32_t j = 0; j < 8 && (i + j) * 8 < fca.numArgs; ++j) {
      vals |= ((uint64_t)fca.inoutArgs[i + j]) << (8 * j);
    }

    uint64_t bits = fca.numArgs - i * 8;
    uint64_t mask = bits >= 64
      ? std::numeric_limits<uint64_t>::max()
      : (1UL << bits) - 1;

    // CheckInOuts only needs to know the number of parameters when there are more
    // than 64 args.
    if (i == 0) {
      numParams = cns(env, 64);
    } else if (!numParams || numParams->hasConstVal()) {
      numParams = gen(env, LdFuncNumParams, callee);
    }

    auto const crData = CheckInOutsData { i * 8, mask, vals };
    ifThen(
      env,
      [&] (Block* taken) {
        gen(env, CheckInOuts, taken, crData, callee, numParams);
      },
      [&] {
        hint(env, Block::Hint::Unlikely);
        gen(env, ThrowParamInOutMismatchRange, crData, callee);
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
  if (callee->isDynamicallyCallable() &&
      !RuntimeOption::EvalForbidDynamicCallsWithAttr) {
        return;
  }
  gen(env, RaiseForbiddenDynCall, cns(env, callee));
}

void emitCallerDynamicCallChecksUnknown(IRGS& env, SSATmp* callee,
                                        bool mightCareAboutDynCall) {
  assertx(!callee->hasConstVal());
  if (!mightCareAboutDynCall) return;

  if(RuntimeOption::EvalForbidDynamicCallsWithAttr) {
    gen(env, RaiseForbiddenDynCall, callee);
  } else {
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

void callUnpack(IRGS& env, SSATmp* callee, const FCallArgs& fca,
                SSATmp* objOrClass, bool dynamicCall, bool unlikely) {
  assertx(fca.hasUnpack());

  if (objOrClass == nullptr) objOrClass = cns(env, TNullptr);
  assertx(objOrClass->isA(TNullptr) || objOrClass->isA(TObj|TCls));

  auto const data = CallUnpackData {
    spOffBCFromIRSP(env),
    fca.numArgs,
    fca.numRets - 1,
    bcOff(env),
    fca.hasGenerics(),
    dynamicCall,
    env.formingRegion
  };
  push(env, gen(env, CallUnpack, data, sp(env), fp(env), callee, objOrClass));
  if (unlikely) gen(env, Jmp, makeExit(env, nextBcOff(env)));
}

SSATmp* callImpl(IRGS& env, SSATmp* callee, const FCallArgs& fca,
                 SSATmp* objOrClass, bool dynamicCall, bool asyncEagerReturn) {
  assertx(!fca.hasUnpack() ||
          callee->funcVal()->numNonVariadicParams() == fca.numArgs);

  // TODO: extend hhbc with bitmap of passed generics, or even better, use one
  // stack value per generic argument and extend hhbc with their count
  auto const genericsBitmap = [&] {
    if (!fca.hasGenerics()) return uint32_t{0};
    auto const type = RuntimeOption::EvalHackArrDVArrs ? TVec : TArr;
    auto const generics = topC(env);
    // Do not bother calculating the bitmap using a C++ helper if generics are
    // not statically known, as the prologue already has the same logic.
    if (!generics->hasConstVal(type)) return uint32_t{0};
    auto const genericsArr = generics->arrLikeVal();
    return getGenericsBitmap(genericsArr);
  }();

  if (objOrClass == nullptr) objOrClass = cns(env, TNullptr);
  assertx(objOrClass->isA(TNullptr) || objOrClass->isA(TObj|TCls));

  auto const data = CallData {
    spOffBCFromIRSP(env),
    fca.numArgs,
    fca.numRets - 1,
    bcOff(env) - curFunc(env)->base(),
    genericsBitmap,
    fca.hasGenerics(),
    fca.hasUnpack(),
    dynamicCall,
    asyncEagerReturn,
    env.formingRegion,
    fca.skipNumArgsCheck
  };
  return gen(env, Call, data, sp(env), fp(env), callee, objOrClass);
}

void handleCallReturn(IRGS& env, const Func* callee, const FCallArgs& fca,
                      SSATmp* retVal, bool asyncEagerReturn, bool unlikely) {
  if (!asyncEagerReturn) {
    push(env, retVal);
    if (unlikely) gen(env, Jmp, makeExit(env, nextBcOff(env)));
    return;
  }

  if (env.formingRegion) {
    // See callReturn() in ir-instruction.cpp.
    callee = nullptr;
  }

  ifThenElse(
    env,
    [&] (Block* taken) {
      auto const aux = gen(env, LdTVAux, LdTVAuxData {}, retVal);
      auto const tst = gen(env, AndInt, aux, cns(env, 1u << 31));
      gen(env, JmpZero, taken, tst);
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

//////////////////////////////////////////////////////////////////////

/*
 * In PGO mode, we use profiling to try to determine the most likely target
 * function at each call site. In profiling translations, this function profiles
 * callees. If the most likely called function based on profiling was seen
 * at least Eval.JitPGOCalledFuncCheckThreshold percent of the times during
 * profiling, optimized translations emit a runtime check whether the callee
 * is the given profiled function and if so, emit a code to invoke it directly.
 */
template<class TKnown, class TUnknown>
void callProfiledFunc(IRGS& env, SSATmp* callee,
                      TKnown callKnown, TUnknown callUnknown) {
  if (!RuntimeOption::RepoAuthoritative) return callUnknown(false);

  auto profile = TargetProfile<CallTargetProfile>(
    env.unit, env.irb->curMarker(), callTargetProfileKey());

  if (profile.profiling()) {
    gen(env, ProfileCall, ProfileCallTargetData { profile.handle() }, callee);
  }

  if (!profile.optimizing()) return callUnknown(false);

  double probability = 0;
  auto const data = profile.data();
  auto const profiledFunc = data.choose(probability);

  // Dump annotations if requested.
  if (RuntimeOption::EvalDumpCallTargets) {
    auto const fnName = curFunc(env)->fullName()->data();
    env.unit.annotationData->add(
      "CallTargets",
      folly::sformat("BC={} FN={}: {}\n", bcOff(env), fnName, data.toString())
    );
  }

  // Don't emit the check if the probability of it succeeding is below the
  // threshold.
  if (profiledFunc == nullptr ||
      probability * 100 < RuntimeOption::EvalJitPGOCalledFuncCheckThreshold) {
    return callUnknown(false);
  }

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

      auto const unlikely = probability * 100 >=
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

void prepareAndCallKnown(IRGS& env, const Func* callee, const FCallArgs& fca,
                         SSATmp* objOrClass, bool dynamicCall) {
  assertx(callee);

  // Caller checks
  if (!emitCallerInOutChecksKnown(env, callee, fca)) return;
  if (dynamicCall) emitCallerDynamicCallChecksKnown(env, callee);
  emitCallerRxChecksKnown(env, callee);

  auto const doCall = [&](const FCallArgs& fca) {
    assertx(fca.numArgs <= callee->numNonVariadicParams());
    assertx(!fca.hasUnpack() || fca.numArgs == callee->numNonVariadicParams());

    // We may have updated the stack, make sure Call can set up its Catch.
    updateMarker(env);
    env.irb->exceptionStackBoundary();

    if (isFCall(curSrcKey(env).op())) {
      if (irGenTryInlineFCall(env, callee, fca, objOrClass, dynamicCall)) {
        return;
      }
    }

    auto const asyncEagerReturn =
      fca.asyncEagerOffset != kInvalidOffset &&
      callee->supportsAsyncEagerReturn();
    auto const retVal = callImpl(env, cns(env, callee), fca, objOrClass,
                                 dynamicCall, asyncEagerReturn);
    handleCallReturn(env, callee, fca, retVal, asyncEagerReturn,
                     false /* unlikely */);
  };

  if (fca.hasUnpack()) {
    // We may have updated the stack, make sure the conversions below plus
    // CallUnpack can set up its Catch.
    updateMarker(env);
    env.irb->exceptionStackBoundary();

    if (fca.numArgs == callee->numNonVariadicParams()) {
      auto const unpackOff = BCSPRelOffset{fca.hasGenerics() ? 1 : 0};
      auto const unpack = topC(env, unpackOff);
      auto const doConvertAndCallImpl = [&](SSATmp* converted) {
        auto const offset = offsetFromIRSP(env, unpackOff);
        gen(env, StStk, IRSPRelOffsetData{offset}, sp(env), converted);
        doCall(fca);
      };
      auto const doConvertAndCall = [&](Opcode op) {
        doConvertAndCallImpl(gen(env, op, unpack));
      };
      if (RuntimeOption::EvalHackArrDVArrs) {
        if (unpack->isA(TVec)) return doCall(fca);
        if (unpack->isA(TArr)) return doConvertAndCall(ConvArrToVec);
        if (unpack->isA(TDict)) return doConvertAndCall(ConvDictToVec);
        if (unpack->isA(TKeyset)) return doConvertAndCall(ConvKeysetToVec);
      } else {
        if (unpack->isA(TArr)) {
          return doConvertAndCallImpl(cond(
            env,
            [&](Block* taken) { return gen(env, CheckVArray, taken, unpack); },
            [&](SSATmp* varr) { return varr; },
            [&] { return gen(env, ConvArrToVArr, unpack); }
          ));
        }
        if (unpack->isA(TVec)) return doConvertAndCall(ConvVecToVArr);
        if (unpack->isA(TDict)) return doConvertAndCall(ConvDictToVArr);
        if (unpack->isA(TKeyset)) return doConvertAndCall(ConvKeysetToVArr);
      }
    }

    // Slow path. TODO: remove and use interpreter once confirmed this is cold
    return callUnpack(env, cns(env, callee), fca, objOrClass, dynamicCall,
                      false /* unlikely */);
  }

  if (fca.numArgs <= callee->numNonVariadicParams()) {
    return doCall(fca);
  }

  // Pack extra arguments. The prologue can handle this even if the function
  // does not accept variadic arguments.
  auto const generics = fca.hasGenerics() ? popC(env) : nullptr;
  auto const numToPack = fca.numArgs - callee->numNonVariadicParams();
  for (auto i = 0; i < numToPack; ++i) {
    assertTypeStack(env, BCSPRelOffset{i}, TInitCell);
  }
  if (RuntimeOption::EvalHackArrDVArrs) {
    emitNewVecArray(env, numToPack);
  } else {
    emitNewVArray(env, numToPack);
  }
  if (generics) push(env, generics);

  doCall(FCallArgs(
    static_cast<FCallArgs::Flags>(fca.flags | FCallArgs::Flags::HasUnpack),
    callee->numNonVariadicParams(),
    fca.numRets,
    nullptr,  // inout-ness already checked
    fca.asyncEagerOffset,
    fca.lockWhileUnwinding,
    fca.skipNumArgsCheck,
    fca.context
  ));
}

void prepareAndCallUnknown(IRGS& env, SSATmp* callee, const FCallArgs& fca,
                           SSATmp* objOrClass, bool dynamicCall,
                           bool mightCareAboutDynCall, bool unlikely) {
  assertx(callee->isA(TFunc));
  if (callee->hasConstVal()) {
    prepareAndCallKnown(env, callee->funcVal(), fca, objOrClass, dynamicCall);
    return;
  }

  // Caller checks
  emitCallerInOutChecksUnknown(env, callee, fca);
  if (dynamicCall) {
    emitCallerDynamicCallChecksUnknown(env, callee, mightCareAboutDynCall);
  }
  emitCallerRxChecksUnknown(env, callee);

  // We may have updated the stack, make sure Call opcode can set up its Catch.
  updateMarker(env);
  env.irb->exceptionStackBoundary();

  if (fca.hasUnpack()) {
    return callUnpack(env, callee, fca, objOrClass, dynamicCall, unlikely);
  }

  // Okay to request async eager return even if it is not supported.
  auto const asyncEagerReturn = fca.asyncEagerOffset != kInvalidOffset;
  auto const retVal = callImpl(env, callee, fca, objOrClass, dynamicCall,
                               asyncEagerReturn);
  handleCallReturn(env, nullptr, fca, retVal, asyncEagerReturn, unlikely);
}

void prepareAndCallProfiled(IRGS& env, SSATmp* callee, const FCallArgs& fca,
                            SSATmp* objOrClass, bool dynamicCall,
                            bool mightCareAboutDynCall) {
  assertx(callee->isA(TFunc));
  auto const handleKnown = [&] (const Func* knownCallee) {
    prepareAndCallKnown(env, knownCallee, fca, objOrClass, dynamicCall);
  };
  if (callee->hasConstVal()) return handleKnown(callee->funcVal());

  auto const handleUnknown = [&] (bool unlikely) {
    prepareAndCallUnknown(env, callee, fca, objOrClass, dynamicCall,
                          mightCareAboutDynCall, unlikely);
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
  uint32_t numExtraInputs,
  TProfile profileMethod,
  bool noCallProfiling
) {
  implIncStat(env, Stats::ObjMethod_cached);

  updateMarker(env);
  env.irb->exceptionStackBoundary();

  auto const callerCtx = [&] {
    if (!fca.context) return curClass(env);
    auto const ret = lookupUniqueClass(env, fca.context, true /* trustUnit */);
    if (!ret) PUNT(no-context);
    return ret;
  }();

  auto const func = [&] {
    auto const cls = gen(env, LdObjClass, obj);
    if (!methodName->hasConstVal()) {
      auto const func =
        gen(env, LdObjMethodD, OptClassData { callerCtx }, cls, methodName);
      return gen(env, CheckNonNull, makeExitSlow(env), func);
    }

    auto const tcCache = gen(env, LdSmashable);
    return cond(
      env,
      [&] (Block* taken) {
        // check for TC cache hit; go to the slow check on miss
        gen(env, CheckSmashableClass, taken, tcCache, cls);
      },
      [&] {
        // fast path: load func from TC cache and proceed to regular call
        return gen(env, LdSmashableFunc, tcCache);
      },
      [&] {
        // slow path: run C++ helper to determine Func*, exit if we can't handle
        // the call in the JIT
        auto const fnData = FuncNameData { methodName->strVal(), callerCtx };
        auto const func = gen(env, LdObjMethodS, fnData, cls, tcCache);
        return gen(env, CheckNonNull, makeExitSlow(env), func);
      }
    );
  }();

  auto const mightCareAboutDynCall =
    RuntimeOption::EvalForbidDynamicCallsToInstMeth > 0
    || RuntimeOption::EvalForbidDynamicCallsToClsMeth > 0;
  profileMethod(func);
  discard(env, numExtraInputs);
  if (noCallProfiling) {
    prepareAndCallUnknown(env, func, fca, obj, dynamic, mightCareAboutDynCall,
                          true);
  } else {
    prepareAndCallProfiled(env, func, fca, obj, dynamic, mightCareAboutDynCall);
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
  auto const slot = cns(env, superFunc->methodSlot());
  auto const func = gen(env, LdClsMethod, cls, slot);
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
    while (val->inst()->is(AssertType, CheckType)) {
      val = val->inst()->src(0);
    }
    return val;
  };

  auto const canUseThis = [&] () -> bool {
    // A static::foo() call can always pass through a $this
    // from the caller (if it has one). Match the common patterns
    auto cc = skipAT(callCtx);
    if (cc->inst()->is(LdFrameCls)) return true;
    if (cc->inst()->is(LdObjClass)) {
      cc = skipAT(cc->inst()->src(0));
      if (cc->inst()->is(LdFrameThis)) return true;
    }
    return maybeUseThis && (exact || cls->attrs() & AttrNoOverride);
  }();

  auto thiz = ldThis(env);

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
      return gen_missing_this();
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
  auto profile = TargetProfile<MethProfile>(env.unit, env.irb->curMarker(),
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
    assertx(uniqueMeth->name()->isame(methodName));
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
          prepareAndCallKnown(env, uniqueMeth, fca, ctx, dynamic);
        },
        fallback
      );
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
        auto const cls = isStaticCall
          ? objOrCls : gen(env, LdObjClass, objOrCls);
        auto const len = gen(env, LdFuncVecLen, cls);
        auto const cmp = gen(env, LteInt, len, slot);
        gen(env, JmpNZero, sideExit, cmp);
        auto const meth = gen(env, LdClsMethod, cls, slot);
        auto const same = gen(env, EqFunc, meth, cns(env, uniqueMeth));
        gen(env, JmpZero, sideExit, same);
        auto const ctx = getCtx(uniqueMeth, objOrCls, nullptr);
        discard(env, numExtraInputs);
        prepareAndCallKnown(env, uniqueMeth, fca, ctx, dynamic);
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
        auto const slot = cns(env, baseMeth->methodSlot());
        auto const meth = gen(env, LdClsMethod, cls, slot);
        auto const ctx = getCtx(baseMeth, objOrCls, nullptr);
        auto const mightCareAboutDynCall =
          RuntimeOption::EvalForbidDynamicCallsToClsMeth > 0
          || RuntimeOption::EvalForbidDynamicCallsToInstMeth > 0;
        discard(env, numExtraInputs);
        prepareAndCallProfiled(env, meth, fca, ctx, dynamic,
                               mightCareAboutDynCall);
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
                               mightCareAboutDynCall);
      },
      fallback
    );
    return;
  }

  emitFCall();
}

void fcallObjMethodMagic(IRGS& env, const Func* callee, const FCallArgs& fca,
                         SSATmp* obj, const StringData* methodName,
                         bool dynamic, uint32_t numExtraInputs) {
  if (fca.hasUnpack() || fca.numRets != 1) return interpOne(env);
  if (fca.enforceInOut()) {
    for (auto i = 0; i < fca.numArgs; ++i) {
      if (fca.isInOut(i)) {
        gen(env, ThrowParamInOutMismatch, ParamData { i }, cns(env, callee));
        return;
      }
    }
  }

  discard(env, numExtraInputs);
  auto const generics = fca.hasGenerics() ? popC(env) : nullptr;
  if (RuntimeOption::EvalHackArrDVArrs) {
    emitNewVecArray(env, fca.numArgs);
  } else {
    emitNewVArray(env, fca.numArgs);
  }
  auto const arr = popC(env);
  push(env, cns(env, methodName));
  push(env, arr);
  if (generics) push(env, generics);

  // We just wrote to the stack, make sure the function call can proceed.
  updateMarker(env);
  env.irb->exceptionStackBoundary();

  assertx(!fca.supportsAsyncEagerReturn());
  auto const fca2 =
    FCallArgs(fca.flags, 2, 1, nullptr, kInvalidOffset, false, false,
              fca.context);
  prepareAndCallKnown(env, callee, fca2, obj, dynamic);
}

void fcallObjMethodObj(IRGS& env, const FCallArgs& fca, SSATmp* obj,
                       const StringData* clsHint, SSATmp* methodName,
                       bool dynamic, uint32_t numExtraInputs) {
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
      auto const cls = lookupUniqueClass(env, clsHint);
      if (cls) {
        assertx(!isInterface(cls));
        obj = gen(env, AssertType, Type::SubObj(cls), obj);
        return lookupImmutableObjMethod(cls, methodName->strVal(),
                                        callContext(env, fca), true);
      }
    }

    if (auto cls = obj->type().clsSpec().cls()) {
      if (!env.irb->constrainValue(obj, GuardConstraint(cls).setWeak())) {
        // We know the class without having to specialize a guard any further.
        // We may still want to use MethProfile to gather more information in
        // case the class isn't known exactly.
        auto const exactClass =
          obj->type().clsSpec().exact() || cls->attrs() & AttrNoOverride;
        return lookupImmutableObjMethod(cls, methodName->strVal(),
                                        callContext(env, fca), exactClass);
      }
    }

    return notFound;
  }();

  const Func* knownIfaceFunc = nullptr;

  // If we know which exact or overridden method to call, we don't need PGO.
  switch (lookup.type) {
    case ImmutableObjMethodLookup::Type::MagicFunc:
      implIncStat(env, Stats::ObjMethod_known);
      assertx(!lookup.func->isStaticInPrologue());
      fcallObjMethodMagic(env, lookup.func, fca, obj, methodName->strVal(),
                          dynamic, numExtraInputs);
      return;
    case ImmutableObjMethodLookup::Type::Func:
      implIncStat(env, Stats::ObjMethod_known);
      if (lookup.func->isStaticInPrologue()) {
        gen(env, ThrowHasThisNeedStatic, cns(env, lookup.func));
        return;
      }
      discard(env, numExtraInputs);
      prepareAndCallKnown(env, lookup.func, fca, obj, dynamic);
      return;
    case ImmutableObjMethodLookup::Type::Class: {
      auto const func = lookupObjMethodNonExactFunc(env, obj, lookup.func);
      auto const mightCareAboutDynCall =
        RuntimeOption::EvalForbidDynamicCallsToInstMeth > 0
        || RuntimeOption::EvalForbidDynamicCallsToClsMeth > 0;
      discard(env, numExtraInputs);
      prepareAndCallProfiled(env, func, fca, obj, dynamic,
                             mightCareAboutDynCall);
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
      fcallObjMethodUnknown(env, fca, obj, methodName, dynamic, numExtraInputs,
                            profileMethod, noCallProfiling);
    } else {
      auto const func = lookupObjMethodInterfaceFunc(env, obj, knownIfaceFunc);
      auto const mightCareAboutDynCall =
        RuntimeOption::EvalForbidDynamicCallsToInstMeth > 0
        || RuntimeOption::EvalForbidDynamicCallsToClsMeth > 0;
      profileMethod(func);
      discard(env, numExtraInputs);
      if (noCallProfiling) {
        prepareAndCallUnknown(env, func, fca, obj, dynamic,
                              mightCareAboutDynCall, true);
      } else {
        prepareAndCallProfiled(env, func, fca, obj, dynamic,
                               mightCareAboutDynCall);
      }
    }
  };

  // If the method has reified generics, we can't burn the value in the JIT
  if (!RuntimeOption::RepoAuthoritative || !methodName->hasConstVal() ||
      fca.hasGenerics()) {
    return emitFCall();
  }

  // If we don't know anything about the object's class, or all we know is an
  // interface that it implements, then enable PGO.
  optimizeProfiledCallMethod(env, fca, obj, knownIfaceFunc != nullptr,
                             methodName->strVal(), dynamic, numExtraInputs,
                             emitFCall);
}

void fcallFuncObj(IRGS& env, const FCallArgs& fca) {
  auto const obj = topC(env);
  assertx(obj->isA(TObj));

  auto const slowExit = makeExitSlow(env);
  auto const cls = gen(env, LdObjClass, obj);
  auto const func = gen(env, LdObjInvoke, slowExit, cls);
  discard(env);
  prepareAndCallProfiled(env, func, fca, obj, false, false);
}

void fcallFuncFunc(IRGS& env, const FCallArgs& fca) {
  auto const func = popC(env);
  assertx(func->isA(TFunc));

  ifElse(
    env,
    [&] (Block* taken) {
      gen(env, CheckNonNull, taken, gen(env, LdFuncCls, func));
      auto const attr = AttrData {static_cast<int32_t>(AttrIsMethCaller)};
      gen(env, JmpNZero, taken, gen(env, FuncHasAttr, attr, func));
    },
    [&] { // next, attrs & IsMethCaller == 0 && Func has Cls
      hint(env, Block::Hint::Unlikely);
      auto const err = cns(env, makeStaticString(Strings::CALL_ILLFORMED_FUNC));
      gen(env, RaiseError, err);
    }
  );
  prepareAndCallProfiled(env, func, fca, nullptr, false, false);
}

void fcallFuncClsMeth(IRGS& env, const FCallArgs& fca) {
  auto const clsMeth = popC(env);
  assertx(clsMeth->isA(TClsMeth));

  auto const cls = gen(env, LdClsFromClsMeth, clsMeth);
  auto const func = gen(env, LdFuncFromClsMeth, clsMeth);
  prepareAndCallProfiled(env, func, fca, cls, false, false);
}

void fcallFuncStr(IRGS& env, const FCallArgs& fca) {
  auto const str = topC(env);
  assertx(str->isA(TStr));

  // TODO: improve this if str->hasConstVal()
  auto const funcN = gen(env, LdFunc, str);
  auto const func = gen(env, CheckNonNull, makeExitSlow(env), funcN);
  popDecRef(env);
  auto const mightCareAboutDynCall =
    RuntimeOption::EvalForbidDynamicCallsToFunc > 0;
  prepareAndCallProfiled(env, func, fca, nullptr, true, mightCareAboutDynCall);
}

void fcallFuncArr(IRGS& env, const FCallArgs& fca) {
  UNUSED auto const arr = topC(env);
  assertx(arr->isA(TArr) || arr->isA(TVec) || arr->isA(TDict));

  return interpOne(env);
}

} // namespace

void emitFCallFuncD(IRGS& env, FCallArgs fca, const StringData* funcName) {
  auto const lookup = lookupImmutableFunc(curUnit(env), funcName);
  auto const callerCtx = [&] {
    if (!fca.context) return curClass(env);
    auto const ret = lookupUniqueClass(env, fca.context, true /* trustUnit */);
    if (!ret) PUNT(no-context);
    return ret;
  }();

  if (lookup.func) {
    // We know the function, but we have to ensure its unit is loaded. Use
    // LdFuncCached, ignoring the result to ensure this.
    if (lookup.needsUnitLoad) {
      gen(env, LdFuncCached, FuncNameData { funcName, callerCtx });
    }
    prepareAndCallKnown(env, lookup.func, fca, nullptr, false);
    return;
  }

  auto const func =
    gen(env, LdFuncCached, FuncNameData { funcName, callerCtx });
  prepareAndCallProfiled(env, func, fca, nullptr, false, false);
}

void emitFCallFunc(IRGS& env, FCallArgs fca) {
  auto const callee = topC(env);
  if (callee->isA(TObj)) return fcallFuncObj(env, fca);
  if (callee->isA(TFunc)) return fcallFuncFunc(env, fca);
  if (callee->isA(TClsMeth)) return fcallFuncClsMeth(env, fca);
  if (callee->isA(TStr)) return fcallFuncStr(env, fca);
  if (callee->isA(TArr)) return fcallFuncArr(env, fca);
  if (callee->isA(TVec)) return fcallFuncArr(env, fca);
  if (callee->isA(TDict)) return fcallFuncArr(env, fca);
  return interpOne(env);
}

void emitResolveFunc(IRGS& env, const StringData* name) {
  auto const lookup = lookupImmutableFunc(curUnit(env), name);
  auto func = lookup.func;
  if (!func) {
    push(env, gen(env, LookupFuncCached, FuncNameData { name, curClass(env) }));
    return;
  }
  if (lookup.needsUnitLoad) {
    gen(env, LookupFuncCached, FuncNameData { name, curClass(env) });
  }
  push(env, cns(env, func));
}

void emitResolveMethCaller(IRGS& env, const StringData* name) {
  auto const lookup = lookupImmutableFunc(curUnit(env), name);
  auto func = lookup.func;
  assertx(func && func->isMethCaller());

  auto const className = func->methCallerClsName();
  auto const methodName = func->methCallerMethName();

  auto const ok = [&] () -> bool {
    auto const cls = lookupUniqueClass(env, className);
    if (cls) {
      auto const res = lookupImmutableObjMethod(cls, methodName, curClass(env),
                                                false);
      return res.func && checkMethCallerTarget(res.func, curClass(env), false);
    }
    return false;
  }();

  if (!ok) return interpOne(env);
  push(env, cns(env, func));
}

namespace {

//////////////////////////////////////////////////////////////////////

SSATmp* specialClsRefToCls(IRGS& env, SpecialClsRef ref) {
  switch (ref) {
    case SpecialClsRef::Static:
      if (!curClass(env)) return nullptr;
      return ldCtxCls(env);
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
  auto const cls = lookupUniqueClass(env, className);
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
    IndexData { curClass(env)->propSlotToIndex(*slot) },
    ty.lval(Ptr::Prop),
    this_
  );
  auto const reified_generic = gen(env, LdMem, ty, addr);
  push(env, gen(env, AllocObjReified, cls, reified_generic));
}

void emitFCallCtor(IRGS& env, FCallArgs fca, const StringData* clsHint) {
  assertx(fca.numRets == 1);
  assertx(fca.asyncEagerOffset == kInvalidOffset);
  auto const objPos = static_cast<int32_t>(fca.numInputs() + 2);
  auto const obj = topC(env, BCSPRelOffset{objPos});
  if (!obj->isA(TObj)) PUNT(FCallCtor-NonObj);

  auto const exactCls = [&] {
    if (!clsHint->empty()) {
      auto const cls = lookupUniqueClass(env, clsHint);
      if (cls) return cls;
    }
    return obj->type().clsSpec().exactCls();
  }();
  if (exactCls) {
    if (auto const ctor = lookupImmutableCtor(exactCls, curClass(env))) {
      return prepareAndCallKnown(env, ctor, fca, obj, false);
    }
  }

  auto const cls = exactCls ? cns(env, exactCls) : gen(env, LdObjClass, obj);
  auto const callee = gen(env, LdClsCtor, cls, fp(env));
  prepareAndCallProfiled(env, callee, fca, obj, false, false);
}

void emitLockObj(IRGS& env) {
  auto obj = topC(env);
  if (!obj->isA(TObj)) PUNT(LockObj-NonObj);
  gen(env, LockObj, obj);
}

namespace {

void fcallObjMethod(IRGS& env, const FCallArgs& fca, const StringData* clsHint,
                    ObjMethodOp subop, SSATmp* methodName, bool dynamic,
                    bool extraInput) {
  assertx(methodName->isA(TStr));
  auto const objPos = fca.numInputs() + (extraInput ? 3 : 2);
  auto const obj = topC(env, BCSPRelOffset { static_cast<int32_t>(objPos) });

  if (obj->type() <= TObj) {
    fcallObjMethodObj(env, fca, obj, clsHint, methodName, dynamic,
                      extraInput ? 1 : 0);
    return;
  }

  // null?->method(...), pop extra stack input, all arguments and two uninits,
  // the null "object" and all uninits for inout returns, then push null.
  if (obj->type() <= TInitNull && subop == ObjMethodOp::NullSafe) {
    if (extraInput) popDecRef(env, DataTypeGeneric);
    if (fca.hasGenerics()) popDecRef(env, DataTypeGeneric);
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
                        ObjMethodOp subop) {
  auto const methodName = topC(env);
  if (!methodName->isA(TStr)) return interpOne(env);
  fcallObjMethod(env, fca, clsHint, subop, methodName, true, true);
}

void emitFCallObjMethodD(IRGS& env, FCallArgs fca, const StringData* clsHint,
                         ObjMethodOp subop, const StringData* methodName) {
  fcallObjMethod(env, fca, clsHint, subop, cns(env, methodName), false, false);
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

}

void emitFCallClsMethodD(IRGS& env,
                         FCallArgs fca,
                         const StringData* classHint,
                         const StringData* className,
                         const StringData* methodName) {
  // TODO: take advantage of classHint if it is unique, but className is not
  auto const cls = lookupUniqueClass(env, className);
  if (cls) {
    auto const func = lookupImmutableClsMethod(cls, methodName,
                                               callContext(env, fca), true);
    if (func) {
      if (!classIsPersistentOrCtxParent(env, cls)) {
        gen(env, LdClsCached, cns(env, className));
      }
      auto const ctx = ldCtxForClsMethod(env, func, cns(env, cls), cls, true);
      return prepareAndCallKnown(env, func, fca, ctx, false);
    }
  }

  auto const callerCtx = [&] {
    if (!fca.context) return curClass(env);
    auto const ret = lookupUniqueClass(env, fca.context, true /* trustUnit */);
    if (!ret) PUNT(no-context);
    return ret;
  }();

  auto const slowExit = makeExitSlow(env);
  auto const ne = NamedEntity::get(className);
  auto const data = ClsMethodData { className, methodName, ne, callerCtx };
  auto const func = loadClsMethodUnknown(env, data, slowExit);
  auto const ctx = gen(env, LdClsMethodCacheCls, data);
  prepareAndCallProfiled(env, func, fca, ctx, false, false);
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
    cls, methodName, curClass(env), exactClass);
  switch (lookup.type) {
    case ImmutableObjMethodLookup::Type::NotFound:
      PUNT(ResolveObjMethod-unknownObjMethod);
    case ImmutableObjMethodLookup::Type::MagicFunc:
      gen(env, ThrowInvalidOperation, cns(env, s_resolveMagicCall.get()));
      return;
    case ImmutableObjMethodLookup::Type::Func:
      if (lookup.func->isStaticInPrologue()) {
        gen(env, ThrowHasThisNeedStatic, cns(env, lookup.func));
        return;
      }
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
  auto methPair = gen(
    env,
    RuntimeOption::EvalHackArrDVArrs ? AllocVecArray : AllocVArray,
    PackedArrayData { 2 }
  );
  gen(env, InitPackedLayoutArray, IndexData { 0 }, methPair, obj);
  gen(env, InitPackedLayoutArray, IndexData { 1 }, methPair, func);
  decRef(env, name);
  popC(env);
  popC(env);
  push(env, methPair);
}

namespace {

SSATmp* forwardCtx(IRGS& env, const Func* parentFunc, SSATmp* funcTmp) {
  assertx(!parentFunc->isClosureBody());
  assertx(funcTmp->type() <= TFunc);

  if (parentFunc->isStatic()) {
    return ldCtxCls(env);
  }

  if (!hasThis(env)) {
    assertx(!parentFunc->isStaticInPrologue());
    gen(env, ThrowMissingThis, funcTmp);
    return cns(env, TBottom);
  }

  auto const obj = ldThis(env);
  gen(env, IncRef, obj);
  return obj;
}

SSATmp* lookupClsMethodKnown(IRGS& env,
                             const StringData* methodName,
                             SSATmp* callerCtx,
                             const Class *baseClass,
                             bool exact,
                             bool forward,
                             SSATmp*& calleeCtx,
                             const Class* ctx) {
  auto const func = lookupImmutableClsMethod(
    baseClass, methodName, ctx, exact);
  if (!func) return nullptr;

  auto funcTmp = exact || func->isImmutableFrom(baseClass)
    ? cns(env, func)
    : gen(env, LdClsMethod, callerCtx, cns(env, func->methodSlot()));

  calleeCtx = forward
    ? forwardCtx(env, func, funcTmp)
    : ldCtxForClsMethod(env, func, callerCtx, baseClass, exact);
  return funcTmp;
}

void resolveClsMethodCommon(IRGS& env, SSATmp* clsVal,
                            const StringData* methodName,
                            uint32_t numExtraInputs) {
  assertx(clsVal->isA(TCls));
  auto const cs = clsVal->type().clsSpec();
  if (!cs) return interpOne(env);
  auto const exactClass = cs.exact() || cs.cls()->attrs() & AttrNoOverride;
  SSATmp* ctx;
  auto const funcTmp = lookupClsMethodKnown(env, methodName, clsVal, cs.cls(),
                                            exactClass, false, ctx,
                                            curClass(env));
  if (!funcTmp) return interpOne(env);
  assertx(funcTmp->isA(TFunc));
  if (!funcTmp->hasConstVal()) return interpOne(env);
  if (!funcTmp->funcVal()->isStaticInPrologue()) {
    gen(env, ThrowMissingThis, funcTmp);
  }
  discard(env, numExtraInputs);
  push(env, gen(env, NewClsMeth, clsVal, funcTmp));
}

} // namespace

void emitResolveClsMethod(IRGS& env, const StringData* methodName) {
  auto const cls = topC(env);
  if (!cls->isA(TCls)) return interpOne(env);
  resolveClsMethodCommon(env, cls, methodName, 1);
}

void emitResolveClsMethodD(IRGS& env, const StringData* className,
                           const StringData* methodName) {
  auto const cls = lookupUniqueClass(env, className, false /* trustUnit */);
  if (cls) {
    auto const func = lookupImmutableClsMethod(cls, methodName, curClass(env),
                                               true);
    if (func) {
      if (!func->isStaticInPrologue()) {
        gen(env, ThrowMissingThis, cns(env, func));
      }
      if (!classIsPersistentOrCtxParent(env, cls)) {
        gen(env, LdClsCached, cns(env, className));
      }
      ldCtxForClsMethod(env, func, cns(env, cls), cls, true);

      // For clsmeth, we want to return the class user gave,
      // not the class where func is associated with.
      push(env, gen(env, NewClsMeth, cns(env, cls), cns(env, func)));
    } else {
      resolveClsMethodCommon(env, cns(env, cls), methodName, 0);
    }
    return;
  }

  auto const slowExit = makeExitSlow(env);
  auto const ne = NamedEntity::get(className);
  auto const data = ClsMethodData { className, methodName, ne, curClass(env) };
  auto const funcTmp = loadClsMethodUnknown(env, data, slowExit);
  auto const clsTmp = gen(env, LdClsCached, cns(env, className));
  push(env, gen(env, NewClsMeth, clsTmp, funcTmp));
}

void emitResolveClsMethodS(IRGS& env, SpecialClsRef ref,
                           const StringData* methodName) {
  auto const cls = specialClsRefToCls(env, ref);
  if (!cls) return interpOne(env);
  resolveClsMethodCommon(env, cls, methodName, 0);
}

namespace {

void fcallClsMethodCommon(IRGS& env,
                          const FCallArgs& fca,
                          const StringData* clsHint,
                          SSATmp* clsVal,
                          SSATmp* methVal,
                          bool forward,
                          bool dynamic,
                          bool allowLogAsDynCall,
                          uint32_t numExtraInputs) {
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

    auto const ctx = forward ? ldCtxCls(env) : clsVal;
    auto const mightCareAboutDynCall = allowLogAsDynCall &&
      RuntimeOption::EvalForbidDynamicCallsToClsMeth > 0;
    decRef(env, methVal);
    discard(env, numExtraInputs);
    if (noCallProfiling) {
      prepareAndCallUnknown(env, func, fca, ctx, dynamic, mightCareAboutDynCall,
                            true);
    } else {
      prepareAndCallProfiled(env, func, fca, ctx, dynamic,
                             mightCareAboutDynCall);
    }
  };

  if (!methVal->hasConstVal()) {
    emitFCall();
    return;
  }

  auto const methodName = methVal->strVal();
  auto const knownClass = [&] () -> std::pair<const Class*, bool> {
    if (!clsHint->empty()) {
      auto const cls = lookupUniqueClass(env, clsHint);
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
                                           forward, ctx, callContext(env, fca));
    if (func) {
      auto const mightCareAboutDynCall =
        RuntimeOption::EvalForbidDynamicCallsToInstMeth > 0
        || RuntimeOption::EvalForbidDynamicCallsToClsMeth > 0;
      discard(env, numExtraInputs);
      return prepareAndCallProfiled(env, func, fca, ctx, dynamic,
                                    mightCareAboutDynCall);
    }
  }

  // If the method has reified generics, we can't burn the value in the JIT
  if (!RuntimeOption::RepoAuthoritative || clsVal->hasConstVal() || forward ||
      fca.hasGenerics()) {
    emitFCall();
    return;
  }

  optimizeProfiledCallMethod(env, fca, clsVal, false, methodName, dynamic,
                             numExtraInputs, emitFCall);
}

} // namespace

void emitFCallClsMethod(IRGS& env, FCallArgs fca, const StringData* clsHint,
                        IsLogAsDynamicCallOp op) {
  auto const cls = topC(env);
  auto const methName = topC(env, BCSPRelOffset { 1 });
  if (!cls->isA(TCls) || !methName->isA(TStr)) {
    return interpOne(env);
  }

  auto const allowLogAsDynamicCall =
    op == IsLogAsDynamicCallOp::LogAsDynamicCall ||
    RuntimeOption::EvalLogKnownMethodsAsDynamicCalls;
  fcallClsMethodCommon(env, fca, clsHint, cls, methName, false, true,
                       allowLogAsDynamicCall, 2);
}

void emitFCallClsMethodS(IRGS& env, FCallArgs fca, const StringData* clsHint,
                         SpecialClsRef ref) {
  auto const cls = specialClsRefToCls(env, ref);
  auto const methName = topC(env);
  if (!cls || !methName->isA(TStr)) return interpOne(env);

  auto const fwd = ref == SpecialClsRef::Self || ref == SpecialClsRef::Parent;
  fcallClsMethodCommon(env, fca, clsHint, cls, methName, fwd, true, true, 1);
}

void emitFCallClsMethodSD(IRGS& env, FCallArgs fca, const StringData* clsHint,
                          SpecialClsRef ref, const StringData* methName) {
  auto const cls = specialClsRefToCls(env, ref);
  if (!cls) return interpOne(env);

  auto const fwd = ref == SpecialClsRef::Self || ref == SpecialClsRef::Parent;
  fcallClsMethodCommon(env, fca, clsHint, cls, cns(env, methName), fwd, false,
                       false, 0);
}

//////////////////////////////////////////////////////////////////////

void emitDirectCall(IRGS& env, Func* callee, uint32_t numParams,
                    SSATmp* const* const args) {
  allocActRec(env);
  for (int32_t i = 0; i < numParams; i++) {
    push(env, args[i]);
  }

  auto const fca = FCallArgs(FCallArgs::Flags::None, numParams, 1, nullptr,
                             kInvalidOffset, false, false, nullptr);
  prepareAndCallKnown(env, callee, fca, nullptr, false);
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
      if (!callee->isInOut(param_idx)) continue;
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
