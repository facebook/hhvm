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
#include "hphp/runtime/vm/runtime.h"

#include "hphp/runtime/vm/jit/call-target-profile.h"
#include "hphp/runtime/vm/jit/guard-constraint.h"
#include "hphp/runtime/vm/jit/meth-profile.h"
#include "hphp/runtime/vm/jit/normalized-instruction.h"
#include "hphp/runtime/vm/jit/target-profile.h"
#include "hphp/runtime/vm/jit/type.h"

#include "hphp/runtime/vm/jit/irgen-basic.h"
#include "hphp/runtime/vm/jit/irgen-builtin.h"
#include "hphp/runtime/vm/jit/irgen-create.h"
#include "hphp/runtime/vm/jit/irgen-exit.h"
#include "hphp/runtime/vm/jit/irgen-internal.h"
#include "hphp/runtime/vm/jit/irgen-types.h"

namespace HPHP { namespace jit { namespace irgen {

namespace {

//////////////////////////////////////////////////////////////////////

const StaticString s_self("self");
const StaticString s_parent("parent");
const StaticString s_static("static");

//////////////////////////////////////////////////////////////////////

bool canInstantiateClass(const Class* cls) {
  return cls && isNormalClass(cls) && !isAbstract(cls);
}

//////////////////////////////////////////////////////////////////////

// Pushing for object method when we don't know the Func* statically.
void fpushObjMethodUnknown(IRGS& env,
                           SSATmp* obj,
                           const StringData* methodName,
                           uint32_t numParams,
                           bool shouldFatal) {
  implIncStat(env, Stats::ObjMethod_cached);
  fpushActRec(env,
              cns(env, TNullptr),  // Will be set by LdObjMethod
              obj,
              numParams,
              nullptr,
              cns(env, false));
  auto const objCls = gen(env, LdObjClass, obj);

  // This is special.  We need to move the stackpointer in case LdObjMethod
  // calls a destructor.  Otherwise it would clobber the ActRec we just pushed.
  updateMarker(env);
  env.irb->exceptionStackBoundary();

  gen(env,
      LdObjMethod,
      LdObjMethodData {
        spOffBCFromIRSP(env), methodName, shouldFatal
      },
      objCls,
      sp(env));
}

/*
 * Looks for a Func named methodName in iface, or any of the interfaces it
 * implements. returns nullptr if none was found, or if its interface's
 * vtableSlot is kInvalidSlot.
 */
const Func* findInterfaceMethod(const Class* iface,
                                const StringData* methodName) {

  auto checkOneInterface = [methodName](const Class* i) -> const Func* {
    if (i->preClass()->ifaceVtableSlot() == kInvalidSlot) return nullptr;

    const Func* func = i->lookupMethod(methodName);
    always_assert(!func || func->cls() == i);
    return func;
  };

  if (auto const func = checkOneInterface(iface)) return func;

  for (auto pface : iface->allInterfaces().range()) {
    if (auto const func = checkOneInterface(pface)) {
      return func;
    }
  }

  return nullptr;
}

SSATmp* lookupObjMethodExactFunc(
  IRGS& env,
  SSATmp* obj,
  const Class* exactClass,
  const Func* func,
  const StringData* methodName,
  SSATmp*& objOrCls
) {
  /*
   * lookupImmutableMethod will return Funcs from AttrUnique classes, but in
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
    objOrCls = exactClass ? cns(env, exactClass) : gen(env, LdObjClass, obj);
    decRef(env, obj);
  }
  return cns(env, func);
}

const Func*
lookupInterfaceFuncForFPushObjMethod(IRGS& /*env*/, const Class* baseClass,
                                     const StringData* methodName) {
  if (!baseClass) return nullptr;
  if (!classIsUniqueInterface(baseClass)) return nullptr;

  return findInterfaceMethod(baseClass, methodName);
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
    decRef(env, obj);
    objOrCls = cls;
  }
  return func;
}

SSATmp* lookupObjMethodNonExactFunc(IRGS& env, SSATmp* obj,
                                    const Class* /*baseClass*/,
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
  magicCall = false;
  if (auto const func = lookupImmutableMethod(
        baseClass, methodName, magicCall,
        /* staticLookup: */ false, curFunc(env), exactClass)) {
    if (exactClass ||
        func->attrs() & AttrPrivate ||
        func->isImmutableFrom(baseClass)) {
      return lookupObjMethodExactFunc(env, obj,
                                      exactClass ? baseClass : nullptr,
                                      func,
                                      magicCall ? methodName : nullptr,
                                      objOrCls);
    }
    return lookupObjMethodNonExactFunc(env, obj, baseClass, func, objOrCls);
  }

  if (auto const func =
      lookupInterfaceFuncForFPushObjMethod(env, baseClass, methodName)) {
    return lookupObjMethodInterfaceFunc(env, obj, func, objOrCls);
  }

  return nullptr;
}

void fpushObjMethodWithBaseClass(
  IRGS& env,
  SSATmp* obj,
  const Class* baseClass,
  const StringData* methodName,
  uint32_t numParams,
  bool shouldFatal,
  bool exactClass
) {
  bool magicCall = false;
  SSATmp* objOrCls = nullptr;
  if (auto func = lookupObjMethodWithBaseClass(
        env, obj, baseClass, methodName, exactClass,
        objOrCls, magicCall)) {
    fpushActRec(env, func, objOrCls, numParams,
                magicCall ? methodName : nullptr, cns(env, false));
    return;
  }

  fpushObjMethodUnknown(env, obj, methodName, numParams, shouldFatal);
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

bool optimizeProfiledPushMethod(IRGS& env,
                                TargetProfile<MethProfile>& profile,
                                SSATmp* objOrCls,
                                const Class* knownClass,
                                Block* sideExit,
                                const StringData* methodName,
                                uint32_t numParams,
                                bool dynamic) {
  if (!profile.optimizing()) return false;
  if (env.transFlags.noProfiledFPush && env.firstBcInst) return false;

  always_assert(objOrCls->type().subtypeOfAny(TObj, TCls));

  auto isStaticCall = objOrCls->type() <= TCls;

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

  MethProfile data = profile.data();

  if (auto const uniqueMeth = data.uniqueMeth()) {
    bool isMagic = !uniqueMeth->name()->isame(methodName);
    if (auto const uniqueClass = data.uniqueClass()) {
      // Profiling saw a unique class.
      // Check for it, then burn in the func
      auto const refined = gen(env, CheckType,
                               isStaticCall ?
                               Type::ExactCls(uniqueClass) :
                               Type::ExactObj(uniqueClass),
                               sideExit, objOrCls);
      env.irb->constrainValue(refined, GuardConstraint(uniqueClass));
      auto const ctx = getCtx(uniqueMeth, refined, uniqueClass);
      fpushActRec(env, cns(env, uniqueMeth), ctx, numParams,
                  isMagic ? methodName : nullptr, cns(env, dynamic));
      return true;
    }

    if (isMagic) return false;

    // Although there were multiple classes, the method was unique
    // (this comes up eg for a final method in a base class).  But
    // note that we can't allow a magic call here since it's possible
    // that an as-yet-unseen derived class defines a method named
    // methodName.
    auto const slot = cns(env, uniqueMeth->methodSlot());
    auto const negSlot = cns(env, -1 - uniqueMeth->methodSlot());
    auto const ctx = getCtx(uniqueMeth, objOrCls, nullptr);
    auto const cls = isStaticCall ? objOrCls : gen(env, LdObjClass, objOrCls);
    auto const len = gen(env, LdFuncVecLen, cls);
    auto const cmp = gen(env, LteInt, len, slot);
    gen(env, JmpNZero, sideExit, cmp);
    auto const meth = gen(env, LdClsMethod, cls, negSlot);
    auto const same = gen(env, EqFunc, meth, cns(env, uniqueMeth));
    gen(env, JmpZero, sideExit, same);
    fpushActRec(
      env,
      cns(env, uniqueMeth),
      ctx,
      numParams,
      nullptr,
      cns(env, dynamic)
    );
    return true;
  }

  // If we know anything about the class, other than it's an interface, the
  // remaining cases aren't worth the extra check.
  if (knownClass != nullptr && !isInterface(knownClass)) return false;

  if (auto const baseMeth = data.baseMeth()) {
    if (!baseMeth->name()->isame(methodName)) {
      return false;
    }

    // The method was defined in a common base class.  We just need to check for
    // an instance of the class, and then use the method from the right slot.
    auto const ctx = getCtx(baseMeth, objOrCls, nullptr);
    auto const cls = isStaticCall ? objOrCls : gen(env, LdObjClass, objOrCls);
    auto flag = gen(env, ExtendsClass,
                    ExtendsClassData{baseMeth->cls(), true}, cls);
    gen(env, JmpZero, sideExit, flag);
    auto negSlot = cns(env, -1 - baseMeth->methodSlot());
    auto meth = gen(env, LdClsMethod, cls, negSlot);
    fpushActRec(
      env,
      meth,
      ctx,
      numParams,
      nullptr,
      cns(env, dynamic)
    );
    return true;
  }

  // If we know anything about the class, the other cases below are not worth
  // the extra checks they insert.
  if (knownClass != nullptr) return false;

  if (auto const intfMeth = data.interfaceMeth()) {
    if (!intfMeth->name()->isame(methodName)) {
      return false;
    }

    // The method was defined in a common interface, so check for that and use
    // LdIfaceMethod.
    auto const ctx = getCtx(intfMeth, objOrCls, nullptr);
    auto const cls = isStaticCall ? objOrCls : gen(env, LdObjClass, objOrCls);
    auto flag = gen(env, InstanceOfIfaceVtable,
                    ClassData{intfMeth->cls()}, cls);
    gen(env, JmpZero, sideExit, flag);
    auto const vtableSlot =
      intfMeth->cls()->preClass()->ifaceVtableSlot();
    auto meth = gen(env, LdIfaceMethod,
                    IfaceMethodData{vtableSlot, intfMeth->methodSlot()},
                    cls);
    fpushActRec(env, meth, ctx, numParams, nullptr, cns(env, dynamic));
    return true;
  }

  return false;
}

void fpushObjMethod(IRGS& env,
                    SSATmp* obj,
                    const StringData* methodName,
                    uint32_t numParams,
                    bool shouldFatal,
                    Block* sideExit) {
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

  // If we don't know anything about the object's class, or all we know is an
  // interface that it implements, then enable PGO.
  const bool usePGO = !knownClass || isInterface(knownClass);

  folly::Optional<TargetProfile<MethProfile>> profile;
  if (usePGO && RuntimeOption::RepoAuthoritative) {
    profile.emplace(env.context, env.irb->curMarker(), methProfileKey.get());

    // If we know the class exactly without profiling, then we don't need PGO.
    if (optimizeProfiledPushMethod(env, *profile, obj, knownClass, sideExit,
                                   methodName, numParams, false)) {
      return;
    }
  }

  fpushObjMethodWithBaseClass(env, obj, knownClass, methodName, numParams,
                              shouldFatal, exactClass);

  if (profile && profile->profiling()) {
    gen(env,
        ProfileMethod,
        ProfileCallTargetData {
          spOffBCFromIRSP(env), profile->handle()
        },
        sp(env),
        cns(env, TNullptr));
  }
}

const StaticString s_funcProfileKey{"FuncProfile"};

void profilePushedFunc(IRGS& env, TargetProfile<CallTargetProfile>& profile) {
  gen(env, ProfileFunc,
      ProfileCallTargetData{spOffBCFromIRSP(env), profile.handle()},
      sp(env));
}

void optimizePushedFunc(IRGS& env, TargetProfile<CallTargetProfile>& profile) {
  auto data = profile.data();
  double probability = 0;
  auto profiledFunc = data.choose(probability);

  // Don't emit the check if the probability of it succeeding is below the
  // threshold.
  if (probability * 100 < RuntimeOption::EvalJitPGOPushedFuncThreshold) return;

  auto liveFuncTmp = gen(env, LdARFuncPtr, TFunc,
                         IRSPRelOffsetData{spOffBCFromIRSP(env)}, sp(env));

  // Don't emit the check if the function in the ActRec is already known.
  if (liveFuncTmp->hasConstVal(TFunc)) return;

  // Compare the pushed function with the profiled one.  If they match, emit an
  // AssertARFunc to pass that information; otherwise, take a side exit to the
  // next bytecode instruction.
  auto profiledFuncTmp = cns(env, profiledFunc);
  auto const equal = gen(env, EqFunc, liveFuncTmp, profiledFuncTmp);
  auto sideExit = makeExit(env, nextBcOff(env));
  gen(env, JmpZero, sideExit, equal);
  gen(env, AssertARFunc, IRSPRelOffsetData{spOffBCFromIRSP(env)}, sp(env),
      profiledFuncTmp);
}

void pgoPushedFunc(IRGS& env) {
  if (!RuntimeOption::RepoAuthoritative) return;

  auto profile = TargetProfile<CallTargetProfile>(env.context,
                                                  env.irb->curMarker(),
                                                  s_funcProfileKey.get());
  if (profile.profiling()) {
    profilePushedFunc(env, profile);
  } else if (profile.optimizing()) {
    optimizePushedFunc(env, profile);
  }
}

void fpushFuncObj(IRGS& env, uint32_t numParams) {
  auto const slowExit = makeExitSlow(env);
  auto const obj      = popC(env);
  auto const cls      = gen(env, LdObjClass, obj);
  auto const func     = gen(env, LdObjInvoke, slowExit, cls);
  fpushActRec(env, func, obj, numParams, nullptr, cns(env, false));
  pgoPushedFunc(env);
}

void fpushFuncArr(IRGS& env, uint32_t numParams) {
  auto const thisAR = fp(env);

  auto const arr = popC(env);
  fpushActRec(
    env,
    cns(env, TNullptr),
    cns(env, TNullptr),
    numParams,
    nullptr,
    cns(env, true)
  );

  // This is special. We need to move the stackpointer incase LdArrFuncCtx
  // calls a destructor. Otherwise it would clobber the ActRec we just
  // pushed.
  updateMarker(env);
  env.irb->exceptionStackBoundary();

  gen(env, LdArrFuncCtx,
      IRSPRelOffsetData { spOffBCFromIRSP(env) },
      arr, sp(env), thisAR);
  decRef(env, arr);
  pgoPushedFunc(env);
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

void fpushFuncCommon(IRGS& env,
                     uint32_t numParams,
                     const StringData* name,
                     const StringData* fallback) {

  auto const lookup = lookupImmutableFunc(curUnit(env), name);
  if (lookup.func) {
    // We know the function, but we have to ensure its unit is loaded. Use
    // LdFuncCached, ignoring the result to ensure this.
    if (lookup.needsUnitLoad) gen(env, LdFuncCached, FuncNameData { name });
    fpushActRec(env,
                cns(env, lookup.func),
                cns(env, TNullptr),
                numParams,
                nullptr,
                cns(env, false));
    return;
  }

  auto const ssaFunc = fallback
    ? gen(env, LdFuncCachedU, LdFuncCachedUData { name, fallback })
    : gen(env, LdFuncCached, FuncNameData { name });
  fpushActRec(env,
              ssaFunc,
              cns(env, TNullptr),
              numParams,
              nullptr,
              cns(env, false));
  pgoPushedFunc(env);
}

void implUnboxR(IRGS& env) {
  auto const exit = makeExit(env);
  auto const srcBox = popR(env);
  auto const unboxed = unbox(env, srcBox, exit);
  if (unboxed == srcBox) {
    // If the Unbox ended up being a noop, don't bother refcounting
    push(env, unboxed);
  } else {
    pushIncRef(env, unboxed);
    decRef(env, srcBox);
  }
}

void implBoxR(IRGS& env) {
  auto const value = pop(env, DataTypeGeneric);
  auto const boxed = boxHelper(
    env,
    gen(env, AssertType, TCell | TBoxedInitCell, value),
    [] (SSATmp* ) {});
  push(env, boxed);
}


//////////////////////////////////////////////////////////////////////

const StaticString
  s_http_response_header("http_response_header"),
  s_php_errormsg("php_errormsg");

/*
 * Could `inst' access the locals in the environment of `caller' according to
 * the given predicate?
 */
template <typename P>
bool callAccessesLocals(const NormalizedInstruction& inst,
                        const Func* caller,
                        P predicate) {
  // We don't handle these two cases, because we don't compile functions
  // containing them:
  assertx(caller->lookupVarId(s_php_errormsg.get()) == -1);
  assertx(caller->lookupVarId(s_http_response_header.get()) == -1);

  auto const unit = caller->unit();

  auto const checkTaintId = [&](Id id) {
    auto const str = unit->lookupLitstrId(id);
    // Only builtins can access a caller's locals or be skip-frame.
    auto const callee = Unit::lookupBuiltin(str);
    return callee && predicate(callee);
  };

  if (inst.op() == OpFCallBuiltin) return checkTaintId(inst.imm[2].u_SA);
  if (!isFCallStar(inst.op())) return false;

  auto const fpi = caller->findFPI(inst.source.offset());
  assertx(fpi != nullptr);
  auto const fpushPC = unit->at(fpi->m_fpushOff);
  auto const op = peek_op(fpushPC);

  switch (op) {
    case OpFPushFunc:
    case OpFPushCufIter:
      // Dynamic calls.  If we've forbidden dynamic calls to functions which
      // access the caller's frame, we know this can't be one.
      return !disallowDynamicVarEnvFuncs();

    case OpFPushFuncD:
      return checkTaintId(getImm(fpushPC, 1).u_SA);

    case OpFPushFuncU:
      return checkTaintId(getImm(fpushPC, 1).u_SA) ||
             checkTaintId(getImm(fpushPC, 2).u_SA);

    case OpFPushObjMethod:
    case OpFPushObjMethodD:
    case OpFPushClsMethod:
    case OpFPushClsMethodS:
    case OpFPushClsMethodSD:
    case OpFPushClsMethodD:
    case OpFPushCtor:
    case OpFPushCtorD:
    case OpFPushCtorI:
    case OpFPushCtorS:
      // None of these access the caller's frame because they all call methods,
      // not top-level functions. However, they might still be marked as
      // skip-frame and therefore something they call can affect our frame. We
      // don't have to worry about this if they're not allowed to call such
      // functions dynamically.
      return !disallowDynamicVarEnvFuncs();

    default:
      always_assert("Unhandled FPush type in callAccessesLocals" && 0);
  }
}

/*
 * Could `inst' write to the locals in the environment of `caller'?
 *
 * This occurs, e.g., if `inst' is a call to extract().
 */
bool callWritesLocals(const NormalizedInstruction& inst,
                      const Func* caller) {
  return callAccessesLocals(inst, caller, funcWritesLocals);
}

/*
 * Could `inst' read from the locals in the environment of `caller'?
 *
 * This occurs, e.g., if `inst' is a call to compact().
 */
bool callReadsLocals(const NormalizedInstruction& inst,
                     const Func* caller) {
  return callAccessesLocals(inst, caller, funcReadsLocals);
}

/*
 * Could `inst' attempt to read the caller frame?
 *
 * This occurs, e.g., if `inst' is a call to is_callable().
 */
bool callNeedsCallerFrame(const NormalizedInstruction& inst,
                          const Func* caller) {
  auto const  unit = caller->unit();
  auto const checkTaintId = [&](Id id) {
    auto const str = unit->lookupLitstrId(id);

    // If the function was invoked dynamically, we can't be sure.
    if (!str) return true;

    // Only builtins can inspect the caller frame; we know these are all
    // loaded ahead of time and unique/persistent.
    auto const f = Unit::lookupBuiltin(str);
    return f && funcNeedsCallerFrame(f);
  };

  if (inst.op() == OpFCallBuiltin) return checkTaintId(inst.imm[2].u_SA);
  if (!isFCallStar(inst.op())) return false;

  auto const fpi = caller->findFPI(inst.source.offset());
  assertx(fpi != nullptr);
  auto const fpushPC = unit->at(fpi->m_fpushOff);
  auto const op = peek_op(fpushPC);

  if (op == OpFPushFunc)  return true;
  if (op == OpFPushFuncD) return checkTaintId(getImm(fpushPC, 1).u_SA);
  if (op == OpFPushFuncU) {
    return checkTaintId(getImm(fpushPC, 1).u_SA) ||
           checkTaintId(getImm(fpushPC, 2).u_SA);
  }

  return false;
}

//////////////////////////////////////////////////////////////////////

}

//////////////////////////////////////////////////////////////////////

void fpushActRec(IRGS& env,
                 SSATmp* func,
                 SSATmp* objOrClass,
                 uint32_t numArgs,
                 const StringData* invName,
                 SSATmp* dynamicCall) {
  ActRecInfo info;
  info.spOffset = offsetFromIRSP(
    env,
    BCSPRelOffset{-int32_t{kNumActRecCells}}
  );
  info.numArgs = numArgs;

  gen(
    env,
    SpillFrame,
    info,
    sp(env),
    func,
    objOrClass,
    invName ? cns(env, invName) : cns(env, TNullptr),
    dynamicCall
  );
}

//////////////////////////////////////////////////////////////////////

void emitFPushCufIter(IRGS& env, uint32_t numParams, int32_t itId) {
  auto const func = gen(env, LdCufIterFunc, TFunc, IterId(itId), fp(env));
  auto const ctx = gen(
    env,
    LdCufIterCtx,
    TCtx | TNullptr,
    IterId(itId),
    fp(env)
  );
  auto const invName = gen(
    env,
    LdCufIterInvName,
    TStr | TNullptr,
    IterId(itId),
    fp(env)
  );
  auto const dynamic = gen(
    env,
    LdCufIterDynamic,
    IterId(itId),
    fp(env)
  );

  ActRecInfo info;
  info.spOffset = offsetFromIRSP(
    env,
    BCSPRelOffset{-int32_t{kNumActRecCells}}
  );
  info.numArgs = numParams;

  ifNonNull(env, ctx, [&](SSATmp* t) { gen(env, IncRef, t); });
  ifNonNull(env, invName, [&](SSATmp* t) { gen(env, IncRef, t); });
  gen(env, SpillFrame, info, sp(env), func, ctx, invName, dynamic);
}

void emitFPushCtor(IRGS& env, uint32_t numParams, uint32_t slot) {
  auto const cls  = takeClsRef(env, slot);
  auto const func = gen(env, LdClsCtor, cls, fp(env));
  auto const obj  = gen(env, AllocObj, cls);
  pushIncRef(env, obj);
  fpushActRec(env, func, obj, numParams, nullptr, cns(env, true));
}

void emitFPushCtorD(IRGS& env,
                    uint32_t numParams,
                    const StringData* className) {
  auto const cls = Unit::lookupUniqueClassInContext(className, curClass(env));
  bool const persistentCls = classIsPersistentOrCtxParent(env, cls);
  bool const canInstantiate = canInstantiateClass(cls);
  bool const fastAlloc =
    persistentCls &&
    canInstantiate &&
    !cls->hasNativePropHandler();

  auto const func = lookupImmutableCtor(cls, curClass(env));

  // We don't need to actually do the load if we have a persistent class
  auto const cachedCls = persistentCls ? nullptr :
    gen(env, LdClsCached, cns(env, className));

  // If we know the Class*, we can use it; if its not persistent,
  // we will have loaded it above.
  auto const ssaCls = cls ? cns(env, cls) : cachedCls;

  auto const ssaFunc = func ? cns(env, func)
                            : gen(env, LdClsCtor, ssaCls, fp(env));
  auto const obj = fastAlloc ? allocObjFast(env, cls)
                             : gen(env, AllocObj, ssaCls);
  pushIncRef(env, obj);
  fpushActRec(env, ssaFunc, obj, numParams, nullptr, cns(env, false));
}

void emitFPushCtorI(IRGS& env,
                    uint32_t numParams,
                    uint32_t clsIx) {
  auto const preClass = curFunc(env)->unit()->lookupPreClassId(clsIx);
  auto const cls = [&] () -> Class* {
    auto const c = preClass->namedEntity()->clsList();
    if (c && (c->attrs() & AttrUnique)) return c;
    return nullptr;
  }();
  bool const persistentCls = classIsPersistentOrCtxParent(env, cls);
  bool const canInstantiate = canInstantiateClass(cls);
  bool const fastAlloc =
    persistentCls &&
    canInstantiate &&
    !cls->hasNativePropHandler();

  auto const func = lookupImmutableCtor(cls, curClass(env));

  auto const ssaCls = [&] {
    if (!persistentCls) {
      auto const cachedCls = cond(
        env,
        [&] (Block* taken) {
          return gen(env, LdClsCachedSafe, taken, cns(env, preClass->name()));
        },
        [&] (SSATmp* val) {
          return val;
        },
        [&] {
          return gen(env, DefCls, cns(env, clsIx));
        }
      );
      if (!cls) return cachedCls;
    }
    return cns(env, cls);
  }();

  auto const ssaFunc = func ? cns(env, func)
                            : gen(env, LdClsCtor, ssaCls, fp(env));
  auto const obj = fastAlloc ? allocObjFast(env, cls)
                             : gen(env, AllocObj, ssaCls);
  pushIncRef(env, obj);
  fpushActRec(env, ssaFunc, obj, numParams, nullptr, cns(env, false));
}

namespace {

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

}

void emitFPushCtorS(IRGS& env, uint32_t numParams, SpecialClsRef ref) {
  auto const cls  = specialClsRefToCls(env, ref);
  auto const func = gen(env, LdClsCtor, cls, fp(env));
  auto const obj  = gen(env, AllocObj, cls);
  pushIncRef(env, obj);
  fpushActRec(env, func, obj, numParams, nullptr, cns(env, false));
}

void emitFPushFuncD(IRGS& env, uint32_t nargs, const StringData* name) {
  fpushFuncCommon(env, nargs, name, nullptr);
}

void emitFPushFuncU(IRGS& env,
                    uint32_t nargs,
                    const StringData* name,
                    const StringData* fallback) {
  fpushFuncCommon(env, nargs, name, fallback);
}

void emitFPushFunc(IRGS& env, uint32_t numParams, const ImmVector& v) {
  if (v.size() != 0) PUNT(InOut-FPushFunc);

  if (topC(env)->isA(TObj)) return fpushFuncObj(env, numParams);
  if (topC(env)->isA(TArr) || topC(env)->isA(TVec)) {
    return fpushFuncArr(env, numParams);
  }
  if (topC(env)->isA(TFunc)) {
    fpushActRec(
      env,
      popC(env),
      cns(env, TNullptr),
      numParams,
      nullptr,
      cns(env, false)
    );
    pgoPushedFunc(env);
    return;
  }

  if (!topC(env)->isA(TStr)) {
    PUNT(FPushFunc_not_Str);
  }

  auto const funcName = popC(env);
  fpushActRec(env,
              cns(env, TNullptr),
              cns(env, TNullptr),
              numParams,
              nullptr,
              cns(env, true));

  updateMarker(env);
  env.irb->exceptionStackBoundary();

  gen(env, LdFunc,
      IRSPRelOffsetData { spOffBCFromIRSP(env) },
      funcName, sp(env), fp(env));

  decRef(env, funcName);
  pgoPushedFunc(env);
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

void emitFPushObjMethodD(IRGS& env,
                         uint32_t numParams,
                         const StringData* methodName,
                         ObjMethodOp subop) {
  TransFlags trFlags;
  trFlags.noProfiledFPush = true;
  auto sideExit = makeExit(env, trFlags);

  auto const obj = popC(env);

  if (obj->type() <= TObj) {
    fpushObjMethod(env, obj, methodName, numParams,
      true /* shouldFatal */, sideExit);
    return;
  }

  if (obj->type() <= TInitNull && subop == ObjMethodOp::NullSafe) {
    fpushActRec(
      env,
      cns(env, SystemLib::s_nullFunc),
      cns(env, TNullptr),
      numParams,
      nullptr,
      cns(env, true));
    return;
  }

  PUNT(FPushObjMethodD-nonObj);
}

namespace {

SSATmp* lookupClsMethodKnown(IRGS& env,
                             const StringData* methodName,
                             SSATmp* callerCtx,
                             const Class *baseClass,
                             bool exact,
                             bool check,
                             bool forward,
                             bool& magicCall,
                             SSATmp*& calleeCtx) {
  magicCall = false;
  auto const func = lookupImmutableMethod(baseClass,
                                          methodName,
                                          magicCall,
                                          true /* staticLookup */,
                                          curFunc(env),
                                          exact);
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
                         bool dynamic) {
  auto magicCall = false;
  SSATmp* ctx = nullptr;
  auto funcTmp = lookupClsMethodKnown(env, methodName, ctxTmp, baseClass, exact,
                                      check, forward, magicCall, ctx);
  if (!funcTmp) return false;
  fpushActRec(env,
              funcTmp,
              ctx,
              numParams,
              magicCall ? methodName : nullptr,
              cns(env, dynamic));
  return true;
}

void emitFPushClsMethodD(IRGS& env,
                         uint32_t numParams,
                         const StringData* methodName,
                         const StringData* className) {
  if (auto const baseClass =
      Unit::lookupUniqueClassInContext(className, curClass(env))) {
    if (fpushClsMethodKnown(env, numParams,
                            methodName, cns(env, baseClass), baseClass,
                            true, true, false, false)) {
      return;
    }
  }

  auto const slowExit = makeExitSlow(env);
  auto const ne = NamedEntity::get(className);
  auto const data = ClsMethodData { className, methodName, ne };
  auto func = loadClsMethodUnknown(env, data, slowExit);
  auto const clsCtx = gen(env, LdClsMethodCacheCls, data);
  fpushActRec(env,
              func,
              clsCtx,
              numParams,
              nullptr,
              cns(env, false));
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
    bool magicCall = false;
    funcTmp = lookupClsMethodKnown(env, methodName, cns(env, baseClass),
                                    baseClass, true, true, false, magicCall,
                                    clsTmp);
    if (magicCall) {
      gen(env, ThrowInvalidOperation, cns(env, s_resolveClsMagicCall.get()));
      return;
    }
  }
  if (!funcTmp) {
    auto const slowExit = makeExitSlow(env);
    auto const ne = NamedEntity::get(className);
    auto const data = ClsMethodData { className, methodName, ne };
    funcTmp = loadClsMethodUnknown(env, data, slowExit);
    clsTmp = gen(env, LdClsCached, cns(env, className));
  }
  auto methPair = gen(env, AllocVArray, PackedArrayData { 2 });
  gen(env, InitPackedLayoutArray, IndexData { 0 }, methPair, clsTmp);
  gen(env, InitPackedLayoutArray, IndexData { 1 }, methPair, funcTmp);
  decRef(env, name);
  decRef(env, cls);
  popC(env);
  popC(env);
  push(env, methPair);
}

namespace {

template <typename Peek, typename Get, typename Kill>
ALWAYS_INLINE void fpushClsMethodCommon(IRGS& env,
                                        uint32_t numParams,
                                        Peek peekCls,
                                        Get getMeth,
                                        Kill killCls,
                                        bool forward,
                                        bool dynamic) {
  TransFlags trFlags;
  trFlags.noProfiledFPush = true;
  auto sideExit = makeExit(env, trFlags);

  // We can side-exit, so peek the slot rather than reading from it.
  auto const clsVal = peekCls();
  auto const methVal = getMeth();

  if (!methVal->isA(TStr)) {
    PUNT(FPushClsMethod-unknownType);
  }

  folly::Optional<TargetProfile<MethProfile>> profile;

  if (methVal->hasConstVal()) {
    auto const methodName = methVal->strVal();
    const Class* cls = nullptr;
    bool exact = false;
    if (auto clsSpec = clsVal->type().clsSpec()) {
      cls = clsSpec.cls();
      exact = clsSpec.exact();
    }

    if (cls) {
      if (fpushClsMethodKnown(env, numParams, methodName, clsVal, cls,
                              exact, false, forward, dynamic)) {
        killCls();
        return;
      }
    }

    if (RuntimeOption::RepoAuthoritative &&
        !clsVal->hasConstVal() &&
        !forward) {
      profile.emplace(env.context, env.irb->curMarker(), methProfileKey.get());

      if (optimizeProfiledPushMethod(env, *profile, clsVal, nullptr, sideExit,
                                     methodName, numParams, dynamic)) {
        killCls();
        return;
      }
    }
  }

  killCls();
  fpushActRec(env,
              cns(env, TNullptr),
              cns(env, TNullptr),
              numParams,
              nullptr,
              cns(env, dynamic));

  /*
   * Similar to FPushFunc/FPushObjMethod, we have an incomplete ActRec on the
   * stack and must handle that properly if we throw or re-enter.
   */
  updateMarker(env);
  env.irb->exceptionStackBoundary();

  gen(env, LookupClsMethod,
      LookupClsMethodData { spOffBCFromIRSP(env), forward },
      clsVal, methVal, sp(env), fp(env));
  decRef(env, methVal);

  if (profile && profile->profiling()) {
    gen(env,
        ProfileMethod,
        ProfileCallTargetData {
          spOffBCFromIRSP(env), profile->handle()
        },
        sp(env),
        clsVal);
  }
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
    [&] { return peekClsRef(env, slot); },
    [&] { return popC(env); },
    [&] { killClsRef(env, slot); },
    false,
    true
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
    []  {},
    ref == SpecialClsRef::Self || ref == SpecialClsRef::Parent,
    true
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
    []  {},
    ref == SpecialClsRef::Self || ref == SpecialClsRef::Parent,
    false
  );
}

//////////////////////////////////////////////////////////////////////

namespace {

SSATmp* ldPreLiveFunc(IRGS& env) {
  auto const& fpiStack = env.irb->fs().fpiStack();
  if (!fpiStack.empty() && fpiStack.back().func) {
    return cns(env, fpiStack.back().func);
  }

  auto off = instrFpToArDelta(curFunc(env), curSrcKey(env).pc());
  if (resumeMode(env) != ResumeMode::None) {
    off -= curFunc(env)->numSlotsInFrame();
  }
  auto const actRecOff = offsetFromIRSP(env, FPInvOffset { off });
  return gen(env, LdARFuncPtr, TFunc, IRSPRelOffsetData { actRecOff }, sp(env));
}

void handleRefMismatch(IRGS& env, SSATmp* func, uint32_t paramId) {
  if (!RuntimeOption::EvalThrowOnCallByRefAnnotationMismatch &&
      !RuntimeOption::EvalWarnOnCallByRefAnnotationMismatch) {
    return;
  }

  gen(env, RaiseParamRefMismatchForFunc, ParamData { (int32_t)paramId }, func);
}

void implFIsParamByRef(IRGS& env, SSATmp* func, uint32_t paramId,
                       FPassHint hint) {
  if (func->hasConstVal(TFunc)) {
    auto const byRef = func->funcVal()->byRef(paramId);
    if (hint == (byRef ? FPassHint::Cell : FPassHint::Ref)) {
      handleRefMismatch(env, func, paramId);
    }
    push(env, cns(env, byRef));
    return;
  }

  push(env, cond(
    env,
    [&] (Block* taken) {
      // CheckRefs only needs to know the number of parameters when there are
      // more than 64 args.
      auto const numParams = paramId < 64
        ? cns(env, 64) : gen(env, LdFuncNumParams, func);
      auto const bucket = paramId / 64 * 64;
      auto const mask = 1UL << (paramId % 64);
      gen(env, CheckRefs, taken, CheckRefsData { bucket, mask, 0 }, func,
          numParams);
    },
    [&] {
      if (hint == FPassHint::Ref) handleRefMismatch(env, func, paramId);
      return cns(env, false);
    },
    [&] {
      if (hint == FPassHint::Cell) handleRefMismatch(env, func, paramId);
      return cns(env, true);
    }
  ));
}

}

void emitFIsParamByRef(IRGS& env, uint32_t paramId, FPassHint hint) {
  implFIsParamByRef(env, ldPreLiveFunc(env), paramId, hint);
}

void emitFIsParamByRefCufIter(IRGS& env, uint32_t paramId, FPassHint hint,
                              int32_t itId) {
  auto const func = gen(env, LdCufIterFunc, TFunc, IterId(itId), fp(env));
  implFIsParamByRef(env, func, paramId, hint);
}

void emitFThrowOnRefMismatch(IRGS& env, const ImmVector& immVec) {
  if (!immVec.size()) return;

  auto const func = ldPreLiveFunc(env);
  auto const byRefs = immVec.vecu8();
  if (func->hasConstVal(TFunc)) {
    auto const f = func->funcVal();
    for (auto i = 0; i < immVec.size(); ++i) {
      if (f->byRef(i) != ((byRefs[i / 8] >> (i % 8)) & 1)) {
        PUNT(FThrowOnRefMismatch-RefMismatch);
      }
    }
    return;
  }

  auto const exitSlow = makeExitSlow(env);

  SSATmp* numParams = nullptr;
  for (uint32_t i = 0; i * 8 < immVec.size(); i += 8) {
    uint64_t vals = 0;
    for (uint32_t j = 0; j < 8 && (i + j) * 8 < immVec.size(); ++j) {
      vals |= ((uint64_t)byRefs[i + j]) << (8 * j);
    }

    uint64_t bits = immVec.size() - i * 8;
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
}

void emitFHandleRefMismatch(IRGS& env, uint32_t paramId, FPassHint hint,
                            const StringData* funcName) {
  assertx(hint != FPassHint::Any);
  if (!RuntimeOption::EvalThrowOnCallByRefAnnotationMismatch &&
      !RuntimeOption::EvalWarnOnCallByRefAnnotationMismatch) {
    return;
  }

  gen(
    env,
    RaiseParamRefMismatchForFuncName,
    ParamData { (int32_t)paramId },
    cns(env, funcName),
    cns(env, hint == FPassHint::Cell)
  );
}

void emitUnboxR(IRGS& env) { implUnboxR(env); }
void emitBoxR(IRGS& env) { implBoxR(env); }

//////////////////////////////////////////////////////////////////////

void emitCallerDynamicCallChecks(IRGS& env,
                                 const Func* callee,
                                 uint32_t numStackInputs) {
  if (RuntimeOption::EvalForbidDynamicCalls <= 0) return;
  if (callee && callee->isDynamicallyCallable()) return;

  SSATmp* func = nullptr;
  ifElse(
    env,
    [&] (Block* skip) {
      auto const calleeAROff = spOffBCFromIRSP(env) + numStackInputs;
      auto const dynamic = gen(
        env,
        LdARIsDynamic,
        IRSPRelOffsetData { calleeAROff },
        sp(env)
      );
      gen(env, JmpZero, skip, dynamic);

      // If we do have a callee, we already know its not dynamically callable
      // (checked above).
      if (!callee) {
        func = gen(
          env,
          LdARFuncPtr,
          TFunc,
          IRSPRelOffsetData { calleeAROff },
          sp(env)
        );
        auto const dyncallable = gen(env, IsFuncDynCallable, func);
        gen(env, JmpNZero, skip, dyncallable);
      } else {
        func = cns(env, callee);
      }
    },
    [&] {
      hint(env, Block::Hint::Unlikely);
      gen(env, RaiseForbiddenDynCall, func);
    }
  );
}

//////////////////////////////////////////////////////////////////////

SSATmp* implFCall(IRGS& env, uint32_t numParams, bool unpack, uint32_t numOut) {
  auto const callee = env.currentNormalizedInstruction->funcd;

  auto const writeLocals = callee
    ? funcWritesLocals(callee)
    : callWritesLocals(*env.currentNormalizedInstruction, curFunc(env));
  auto const readLocals = callee
    ? funcReadsLocals(callee)
    : callReadsLocals(*env.currentNormalizedInstruction, curFunc(env));

  emitCallerDynamicCallChecks(env, callee, numParams + (unpack ? 1 : 0));

  if (unpack) {
    auto const data = CallUnpackData {
      spOffBCFromIRSP(env),
      numParams + 1,
      numOut,
      bcOff(env),
      nextBcOff(env),
      callee,
      writeLocals,
      readLocals
    };
    auto const retVal = gen(env, CallUnpack, data, sp(env), fp(env));
    push(env, retVal);
    return retVal;
  } else {
    auto const returnBcOffset = nextBcOff(env) - curFunc(env)->base();
    auto const needsCallerFrame = callee
      ? funcNeedsCallerFrame(callee)
      : callNeedsCallerFrame(
        *env.currentNormalizedInstruction,
        curFunc(env)
      );

    auto op = curFunc(env)->unit()->getOp(bcOff(env));
    auto const retVal = gen(
      env,
      Call,
      CallData {
        spOffBCFromIRSP(env),
        static_cast<uint32_t>(numParams),
        numOut,
        returnBcOffset,
        callee,
        writeLocals,
        readLocals,
        needsCallerFrame,
        op == Op::FCallAwait
      },
      sp(env),
      fp(env)
    );

    push(env, retVal);
    return retVal;
  }
}

void emitFCall(IRGS& env,
               uint32_t numParams,
               uint32_t unpack,
               uint32_t numRets,
               const StringData*,
               const StringData*) {
  implFCall(env, numParams, unpack != 0, numRets - 1);
}

void emitDirectCall(IRGS& env, Func* callee, uint32_t numParams,
                    SSATmp* const* const args) {
  auto const returnBcOffset = nextBcOff(env) - curFunc(env)->base();

  env.irb->fs().setFPushOverride(Op::FPushFuncD);
  fpushActRec(
    env,
    cns(env, callee),
    cns(env, TNullptr),
    numParams,
    nullptr,
    cns(env, false)
  );
  assertx(!env.irb->fs().hasFPushOverride());

  for (int32_t i = 0; i < numParams; i++) {
    push(env, args[i]);
  }
  updateMarker(env);
  env.irb->exceptionStackBoundary();

  auto const retVal = gen(
    env,
    Call,
    CallData {
      spOffBCFromIRSP(env),
      static_cast<uint32_t>(numParams),
      0,
      returnBcOffset,
      callee,
      funcWritesLocals(callee),
      funcReadsLocals(callee),
      funcNeedsCallerFrame(callee),
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
    return TInitGen;
  }

  if (callee->isCPPBuiltin()) {
    // If the function is builtin, use the builtin's return type, then take into
    // account coercion failures.
    auto type = builtinReturnType(callee);
    if (callee->attrs() & AttrParamCoerceModeNull) type |= TInitNull;
    if (callee->attrs() & AttrParamCoerceModeFalse) type |= Type::cns(false);
    return type;
  }

  // Otherwise use HHBBC's analysis if present
  return typeFromRAT(callee->repoReturnType(), callee->cls());
}

//////////////////////////////////////////////////////////////////////

}}}
