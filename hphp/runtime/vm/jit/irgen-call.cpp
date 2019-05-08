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
#include "hphp/runtime/vm/jit/type.h"

#include "hphp/runtime/vm/jit/irgen-basic.h"
#include "hphp/runtime/vm/jit/irgen-builtin.h"
#include "hphp/runtime/vm/jit/irgen-control.h"
#include "hphp/runtime/vm/jit/irgen-create.h"
#include "hphp/runtime/vm/jit/irgen-exit.h"
#include "hphp/runtime/vm/jit/irgen-internal.h"
#include "hphp/runtime/vm/jit/irgen-types.h"

namespace HPHP { namespace jit { namespace irgen {

//////////////////////////////////////////////////////////////////////

namespace {

void emitCallerDynamicCallChecksKnown(IRGS& env, const Func* callee) {
  assertx(callee);
  if (RuntimeOption::EvalForbidDynamicCalls <= 0) return;
  if (callee->isDynamicallyCallable()) return;
  gen(env, RaiseForbiddenDynCall, cns(env, callee));
}

void emitCallerDynamicCallChecksUnknown(IRGS& env, SSATmp* callee) {
  assertx(!callee->hasConstVal());
  if (RuntimeOption::EvalForbidDynamicCalls <= 0) return;
  ifElse(
    env,
    [&] (Block* skip) {
      auto const dynCallable = gen(env, IsFuncDynCallable, callee);
      gen(env, JmpNZero, skip, dynCallable);
    },
    [&] {
      hint(env, Block::Hint::Unlikely);
      gen(env, RaiseForbiddenDynCall, callee);
    }
  );
}

IRSPRelOffset fsetActRec(
  IRGS& env,
  SSATmp* func,
  SSATmp* objOrClass,
  uint32_t numArgs,
  const StringData* invName,
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
    invName ? cns(env, invName) : cns(env, TNullptr),
    cns(env, dynamicCall),
    tsList ? tsList : cns(env, TNullptr)
  );

