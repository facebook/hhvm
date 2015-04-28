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
#include "hphp/runtime/vm/jit/irgen-call.h"

#include "hphp/runtime/vm/jit/mc-generator.h"
#include "hphp/runtime/vm/jit/normalized-instruction.h"
#include "hphp/runtime/vm/jit/type-constraint.h"
#include "hphp/runtime/vm/jit/type.h"

#include "hphp/runtime/vm/jit/irgen-exit.h"
#include "hphp/runtime/vm/jit/irgen-create.h"
#include "hphp/runtime/vm/jit/irgen-internal.h"

namespace HPHP { namespace jit { namespace irgen {

namespace {

//////////////////////////////////////////////////////////////////////

const StaticString s_self("self");
const StaticString s_parent("parent");
const StaticString s_static("static");

//////////////////////////////////////////////////////////////////////

const Func* findCuf(Op op,
                    SSATmp* callable,
                    const Class* ctx,
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

  if (sclass->isame(s_self.get())) {
    if (!ctx) return nullptr;
    cls = ctx;
    forward = true;
  } else if (sclass->isame(s_parent.get())) {
    if (!ctx || !ctx->parent()) return nullptr;
    cls = ctx->parent();
    forward = true;
  } else if (sclass->isame(s_static.get())) {
    return nullptr;
  } else {
    cls = Unit::lookupClassOrUniqueClass(sclass);
    if (!cls) return nullptr;
  }

  bool magicCall = false;
  const Func* f = lookupImmutableMethod(cls, sname, magicCall,
                                        /* staticLookup = */ true, ctx);
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
  spillStack(env);
  fpushActRec(env,
              cns(env, TNullptr),  // Will be set by LdObjMethod
              obj,
              numParams,
              nullptr);
  spillStack(env);
  auto const objCls = gen(env, LdObjClass, obj);

  // This is special.  We need to move the stackpointer in case LdObjMethod
  // calls a destructor.  Otherwise it would clobber the ActRec we just pushed.
  updateMarker(env);
  env.irb->exceptionStackBoundary();

  gen(env,
      LdObjMethod,
      LdObjMethodData {
        offsetFromIRSP(env, BCSPOffset{0}), methodName, shouldFatal
      },
      objCls,
      sp(env));
}

void fpushObjMethodCommon(IRGS& env,
                          SSATmp* obj,
                          const StringData* methodName,
                          int32_t numParams,
                          bool shouldFatal) {
  SSATmp* objOrCls = obj;
  const Class* baseClass = nullptr;
  if (auto cls = obj->type().clsSpec().cls()) {
    if (!env.irb->constrainValue(obj, TypeConstraint(cls).setWeak())) {
      // If we know the class without having to specialize a guard any further,
      // use it.
      baseClass = cls;
    }
  }

  bool magicCall = false;
  const Func* func = lookupImmutableMethod(baseClass,
                                           methodName,
                                           magicCall,
                                           /* staticLookup: */
                                           false,
                                           curClass(env));

  if (!func) {
    if (baseClass && !(baseClass->attrs() & AttrInterface)) {
      auto const res =
        g_context->lookupObjMethod(func, baseClass, methodName, curClass(env),
                                   false);
      if (res == LookupResult::MethodFoundWithThis ||
          /*
           * TODO(#4455926): We don't allow vtable-style dispatch of
           * abstract static methods, but not for any real reason
           * here.  It should be able to work, but needs further
           * testing to be enabled.
           */
          (res == LookupResult::MethodFoundNoThis && !func->isAbstract())) {
        /*
         * If we found the func in baseClass, then either:
         *  a) its private, and this is always going to be the
         *     called function. This case is handled further down.
         * OR
         *  b) any derived class must have a func that matches in staticness
         *     and is at least as accessible (and in particular, you can't
         *     override a public/protected method with a private method).  In
         *     this case, we emit code to dynamically lookup the method given
         *     the Object and the method slot, which is the same as func's.
         */
        if (!(func->attrs() & AttrPrivate)) {
          auto const clsTmp = gen(env, LdObjClass, obj);
          auto const funcTmp = gen(
            env,
            LdClsMethod,
            clsTmp,
            cns(env, -(func->methodSlot() + 1))
          );
          if (res == LookupResult::MethodFoundNoThis) {
            gen(env, DecRef, obj);
            objOrCls = clsTmp;
          }
          fpushActRec(env,
                      funcTmp,
                      objOrCls,
                      numParams,
                      magicCall ? methodName : nullptr);
          return;
        }
      } else {
        // method lookup did not find anything
        func = nullptr; // force lookup
      }
    }
  }

  if (func != nullptr) {
    /*
     * static function: store base class into this slot instead of obj
     * and decref the obj that was pushed as the this pointer since
     * the obj won't be in the actrec and thus MethodCache::lookup won't
     * decref it
     *
     * static closure body: we still need to pass the object instance
     * for the closure prologue to properly do its dispatch (and
     * extract use vars).  It will decref it and put the class on the
     * actrec before entering the "real" cloned closure body.
     */
    if (func->attrs() & AttrStatic && !func->isClosureBody()) {
      assertx(baseClass);
      gen(env, DecRef, obj);
      objOrCls = cns(env, baseClass);
    }
    fpushActRec(env,
                cns(env, func),
                objOrCls,
                numParams,
                magicCall ? methodName : nullptr);
    return;
  }

  fpushObjMethodUnknown(env, obj, methodName, numParams, shouldFatal);
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
  spillStack(env);

  // This is special. We need to move the stackpointer incase LdArrFuncCtx
  // calls a destructor. Otherwise it would clobber the ActRec we just
  // pushed.
  updateMarker(env);
  env.irb->exceptionStackBoundary();

  gen(env, LdArrFuncCtx, IRSPOffsetData { offsetFromIRSP(env, BCSPOffset{0}) },
    arr, sp(env), thisAR);
  gen(env, DecRef, arr);
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
  spillStack(env);

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
  gen(env, opcode, IRSPOffsetData { offsetFromIRSP(env, BCSPOffset{0}) },
    callable, sp(env), fp(env));
  gen(env, DecRef, callable);
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
  if (env.irb->thisAvailable()) {
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
  SSATmp* callable = topC(env, BCSPOffset{safe ? 1 : 0});

  const Class* cls = nullptr;
  StringData* invName = nullptr;
  auto const callee = findCuf(op, callable, curClass(env), cls, invName,
    forward);
  if (!callee) return fpushCufUnknown(env, op, numArgs);

  SSATmp* ctx;
  auto const safeFlag = cns(env, true); // This is always true until the slow exits
                                        // below are implemented
  auto func = cns(env, callee);
  if (cls) {
    auto const exitSlow = makeExitSlow(env);
    if (!rds::isPersistentHandle(cls->classHandle())) {
      // The miss path is complicated and rare.  Punt for now.  This must be
      // checked before we IncRef the context below, because the slow exit will
      // want to do that same IncRef via InterpOne.
      auto const clsOrNull = gen(env, LdClsCachedSafe, cns(env, cls->name()));
      gen(env, CheckNonNull, exitSlow, clsOrNull);
    }

    if (forward) {
      ctx = ldCtx(env);
      ctx = gen(env, GetCtxFwdCall, ctx, cns(env, callee));
    } else {
      ctx = clsMethodCtx(env, callee, cls);
    }
  } else {
    ctx = cns(env, TNullptr);
    if (!rds::isPersistentHandle(callee->funcHandle())) {
      // The miss path is complicated and rare. Punt for now.
      func = gen(env, LdFuncCachedSafe, LdFuncCachedData(callee->name()));
      func = gen(env, CheckNonNull, makeExitSlow(env), func);
    }
  }

  auto const defaultVal = safe ? popC(env) : nullptr;
  popDecRef(env, TCell); // callable
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
    gen(env, DecRef, srcBox);
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
  spillStack(env);
  auto const returnSPOff = env.irb->syncedSpLevel();

  ActRecInfo info;
  info.spOffset = offsetFromIRSP(env, BCSPOffset{-int32_t{kNumActRecCells}});
  info.numArgs = numArgs;
  info.invName = invName;
  gen(
    env,
    SpillFrame,
    info,
    sp(env),
    func,
    objOrClass
  );
  auto const sframe = &env.irb->curBlock()->back();
  assertx(sframe->is(SpillFrame));

  env.fpiStack.push(FPIInfo { sp(env), returnSPOff, sframe });

  assertx(env.irb->stackDeficit() == 0);
}

//////////////////////////////////////////////////////////////////////

void emitFPushCufIter(IRGS& env, int32_t numParams, int32_t itId) {
  spillStack(env);
  env.fpiStack.push(FPIInfo { sp(env), env.irb->spOffset(), nullptr });
  gen(
    env,
    CufIterSpillFrame,
    FPushCufData {
      offsetFromIRSP(env, BCSPOffset{-int32_t{kNumActRecCells}}),
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
  auto const func = gen(env, LdClsCtor, cls);
  auto const obj  = gen(env, AllocObj, cls);
  gen(env, IncRef, obj);
  pushIncRef(env, obj);
  auto numArgsAndFlags = ActRec::encodeNumArgs(numParams, false, false, true);
  fpushActRec(env, func, obj, numArgsAndFlags, nullptr);
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
    !cls->callsCustomInstanceInit() &&
    !cls->hasNativePropHandler();

  const Func* func = uniqueCls ? cls->getCtor() : nullptr;
  if (func && !(func->attrs() & AttrPublic)) {
    auto const ctx = curClass(env);
    if (!ctx) {
      func = nullptr;
    } else if (ctx != cls) {
      if ((func->attrs() & AttrPrivate) ||
        !(ctx->classof(cls) || cls->classof(ctx))) {
        func = nullptr;
      }
    }
  }

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
                            : gen(env, LdClsCtor, ssaCls);
  auto const obj = fastAlloc ? allocObjFast(env, cls)
                             : gen(env, AllocObj, ssaCls);
  gen(env, IncRef, obj);
  pushIncRef(env, obj);
  auto numArgsAndFlags = ActRec::encodeNumArgs(numParams, false, false, true);
  fpushActRec(env, ssaFunc, obj, numArgsAndFlags, nullptr);
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
  auto const obj = popC(env);
  if (!obj->isA(TObj)) PUNT(FPushObjMethodD-nonObj);
  fpushObjMethodCommon(env, obj, methodName, numParams,
    true /* shouldFatal */);
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
                                              curClass(env))) {
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
      auto const mcFunc = gen(env, LdClsMethodCacheFunc, data);
      return gen(env, CheckNonNull, taken, mcFunc);
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
  auto const clsVal  = popA(env);
  auto const methVal = popC(env);

  if (!methVal->isA(TStr) || !clsVal->isA(TCls)) {
    PUNT(FPushClsMethod-unknownType);
  }

  if (methVal->hasConstVal()) {
    const Class* cls = nullptr;
    if (clsVal->hasConstVal()) {
      cls = clsVal->clsVal();
    } else if (clsVal->inst()->op() == LdClsCctx) {
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
                                     methVal->strVal(),
                                     nullptr,
                                     cls,
                                     false);
      if (res == LookupResult::MethodFoundNoThis && func->isStatic()) {
        auto funcTmp = clsVal->hasConstVal()
          ? cns(env, func)
          : gen(env, LdClsMethod, clsVal, cns(env, -(func->methodSlot() + 1)));
        fpushActRec(env, funcTmp, clsVal, numParams, nullptr);
        return;
      }
    }
  }

  fpushActRec(env,
              cns(env, TNullptr),
              cns(env, TNullptr),
              numParams,
              nullptr);
  spillStack(env);

  /*
   * Similar to FPushFunc/FPushObjMethod, we have an incomplete ActRec on the
   * stack and must handle that properly if we throw or re-enter.
   */
  updateMarker(env);
  env.irb->exceptionStackBoundary();

  gen(env, LookupClsMethod,
    IRSPOffsetData { offsetFromIRSP(env, BCSPOffset{0}) },
    clsVal, methVal, sp(env), fp(env));
  gen(env, DecRef, methVal);
}

void emitFPushClsMethodF(IRGS& env, int32_t numParams) {
  auto const exitBlock = makeExitSlow(env);

  auto classTmp = top(env, TCls);
  auto methodTmp = topC(env, BCSPOffset{1}, DataTypeGeneric);
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
                                            curClass(env));
  discard(env, 2);

  auto const curCtxTmp = ldCtx(env);
  if (vmfunc) {
    auto const funcTmp = cns(env, vmfunc);
    auto const newCtxTmp = gen(env, GetCtxFwdCall, curCtxTmp, funcTmp);
    fpushActRec(env, funcTmp, newCtxTmp, numParams,
      (magicCall ? methName : nullptr));
    return;
  }

  auto const data = ClsMethodData{cls->name(), methName};
  auto const funcTmp = cond(
    env,
    [&](Block* taken) {
      auto const fcacheFunc = gen(env, LdClsMethodFCacheFunc, data);
      return gen(env, CheckNonNull, taken, fcacheFunc);
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
  spillStack(env);
}

void emitFPassS(IRGS& env, int32_t argNum) {
  if (env.currentNormalizedInstruction->preppedByRef) {
    emitVGetS(env);
  } else {
    emitCGetS(env);
  }
  spillStack(env);
}

void emitFPassG(IRGS& env, int32_t argNum) {
  if (env.currentNormalizedInstruction->preppedByRef) {
    emitVGetG(env);
  } else {
    emitCGetG(env);
  }
  spillStack(env);
}

void emitFPassR(IRGS& env, int32_t argNum) {
  if (env.currentNormalizedInstruction->preppedByRef) {
    PUNT(FPassR-byRef);
  }

  implUnboxR(env);
  spillStack(env);
}

void emitFPassM(IRGS& env, int32_t, int x) {
  if (env.currentNormalizedInstruction->preppedByRef) {
    emitVGetM(env, x);
  } else {
    emitCGetM(env, x);
  }
  spillStack(env);
}

void emitUnboxR(IRGS& env) { implUnboxR(env); }

void emitFPassV(IRGS& env, int32_t argNum) {
  if (env.currentNormalizedInstruction->preppedByRef) {
    // FPassV is a no-op when the callee expects by ref.
    return;
  }

  auto const tmp = popV(env);
  pushIncRef(env, gen(env, LdRef, TInitCell, tmp));
  gen(env, DecRef, tmp);
  spillStack(env);
}

void emitFPassCE(IRGS& env, int32_t argNum) {
  if (env.currentNormalizedInstruction->preppedByRef) {
    // Need to raise an error
    PUNT(FPassCE-byRef);
  }
  spillStack(env);
}

void emitFPassCW(IRGS& env, int32_t argNum) {
  if (env.currentNormalizedInstruction->preppedByRef) {
    // Need to raise a warning
    PUNT(FPassCW-byRef);
  }
  spillStack(env);
}

//////////////////////////////////////////////////////////////////////

void emitFCallArray(IRGS& env) {
  spillStack(env);
  auto const data = CallArrayData {
    offsetFromIRSP(env, BCSPOffset{0}),
    bcOff(env),
    nextBcOff(env),
    callDestroysLocals(*env.currentNormalizedInstruction, curFunc(env))
  };
  env.irb->exceptionStackBoundary();
  gen(env, CallArray, data, sp(env), fp(env));
}

void emitFCallD(IRGS& env,
                int32_t numParams,
                const StringData*,
                const StringData*) {
  emitFCall(env, numParams);
}

void emitFCall(IRGS& env, int32_t numParams) {
  auto const returnBcOffset = nextBcOff(env) - curFunc(env)->base();
  auto const callee = env.currentNormalizedInstruction->funcd;
  auto const destroyLocals = callDestroysLocals(
    *env.currentNormalizedInstruction,
    curFunc(env)
  );

  spillStack(env);
  env.irb->exceptionStackBoundary();
  gen(
    env,
    Call,
    CallData {
      offsetFromIRSP(env, BCSPOffset{0}),
      static_cast<uint32_t>(numParams),
      returnBcOffset,
      callee,
      destroyLocals
    },
    sp(env),
    fp(env)
  );

  if (!env.fpiStack.empty()) {
    env.fpiStack.pop();
  }
}

//////////////////////////////////////////////////////////////////////

}}}
