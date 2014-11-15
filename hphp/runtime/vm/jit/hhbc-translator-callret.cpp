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
#include "hphp/runtime/vm/jit/hhbc-translator.h"

#include <cstdint>

#include "hphp/util/trace.h"
#include "hphp/runtime/vm/jit/punt.h"
#include "hphp/runtime/vm/jit/mc-generator.h"
#include "hphp/runtime/ext/asio/wait_handle.h"
#include "hphp/runtime/ext/ext_generator.h"

namespace HPHP { namespace jit {

TRACE_SET_MOD(hhir);

namespace {

//////////////////////////////////////////////////////////////////////

const StaticString s_self("self");
const StaticString s_parent("parent");
const StaticString s_static("static");

const Func* findCuf(Op op,
                    SSATmp* callable,
                    Class* ctx,
                    Class*& cls,
                    StringData*& invName,
                    bool& forward) {
  cls = nullptr;
  invName = nullptr;

  const StringData* str =
    callable->isA(Type::Str) && callable->isConst() ? callable->strVal()
                                                    : nullptr;
  const ArrayData* arr =
    callable->isA(Type::Arr) && callable->isConst() ? callable->arrVal()
                                                    : nullptr;

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
    cls = Unit::lookupUniqueClass(sclass);
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

}

//////////////////////////////////////////////////////////////////////

void HhbcTranslator::emitFPushCufIter(int32_t numParams,
                                      int32_t itId) {
  auto sp = spillStack();
  m_fpiStack.emplace(sp, m_irb->spOffset());
  gen(CufIterSpillFrame,
      FPushCufData(numParams, itId),
      sp, m_irb->fp());
}

bool HhbcTranslator::emitFPushCufArray(SSATmp* callable, int32_t numParams) {
  if (!callable->isA(Type::Arr)) return false;

  auto callableInst = callable->inst();
  if (!callableInst->is(NewPackedArray)) return false;

  auto callableSize = callableInst->src(0);
  if (!callableSize->isConst() ||
      callableSize->intVal() != 2) {
    return false;
  }

  auto method = getStackValue(m_irb->sp(), 0).value;
  auto object = getStackValue(m_irb->sp(), 1).value;
  if (!method || !object) return false;

  if (!method->isConst(Type::Str) ||
      strstr(method->strVal()->data(), "::") != nullptr) {
    return false;
  }

  if (!object->isA(Type::Obj)) {
    if (!object->type().equals(Type::Cell)) return false;
    // This is probably an object, and we just haven't guarded on
    // the type.  Do so now.
    auto exit = makeExit();
    object = gen(CheckType, Type::Obj, exit, object);
  }
  m_irb->constrainValue(object, DataTypeSpecific);

  popC();

  gen(IncRef, object);
  emitFPushObjMethodCommon(object,
                           method->strVal(),
                           numParams,
                           false /* shouldFatal */,
                           callable);
  gen(DecRef, callable);
  return true;
}

// FPushCuf when the callee is not known at compile time.
void HhbcTranslator::emitFPushCufUnknown(Op op, int32_t numParams) {
  if (op != Op::FPushCuf) {
    PUNT(emitFPushCufUnknown-nonFPushCuf);
  }

  if (topC()->isA(Type::Obj)) {
    return emitFPushFuncObj(numParams);
  }

  if (!topC()->type().subtypeOfAny(Type::Arr, Type::Str)) {
    PUNT(emitFPushCufUnknown);
  }

  // Peek at the top of the stack before deciding to pop it.
  auto const callable = topC();
  if (emitFPushCufArray(callable, numParams)) return;

  popC();

  emitFPushActRec(
    cns(Type::Nullptr),
    cns(Type::Nullptr),
    numParams,
    nullptr
  );
  auto const actRec = spillStack();

  /*
   * This is a similar case to lookup for functions in FPushFunc or
   * FPushObjMethod.  We can throw in a weird situation where the
   * ActRec is already on the stack, but this bytecode isn't done
   * executing yet.  See arPreliveOverwriteCells for details about why
   * we need this marker.
   */
  updateMarker();

  auto const opcode = callable->isA(Type::Arr) ? LdArrFPushCuf
                                               : LdStrFPushCuf;
  gen(opcode, makeCatch({callable}, 1), callable, actRec, m_irb->fp());
  gen(DecRef, callable);
}


void HhbcTranslator::emitFPushCufOp(Op op, int32_t numArgs) {
  const bool safe = op == OpFPushCufSafe;
  bool forward = op == OpFPushCufF;
  SSATmp* callable = topC(safe ? 1 : 0);

  Class* cls = nullptr;
  StringData* invName = nullptr;
  auto const callee = findCuf(op, callable, curClass(), cls, invName, forward);
  if (!callee) return emitFPushCufUnknown(op, numArgs);

  SSATmp* ctx;
  SSATmp* safeFlag = cns(true); // This is always true until the slow exits
                                // below are implemented
  SSATmp* func = cns(callee);
  if (cls) {
    auto const exitSlow = makeExitSlow();
    if (!RDS::isPersistentHandle(cls->classHandle())) {
      // The miss path is complicated and rare.  Punt for now.  This
      // must be checked before we IncRef the context below, because
      // the slow exit will want to do that same IncRef via InterpOne.
      gen(LdClsCachedSafe, exitSlow, cns(cls->name()));
    }

    if (forward) {
      ctx = gen(LdCtx, FuncData(curFunc()), m_irb->fp());
      ctx = gen(GetCtxFwdCall, ctx, cns(callee));
    } else {
      ctx = genClsMethodCtx(callee, cls);
    }
  } else {
    ctx = cns(Type::Nullptr);
    if (!RDS::isPersistentHandle(callee->funcHandle())) {
      // The miss path is complicated and rare. Punt for now.
      func = gen(
        LdFuncCachedSafe, LdFuncCachedData(callee->name()), makeExitSlow()
      );
    }
  }

  SSATmp* defaultVal = safe ? popC() : nullptr;
  popDecRef(Type::Cell); // callable
  if (safe) {
    push(defaultVal);
    push(safeFlag);
  }

  emitFPushActRec(func, ctx, numArgs, invName);
}

void HhbcTranslator::emitFPushActRec(SSATmp* func,
                                     SSATmp* objOrClass,
                                     int32_t numArgs,
                                     const StringData* invName) {
  /*
   * Before allocating an ActRec, we do a spillStack so we'll have a
   * StkPtr that represents what the stack will look like after the
   * ActRec is popped.
   */
  auto actualStack = spillStack();
  auto returnSp = actualStack;

  m_fpiStack.emplace(returnSp, m_irb->spOffset());

  ActRecInfo info;
  info.numArgs = numArgs;
  info.invName = invName;
  gen(
    SpillFrame,
    info,
    // Using actualStack instead of returnSp so SpillFrame still gets
    // the src in rVmSp.  (TODO(#2288359).)
    actualStack,
    func,
    objOrClass
  );
  assert(m_irb->stackDeficit() == 0);
}

void HhbcTranslator::emitFPushCtorCommon(SSATmp* cls,
                                         SSATmp* obj,
                                         const Func* func,
                                         int32_t numParams) {
  push(obj);
  auto const fn = [&] {
    if (func) return cns(func);
    /*
      Without the updateMarker, the catch trace will write
      obj onto the stack, but the VMRegAnchor will setup the
      stack as it was before the FPushCtor*, which (for
      FPushCtorD at least) won't include obj
    */
    updateMarker();
    return gen(LdClsCtor, makeCatch(), cls);
  }();
  gen(IncRef, obj);
  auto numArgsAndFlags = ActRec::encodeNumArgs(numParams, false, false, true);
  emitFPushActRec(fn, obj, numArgsAndFlags, nullptr);
}

void HhbcTranslator::emitFPushCtor(int32_t numParams) {
  auto catchBlock = makeCatch();
  SSATmp* cls = popA();
  SSATmp* obj = gen(AllocObj, catchBlock, cls);
  gen(IncRef, obj);
  emitFPushCtorCommon(cls, obj, nullptr, numParams);
}

void HhbcTranslator::emitFPushCtorD(int32_t numParams, int32_t classNameStrId) {
  const StringData* className = lookupStringId(classNameStrId);

  const Class* cls = Unit::lookupUniqueClass(className);
  bool uniqueCls = classIsUnique(cls);
  bool persistentCls = classHasPersistentRDS(cls);
  bool canInstantiate = canInstantiateClass(cls);
  bool fastAlloc =
    persistentCls &&
    canInstantiate &&
    !cls->callsCustomInstanceInit();

  const Func* func = uniqueCls ? cls->getCtor() : nullptr;
  if (func && !(func->attrs() & AttrPublic)) {
    Class* ctx = curClass();
    if (!ctx) {
      func = nullptr;
    } else if (ctx != cls) {
      if ((func->attrs() & AttrPrivate) ||
        !(ctx->classof(cls) || cls->classof(ctx))) {
        func = nullptr;
      }
    }
  }

  auto ssaCls = persistentCls ? cns(cls)
                              : gen(LdClsCached, makeCatch(), cns(className));
  if (!ssaCls->isConst() && uniqueCls) {
    // If the Class is unique but not persistent, it's safe to use it as a
    // const after the LdClsCached, which will throw if the class can't be
    // defined.
    ssaCls = cns(cls);
  }

  auto const obj = fastAlloc ? emitAllocObjFast(cls)
                             : gen(AllocObj, makeCatch(), ssaCls);
  gen(IncRef, obj);
  emitFPushCtorCommon(ssaCls, obj, func, numParams);
}

void HhbcTranslator::emitFPushFuncCommon(const Func* func,
                                         const StringData* name,
                                         const StringData* fallback,
                                         int32_t numParams) {
  if (func) {
    func->validate();
    if (func->isNameBindingImmutable(curUnit())) {
      emitFPushActRec(cns(func),
                      cns(Type::Nullptr),
                      numParams,
                      nullptr);
      return;
    }
  }

  // LdFuncCached can throw
  auto const catchBlock = makeCatch();
  auto const ssaFunc = fallback
    ? gen(LdFuncCachedU,
          LdFuncCachedUData { name, fallback },
          catchBlock)
    : gen(LdFuncCached,
          LdFuncCachedData { name },
          catchBlock);
  emitFPushActRec(ssaFunc,
                  cns(Type::Nullptr),
                  numParams,
                  nullptr);
}

void HhbcTranslator::emitFPushFuncD(int32_t numParams, int32_t funcId) {
  const NamedEntityPair& nep = lookupNamedEntityPairId(funcId);
  const StringData* name = nep.first;
  const Func* func       = Unit::lookupFunc(nep.second);
  emitFPushFuncCommon(func, name, nullptr, numParams);
}

void HhbcTranslator::emitFPushFuncU(int32_t numParams,
                                    int32_t funcId,
                                    int32_t fallbackFuncId) {
  const NamedEntityPair& nep = lookupNamedEntityPairId(funcId);
  const StringData* name     = nep.first;
  const Func* func           = Unit::lookupFunc(nep.second);
  const NamedEntityPair& fallbackNep = lookupNamedEntityPairId(fallbackFuncId);
  const StringData* fallbackName     = fallbackNep.first;
  emitFPushFuncCommon(func, name, fallbackName, numParams);
}

void HhbcTranslator::emitFPushFunc(int32_t numParams) {
  if (topC()->isA(Type::Obj)) {
    return emitFPushFuncObj(numParams);
  }

  if (topC()->isA(Type::Arr)) {
    return emitFPushFuncArr(numParams);
  }

  if (!topC()->isA(Type::Str)) {
    PUNT(FPushFunc_not_Str);
  }

  auto const catchBlock = makeCatch();
  auto const funcName = popC();
  emitFPushActRec(gen(LdFunc, catchBlock, funcName),
                  cns(Type::Nullptr),
                  numParams,
                  nullptr);
}

void HhbcTranslator::emitFPushFuncObj(int32_t numParams) {
  auto const slowExit = makeExitSlow();
  auto const obj      = popC();
  auto const cls      = gen(LdObjClass, obj);
  auto const func     = gen(LdObjInvoke, slowExit, cls);
  emitFPushActRec(func, obj, numParams, nullptr);
}

void HhbcTranslator::emitFPushFuncArr(int32_t numParams) {
  auto const thisAR = m_irb->fp();

  auto const arr    = popC();
  emitFPushActRec(
    cns(Type::Nullptr),
    cns(Type::Nullptr),
    numParams,
    nullptr);
  auto const actRec = spillStack();

  // This is special. We need to move the stackpointer incase LdArrFuncCtx
  // calls a destructor. Otherwise it would clobber the ActRec we just
  // pushed.
  updateMarker();

  gen(LdArrFuncCtx, makeCatch({arr}, 1), arr, actRec, thisAR);
  gen(DecRef, arr);
}

void HhbcTranslator::emitFPushObjMethodCommon(SSATmp* obj,
                                              const StringData* methodName,
                                              int32_t numParams,
                                              bool shouldFatal,
                                              SSATmp* extraSpill) {
  SSATmp* objOrCls = obj;
  const Class* baseClass = nullptr;
  if (obj->type().isSpecialized()) {
    auto cls = obj->type().getClass();
    if (!m_irb->constrainValue(obj, TypeConstraint(cls).setWeak())) {
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
                                           curClass());

  if (!func) {
    if (baseClass && !(baseClass->attrs() & AttrInterface)) {
      LookupResult res =
        g_context->lookupObjMethod(func, baseClass, methodName, curClass(),
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
          auto const clsTmp = gen(LdObjClass, obj);
          auto const funcTmp = gen(
            LdClsMethod, clsTmp, cns(-(func->methodSlot() + 1))
          );
          if (res == LookupResult::MethodFoundNoThis) {
            gen(DecRef, obj);
            objOrCls = clsTmp;
          }
          emitFPushActRec(funcTmp, objOrCls, numParams,
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
      assert(baseClass);
      gen(DecRef, obj);
      objOrCls = cns(baseClass);
    }
    emitFPushActRec(cns(func),
                    objOrCls,
                    numParams,
                    magicCall ? methodName : nullptr);
    return;
  }

  fpushObjMethodUnknown(obj, methodName, numParams, shouldFatal, extraSpill);
}

// Pushing for object method when we don't know the Func* statically.
void HhbcTranslator::fpushObjMethodUnknown(SSATmp* obj,
                                           const StringData* methodName,
                                           int32_t numParams,
                                           bool shouldFatal,
                                           SSATmp* extraSpill) {
  spillStack();
  emitFPushActRec(cns(Type::Nullptr),  // Will be set by LdObjMethod
                  obj,
                  numParams,
                  nullptr);
  auto const actRec = spillStack();
  auto const objCls = gen(LdObjClass, obj);

  // This is special. We need to move the stackpointer in case
  // LdObjMethod calls a destructor. Otherwise it would clobber the
  // ActRec we just pushed.
  updateMarker();
  Block* catchBlock;
  if (extraSpill) {
    /*
     * If LdObjMethod throws, it nulls out the ActRec (since the unwinder
     * will attempt to destroy it as if it were cells), and then writes
     * obj into the last entry, since we need it to be destroyed.
     * If we have another object to destroy, we should write it in
     * the first - so pop 1 cell, then push extraSpill.
     */
    std::vector<SSATmp*> spill{extraSpill};
    catchBlock = makeCatch(spill, 1);
  } else {
    catchBlock = makeCatchNoSpill();
  }
  gen(LdObjMethod,
      LdObjMethodData { methodName, shouldFatal },
      catchBlock,
      objCls,
      actRec);
}

void HhbcTranslator::emitFPushObjMethodD(int32_t numParams,
                                         int32_t methodNameStrId,
                                         unsigned char subop) {
  auto const obj = popC();
  if (!obj->isA(Type::Obj)) PUNT(FPushObjMethodD-nonObj);
  auto const methodName = lookupStringId(methodNameStrId);
  emitFPushObjMethodCommon(obj, methodName, numParams, true /* shouldFatal */);
}

SSATmp* HhbcTranslator::genClsMethodCtx(const Func* callee, const Class* cls) {
  bool mustBeStatic = true;

  if (!(callee->attrs() & AttrStatic) &&
      !(curFunc()->attrs() & AttrStatic) &&
      curClass()) {
    if (curClass()->classof(cls)) {
      // In this case, it might not be static, but we can be sure
      // we're going to forward $this if thisAvailable.
      mustBeStatic = false;
    } else if (cls->classof(curClass())) {
      // Unlike the above, we might be calling down to a subclass that
      // is not related to the current instance.  To know whether this
      // call forwards $this requires a runtime type check, so we have
      // to punt instead of trying the thisAvailable path below.
      PUNT(getClsMethodCtx-PossibleStaticRelatedCall);
    }
  }

  if (mustBeStatic) {
    return ldCls(makeCatch(), cns(cls->name()));
  }
  if (m_irb->thisAvailable()) {
    // might not be a static call and $this is available, so we know it's
    // definitely not static
    assert(curClass());
    auto this_ = gen(LdThis, m_irb->fp());
    gen(IncRef, this_);
    return this_;
  }
  // might be a non-static call. we have to inspect the func at runtime
  PUNT(getClsMethodCtx-MightNotBeStatic);
}

void HhbcTranslator::emitFPushClsMethodD(int32_t numParams,
                                         int32_t methodNameStrId,
                                         int32_t clssNamedEntityPairId) {

  auto const methodName = lookupStringId(methodNameStrId);
  auto const& np        = lookupNamedEntityPairId(clssNamedEntityPairId);
  auto const className  = np.first;
  auto const baseClass  = Unit::lookupUniqueClass(np.second);
  bool magicCall        = false;

  if (auto const func = lookupImmutableMethod(baseClass,
                                              methodName,
                                              magicCall,
                                              true /* staticLookup */,
                                              curClass())) {
    auto const objOrCls = genClsMethodCtx(func, baseClass);
    emitFPushActRec(cns(func),
                    objOrCls,
                    numParams,
                    func && magicCall ? methodName : nullptr);
    return;
  }

  auto const slowExit = makeExitSlow();
  auto const data = ClsMethodData{className, methodName, np.second};

  // Look up the Func* in the targetcache. If it's not there, try the slow
  // path. If that fails, slow exit.
  auto const func = m_irb->cond(
    0,
    [&] (Block* taken) {
      return gen(CheckNonNull, taken, gen(LdClsMethodCacheFunc, data));
    },
    [&] (SSATmp* func) { // next
      return func;
    },
    [&] { // taken
      m_irb->hint(Block::Hint::Unlikely);
      auto result = gen(LookupClsMethodCache, makeCatch(), data,
                        m_irb->fp());
      return gen(CheckNonNull, slowExit, result);
    }
  );
  auto const clsCtx = gen(LdClsMethodCacheCls, data);

  emitFPushActRec(func,
                  clsCtx,
                  numParams,
                  nullptr);
}

void HhbcTranslator::emitFPushClsMethod(int32_t numParams) {
  auto const clsVal  = popA();
  auto const methVal = popC();

  if (!methVal->isA(Type::Str) || !clsVal->isA(Type::Cls)) {
    PUNT(FPushClsMethod-unknownType);
  }

  if (methVal->isConst()) {
    const Class* cls = nullptr;
    if (clsVal->isConst()) {
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
      cls = curClass();
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
        auto funcTmp = clsVal->isConst()
          ? cns(func)
          : gen(LdClsMethod, clsVal, cns(-(func->methodSlot() + 1)));
        emitFPushActRec(funcTmp, clsVal, numParams, nullptr);
        return;
      }
    }
  }

  emitFPushActRec(cns(Type::Nullptr),
                  cns(Type::Nullptr),
                  numParams,
                  nullptr);
  auto const actRec = spillStack();

  /*
   * Similar to FPushFunc/FPushObjMethod, we have an incomplete ActRec
   * on the stack and must handle that properly if we throw.
   */
  updateMarker();

  gen(LookupClsMethod, makeCatch({methVal, clsVal}), clsVal, methVal, actRec,
      m_irb->fp());
  gen(DecRef, methVal);
}


void HhbcTranslator::emitFPushClsMethodF(int32_t numParams) {
  auto const exitBlock = makeExitSlow();

  auto classTmp = top(Type::Cls);
  auto methodTmp = topC(1, DataTypeGeneric);
  assert(classTmp->isA(Type::Cls));
  if (!classTmp->isConst() || !methodTmp->isConst(Type::Str)) {
    PUNT(FPushClsMethodF-unknownClassOrMethod);
  }
  m_irb->constrainValue(methodTmp, DataTypeSpecific);

  auto const cls = classTmp->clsVal();
  auto const methName = methodTmp->strVal();

  bool magicCall = false;
  auto const vmfunc = lookupImmutableMethod(cls,
                                            methName,
                                            magicCall,
                                            true /* staticLookup */,
                                            curClass());
  auto const catchBlock = vmfunc ? nullptr : makeCatch();
  discard(2);

  auto const curCtxTmp = gen(LdCtx, FuncData(curFunc()), m_irb->fp());
  if (vmfunc) {
    auto const funcTmp = cns(vmfunc);
    auto const newCtxTmp = gen(GetCtxFwdCall, curCtxTmp, funcTmp);
    emitFPushActRec(funcTmp, newCtxTmp, numParams,
                    (magicCall ? methName : nullptr));
    return;
  }

  auto const data = ClsMethodData{cls->name(), methName};
  auto const funcTmp = m_irb->cond(
    0,
    [&](Block* taken) {
      return gen(CheckNonNull, taken, gen(LdClsMethodFCacheFunc, data));
    },
    [&](SSATmp* func) { // next
      return func;
    },
    [&] { // taken
      m_irb->hint(Block::Hint::Unlikely);
      auto result = gen(LookupClsMethodFCache, catchBlock, data,
                        cns(cls), m_irb->fp());
      return gen(CheckNonNull, exitBlock, result);
    }
  );
  auto const ctx = gen(GetCtxFwdCallDyn, data, curCtxTmp);

  emitFPushActRec(funcTmp,
                  ctx,
                  numParams,
                  magicCall ? methName : nullptr);
}

//////////////////////////////////////////////////////////////////////

void HhbcTranslator::emitDecodeCufIter(uint32_t iterId, int offset,
                                       JmpFlags jmpFlags) {
  auto catchBlock = makeCatch();
  SSATmp* src = popC();
  Type type = src->type();
  if (type.subtypeOfAny(Type::Arr, Type::Str, Type::Obj)) {
    SSATmp* res = gen(DecodeCufIter, Type::Bool,
                      IterId(iterId), catchBlock, src, m_irb->fp());
    gen(DecRef, src);
    jmpCondHelper(offset, true, jmpFlags, res);
  } else {
    gen(DecRef, src);
    jmpImpl(offset, JmpFlagEndsRegion);
  }
}

void HhbcTranslator::emitCIterFree(uint32_t iterId) {
  gen(CIterFree, IterId(iterId), m_irb->fp());
}

void HhbcTranslator::emitFPassL(int32_t id) {
  auto ldrefExit = makeExit();
  auto ldPMExit = makePseudoMainExit();
  pushIncRef(ldLocInnerWarn(id, ldrefExit, ldPMExit, DataTypeSpecific));
}

void HhbcTranslator::emitFPassR() {
  emitUnboxRAux();
}

void HhbcTranslator::emitFPassV() {
  Block* exit = makeExit();
  SSATmp* tmp = popV();
  pushIncRef(gen(LdRef, exit, tmp->type().innerType(), tmp));
  gen(DecRef, tmp);
}

void HhbcTranslator::emitUnboxRAux() {
  Block* exit = makeExit();
  SSATmp* srcBox = popR();
  SSATmp* unboxed = unbox(srcBox, exit);
  if (unboxed == srcBox) {
    // If the Unbox ended up being a noop, don't bother refcounting
    push(unboxed);
  } else {
    pushIncRef(unboxed);
    gen(DecRef, srcBox);
  }
}

void HhbcTranslator::emitUnboxR() {
  emitUnboxRAux();
}

//////////////////////////////////////////////////////////////////////

void HhbcTranslator::emitFCallArray(const Offset pcOffset,
                                    const Offset after,
                                    bool destroyLocals) {
  auto const stack = spillStack();
  gen(CallArray, CallArrayData { pcOffset, after, destroyLocals }, stack,
      m_irb->fp());
}

void HhbcTranslator::emitFCall(uint32_t numParams,
                               Offset returnBcOffset,
                               const Func* callee,
                               bool destroyLocals) {
  if (RuntimeOption::EvalRuntimeTypeProfile) {
    for (auto i = uint32_t{0}; i < numParams; ++i) {
      auto const val = topF(numParams - i - 1);
      if (callee != nullptr) {
        gen(TypeProfileFunc, TypeProfileData(i), val, cns(callee));
      } else  {
        auto const func = gen(LdARFuncPtr, m_irb->sp(), cns(0));
        gen(TypeProfileFunc, TypeProfileData(i), val, func);
      }
    }
  }

  /*
   * Figure out if we know where we're going already (if a prologue
   * was already generated, we don't need to do a whole bind call
   * thing again).
   *
   * We're skipping magic calls right now because 'callee' will be set
   * to __call in some cases (with 86ctor) where we shouldn't really
   * call that function (arguable bug in annotation).
   *
   * TODO(#4357498): This is currently disabled, because we haven't
   * set things up properly to be able to eagerly bind.  Because
   * code-gen can punt, the code there needs to delay adding these
   * smash locations until after we know the translation isn't punted.
   */
  auto const knownPrologue = [&]() -> TCA {
    if (false) {
      if (!callee || callee->isMagic()) return nullptr;
      auto const prologueIndex =
        numParams <= callee->numNonVariadicParams()
          ? numParams
          : callee->numNonVariadicParams() + 1;
      TCA ret;
      if (!mcg->checkCachedPrologue(callee, prologueIndex, ret)) {
        return nullptr;
      }
      return ret;
    }
    return nullptr;
  }();

  auto const sp = spillStack();
  gen(
    Call,
    CallData {
      numParams,
      returnBcOffset,
      callee,
      destroyLocals,
      knownPrologue
    },
    sp,
    m_irb->fp()
  );
  if (!m_fpiStack.empty()) {
    m_fpiStack.pop();
  }
}

//////////////////////////////////////////////////////////////////////

void HhbcTranslator::emitRetSurpriseCheck(SSATmp* fp, SSATmp* retVal,
                                          Block* catchBlock,
                                          bool suspendingResumed) {
  emitRB(Trace::RBTypeFuncExit, curFunc()->fullName());
  m_irb->ifThen([&](Block* taken) {
                 gen(CheckSurpriseFlags, taken);
               },
               [&] {
                 m_irb->hint(Block::Hint::Unlikely);
                 if (retVal != nullptr) {
                   gen(FunctionReturnHook, RetCtrlData(suspendingResumed),
                       catchBlock, fp, retVal);
                 } else {
                   gen(FunctionSuspendHook, RetCtrlData(suspendingResumed),
                       catchBlock, fp, cns(suspendingResumed));
                 }
               });
}

void HhbcTranslator::emitRetFromInlined(Type type) {
  auto const retVal = pop(type, DataTypeGeneric);
  // Before we leave the inlined frame, grab a type prediction from
  // our DefInlineFP.
  auto const retPred = m_irb->fp()->inst()->extra<DefInlineFP>()->retTypePred;
  emitEndInlinedCommon();
  push(retVal);
  if (retPred < retVal->type()) { // TODO: this if statement shouldn't
                                  // be here, because check type
                                  // resolves to the intersection of
                                  // the two types
    // If we had a predicted output type that's useful, check that here.
    checkTypeStack(0, retPred, curSrcKey().advanced().offset());
  }
}

void HhbcTranslator::emitDecRefLocalsInline() {
  for (int id = curFunc()->numLocals() - 1; id >= 0; --id) {
    gen(DecRefLoc, Type::Gen, LocalId(id), m_irb->fp());
  }
}

void HhbcTranslator::emitRet(Type type) {
  auto const func = curFunc();
  if (func->attrs() & AttrMayUseVV) {
    // Note: this has to be the first thing, because we cannot bail after
    //       we start decRefing locs because then there'll be no corresponding
    //       bytecode boundaries until the end of RetC
    gen(ReleaseVVOrExit, makeExitSlow(), m_irb->fp());
  }

  // Pop the return value. Since it will be teleported to its place in memory,
  // we don't care about the type.
  auto catchBlock = makeCatch();
  SSATmp* retVal = pop(type, func->isGenerator() ? DataTypeSpecific
                                                 : DataTypeGeneric);

  // Free local variables.  We do the decrefs inline if there are less
  // refcounted locals than a threshold.
  auto const localCount = func->numLocals();
  auto const shouldFreeInline = mcg->useLLVM() || [&]() -> bool {
    auto const count = mcg->numTranslations(m_irb->unit().context().srcKey());
    constexpr int kTooPolyRet = 6;
    if (localCount > 0 && count > kTooPolyRet) return false;
    auto numRefCounted = int{0};
    for (auto i = uint32_t{0}; i < localCount; ++i) {
      if (m_irb->localType(i, DataTypeGeneric).maybeCounted()) {
        ++numRefCounted;
      }
    }
    return numRefCounted <= RuntimeOption::EvalHHIRInliningMaxReturnDecRefs;
  }();
  if (shouldFreeInline) {
    emitDecRefLocalsInline();
    for (unsigned i = 0; i < localCount; ++i) {
      m_irb->constrainLocal(i, DataTypeCountness, "inlined RetC/V");
    }
  } else {
    gen(GenericRetDecRefs, m_irb->fp());
  }

  // Free $this.
  if (func->mayHaveThis()) {
    gen(DecRefThis, m_irb->fp());
  }

  // Call the FunctionReturn hook and put the return value on the stack so that
  // the unwinder would decref it.
  emitRetSurpriseCheck(m_irb->fp(), retVal, catchBlock, false);

  // In async function, wrap the return value into succeeded StaticWaitHandle.
  if (!resumed() && func->isAsyncFunction()) {
    retVal = gen(CreateSSWH, retVal);
  }

  // Type profile return value.
  if (RuntimeOption::EvalRuntimeTypeProfile) {
    gen(TypeProfileFunc, TypeProfileData(-1), retVal, cns(func));
  }

  SSATmp* sp;
  SSATmp* resumableObj = nullptr;
  if (!resumed()) {
    // Store the return value.
    gen(StRetVal, m_irb->fp(), retVal);

    // Free ActRec.
    sp = gen(RetAdjustStack, m_irb->fp());
  } else if (func->isAsyncFunction()) {
    // Load the parent chain.
    auto parentChain = gen(LdAsyncArParentChain, m_irb->fp());

    // Mark the async function as succeeded.
    gen(StAsyncArSucceeded, m_irb->fp());

    // Store the return value.
    gen(StAsyncArResult, m_irb->fp(), retVal);

    // Unblock parents.
    gen(ABCUnblock, parentChain);

    // Sync SP.
    sp = spillStack();

    // Get the AsyncFunctionWaitHandle.
    resumableObj = gen(LdResumableArObj, m_irb->fp());
  } else if (func->isNonAsyncGenerator()) {
    // Clear generator's key and value.
    auto const oldKey = gen(LdContArKey, Type::Cell, m_irb->fp());
    gen(StContArKey, m_irb->fp(), cns(Type::InitNull));
    gen(DecRef, oldKey);

    auto const oldValue = gen(LdContArValue, Type::Cell, m_irb->fp());
    gen(StContArValue, m_irb->fp(), cns(Type::InitNull));
    gen(DecRef, oldValue);

    // Mark generator as finished.
    gen(StContArState,
        GeneratorState { BaseGenerator::State::Done },
        m_irb->fp());

    // Push return value of next()/send()/raise().
    push(cns(Type::InitNull));

    // Sync SP.
    sp = spillStack();
  } else {
    not_reached();
  }

  // Grab caller info from ActRec.
  SSATmp* retAddr = gen(LdRetAddr, m_irb->fp());
  SSATmp* fp = gen(FreeActRec, m_irb->fp());

  // Drop reference to this resumable. The reference to the object storing
  // the frame is implicitly owned by the execution. TakeRef is used to inform
  // the refcount optimizer about this fact.
  if (resumableObj != nullptr) {
    gen(TakeRef, resumableObj);
    gen(DecRef, resumableObj);
  }

  // Return control to the caller.
  gen(RetCtrl, RetCtrlData(false), sp, fp, retAddr);
}

void HhbcTranslator::emitRetC() {
  if (curFunc()->isAsyncGenerator()) PUNT(RetC-AsyncGenerator);

  if (isInlining()) {
    assert(!resumed());
    emitRetFromInlined(Type::Cell);
  } else {
    emitRet(Type::Cell);
  }
}

void HhbcTranslator::emitRetV() {
  assert(!resumed());
  assert(!curFunc()->isResumable());
  if (isInlining()) {
    emitRetFromInlined(Type::BoxedCell);
  } else {
    emitRet(Type::BoxedCell);
  }
}

//////////////////////////////////////////////////////////////////////

}}