  return arOffset;
}

void prepareToCallKnown(IRGS& env, const Func* callee, SSATmp* objOrClass,
                        uint32_t numArgs, const StringData* invName,
                        bool dynamicCall, SSATmp* tsList) {
  assertx(callee);

  // Caller checks
  if (dynamicCall) emitCallerDynamicCallChecksKnown(env, callee);

  auto const func = cns(env, callee);
  fsetActRec(env, func, objOrClass, numArgs, invName, dynamicCall, tsList);
}

void prepareToCallUnknown(IRGS& env, SSATmp* callee, SSATmp* objOrClass,
                          uint32_t numArgs, const StringData* invName,
                          bool dynamicCall, SSATmp* tsList) {
  assertx(callee->isA(TFunc));
  if (callee->hasConstVal()) {
    return prepareToCallKnown(env, callee->funcVal(), objOrClass, numArgs,
                              invName, dynamicCall, tsList);
  }

  // Caller checks
  if (dynamicCall) emitCallerDynamicCallChecksUnknown(env, callee);

  fsetActRec(env, callee, objOrClass, numArgs, invName, dynamicCall, tsList);
}

template<class Fn>
void prepareToCallCustom(IRGS& env, SSATmp* objOrClass, uint32_t numArgs,
                         bool dynamicCall, SSATmp* tsList, Fn prepare) {
  auto const arOffset = fsetActRec(
    env, cns(env, TNullptr), objOrClass, numArgs, nullptr, dynamicCall, tsList);

  // This is special. We need to sync SP in case prepare() reenters. Otherwise
  // it would clobber the ActRec we just pushed.
  updateMarker(env);
  env.irb->exceptionStackBoundary();

  // Responsible for:
  // - performing caller checks
  // - popualting missing ActRec fields (m_func at minimum)
  prepare(arOffset);
}

//////////////////////////////////////////////////////////////////////

// Pushing for object method when we don't know the Func* statically.
void fpushObjMethodUnknown(
  IRGS& env,
  SSATmp* obj,
  const StringData* methodName,
  uint32_t numParams,
  SSATmp* ts
) {
  implIncStat(env, Stats::ObjMethod_cached);
  auto const prepare = [&] (IRSPRelOffset arOffset) {
    auto const objCls = gen(env, LdObjClass, obj);
    auto const lomData = LdObjMethodData { arOffset, methodName };
    gen(env, LdObjMethod, lomData, objCls, sp(env));
  };
  return prepareToCallCustom(env, obj, numParams, false, ts, prepare);
}

SSATmp* lookupObjMethodExactFunc(
  IRGS& env,
  SSATmp* obj,
  const Class* exactClass,
  const Func* func,
  SSATmp*& objOrCls
) {
  /*
   * lookupImmutableObjMethod will return Funcs from AttrUnique classes, but in
   * this case, we have an object, so there's no need to check that the class
   * exists.
   *
   * Static function: store base class into this slot instead of obj and decref
   * the obj that was pushed as the this pointer since the obj won't be owned by
   * the runtime and thus MethodCache::lookup won't decref it.
   *
   * Static closure body: we still need to pass the object instance for the
   * closure prologue to properly do its dispatch (and extract use vars). It
   * will decref it and set up the alternative class pointer before entering the
   * "real" cloned closure body.
   */
  objOrCls = obj;
  implIncStat(env, Stats::ObjMethod_known);
  if (func->isStaticInPrologue()) {
    gen(env, RaiseHasThisNeedStatic, cns(env, func));
    objOrCls = exactClass ? cns(env, exactClass) : gen(env, LdObjClass, obj);
    decRef(env, obj);
  }
  return cns(env, func);
}

SSATmp* lookupObjMethodInterfaceFunc(
  IRGS& env,
  SSATmp* obj,
  const Func* ifaceFunc,
  SSATmp*& objOrCls
) {
  auto const vtableSlot = ifaceFunc->cls()->preClass()->ifaceVtableSlot();

  implIncStat(env, Stats::ObjMethod_ifaceslot);
  auto cls = gen(env, LdObjClass, obj);
  auto func = gen(env, LdIfaceMethod,
                  IfaceMethodData{vtableSlot, ifaceFunc->methodSlot()},
                  cls);
  objOrCls = obj;
  if (ifaceFunc->attrs() & AttrStatic) {
    gen(env, RaiseHasThisNeedStatic, func);
    decRef(env, obj);
    objOrCls = cls;
  }
  return func;
}

SSATmp* lookupObjMethodNonExactFunc(IRGS& env, SSATmp* obj,
                                    const Func* func,
                                    SSATmp*& objOrCls) {
  implIncStat(env, Stats::ObjMethod_methodslot);
  auto const clsTmp = gen(env, LdObjClass, obj);
  auto funcTmp = gen(
    env,
    LdClsMethod,
    clsTmp,
    cns(env, -(func->methodSlot() + 1))
  );
  objOrCls = obj;
  if (func->isStaticInPrologue()) {
    gen(env, RaiseHasThisNeedStatic, funcTmp);
    decRef(env, obj);
    objOrCls = clsTmp;
  }
  return funcTmp;
}

SSATmp* lookupObjMethodWithBaseClass(
  IRGS& env,
  SSATmp* obj,
  const Class* baseClass,
  const StringData* methodName,
  bool exactClass,
  SSATmp*& objOrCls,
  bool& magicCall
) {
  if (!baseClass) return nullptr;
  auto const lookup = lookupImmutableObjMethod(
    baseClass, methodName, curFunc(env), exactClass);
  switch (lookup.type) {
    case ImmutableObjMethodLookup::Type::NotFound:
      return nullptr;
    case ImmutableObjMethodLookup::Type::MagicFunc:
      magicCall = true;
      // fallthru
    case ImmutableObjMethodLookup::Type::Func:
      return lookupObjMethodExactFunc(
        env, obj, exactClass ? baseClass : nullptr, lookup.func, objOrCls);
    case ImmutableObjMethodLookup::Type::Class:
      return lookupObjMethodNonExactFunc(env, obj, lookup.func, objOrCls);
    case ImmutableObjMethodLookup::Type::Interface:
      return lookupObjMethodInterfaceFunc(env, obj, lookup.func, objOrCls);
  }

  not_reached();
}

void fpushObjMethodWithBaseClass(
  IRGS& env,
  SSATmp* obj,
  const Class* baseClass,
  const StringData* methodName,
  uint32_t numParams,
  bool exactClass,
  SSATmp* ts
) {
  bool magicCall = false;
  SSATmp* objOrCls = nullptr;
  if (auto func = lookupObjMethodWithBaseClass(
        env, obj, baseClass, methodName, exactClass,
        objOrCls, magicCall)) {
    prepareToCallUnknown(env, func, objOrCls, numParams,
                         magicCall ? methodName : nullptr, false, ts);
  } else {
    fpushObjMethodUnknown(env, obj, methodName, numParams, ts);
  }
}

const StaticString methProfileKey{ "MethProfile-FPushObjMethod" };

inline SSATmp* ldCtxForClsMethod(IRGS& env,
                                 const Func* callee,
                                 SSATmp* callCtx,
                                 const Class* cls,
                                 bool exact) {

  assertx(callCtx->isA(TCls));

  auto gen_missing_this = [&] {
    if (needs_missing_this_check(callee)) {
      gen(env, RaiseMissingThis, cns(env, callee));
    }
    return callCtx;
  };

  if (callee->isStatic()) return callCtx;
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
void optimizeProfiledPushMethod(IRGS& env,
                                SSATmp* objOrCls,
                                const Class* knownClass,
                                const StringData* methodName,
                                uint32_t numParams,
                                bool dynamic,
                                Fn emitFPush) {
  always_assert(objOrCls->type().subtypeOfAny(TObj, TCls));
  auto const isStaticCall = objOrCls->type() <= TCls;
  auto profile = TargetProfile<MethProfile>(env.context, env.irb->curMarker(),
                                            methProfileKey.get());
  if (!profile.optimizing()) {
    emitFPush();
    if (profile.profiling()) {
      auto const arOffset =
        offsetFromIRSP(env, BCSPRelOffset{static_cast<int32_t>(numParams)});
      gen(
        env,
        ProfileMethod,
        ProfileCallTargetData { arOffset, profile.handle() },
        sp(env),
        isStaticCall ? objOrCls : cns(env, TNullptr)
      );
    }
    return;
  }

  auto getCtx = [&](const Func* callee,
                    SSATmp* ctx,
                    const Class* cls) -> SSATmp* {
    if (isStaticCall) {
      return ldCtxForClsMethod(env, callee, ctx,
                               cls ? cls : callee->cls(), cls != nullptr);
    }
    if (!callee->isStatic()) return ctx;
    assertx(ctx->type() <= TObj);
    auto ret = cls ? cns(env, cls) : gen(env, LdObjClass, ctx);
    decRef(env, ctx);
    return ret;
  };

  auto const fallback = [&] {
    hint(env, Block::Hint::Unlikely);
    IRUnit::Hinter h(env.irb->unit(), Block::Hint::Unlikely);

    emitFPush();
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
          prepareToCallKnown(env, uniqueMeth, ctx, numParams,
                             isMagic ? methodName : nullptr,
                             dynamic, nullptr);
        },
        fallback
      );
      return;
    }

