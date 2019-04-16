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
IRSPRelOffset fpushObjMethodUnknown(
  IRGS& env,
  SSATmp* obj,
  const StringData* methodName,
  uint32_t numParams,
  SSATmp* ts
) {
  implIncStat(env, Stats::ObjMethod_cached);
  auto const arOffset = fsetActRec(
    env,
    cns(env, TNullptr),  // Will be set by LdObjMethod
    obj,
    numParams,
    nullptr,
    false,
    ts
  );
  auto const objCls = gen(env, LdObjClass, obj);

  // This is special.  We need to move the stackpointer in case LdObjMethod
  // calls a destructor.  Otherwise it would clobber the ActRec we just pushed.
  updateMarker(env);
  env.irb->exceptionStackBoundary();

  gen(env,
      LdObjMethod,
      LdObjMethodData { arOffset, methodName },
      objCls,
      sp(env));
  return arOffset;
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
    gen(env, RaiseHasThisNeedStatic, cns(env, func));
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
    gen(env, RaiseHasThisNeedStatic, func);
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

IRSPRelOffset fpushObjMethodWithBaseClass(
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
    return fsetActRec(env, func, objOrCls, numParams,
                      magicCall ? methodName : nullptr,
                      false, ts);
  }

  return fpushObjMethodUnknown(env, obj, methodName, numParams, ts);
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
    auto const arOffset = emitFPush();
    if (profile.profiling()) {
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
          fsetActRec(env, cns(env, uniqueMeth), ctx, numParams,
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
        fsetActRec(env, cns(env, uniqueMeth), ctx, numParams, nullptr, dynamic,
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
        fsetActRec(env, meth, ctx, numParams, nullptr, dynamic, nullptr);
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
        fsetActRec(env, meth, ctx, numParams, nullptr, dynamic, nullptr);
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
    return fpushObjMethodWithBaseClass(env, obj, knownClass, methodName,
                                       numParams, exactClass, tsList);
  };

  // If we know the class exactly without profiling, then we don't need PGO.
  if (!RuntimeOption::RepoAuthoritative ||
      (knownClass && !isInterface(knownClass))) {
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
  fsetActRec(env, func, obj, numParams, nullptr, false, nullptr);
}

void fpushFuncArr(IRGS& env, uint32_t numParams) {
  auto const thisAR = fp(env);

  auto const arr = popC(env);
  auto const arOffset = fsetActRec(
    env,
    cns(env, TNullptr),
    cns(env, TNullptr),
    numParams,
    nullptr,
    true,
    nullptr
  );

  // This is special. We need to move the stackpointer incase LdArrFuncCtx
  // calls a destructor. Otherwise it would clobber the ActRec we just
  // pushed.
  updateMarker(env);
  env.irb->exceptionStackBoundary();

  gen(env, LdArrFuncCtx,
      IRSPRelOffsetData { arOffset },
      arr, sp(env), thisAR);
  decRef(env, arr);
}

void fpushFuncClsMeth(IRGS& env, uint32_t numParams) {
  auto const clsMeth = popC(env);
  auto const cls = gen(env, LdClsFromClsMeth, clsMeth);
  auto const func = gen(env, LdFuncFromClsMeth, clsMeth);
  fsetActRec(
    env,
    func,
    cls,
    numParams,
    nullptr,
    true,
    nullptr
  );
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
    fsetActRec(env,
               cns(env, lookup.func),
               cns(env, TNullptr),
               numParams,
               nullptr,
               false,
               nullptr);
    return;
  }

  fsetActRec(env,
             gen(env, LdFuncCached, FuncNameData { name }),
             cns(env, TNullptr),
             numParams,
             nullptr,
             false,
             nullptr);
}

namespace {

//////////////////////////////////////////////////////////////////////

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

  return false;
}

//////////////////////////////////////////////////////////////////////

}

//////////////////////////////////////////////////////////////////////

IRSPRelOffset fsetActRec(
  IRGS& env,
  SSATmp* func,
  SSATmp* objOrClass,
  uint32_t numArgs,
  const StringData* invName,
  bool dynamicCall,
  SSATmp* tsList
) {
  ActRecInfo info;
  info.spOffset = offsetFromIRSP(
    env,
    BCSPRelOffset{static_cast<int32_t>(numArgs)}
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
    cns(env, dynamicCall),
    tsList ? tsList : cns(env, TNullptr)
  );

  return info.spOffset;
}

//////////////////////////////////////////////////////////////////////

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
    emitCallerDynamicConstructChecks(env, cls);
    push(env, gen(env, AllocObj, cls));
    return;
  }

  auto const clsref = takeClsRef(env, slot, HasGenericsOp::HasGenerics == op);
  auto const reified_generic = clsref.first;
  auto const cls  = clsref.second;
  emitCallerDynamicConstructChecks(env, cls);
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
  bool const canInstantiate = canInstantiateClass(cls);
  if (persistentCls && canInstantiate && !cls->hasNativePropHandler()) {
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

  auto const func = [&] {
    auto const exactCls = obj->type().clsSpec().exactCls();
    if (!exactCls) {
      auto const cls = gen(env, LdObjClass, obj);
      return gen(env, LdClsCtor, cls, fp(env));
    }

    auto const ctor = lookupImmutableCtor(exactCls, curClass(env));
    if (ctor) return cns(env, ctor);
    return gen(env, LdClsCtor, cns(env, exactCls), fp(env));
  }();

  fsetActRec(env, func, obj, numParams, nullptr, false, nullptr);
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
    fsetActRec(
      env,
      callee,
      cns(env, TNullptr),
      numParams,
      nullptr,
      false,
      nullptr
    );
    return;
  }
  if (topC(env)->isA(TClsMeth)) {
    return fpushFuncClsMeth(env, numParams);
  }

  if (!callee->isA(TStr)) {
    PUNT(FPushFunc_not_Str);
  }

  popC(env);
  auto const arOffset = fsetActRec(
    env,
    cns(env, TNullptr),
    cns(env, TNullptr),
    numParams,
    nullptr,
    true,
    getReifiedGenerics(env, callee)
  );

  updateMarker(env);
  env.irb->exceptionStackBoundary();

  gen(env, LdFunc,
      IRSPRelOffsetData { arOffset },
      callee, sp(env), fp(env));

  decRef(env, callee);
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
    fsetActRec(
      env,
      cns(env, SystemLib::s_nullFunc),
      cns(env, TNullptr),
      numParams,
      nullptr,
      true,
      tsList);
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
  fsetActRec(env,
             funcTmp,
             ctx,
             numParams,
             magicCall ? methodName : nullptr,
             dynamic,
             nullptr);
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
  fsetActRec(env,
             func,
             clsCtx,
             numParams,
             nullptr,
             false,
             nullptr);
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
    SSATmp* ctx = nullptr;
    funcTmp = lookupClsMethodKnown(env, methodName, cns(env, baseClass),
                                    baseClass, true, true, false, magicCall,
                                    ctx);
    if (magicCall) {
      gen(env, ThrowInvalidOperation, cns(env, s_resolveClsMagicCall.get()));
      return;
    }
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
                                        bool dynamic) {
  auto const clsVal = takeCls();
  auto const methVal = getMeth();

  if (!methVal->isA(TStr)) {
    PUNT(FPushClsMethod-unknownType);
  }

  auto const emitFPush = [&] {
    auto const arOffset = fsetActRec(
      env,
      cns(env, TNullptr),
      cns(env, TNullptr),
      numParams,
      nullptr,
      dynamic,
      nullptr
    );

    /*
     * Similar to FPushFunc/FPushObjMethod, we have an incomplete ActRec on the
     * stack and must handle that properly if we throw or re-enter.
     */
    updateMarker(env);
    env.irb->exceptionStackBoundary();

    auto const lcmData = LookupClsMethodData { arOffset, forward };
    gen(env, LookupClsMethod, lcmData, clsVal, methVal, sp(env), fp(env));
    decRef(env, methVal);
    return arOffset;
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
                                 exact, false, forward, dynamic)) {
    return;
  }

  if (!RuntimeOption::RepoAuthoritative || clsVal->hasConstVal() || forward) {
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
    ref == SpecialClsRef::Self || ref == SpecialClsRef::Parent,
    false
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

void emitCallerDynamicCallChecks(IRGS& env,
                                 const Func* callee,
                                 IRSPRelOffset actRecOff) {
  if (RuntimeOption::EvalForbidDynamicCalls <= 0) return;
  if (callee && callee->isDynamicallyCallable()) return;

  SSATmp* func = nullptr;
  ifElse(
    env,
    [&] (Block* skip) {
      auto const dynamic = gen(
        env,
        LdARIsDynamic,
        IRSPRelOffsetData { actRecOff },
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
          IRSPRelOffsetData { actRecOff },
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

void emitCallerDynamicConstructChecks(IRGS& env, SSATmp* cls) {
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

void emitCallerRxChecks(IRGS& env, const Func* callee,
                        IRSPRelOffset actRecOff) {
  if (RuntimeOption::EvalRxEnforceCalls <= 0) return;
  auto const callerLevel = curRxLevel(env);
  if (!rxEnforceCallsInLevel(callerLevel)) return;

  auto const minReqCalleeLevel = rxRequiredCalleeLevel(callerLevel);
  if (callee) {
    // Let interpreter handle the bad call.
    if (callee->rxLevel() < minReqCalleeLevel) PUNT(FCall-RxViolation);
    return;
  }

  ifThen(
    env,
    [&] (Block* taken) {
      auto const func = ldPreLiveFunc(env, actRecOff);
      auto const calleeLevel = gen(env, LdFuncRxLevel, func);
      auto const lt = gen(env, LtInt, calleeLevel, cns(env, minReqCalleeLevel));
      gen(env, JmpNZero, taken, lt);
    },
    [&] {
      hint(env, Block::Hint::Unlikely);
      gen(env, Jmp, makeExitSlow(env));
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
  emitCallerDynamicCallChecks(env, callee, actRecOff);
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

  auto const needsCallerFrame = callee
    ? funcNeedsCallerFrame(callee)
    : callNeedsCallerFrame(
      *env.currentNormalizedInstruction,
      curFunc(env)
    );
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
        needsCallerFrame,
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
  fsetActRec(
    env,
    cns(env, callee),
    cns(env, TNullptr),
    numParams,
    nullptr,
    false,
    nullptr
  );
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
