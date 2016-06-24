/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/vm/jit/func-effects.h"
#include "hphp/runtime/vm/jit/mc-generator.h"
#include "hphp/runtime/vm/jit/normalized-instruction.h"
#include "hphp/runtime/vm/jit/target-profile.h"
#include "hphp/runtime/vm/jit/type-constraint.h"
#include "hphp/runtime/vm/jit/type.h"

#include "hphp/runtime/vm/jit/irgen-exit.h"
#include "hphp/runtime/vm/jit/irgen-create.h"
#include "hphp/runtime/vm/jit/irgen-internal.h"
#include "hphp/runtime/vm/jit/irgen-types.h"

namespace HPHP { namespace jit { namespace irgen {

namespace {

//////////////////////////////////////////////////////////////////////

const StaticString s_self("self");
const StaticString s_parent("parent");
const StaticString s_static("static");

//////////////////////////////////////////////////////////////////////

const Func* findCuf(Op op,
                    SSATmp* callable,
                    const Func* ctxFunc,
                    const Class*& cls,
                    StringData*& invName,
                    bool& forward) {
  cls = nullptr;
  invName = nullptr;

  const StringData* str =
    callable->hasConstVal(TStr) ? callable->strVal() : nullptr;
  const ArrayData* arr =
    callable->hasConstVal(TArr) ? callable->arrVal() : nullptr;

  StringData* sclass = nullptr;
  StringData* sname = nullptr;
  if (str) {
    Func* f = Unit::lookupFunc(str);
    if (f) return f;
    String name(const_cast<StringData*>(str));
    int pos = name.find("::");
    if (pos <= 0 || pos + 2 >= name.size() ||
        name.find("::", pos + 2) != String::npos) {
      return nullptr;
    }
    sclass = makeStaticString(name.substr(0, pos).get());
    sname = makeStaticString(name.substr(pos + 2).get());
  } else if (arr) {
    if (arr->size() != 2) return nullptr;
    const Variant& e0 = arr->get(int64_t(0), false);
    const Variant& e1 = arr->get(int64_t(1), false);
    if (!e0.isString() || !e1.isString()) return nullptr;
    sclass = e0.getStringData();
    sname = e1.getStringData();
    String name(sname);
    if (name.find("::") != String::npos) return nullptr;
  } else {
    return nullptr;
  }

  auto ctx = ctxFunc->cls();
  bool isExact = true;
  if (sclass->isame(s_self.get())) {
    if (!ctx) return nullptr;
    cls = ctx;
    forward = true;
  } else if (sclass->isame(s_parent.get())) {
    if (!ctx || !ctx->parent()) return nullptr;
    cls = ctx->parent();
    forward = true;
  } else if (sclass->isame(s_static.get())) {
    if (!ctx) return nullptr;
    cls = ctx;
    isExact = false;
  } else {
    cls = Unit::lookupClassOrUniqueClass(sclass);
    if (!cls) return nullptr;
  }

  bool magicCall = false;
  const Func* f = lookupImmutableMethod(
    cls, sname, magicCall, /* staticLookup = */ true, ctxFunc, isExact);
  if (!f || (forward && !ctx->classof(f->cls()))) {
    /*
     * To preserve the invariant that the lsb class
     * is an instance of the context class, we require
     * that f's class is an instance of the context class.
     * This is conservative, but without it, we would need
     * a runtime check to decide whether or not to forward
     * the lsb class
     */
    return nullptr;
  }
  if (magicCall) invName = sname;
  return f;
}

bool canInstantiateClass(const Class* cls) {
  return cls && isNormalClass(cls) && !isAbstract(cls);
}

//////////////////////////////////////////////////////////////////////

// Pushing for object method when we don't know the Func* statically.
void fpushObjMethodUnknown(IRGS& env,
                           SSATmp* obj,
                           const StringData* methodName,
                           int32_t numParams,
                           bool shouldFatal) {
  emitIncStat(env, Stats::ObjMethod_cached, 1);
  fpushActRec(env,
              cns(env, TNullptr),  // Will be set by LdObjMethod
              obj,
              numParams,
              nullptr);
  auto const objCls = gen(env, LdObjClass, obj);

  // This is special.  We need to move the stackpointer in case LdObjMethod
  // calls a destructor.  Otherwise it would clobber the ActRec we just pushed.
  updateMarker(env);
  env.irb->exceptionStackBoundary();

  gen(env,
      LdObjMethod,
      LdObjMethodData {
        bcSPOffset(env), methodName, shouldFatal
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

void fpushObjMethodExactFunc(
  IRGS& env,
  SSATmp* obj,
  const Class* exactClass,
  const Func* func,
  const StringData* methodName,
  int32_t numParams
) {
  /*
   * lookupImmutableMethod will return Funcs from AttrUnique classes, but in
   * this case, we have an object, so there's no need to check that the class
   * exists.
   *
   * Static function: store base class into this slot instead of obj and decref
   * the obj that was pushed as the this pointer since the obj won't be in the
   * actrec and thus MethodCache::lookup won't decref it.
   *
   * Static closure body: we still need to pass the object instance for the
   * closure prologue to properly do its dispatch (and extract use vars). It
   * will decref it and put the class on the actrec before entering the "real"
   * cloned closure body.
   */
  SSATmp* objOrCls = obj;
  emitIncStat(env, Stats::ObjMethod_known, 1);
  if (func->isStatic() && !func->isClosureBody()) {
    objOrCls = exactClass ? cns(env, exactClass) : gen(env, LdObjClass, obj);
    decRef(env, obj);
  }
  fpushActRec(
    env,
    cns(env, func),
    objOrCls,
    numParams,
    methodName
  );
}

const Func* lookupInterfaceFuncForFPushObjMethod(
  IRGS& env,
  const Class* baseClass,
  const StringData* methodName
) {
  if (!baseClass) return nullptr;
  if (!classIsUniqueInterface(baseClass)) return nullptr;

  return findInterfaceMethod(baseClass, methodName);
}

void fpushObjMethodInterfaceFunc(
  IRGS& env,
  SSATmp* obj,
  const Func* ifaceFunc,
  int32_t numParams
) {
  auto const vtableSlot = ifaceFunc->cls()->preClass()->ifaceVtableSlot();

  emitIncStat(env, Stats::ObjMethod_ifaceslot, 1);
  auto cls = gen(env, LdObjClass, obj);
  auto func = gen(env, LdIfaceMethod,
                  IfaceMethodData{vtableSlot, ifaceFunc->methodSlot()},
                  cls);
  SSATmp* objOrCls = obj;
  if (ifaceFunc->attrs() & AttrStatic) {
    decRef(env, obj);
    objOrCls = cls;
  }
  fpushActRec(env, func, objOrCls, numParams, /* invName */nullptr);
  return;
}


const Func* lookupNonExactFuncForFPushObjMethod(
  IRGS& env,
  const Class* baseClass,
  const StringData* methodName
) {
  if (!baseClass) return nullptr;

  const Func* func = nullptr;
  auto const res = g_context->lookupObjMethod(func, baseClass, methodName,
                                              curClass(env), /* raise */false);

  if (res != LookupResult::MethodFoundWithThis &&
      res != LookupResult::MethodFoundNoThis) {
    // Method lookup did not find anything.
    return nullptr;
  }

  /*
   * If we found the func in baseClass and it's private, this would have already
   * been handled as an exact Func.
   */
  assertx(!(func->attrs() & AttrPrivate));

  /*
   * If we found the func in the baseClass and it's not private, any
   * derived class must have a func that matches in staticness
   * and is at least as accessible (and in particular, you can't
   * override a public/protected method with a private method).  In
   * this case, we emit code to dynamically lookup the method given
   * the Object and the method slot, which is the same as func's.
   */
  return func;
}

void fpushObjMethodNonExactFunc(
  IRGS& env,
  SSATmp* obj,
  const Class* baseClass,
  const Func* func,
  int32_t numParams
) {
  emitIncStat(env, Stats::ObjMethod_methodslot, 1);
  auto const clsTmp = gen(env, LdObjClass, obj);
  auto const funcTmp = gen(
    env,
    LdClsMethod,
    clsTmp,
    cns(env, -(func->methodSlot() + 1))
  );
  SSATmp* objOrCls = obj;
  if (func->attrs() & AttrStatic && !func->isClosureBody()) {
    decRef(env, obj);
    objOrCls = clsTmp;
  }
  fpushActRec(env,
    funcTmp,
    objOrCls,
    numParams,
    /* invName */nullptr
  );
}

void fpushObjMethodWithBaseClass(
  IRGS& env,
  SSATmp* obj,
  const Class* baseClass,
  const StringData* methodName,
  int32_t numParams,
  bool shouldFatal,
  bool exactClass
) {
  bool magicCall = false;
  if (auto const exactFunc = lookupImmutableMethod(
        baseClass, methodName, magicCall,
        /* staticLookup: */ false, curFunc(env), exactClass)) {
    fpushObjMethodExactFunc(env, obj,
                            exactClass ? baseClass : nullptr,
                            exactFunc,
                            magicCall ? methodName : nullptr,
                            numParams);
    return;
  }

  if (auto const func =
      lookupInterfaceFuncForFPushObjMethod(env, baseClass, methodName)) {
    fpushObjMethodInterfaceFunc(env, obj, func, numParams);
    return;
  }

  if (auto const nonExactFunc = lookupNonExactFuncForFPushObjMethod(
      env, baseClass, methodName)) {
    fpushObjMethodNonExactFunc(env, obj, baseClass, nonExactFunc, numParams);
    return;
  }

  fpushObjMethodUnknown(env, obj, methodName, numParams, shouldFatal);
}

const StaticString methProfileKey{ "MethProfile-FPushObjMethod" };

bool optimizeProfiledPushMethod(IRGS& env,
                                TargetProfile<MethProfile>& profile,
                                SSATmp* objOrCls,
                                Block* sideExit,
                                const StringData* methodName,
                                int32_t numParams) {
  if (!profile.optimizing()) return false;
  if (env.transFlags.noProfiledFPush && env.firstBcInst) return false;

  always_assert(objOrCls->type().subtypeOfAny(TObj, TCls));

  auto isStaticCall = objOrCls->type() <= TCls;

  enum class CtxKind {
    Invalid,
    Normal,
    This
  };

  auto getCtx = [&](const Func* callee,
                    CtxKind ctxKind,
                    SSATmp* ctx,
                    const Class* cls) -> SSATmp* {
    if (isStaticCall) {
      if (ctxKind == CtxKind::This) {
        assertx(!callee->isStatic());
        auto this_ = ldThis(env);
        gen(env, IncRef, this_);
        return this_;
      }
      return ctx;
    }
    if (!callee->isStatic()) return ctx;
    assertx(ctx->type() <= TObj);
    auto ret = cls ? cns(env, cls) : gen(env, LdObjClass, ctx);
    decRef(env, ctx);
    return ret;
  };

  // Determine whether it's ok to call this method from the current
  // context. We don't want to deal with static calls to non-static
  // methods in object context because it involves runtime checking.
  auto checkMeth = [&](const Func* callee) -> CtxKind {
    if (!isStaticCall || callee->isStatic()) return CtxKind::Normal;
    auto ctx = curClass(env);
    if (!ctx || curFunc(env)->isStatic()) return CtxKind::Normal;
    if (isInterface(callee->cls())) return CtxKind::Invalid;
    if (ctx->classof(callee->cls())) {
      return env.irb->fs().thisAvailable() ? CtxKind::This : CtxKind::Invalid;
    }
    return callee->cls()->classof(ctx) ? CtxKind::Invalid : CtxKind::Normal;
  };

  MethProfile data = profile.data(MethProfile::reduce);

  if (auto const uniqueMeth = data.uniqueMeth()) {
    auto ctxKind = checkMeth(uniqueMeth);
    if (ctxKind == CtxKind::Invalid) return false;

    bool isMagic = !uniqueMeth->name()->isame(methodName);
    if (auto const uniqueClass = data.uniqueClass()) {
      // Profiling saw a unique class.
      // Check for it, then burn in the func
      auto const refined = gen(env, CheckType,
                               isStaticCall ?
                               Type::ExactCls(uniqueClass) :
                               Type::ExactObj(uniqueClass),
                               sideExit, objOrCls);
      env.irb->constrainValue(refined, TypeConstraint(uniqueClass));
      auto const ctx = getCtx(uniqueMeth, ctxKind, refined, uniqueClass);
      fpushActRec(env, cns(env, uniqueMeth), ctx, numParams,
                  isMagic ? methodName : nullptr);
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
    auto const ctx = getCtx(uniqueMeth, ctxKind, objOrCls, nullptr);
    auto const cls = objOrCls->type() <= TCls ?
      objOrCls : gen(env, LdObjClass, objOrCls);
    auto const len = gen(env, LdFuncVecLen, cls);
    auto const cmp = gen(env, LteInt, len, slot);
    gen(env, JmpNZero, sideExit, cmp);
    auto const meth = gen(env, LdClsMethod, cls, negSlot);
    auto const same = gen(env, EqFunc, meth, cns(env, uniqueMeth));
    gen(env, JmpZero, sideExit, same);
    fpushActRec(env, cns(env, uniqueMeth), ctx, numParams, nullptr);
    return true;
  }

  if (auto const baseMeth = data.baseMeth()) {
    if (!baseMeth->name()->isame(methodName) ||
        checkMeth(baseMeth) != CtxKind::Normal) {
      return false;
    }

    // The method was defined in a common base class.  We just need to
    // check for an instance of the class, and then use the method
    // from the right slot.
    auto const ctx = getCtx(baseMeth, CtxKind::Normal, objOrCls, nullptr);
    auto const cls = objOrCls->type() <= TCls ?
      objOrCls : gen(env, LdObjClass, objOrCls);
    auto flag = gen(env, ExtendsClass,
                    ExtendsClassData{baseMeth->cls(), true}, cls);
    gen(env, JmpZero, sideExit, flag);
    auto negSlot = cns(env, -1 - baseMeth->methodSlot());
    auto meth = gen(env, LdClsMethod, cls, negSlot);
    fpushActRec(env, meth, ctx, numParams, nullptr);
    return true;
  }

  if (auto const intfMeth = data.interfaceMeth()) {
    if (!intfMeth->name()->isame(methodName) ||
        checkMeth(intfMeth) != CtxKind::Normal) {
      return false;
    }
    // The method was defined in a common interface
    auto const ctx = getCtx(intfMeth, CtxKind::Normal, objOrCls, nullptr);
    auto const cls = objOrCls->type() <= TCls ?
      objOrCls : gen(env, LdObjClass, objOrCls);
    auto flag = gen(env, InstanceOfIfaceVtable,
                    ClassData{intfMeth->cls()}, cls);
    gen(env, JmpZero, sideExit, flag);
    auto const vtableSlot =
      intfMeth->cls()->preClass()->ifaceVtableSlot();
    auto meth = gen(env, LdIfaceMethod,
                    IfaceMethodData{vtableSlot, intfMeth->methodSlot()},
                    cls);
    fpushActRec(env, meth, ctx, numParams, nullptr);
    return true;
  }

  return false;
}

void fpushObjMethod(IRGS& env,
                    SSATmp* obj,
                    const StringData* methodName,
                    int32_t numParams,
                    bool shouldFatal,
                    Block* sideExit) {
  emitIncStat(env, Stats::ObjMethod_total, 1);

  if (auto cls = obj->type().clsSpec().cls()) {
    if (!env.irb->constrainValue(obj, TypeConstraint(cls).setWeak())) {
      // If we know the class without having to specialize a guard any further,
      // use it.
      fpushObjMethodWithBaseClass(
        env, obj, cls, methodName, numParams, shouldFatal,
        obj->type().clsSpec().exact() || cls->attrs() & AttrNoOverride);
      return;
    }
  }

  folly::Optional<TargetProfile<MethProfile>> profile;
  if (RuntimeOption::RepoAuthoritative) {
    profile.emplace(env.context, env.irb->curMarker(), methProfileKey.get());

    if (optimizeProfiledPushMethod(env, *profile,
                                   obj, sideExit, methodName, numParams)) {
      return;
    }
  }

  fpushObjMethodWithBaseClass(env, obj, nullptr, methodName, numParams,
                              shouldFatal, false);

  if (profile && profile->profiling()) {
    gen(env,
        ProfileMethod,
        ProfileMethodData {
          bcSPOffset(env), profile->handle()
        },
        sp(env),
        cns(env, TNullptr));
  }
}

void fpushFuncObj(IRGS& env, int32_t numParams) {
  auto const slowExit = makeExitSlow(env);
  auto const obj      = popC(env);
  auto const cls      = gen(env, LdObjClass, obj);
  auto const func     = gen(env, LdObjInvoke, slowExit, cls);
  fpushActRec(env, func, obj, numParams, nullptr);
}

void fpushFuncArr(IRGS& env, int32_t numParams) {
  auto const thisAR = fp(env);

  auto const arr = popC(env);
  fpushActRec(
    env,
    cns(env, TNullptr),
    cns(env, TNullptr),
    numParams,
    nullptr
  );

  // This is special. We need to move the stackpointer incase LdArrFuncCtx
  // calls a destructor. Otherwise it would clobber the ActRec we just
  // pushed.
  updateMarker(env);
  env.irb->exceptionStackBoundary();

  gen(env, LdArrFuncCtx,
      IRSPRelOffsetData { bcSPOffset(env) },
      arr, sp(env), thisAR);
  decRef(env, arr);
}

// FPushCuf when the callee is not known at compile time.
void fpushCufUnknown(IRGS& env, Op op, int32_t numParams) {
  if (op != Op::FPushCuf) {
    PUNT(fpushCufUnknown-nonFPushCuf);
  }

  if (topC(env)->isA(TObj)) return fpushFuncObj(env, numParams);

  if (!topC(env)->type().subtypeOfAny(TArr, TStr)) {
    PUNT(fpushCufUnknown);
  }

  auto const callable = popC(env);
  fpushActRec(
    env,
    cns(env, TNullptr),
    cns(env, TNullptr),
    numParams,
    nullptr
  );

  /*
   * This is a similar case to lookup for functions in FPushFunc or
   * FPushObjMethod.  We can throw in a weird situation where the
   * ActRec is already on the stack, but this bytecode isn't done
   * executing yet.  See arPreliveOverwriteCells for details about why
   * we need this marker.
   */
  updateMarker(env);
  env.irb->exceptionStackBoundary();

  auto const opcode = callable->isA(TArr) ? LdArrFPushCuf
                                               : LdStrFPushCuf;
  gen(env, opcode,
      IRSPRelOffsetData { bcSPOffset(env) },
      callable, sp(env), fp(env));
  decRef(env, callable);
}

SSATmp* clsMethodCtx(IRGS& env, const Func* callee, const Class* cls) {
  bool mustBeStatic = true;

  if (!(callee->attrs() & AttrStatic) &&
      !(curFunc(env)->attrs() & AttrStatic) &&
      curClass(env)) {
    if (curClass(env)->classof(cls)) {
      // In this case, it might not be static, but we can be sure
      // we're going to forward $this if thisAvailable.
      mustBeStatic = false;
    } else if (cls->classof(curClass(env))) {
      // Unlike the above, we might be calling down to a subclass that
      // is not related to the current instance.  To know whether this
      // call forwards $this requires a runtime type check, so we have
      // to punt instead of trying the thisAvailable path below.
      PUNT(getClsMethodCtx-PossibleStaticRelatedCall);
    }
  }

  if (mustBeStatic) {
    return ldCls(env, cns(env, cls->name()));
  }
  if (env.irb->fs().thisAvailable()) {
    // might not be a static call and $this is available, so we know it's
    // definitely not static
    assertx(curClass(env));
    auto this_ = ldThis(env);
    gen(env, IncRef, this_);
    return this_;
  }
  // might be a non-static call. we have to inspect the func at runtime
  PUNT(getClsMethodCtx-MightNotBeStatic);
}

void implFPushCufOp(IRGS& env, Op op, int32_t numArgs) {
  const bool safe = op == OpFPushCufSafe;
  bool forward = op == OpFPushCufF;
  SSATmp* callable = topC(env, BCSPRelOffset{safe ? 1 : 0});

  const Class* cls = nullptr;
  StringData* invName = nullptr;
  auto const callee = findCuf(op, callable, curFunc(env), cls, invName,
                              forward);
  if (!callee) return fpushCufUnknown(env, op, numArgs);

  SSATmp* ctx;
  auto const safeFlag = cns(env, true); // This is always true until the slow
                                        // exits below are implemented
  auto func = cns(env, callee);
  if (cls) {
    auto const exitSlow = makeExitSlow(env);
    if (!classHasPersistentRDS(cls)) {
      // The miss path is complicated and rare.  Punt for now.  This must be
      // checked before we IncRef the context below, because the slow exit will
      // want to do that same IncRef via InterpOne.
      gen(env, LdClsCachedSafe, exitSlow, cns(env, cls->name()));
    }

    if (forward) {
      ctx = ldCtx(env);
      ctx = gen(env, GetCtxFwdCall, ctx, cns(env, callee));
    } else {
      ctx = clsMethodCtx(env, callee, cls);
    }
  } else {
    ctx = cns(env, TNullptr);
    auto const handle = callee->funcHandle();
    if (handle == rds::kInvalidHandle ||
        !rds::isPersistentHandle(handle)) {
      // The miss path is complicated and rare. Punt for now.
      func = gen(env, LdFuncCachedSafe,
                 LdFuncCachedData { callee->name() },
                 makeExitSlow(env));
    }
  }

  auto const defaultVal = safe ? popC(env) : nullptr;
  popDecRef(env); // callable
  if (safe) {
    push(env, defaultVal);
    push(env, safeFlag);
  }

  fpushActRec(env, func, ctx, numArgs, invName);
}

void fpushFuncCommon(IRGS& env,
                     int32_t numParams,
                     const StringData* name,
                     const StringData* fallback) {
  if (auto const func = Unit::lookupFunc(name)) {
    if (func->isNameBindingImmutable(curUnit(env))) {
      fpushActRec(env,
                  cns(env, func),
                  cns(env, TNullptr),
                  numParams,
                  nullptr);
      return;
    }
  }

  auto const ssaFunc = fallback
    ? gen(env, LdFuncCachedU, LdFuncCachedUData { name, fallback })
    : gen(env, LdFuncCached, LdFuncCachedData { name });
  fpushActRec(env,
              ssaFunc,
              cns(env, TNullptr),
              numParams,
              nullptr);
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

//////////////////////////////////////////////////////////////////////

}

//////////////////////////////////////////////////////////////////////

void fpushActRec(IRGS& env,
                 SSATmp* func,
                 SSATmp* objOrClass,
                 int32_t numArgs,
                 const StringData* invName) {
  ActRecInfo info;
  info.spOffset = offsetFromIRSP(
    env,
    BCSPRelOffset{-int32_t{kNumActRecCells}}
  );
  info.invName = invName;
  info.numArgs = numArgs;

  gen(
    env,
    SpillFrame,
    info,
    sp(env),
    func,
    objOrClass
  );
}

//////////////////////////////////////////////////////////////////////

void emitFPushCufIter(IRGS& env, int32_t numParams, int32_t itId) {
  gen(
    env,
    CufIterSpillFrame,
    FPushCufData {
      offsetFromIRSP(
        env,
        BCSPRelOffset{-int32_t{kNumActRecCells}}
      ),
      static_cast<uint32_t>(numParams),
      itId
    },
    sp(env),
    fp(env)
  );
}

void emitFPushCuf(IRGS& env, int32_t numArgs) {
  implFPushCufOp(env, Op::FPushCuf, numArgs);
}
void emitFPushCufF(IRGS& env, int32_t numArgs) {
  implFPushCufOp(env, Op::FPushCufF, numArgs);
}
void emitFPushCufSafe(IRGS& env, int32_t numArgs) {
  implFPushCufOp(env, Op::FPushCufSafe, numArgs);
}

void emitFPushCtor(IRGS& env, int32_t numParams) {
  auto const cls  = popA(env);
  auto const func = gen(env, LdClsCtor, cls, fp(env));
  auto const obj  = gen(env, AllocObj, cls);
  pushIncRef(env, obj);
  fpushActRec(env, func, obj, numParams, nullptr);
}

void emitFPushCtorD(IRGS& env,
                    int32_t numParams,
                    const StringData* className) {
  auto const cls = Unit::lookupClassOrUniqueClass(className);
  bool const uniqueCls = classIsUnique(cls);
  bool const persistentCls = classHasPersistentRDS(cls);
  bool const canInstantiate = canInstantiateClass(cls);
  bool const fastAlloc =
    persistentCls &&
    canInstantiate &&
    !cls->hasNativePropHandler();

  auto const func = lookupImmutableCtor(cls, curClass(env));

  auto ssaCls = persistentCls
    ? cns(env, cls)
    : gen(env, LdClsCached, cns(env, className));
  if (!ssaCls->hasConstVal() && uniqueCls) {
    // If the Class is unique but not persistent, it's safe to use it as a
    // const after the LdClsCached, which will throw if the class can't be
    // defined.
    ssaCls = cns(env, cls);
  }

  auto const ssaFunc = func ? cns(env, func)
                            : gen(env, LdClsCtor, ssaCls, fp(env));
  auto const obj = fastAlloc ? allocObjFast(env, cls)
                             : gen(env, AllocObj, ssaCls);
  pushIncRef(env, obj);
  fpushActRec(env, ssaFunc, obj, numParams, nullptr);
}

void emitFPushFuncD(IRGS& env, int32_t nargs, const StringData* name) {
  fpushFuncCommon(env, nargs, name, nullptr);
}

void emitFPushFuncU(IRGS& env,
                    int32_t nargs,
                    const StringData* name,
                    const StringData* fallback) {
  fpushFuncCommon(env, nargs, name, fallback);
}

void emitFPushFunc(IRGS& env, int32_t numParams) {
  if (topC(env)->isA(TObj)) return fpushFuncObj(env, numParams);
  if (topC(env)->isA(TArr)) return fpushFuncArr(env, numParams);

  if (!topC(env)->isA(TStr)) {
    PUNT(FPushFunc_not_Str);
  }

  auto const funcName = popC(env);
  fpushActRec(env,
              gen(env, LdFunc, funcName),
              cns(env, TNullptr),
              numParams,
              nullptr);
}

void emitFPushObjMethodD(IRGS& env,
                         int32_t numParams,
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
      nullptr);
    return;
  }

  PUNT(FPushObjMethodD-nonObj);
}

static void checkImmutableClsMethod(IRGS& env, Func const* func) {
  if (!classHasPersistentRDS(func->cls())) {
    if (auto ctx = curClass(env)) {
      // The current context, and all its parents are definitely loaded
      if (ctx->classof(func->cls())) return;
    }
    // we're only guaranteed uniqueness of the class. If its
    // not persistent, we must make sure its loaded.
    auto clsName = cns(env, func->cls()->name());
    ifThen(env,
           [&] (Block* notLoaded) {
             gen(env, LdClsCachedSafe, notLoaded, clsName);
           },
           [&] {
             hint(env, Block::Hint::Unlikely);
             gen(env, LdClsCached, clsName);
           });
  }
}

void emitFPushClsMethodD(IRGS& env,
                         int32_t numParams,
                         const StringData* methodName,
                         const StringData* className) {
  auto const baseClass  = Unit::lookupClassOrUniqueClass(className);
  bool magicCall        = false;

  if (auto const func = lookupImmutableMethod(baseClass,
                                              methodName,
                                              magicCall,
                                              true /* staticLookup */,
                                              curFunc(env),
                                              true /* isExact */)) {
    checkImmutableClsMethod(env, func);
    auto const objOrCls = clsMethodCtx(env, func, baseClass);
    fpushActRec(env,
                cns(env, func),
                objOrCls,
                numParams,
                func && magicCall ? methodName : nullptr);
    return;
  }

  auto const slowExit = makeExitSlow(env);
  auto const ne = NamedEntity::get(className);
  auto const data = ClsMethodData { className, methodName, ne };

  // Look up the Func* in the targetcache. If it's not there, try the slow
  // path. If that fails, slow exit.
  auto const func = cond(
    env,
    [&] (Block* taken) {
      return gen(env, LdClsMethodCacheFunc, data, taken);
    },
    [&] (SSATmp* func) { // next
      return func;
    },
    [&] { // taken
      hint(env, Block::Hint::Unlikely);
      auto const result = gen(env, LookupClsMethodCache, data, fp(env));
      return gen(env, CheckNonNull, slowExit, result);
    }
  );
  auto const clsCtx = gen(env, LdClsMethodCacheCls, data);

  fpushActRec(env,
              func,
              clsCtx,
              numParams,
              nullptr);
}

void emitFPushClsMethod(IRGS& env, int32_t numParams) {
  TransFlags trFlags;
  trFlags.noProfiledFPush = true;
  auto sideExit = makeExit(env, trFlags);

  auto const clsVal  = popA(env);
  auto const methVal = popC(env);

  if (!methVal->isA(TStr) || !clsVal->isA(TCls)) {
    PUNT(FPushClsMethod-unknownType);
  }

  folly::Optional<TargetProfile<MethProfile>> profile;

  if (methVal->hasConstVal()) {
    auto const methodName = methVal->strVal();
    const Class* cls = nullptr;
    if (clsVal->hasConstVal()) {
      cls = clsVal->clsVal();
    } else if (clsVal->inst()->is(LdClsCtx, LdClsCctx)) {
      /*
       * Optimize FPushClsMethod when the method is a known static
       * string and the input class is the context.  The common bytecode
       * pattern here is LateBoundCls ; FPushClsMethod.
       *
       * This logic feels like it belongs in the simplifier, but the
       * generated code for this case is pretty different, since we
       * don't need the pre-live ActRec trick.
       */
      cls = curClass(env);
    }

    if (cls) {
      const Func* func;
      auto res =
        g_context->lookupClsMethod(func,
                                   cls,
                                   methodName,
                                   nullptr,
                                   cls,
                                   false);
      if (res == LookupResult::MethodFoundNoThis && func->isStatic()) {
        auto funcTmp = clsVal->hasConstVal() || func->isImmutableFrom(cls) ?
          cns(env, func) :
          gen(env, LdClsMethod, clsVal, cns(env, -(func->methodSlot() + 1)));

        fpushActRec(env, funcTmp, clsVal, numParams, nullptr);
        return;
      }
    }

    if (RuntimeOption::RepoAuthoritative && !clsVal->hasConstVal()) {
      profile.emplace(env.context, env.irb->curMarker(), methProfileKey.get());

      if (optimizeProfiledPushMethod(env, *profile,
                                     clsVal, sideExit, methodName, numParams)) {
        return;
      }
    }
  }

  fpushActRec(env,
              cns(env, TNullptr),
              cns(env, TNullptr),
              numParams,
              nullptr);

  /*
   * Similar to FPushFunc/FPushObjMethod, we have an incomplete ActRec on the
   * stack and must handle that properly if we throw or re-enter.
   */
  updateMarker(env);
  env.irb->exceptionStackBoundary();

  gen(env, LookupClsMethod,
      IRSPRelOffsetData { bcSPOffset(env) },
      clsVal, methVal, sp(env), fp(env));
  decRef(env, methVal);

  if (profile && profile->profiling()) {
    gen(env,
        ProfileMethod,
        ProfileMethodData {
          bcSPOffset(env), profile->handle()
        },
        sp(env),
        clsVal);
  }
}

void emitFPushClsMethodF(IRGS& env, int32_t numParams) {
  auto const exitBlock = makeExitSlow(env);

  auto classTmp = top(env);
  auto methodTmp = topC(env, BCSPRelOffset{1}, DataTypeGeneric);
  assertx(classTmp->isA(TCls));
  if (!classTmp->hasConstVal() || !methodTmp->hasConstVal(TStr)) {
    PUNT(FPushClsMethodF-unknownClassOrMethod);
  }
  env.irb->constrainValue(methodTmp, DataTypeSpecific);

  auto const cls = classTmp->clsVal();
  auto const methName = methodTmp->strVal();

  bool magicCall = false;
  auto const vmfunc = lookupImmutableMethod(cls,
                                            methName,
                                            magicCall,
                                            true /* staticLookup */,
                                            curFunc(env),
                                            true /* isExact */);
  discard(env, 2);

  auto const curCtxTmp = ldCtx(env);
  if (vmfunc) {
    // FPushClsMethodF should only ever have self or parent as the
    // class, which will always be loaded. checkImmutableClsMethod
    // tests for that though, so call it just in case.
    checkImmutableClsMethod(env, vmfunc);
    auto const funcTmp = cns(env, vmfunc);
    auto const newCtxTmp = gen(env, GetCtxFwdCall, curCtxTmp, funcTmp);
    fpushActRec(env, funcTmp, newCtxTmp, numParams,
                magicCall ? methName : nullptr);
    return;
  }

  auto const data = ClsMethodData{cls->name(), methName};
  auto const funcTmp = cond(
    env,
    [&](Block* taken) {
      return gen(env, LdClsMethodFCacheFunc, data, taken);
    },
    [&](SSATmp* func) { // next
      return func;
    },
    [&] { // taken
      hint(env, Block::Hint::Unlikely);
      auto const result = gen(
        env,
        LookupClsMethodFCache,
        data,
        cns(env, cls),
        fp(env)
      );
      return gen(env, CheckNonNull, exitBlock, result);
    }
  );

  auto const ctx = gen(env, GetCtxFwdCallDyn, data, curCtxTmp);
  fpushActRec(env,
              funcTmp,
              ctx,
              numParams,
              magicCall ? methName : nullptr);
}

//////////////////////////////////////////////////////////////////////

/*
 * All fpass instructions spill the stack after they execute, because we are
 * sure to need that value in memory, regardless of whether we side-exit or
 * throw.  At the level of HHBC semantics, it's illegal to pop them from the
 * stack until we've left the FPI region, and we will be spilling the whole
 * stack when we get to the FCall{D,} at the end of the region.  This should
 * also potentially reduce the number of live registers during call sequences.
 *
 * Note: there is a general problem with the spillStack mechanism, in that it
 * may sink stores that are not profitable to sink, but in this case we can
 * work around it easily.
 */

void emitFPassL(IRGS& env, int32_t argNum, int32_t id) {
  if (env.currentNormalizedInstruction->preppedByRef) {
    emitVGetL(env, id);
  } else {
    emitCGetL(env, id);
  }
}

void emitFPassS(IRGS& env, int32_t argNum) {
  if (env.currentNormalizedInstruction->preppedByRef) {
    emitVGetS(env);
  } else {
    emitCGetS(env);
  }
}

void emitFPassG(IRGS& env, int32_t argNum) {
  if (env.currentNormalizedInstruction->preppedByRef) {
    emitVGetG(env);
  } else {
    emitCGetG(env);
  }
}

void emitFPassR(IRGS& env, int32_t argNum) {
  if (env.currentNormalizedInstruction->preppedByRef) {
    PUNT(FPassR-byRef);
  }

  implUnboxR(env);
}

void emitUnboxR(IRGS& env) { implUnboxR(env); }

void emitFPassV(IRGS& env, int32_t argNum) {
  if (env.currentNormalizedInstruction->preppedByRef) {
    // FPassV is a no-op when the callee expects by ref.
    return;
  }

  auto const tmp = popV(env);
  pushIncRef(env, gen(env, LdRef, TInitCell, tmp));
  decRef(env, tmp);
}

void emitFPassCE(IRGS& env, int32_t argNum) {
  if (env.currentNormalizedInstruction->preppedByRef) {
    // Need to raise an error
    PUNT(FPassCE-byRef);
  }
}

void emitFPassCW(IRGS& env, int32_t argNum) {
  if (env.currentNormalizedInstruction->preppedByRef) {
    // Need to raise a warning
    PUNT(FPassCW-byRef);
  }
}

//////////////////////////////////////////////////////////////////////

void emitFCallArray(IRGS& env) {
  auto const data = CallArrayData {
    bcSPOffset(env),
    0,
    bcOff(env),
    nextBcOff(env),
    callDestroysLocals(*env.currentNormalizedInstruction, curFunc(env))
  };
  auto const retVal = gen(env, CallArray, data, sp(env), fp(env));
  push(env, retVal);
}

void emitFCallUnpack(IRGS& env, int32_t numParams) {
  auto const data = CallArrayData {
    bcSPOffset(env),
    numParams,
    bcOff(env),
    nextBcOff(env),
    callDestroysLocals(*env.currentNormalizedInstruction, curFunc(env))
  };
  auto const retVal = gen(env, CallArray, data, sp(env), fp(env));
  push(env, retVal);
}

void emitFCallD(IRGS& env,
                int32_t numParams,
                const StringData*,
                const StringData*) {
  emitFCall(env, numParams);
}

SSATmp* implFCall(IRGS& env, int32_t numParams) {
  auto const returnBcOffset = nextBcOff(env) - curFunc(env)->base();
  auto const callee = env.currentNormalizedInstruction->funcd;

  auto const destroyLocals = callee
    ? callee->isCPPBuiltin() && builtinFuncDestroysLocals(callee)
    : callDestroysLocals(
      *env.currentNormalizedInstruction,
      curFunc(env)
    );
  auto const needsCallerFrame = callee
    ? callee->isCPPBuiltin() && builtinFuncNeedsCallerFrame(callee)
    : callNeedsCallerFrame(
      *env.currentNormalizedInstruction,
      curFunc(env)
    );

  auto op = curFunc(env)->unit()->getOp(bcOff(env));

  auto const retVal = gen(
    env,
    Call,
    CallData {
      bcSPOffset(env),
      static_cast<uint32_t>(numParams),
      returnBcOffset,
      callee,
      destroyLocals,
      needsCallerFrame,
      op == Op::FCallAwait
    },
    sp(env),
    fp(env)
  );
  push(env, retVal);
  return retVal;
}

void emitFCall(IRGS& env, int32_t numParams) {
  implFCall(env, numParams);
}

void emitDirectCall(IRGS& env, Func* callee, int32_t numParams,
                    SSATmp* const* const args) {
  auto const returnBcOffset = nextBcOff(env) - curFunc(env)->base();

  env.irb->fs().setFPushOverride(Op::FPushFuncD);
  fpushActRec(env, cns(env, callee), cns(env, TNullptr), numParams, nullptr);
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
      bcSPOffset(env),
      static_cast<uint32_t>(numParams),
      returnBcOffset,
      callee,
      false,
      false,
      false
    },
    sp(env),
    fp(env)
  );
  push(env, retVal);
}

//////////////////////////////////////////////////////////////////////

}}}