    if (isMagic) {
      emitFPush();
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
        prepareToCallKnown(env, uniqueMeth, ctx, numParams, nullptr, dynamic,
                           nullptr);
      },
      fallback
    );
    return;
  }

  // If we know anything about the class, other than it's an interface, the
  // remaining cases aren't worth the extra check.
  if (knownClass != nullptr && !isInterface(knownClass)) {
    emitFPush();
    return;
  }

  if (auto const baseMeth = data.baseMeth()) {
    if (!baseMeth->name()->isame(methodName)) {
      emitFPush();
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
        prepareToCallUnknown(env, meth, ctx, numParams, nullptr, dynamic,
                             nullptr);
      },
      fallback
    );
    return;
  }

  // If we know anything about the class, the other cases below are not worth
  // the extra checks they insert.
  if (knownClass != nullptr) {
    emitFPush();
    return;
  }

  if (auto const intfMeth = data.interfaceMeth()) {
    if (!intfMeth->name()->isame(methodName)) {
      emitFPush();
      return;
    }

    // The method was defined in a common interface, so check for that and use
    // LdIfaceMethod.
    ifThen(
      env,
      [&] (Block* sideExit) {
        auto const cls = isStaticCall
          ? objOrCls : gen(env, LdObjClass, objOrCls);
        auto const cData = ClassData{intfMeth->cls()};
        auto const flag = gen(env, InstanceOfIfaceVtable, cData, cls);
        gen(env, JmpZero, sideExit, flag);
        auto const vtableSlot = intfMeth->cls()->preClass()->ifaceVtableSlot();
        auto const imData = IfaceMethodData{vtableSlot, intfMeth->methodSlot()};
        auto const meth = gen(env, LdIfaceMethod, imData, cls);
        auto const ctx = getCtx(intfMeth, objOrCls, nullptr);
        prepareToCallUnknown(env, meth, ctx, numParams, nullptr, dynamic,
                             nullptr);
      },
      fallback
    );
    return;
  }

  emitFPush();
}

void fpushObjMethod(IRGS& env,
                    SSATmp* obj,
                    const StringData* methodName,
                    uint32_t numParams,
                    SSATmp* tsList) {
  implIncStat(env, Stats::ObjMethod_total);

  assertx(obj->type() <= TObj);
  const Class* knownClass = nullptr;
  bool exactClass = false;

  if (auto cls = obj->type().clsSpec().cls()) {
    if (!env.irb->constrainValue(obj, GuardConstraint(cls).setWeak())) {
      // We know the class without having to specialize a guard any further.  We
      // may still want to use MethProfile to gather more information in case
      // the class isn't known exactly.
      knownClass = cls;
      exactClass = obj->type().clsSpec().exact() ||
                   cls->attrs() & AttrNoOverride;
    }
  }

  auto const emitFPush = [&] {
    fpushObjMethodWithBaseClass(env, obj, knownClass, methodName, numParams,
                                exactClass, tsList);
  };

  // If we know the class exactly without profiling, then we don't need PGO.
  // If the method has reified generics, we can't burn the value in the JIT
  if (!RuntimeOption::RepoAuthoritative ||
      (knownClass && !isInterface(knownClass)) ||
      tsList) {
    emitFPush();
    return;
  }

  // If we don't know anything about the object's class, or all we know is an
  // interface that it implements, then enable PGO.
  optimizeProfiledPushMethod(env, obj, knownClass, methodName, numParams, false,
                             emitFPush);
}

void fpushFuncObj(IRGS& env, uint32_t numParams) {
  auto const slowExit = makeExitSlow(env);
  auto const obj      = popC(env);
  auto const cls      = gen(env, LdObjClass, obj);
  auto const func     = gen(env, LdObjInvoke, slowExit, cls);
  prepareToCallUnknown(env, func, obj, numParams, nullptr, false, nullptr);
}

void fpushFuncArr(IRGS& env, uint32_t numParams) {
  auto const arr = popC(env);
  auto const prepare = [&] (IRSPRelOffset arOffset) {
    gen(env, LdArrFuncCtx, IRSPRelOffsetData { arOffset }, arr,
        sp(env), fp(env));
    decRef(env, arr);
  };
  prepareToCallCustom(env, nullptr, numParams, true, nullptr, prepare);
}

void fpushFuncClsMeth(IRGS& env, uint32_t numParams) {
  auto const clsMeth = popC(env);
  auto const cls = gen(env, LdClsFromClsMeth, clsMeth);
  auto const func = gen(env, LdFuncFromClsMeth, clsMeth);
  prepareToCallUnknown(env, func, cls, numParams, nullptr, false, nullptr);
}

SSATmp* forwardCtx(IRGS& env, SSATmp* ctx, SSATmp* funcTmp) {
  assertx(ctx->type() <= TCtx);
  assertx(funcTmp->type() <= TFunc);

  auto forwardDynamicCallee = [&] {
    if (!hasThis(env)) {
      gen(env, RaiseMissingThis, funcTmp);
      return ctx;
    }

    auto const obj = castCtxThis(env, ctx);
    gen(env, IncRef, obj);
    return obj;
  };

  if (funcTmp->hasConstVal()) {
    assertx(!funcTmp->funcVal()->isClosureBody());
    if (funcTmp->funcVal()->isStatic()) {
      return gen(env, FwdCtxStaticCall, ctx);
    } else {
      return forwardDynamicCallee();
    }
  }

  return cond(env,
              [&](Block* target) {
                gen(env, CheckFuncStatic, target, funcTmp);
              },
              forwardDynamicCallee,
              [&] {
                return gen(env, FwdCtxStaticCall, ctx);
              });
}

} // namespace

void emitFPushFuncD(IRGS& env, uint32_t numParams, const StringData* name) {
  auto const lookup = lookupImmutableFunc(curUnit(env), name);
  if (lookup.func) {
    // We know the function, but we have to ensure its unit is loaded. Use
    // LdFuncCached, ignoring the result to ensure this.
    if (lookup.needsUnitLoad) gen(env, LdFuncCached, FuncNameData { name });
    prepareToCallKnown(env, lookup.func, nullptr, numParams, nullptr, false,
                       nullptr);
    return;
  }

  auto const func = gen(env, LdFuncCached, FuncNameData { name });
  prepareToCallUnknown(env, func, nullptr, numParams, nullptr, false, nullptr);
}

namespace {

//////////////////////////////////////////////////////////////////////

SSATmp* specialClsRefToCls(IRGS& env, SpecialClsRef ref) {
  switch (ref) {
    case SpecialClsRef::Static:
      if (!curClass(env)) PUNT(SpecialClsRef-NoCls);
      return gen(env, LdClsCtx, ldCtx(env));
    case SpecialClsRef::Self:
      if (auto const clss = curClass(env)) return cns(env, clss);
      PUNT(SpecialClsRef-NoCls);
      break;
    case SpecialClsRef::Parent:
      if (auto const clss = curClass(env)) {
        if (auto const parent = clss->parent()) return cns(env, parent);
      }
      PUNT(SpecialClsRef-NoCls);
      break;
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
  if (RuntimeOption::EvalForbidDynamicCalls <= 0) return;
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

void emitNewObj(IRGS& env, uint32_t slot, HasGenericsOp op) {
  /*
   * NoGenerics:    Do not read the generic part of clsref and emit AllocObj
   * HasGenerics:   Read the full clsref and emit AllocObjReified
   * MaybeGenerics: Read the full clsref, if generic part is not nullptr,
   *                emit AllocObjReified, otherwise use AllocObj
   */
  if (HasGenericsOp::NoGenerics == op) {
    auto const cls  = takeClsRefCls(env, slot);
    emitDynamicConstructChecks(env, cls);
    push(env, gen(env, AllocObj, cls));
    return;
  }

  auto const clsref = takeClsRef(env, slot, HasGenericsOp::HasGenerics == op);
  auto const reified_generic = clsref.first;
  auto const cls  = clsref.second;
  emitDynamicConstructChecks(env, cls);
  if (HasGenericsOp::HasGenerics == op) {
    push(env, gen(env, AllocObjReified, cls, reified_generic));
    return;
  }
  assertx(HasGenericsOp::MaybeGenerics == op);
  push(env, cond(
    env,
    [&] (Block* taken) {
      return gen(env, CheckNonNull, taken, reified_generic);
    },
    [&] (SSATmp* generics) { return gen(env, AllocObjReified, cls, generics); },
    [&] { return gen(env, AllocObj, cls); }
  ));
}

void emitNewObjD(IRGS& env, const StringData* className) {
  auto const cls = Unit::lookupUniqueClassInContext(className, curClass(env));
  bool const persistentCls = classIsPersistentOrCtxParent(env, cls);
  bool const canInstantiate = cls && isNormalClass(cls) && !isAbstract(cls);
  if (persistentCls && canInstantiate && !cls->hasNativePropHandler() &&
      !cls->hasReifiedGenerics() && !cls->hasReifiedParent()) {
    push(env, allocObjFast(env, cls));
    return;
  }

  if (persistentCls) {
    push(env, gen(env, AllocObj, cns(env, cls)));
    return;
  }

  auto const cachedCls = gen(env, LdClsCached, cns(env, className));
  push(env, gen(env, AllocObj, cls ? cns(env, cls) : cachedCls));
}

void emitNewObjS(IRGS& env, SpecialClsRef ref) {
  auto const cls  = specialClsRefToCls(env, ref);
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

void emitFPushCtor(IRGS& env, uint32_t numParams) {
  auto const objPos = static_cast<int32_t>(numParams + 2);
  auto const obj = topC(env, BCSPRelOffset{objPos});
  if (!obj->isA(TObj)) PUNT(FPushCtor-NonObj);

  auto const exactCls = obj->type().clsSpec().exactCls();
  if (exactCls) {
    if (auto const ctor = lookupImmutableCtor(exactCls, curClass(env))) {
      prepareToCallKnown(env, ctor, obj, numParams, nullptr, false, nullptr);
      return;
    }
  }

  auto const cls = exactCls ? cns(env, exactCls) : gen(env, LdObjClass, obj);
  auto const func = gen(env, LdClsCtor, cls, fp(env));
  prepareToCallUnknown(env, func, obj, numParams, nullptr, false, nullptr);
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
    prepareToCallUnknown(env, callee, nullptr, numParams, nullptr, false,
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
    gen(env, LdFunc, IRSPRelOffsetData { arOffset }, callee, sp(env), fp(env));
    decRef(env, callee);
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

void implFPushObjMethodD(IRGS& env,
                         uint32_t numParams,
                         const StringData* methodName,
                         ObjMethodOp subop,
                         SSATmp* tsList) {
  auto const objPos = static_cast<int32_t>(numParams + 2);
  auto const obj = topC(env, BCSPRelOffset{objPos});

  if (obj->type() <= TObj) {
    fpushObjMethod(env, obj, methodName, numParams, tsList);
    return;
  }

  if (obj->type() <= TInitNull && subop == ObjMethodOp::NullSafe) {
    prepareToCallKnown(env, SystemLib::s_nullFunc, nullptr, numParams, nullptr,
                       false, tsList);
    return;
  }

  PUNT(FPushObjMethodD-nonObj);
}

} // namespace

void emitFPushObjMethodD(IRGS& env,
                         uint32_t numParams,
                         const StringData* methodName,
                         ObjMethodOp subop) {
  implFPushObjMethodD(env, numParams, methodName, subop, nullptr);
}


void emitFPushObjMethodRD(IRGS& env,
                         uint32_t numParams,
                         const StringData* methodName,
                         ObjMethodOp subop) {
  implFPushObjMethodD(env, numParams, methodName, subop, popC(env));
}

namespace {

SSATmp* lookupClsMethodKnown(IRGS& env,
                             const StringData* methodName,
                             SSATmp* callerCtx,
                             const Class *baseClass,
                             bool exact,
                             bool check,
                             bool forward,
                             SSATmp*& calleeCtx) {
  auto const func = lookupImmutableClsMethod(
    baseClass, methodName, curFunc(env), exact);
  if (!func) return nullptr;

  auto const objOrCls = forward ?
                        ldCtx(env) :
                        ldCtxForClsMethod(env, func, callerCtx, baseClass,
                                          exact);
  if (check) {
    assertx(exact);
    if (!classIsPersistentOrCtxParent(env, baseClass)) {
      gen(env, LdClsCached, cns(env, baseClass->name()));
    }
  }
  auto funcTmp = exact || func->isImmutableFrom(baseClass) ?
    cns(env, func) :
    gen(env, LdClsMethod, callerCtx, cns(env, -(func->methodSlot() + 1)));

  calleeCtx = forward ?
              forwardCtx(env, objOrCls, funcTmp) :
              objOrCls;
  return funcTmp;
}

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

bool fpushClsMethodKnown(IRGS& env,
                         uint32_t numParams,
                         const StringData* methodName,
                         SSATmp* ctxTmp,
                         const Class *baseClass,
                         bool exact,
                         bool check,
                         bool forward,
                         bool dynamic,
                         SSATmp* tsList) {
  SSATmp* ctx = nullptr;
  auto const func = lookupClsMethodKnown(env, methodName, ctxTmp, baseClass,
                                         exact, check, forward, ctx);
  if (!func) return false;
  prepareToCallUnknown(env, func, ctx, numParams, nullptr, dynamic, tsList);
  return true;
}

void implFPushClsMethodD(IRGS& env,
                         uint32_t numParams,
                         const StringData* methodName,
                         const StringData* className,
                         SSATmp* tsList) {

  if (auto const baseClass =
      Unit::lookupUniqueClassInContext(className, curClass(env))) {
    if (fpushClsMethodKnown(env, numParams,
                            methodName, cns(env, baseClass), baseClass,
                            true, true, false, false, tsList)) {
      return;
    }
  }

  if (tsList) push(env, tsList);
  env.irb->exceptionStackBoundary();
  auto const slowExit = makeExitSlow(env);
  if (tsList) popC(env);
  auto const ne = NamedEntity::get(className);
  auto const data = ClsMethodData { className, methodName, ne };
  auto const func = loadClsMethodUnknown(env, data, slowExit);
  auto const clsCtx = gen(env, LdClsMethodCacheCls, data);
  prepareToCallUnknown(env, func, clsCtx, numParams, nullptr, false, tsList);
}

void emitFPushClsMethodD(IRGS& env,
                         uint32_t numParams,
                         const StringData* methodName,
                         const StringData* className) {
  implFPushClsMethodD(env, numParams, methodName, className, nullptr);
}

void emitFPushClsMethodRD(IRGS& env,
                         uint32_t numParams,
                         const StringData* methodName,
                         const StringData* className) {
  auto const tsList = popC(env);
  implFPushClsMethodD(env, numParams, methodName, className, tsList);
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
  bool magicCall = false;
  SSATmp* objOrCls = nullptr;
  if (auto funcTmp = lookupObjMethodWithBaseClass(env, obj, cls, methodName,
        exactClass, objOrCls, magicCall)) {
    if (magicCall) {
      gen(env, ThrowInvalidOperation, cns(env, s_resolveMagicCall.get()));
      return;
    }
    auto methPair = gen(env, AllocVArray, PackedArrayData { 2 });
    gen(env, InitPackedLayoutArray, IndexData { 0 }, methPair, objOrCls);
    gen(env, InitPackedLayoutArray, IndexData { 1 }, methPair, funcTmp);
    decRef(env, name);
    popC(env);
    popC(env);
    push(env, methPair);
    return;
  }
  PUNT(ResolveObjMethod-unkownObjMethod);
}


const StaticString s_resolveClsMagicCall(
  "Unable to resolve magic call for class_meth()");

void emitResolveClsMethod(IRGS& env) {
  auto const name = topC(env, BCSPRelOffset { 0 });
  auto const cls = topC(env, BCSPRelOffset { 1 });
  if (!(cls->type() <= TStr) || !(name->type() <= TStr)) {
    PUNT(ResolveClsMethod-nonStr);
  }
  if (!name->hasConstVal() || !cls->hasConstVal()) {
    PUNT(ResolveClsMethod-nonConstStr);
  }
  auto className = cls->strVal();
  auto methodName = name->strVal();
  SSATmp* clsTmp = nullptr;
  SSATmp* funcTmp = nullptr;
  if (auto const baseClass =
      Unit::lookupUniqueClassInContext(className, curClass(env))) {
    SSATmp* ctx = nullptr;
    funcTmp = lookupClsMethodKnown(env, methodName, cns(env, baseClass),
                                    baseClass, true, true, false, ctx);
    // For clsmeth, we want to return the class user gave,
    // not the class where func is associated with.
    clsTmp = cns(env, baseClass);
  }
  if (!funcTmp) {
    auto const slowExit = makeExitSlow(env);
    auto const ne = NamedEntity::get(className);
    auto const data = ClsMethodData { className, methodName, ne };
    funcTmp = loadClsMethodUnknown(env, data, slowExit);
    clsTmp = gen(env, LdClsCached, cns(env, className));
  }
  assertx(clsTmp);
  assertx(funcTmp);
  auto const clsMeth = gen(env, NewClsMeth, clsTmp, funcTmp);
  decRef(env, name);
  decRef(env, cls);
  popC(env);
  popC(env);
  push(env, clsMeth);
}

namespace {

template <typename Take, typename Get>
ALWAYS_INLINE void fpushClsMethodCommon(IRGS& env,
                                        uint32_t numParams,
                                        Take takeCls,
                                        Get getMeth,
                                        bool forward,
                                        bool dynamic,
                                        SSATmp* tsList) {
  auto const clsVal = takeCls();
  auto const methVal = getMeth();

  if (!methVal->isA(TStr)) {
    PUNT(FPushClsMethod-unknownType);
  }

  auto const emitFPush = [&] {
    auto const prepare = [&] (IRSPRelOffset arOffset) {
      auto const lcmData = LookupClsMethodData { arOffset, forward, dynamic };
      gen(env, LookupClsMethod, lcmData, clsVal, methVal, sp(env), fp(env));
      decRef(env, methVal);
    };

    prepareToCallCustom(env, nullptr, numParams, dynamic, tsList, prepare);
  };

  if (!methVal->hasConstVal()) {
    emitFPush();
    return;
  }

  auto const methodName = methVal->strVal();
  const Class* cls = nullptr;
  bool exact = false;
  if (auto clsSpec = clsVal->type().clsSpec()) {
    cls = clsSpec.cls();
    exact = clsSpec.exact();
  }

  if (cls && fpushClsMethodKnown(env, numParams, methodName, clsVal, cls,
                                 exact, false, forward, dynamic, tsList)) {
    return;
  }

  // If the method has reified generics, we can't burn the value in the JIT
  if (!RuntimeOption::RepoAuthoritative || clsVal->hasConstVal() || forward ||
      tsList) {
    emitFPush();
    return;
  }

  optimizeProfiledPushMethod(env, clsVal, nullptr, methodName, numParams,
                             dynamic, emitFPush);
}

}

void emitFPushClsMethod(IRGS& env,
                        uint32_t numParams,
                        uint32_t slot,
                        const ImmVector& v) {
  if (v.size() != 0) PUNT(InOut-FPushClsMethod);
  fpushClsMethodCommon(
    env,
    numParams,
    [&] { return takeClsRefCls(env, slot); },
    [&] { return popC(env); },
    false,
    true,
    nullptr
  );
}

void emitFPushClsMethodS(IRGS& env,
                         uint32_t numParams,
                         SpecialClsRef ref,
                         const ImmVector& v) {
  if (v.size() != 0) PUNT(InOut-FPushClsMethodS);
  fpushClsMethodCommon(
    env,
    numParams,
    [&] { return specialClsRefToCls(env, ref); },
    [&] { return popC(env); },
    ref == SpecialClsRef::Self || ref == SpecialClsRef::Parent,
    true,
    nullptr
  );
}

void emitFPushClsMethodSD(IRGS& env,
                          uint32_t numParams,
                          SpecialClsRef ref,
                          const StringData* name) {
  fpushClsMethodCommon(
    env,
    numParams,
    [&] { return specialClsRefToCls(env, ref); },
    [&] { return cns(env, name); },
    ref == SpecialClsRef::Self || ref == SpecialClsRef::Parent,
    false,
    nullptr
  );
}

void emitFPushClsMethodSRD(IRGS& env,
                           uint32_t numParams,
                           SpecialClsRef ref,
                           const StringData* name) {
  auto const tsList = popC(env);
  fpushClsMethodCommon(
    env,
    numParams,
    [&] { return specialClsRefToCls(env, ref); },
    [&] { return cns(env, name); },
    ref == SpecialClsRef::Self || ref == SpecialClsRef::Parent,
    false,
    tsList
  );
}

//////////////////////////////////////////////////////////////////////

namespace {

SSATmp* ldPreLiveFunc(IRGS& env, IRSPRelOffset actRecOff) {
  auto const& fpiStack = env.irb->fs().fpiStack();
  if (!fpiStack.empty() && fpiStack.back().func) {
    return cns(env, fpiStack.back().func);
  }

  return gen(env, LdARFuncPtr, TFunc, IRSPRelOffsetData { actRecOff }, sp(env));
}

}

//////////////////////////////////////////////////////////////////////

namespace {

bool emitCallerReffinessChecks(IRGS& env, const Func* callee,
                               const FCallArgs& fca, IRSPRelOffset actRecOff) {
  if (!fca.enforceReffiness()) return true;

  if (callee) {
    for (auto i = 0; i < fca.numArgs; ++i) {
      if (callee->byRef(i) != fca.byRef(i)) {
        auto const func = cns(env, callee);
        gen(env, RaiseParamRefMismatchForFunc, ParamData { i }, func);
        return false;
      }
    }
    return true;
  }

  auto const func = ldPreLiveFunc(env, actRecOff);
  auto const exitSlow = makeExitSlow(env);

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
      numParams = gen(env, LdFuncNumParams, func);
    }

    gen(env, CheckRefs, exitSlow, CheckRefsData { i * 8, mask, vals }, func,
        numParams);
  }

  return true;
}

}

void emitCallerRxChecks(IRGS& env, const Func* callee,
                        IRSPRelOffset actRecOff) {
  if (RuntimeOption::EvalRxEnforceCalls <= 0) return;
  auto const callerLevel = curRxLevel(env);
  if (!rxEnforceCallsInLevel(callerLevel)) return;

  auto const minReqCalleeLevel = rxRequiredCalleeLevel(callerLevel);
  if (callee) {
    if (callee->rxLevel() >= minReqCalleeLevel) return;
    gen(env, RaiseRxCallViolation, fp(env), cns(env, callee));
    return;
  }

  auto const func = ldPreLiveFunc(env, actRecOff);
  ifThen(
    env,
    [&] (Block* taken) {
      auto const calleeLevel = gen(env, LdFuncRxLevel, func);
      auto const lt = gen(env, LtInt, calleeLevel, cns(env, minReqCalleeLevel));
      gen(env, JmpNZero, taken, lt);
    },
    [&] {
      hint(env, Block::Hint::Unlikely);
      gen(env, RaiseRxCallViolation, fp(env), func);
    }
  );
}

//////////////////////////////////////////////////////////////////////

void emitFCall(IRGS& env,
               FCallArgs fca,
               const StringData*,
               const StringData*) {
  auto const callee = env.currentNormalizedInstruction->funcd;
  auto const numStackInputs = fca.numArgs + (fca.hasUnpack() ? 1 : 0);
  auto const actRecOff = spOffBCFromIRSP(env) + numStackInputs;

  if (!emitCallerReffinessChecks(env, callee, fca, actRecOff)) return;
  emitCallerRxChecks(env, callee, actRecOff);

  if (fca.hasUnpack()) {
    auto const data = CallUnpackData {
      spOffBCFromIRSP(env),
      fca.numArgs + 1,
      fca.numRets - 1,
      bcOff(env),
      callee,
    };
    push(env, gen(env, CallUnpack, data, sp(env), fp(env)));
    return;
  }

  auto const call = [&](bool asyncEagerReturn) {
    return gen(
      env,
      Call,
      CallData {
        spOffBCFromIRSP(env),
        fca.numArgs,
        fca.numRets - 1,
        bcOff(env) - curFunc(env)->base(),
        callee,
        asyncEagerReturn,
      },
      sp(env),
      fp(env)
    );
  };

  auto const emitCallWithoutAsyncEagerReturn = [&] {
    push(env, call(false));
  };
  auto const emitCallWithAsyncEagerReturn = [&] {
    auto const retVal = call(true);

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
        jmpImpl(env, bcOff(env) + fca.asyncEagerOffset);
      },
      [&] {
        hint(env, Block::Hint::Unlikely);
        auto const ty = callee ? callReturnType(callee) : TInitCell;
        push(env, gen(env, AssertType, ty, retVal));
      }
    );
  };

  if (fca.asyncEagerOffset == kInvalidOffset ||
      (callee && !callee->supportsAsyncEagerReturn())) {
    return emitCallWithoutAsyncEagerReturn();
  }

  if (fca.supportsAsyncEagerReturn() ||
      (callee && callee->supportsAsyncEagerReturn())) {
    assertx(!callee || callee->supportsAsyncEagerReturn());
    return emitCallWithAsyncEagerReturn();
  }

  ifThenElse(
    env,
    [&] (Block* taken) {
      auto const supportsAsyncEagerReturn =
        gen(env, FuncSupportsAsyncEagerReturn, ldPreLiveFunc(env, actRecOff));
      gen(env, JmpNZero, taken, supportsAsyncEagerReturn);
    },
    [&] {
      hint(env, Block::Hint::Unlikely);
      emitCallWithoutAsyncEagerReturn();
    },
    [&] {
      emitCallWithAsyncEagerReturn();
    }
  );
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

  // Otherwise use HHBBC's analysis if present
  return typeFromRAT(callee->repoReturnType(), callee->cls());
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

const Func* profiledCalledFunc(IRGS& env, uint32_t numArgs,
                               IRInstruction*& checkInst) {
  checkInst = nullptr;

  if (!RuntimeOption::RepoAuthoritative) return nullptr;

  auto profile = TargetProfile<CallTargetProfile>(env.unit.context(),
                                                  env.irb->curMarker(),
                                                  callTargetProfileKey());

  // NB: the profiling used here is shared and done in getCallTarget() in
  // irlower-call.cpp, so we only handle the optimization phase here.
  if (!profile.optimizing()) return nullptr;

  auto const data = profile.data();
  double probability = 0;
  auto profiledFunc = data.choose(probability);

  if (profiledFunc == nullptr) return nullptr;

  // Don't emit the check if the probability of it succeeding is below the
  // threshold.
  if (probability * 100 < RuntimeOption::EvalJitPGOCalledFuncThreshold) {
    return nullptr;
  }

  auto const spOff = spOffBCFromIRSP(env);
  auto const calleeAROff = spOff + numArgs;
  auto liveFuncTmp = gen(env, LdARFuncPtr, TFunc,
                         IRSPRelOffsetData{calleeAROff}, sp(env));

  auto profiledFuncTmp = cns(env, profiledFunc);
  auto const equal = gen(env, EqFunc, liveFuncTmp, profiledFuncTmp);

  auto sideExit = makeExit(env, bcOff(env));
  gen(env, JmpZero, sideExit, equal);

  auto curBlock = env.irb->curBlock();
  checkInst = &(curBlock->back());
  always_assert_flog(checkInst->op() == JmpZero, "checkInst = {}\n",
                     *checkInst);
  gen(env, AssertARFunc, IRSPRelOffsetData{calleeAROff}, sp(env),
      profiledFuncTmp);

  return profiledFunc;
}

void dropCalledFuncCheck(IRGS& env, IRInstruction* checkInst) {
  always_assert_flog(checkInst && checkInst->op() == JmpZero,
                     "checkInst: {}\n", *checkInst);

  // Turn the AssertARFunc following checkInst into a Nop.
  auto assertInst = checkInst->next()->skipHeader();
  assertx(assertInst->op() == AssertARFunc);
  assertInst->convertToNop();

  // Make both directions of the JmpZero jump to it's fall-through block.
  // Later, optimizations will eliminate this instruction, and also the EqFunc
  // and LdARFuncPtr above it.
  checkInst->setTaken(checkInst->next());

  // Clear the information about the top function in FrameState.
  env.irb->fs().clearTopFunc();;
}

//////////////////////////////////////////////////////////////////////

}}}
