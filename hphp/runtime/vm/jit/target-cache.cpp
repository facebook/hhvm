/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/runtime/vm/jit/target-cache.h"

#include <cassert>
#include <string>
#include <vector>
#include <mutex>

#include "hphp/runtime/base/complex-types.h"
#include "hphp/runtime/base/stats.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/runtime-error.h"
#include "hphp/runtime/vm/jit/translator-inline.h"

namespace HPHP { namespace JIT {

using namespace HPHP::MethodLookup;

TRACE_SET_MOD(targetcache);

//////////////////////////////////////////////////////////////////////

namespace {

const StaticString s_call("__call");

inline bool stringMatches(const StringData* rowString, const StringData* sd) {
  return rowString &&
    (rowString == sd ||
     rowString->data() == sd->data() ||
     (rowString->hash() == sd->hash() &&
      rowString->same(sd)));
}

template<class T = void>
T* handleToPtr(RDS::Handle h) {
  return (T*)((char*)RDS::tl_base + h);
}

template<class Cache>
typename Cache::Pair* keyToPair(Cache* cache, const StringData* k) {
  assert(Util::isPowerOfTwo(Cache::kNumLines));
  return cache->m_pairs + (k->hash() & (Cache::kNumLines - 1));
}

}

//////////////////////////////////////////////////////////////////////
// FuncCache

// Set of FuncCache handles for dynamic function callsites, used for
// invalidation when a function is renamed.
static std::mutex funcCacheMutex;
static std::vector<RDS::Link<FuncCache> > funcCacheEntries;

RDS::Handle FuncCache::alloc() {
  auto const link = RDS::alloc<FuncCache,sizeof(Pair)>();
  std::lock_guard<std::mutex> g(funcCacheMutex);
  funcCacheEntries.push_back(link);
  return link.handle();
}

const Func* FuncCache::lookup(RDS::Handle handle, StringData* sd) {
  Func* func;
  auto const thiz = handleToPtr<FuncCache>(handle);
  auto const pair = keyToPair(thiz, sd);
  const StringData* pairSd = pair->m_key;
  if (!stringMatches(pairSd, sd)) {
    // Miss. Does it actually exist?
    func = Unit::lookupFunc(sd);
    if (UNLIKELY(!func)) {
      Transl::VMRegAnchor _;
      func = Unit::loadFunc(sd);
      if (!func) {
        raise_error("Undefined function: %s", sd->data());
      }
    }
    func->validate();
    pair->m_key = const_cast<StringData*>(func->name()); // use a static name
    pair->m_value = func;
  }
  // DecRef the string here; more compact than doing so in callers.
  decRefStr(sd);
  assert(stringMatches(pair->m_key, pair->m_value->name()));
  pair->m_value->validate();
  return pair->m_value;
}

void invalidateForRenameFunction(const StringData* name) {
  assert(name);
  std::lock_guard<std::mutex> g(funcCacheMutex);
  for (auto& h : funcCacheEntries) {
    memset(h.get(), 0, sizeof *h);
  }
}

//////////////////////////////////////////////////////////////////////
// ClassCache

RDS::Handle ClassCache::alloc() {
  return RDS::alloc<ClassCache,sizeof(Pair)>().handle();
}

const Class* ClassCache::lookup(RDS::Handle handle, StringData* name) {
  auto const thiz = handleToPtr<ClassCache>(handle);
  auto const pair = keyToPair(thiz, name);
  const StringData* pairSd = pair->m_key;
  if (!stringMatches(pairSd, name)) {
    TRACE(1, "ClassCache miss: %s\n", name->data());
    const NamedEntity *ne = Unit::GetNamedEntity(name);
    Class *c = Unit::lookupClass(ne);
    if (UNLIKELY(!c)) {
      String normName = normalizeNS(name);
      if (normName) {
        return lookup(handle, normName.get());
      } else {
        c = Unit::loadMissingClass(ne, name);
      }
      if (UNLIKELY(!c)) {
        raise_error(Strings::UNKNOWN_CLASS, name->data());
      }
    }

    if (pair->m_key) decRefStr(pair->m_key);
    pair->m_key = name;
    name->incRefCount();
    pair->m_value = c;
  } else {
    TRACE(1, "ClassCache hit: %s\n", name->data());
  }
  return pair->m_value;
}

//=============================================================================
// MethodCache

/*
 * We have a call site for an object method, which previously invoked
 * func, but this call has a different Class (cls).  See if we can
 * figure out the correct Func to call.
 */
static inline const Func* wouldCall(const Class* cls, const Func* prev) {
  if (LIKELY(cls->numMethods() > prev->methodSlot())) {
    const Func* cand = cls->methods()[prev->methodSlot()];
    /* If this class has the same func at the same method slot
       we're good to go. No need to recheck permissions,
       since we already checked them first time around */
    if (LIKELY(cand == prev)) return cand;
    if (prev->attrs() & AttrPrivate) {
      /* If the previously called function was private, then
         the context class must be prev->cls() - so its
         definitely accessible. So if this derives from
         prev->cls() its the function that would be picked.
         Note that we can only get here if there is a same
         named function deeper in the class hierarchy */
      if (cls->classof(prev->cls())) return prev;
    }
    if (cand->name() == prev->name()) {
      /*
       * We have the same name - so its probably the right function.
       * If its not public, check that both funcs were originally
       * defined in the same base class.
       */
      if ((cand->attrs() & AttrPublic) ||
          cand->baseCls() == prev->baseCls()) {
        return cand;
      }
    }
  }
  return nullptr;
}

HOT_FUNC_VM
void methodCacheSlowPath(MethodCache* mce,
                         ActRec* ar,
                         StringData* name,
                         Class* cls) {
  assert(ar->hasThis());
  assert(ar->getThis()->getVMClass() == cls);
  assert(IMPLIES(mce->m_key, mce->m_value));

  try {
    bool isMagicCall = mce->m_key & 0x1u;
    bool isStatic;
    const Func* func;

    auto* storedClass = reinterpret_cast<Class*>(mce->m_key & ~0x3u);
    if (storedClass == cls) {
      isStatic = mce->m_key & 0x2u;
      func = mce->m_value;
    } else {
      if (LIKELY(storedClass != nullptr &&
                 ((func = wouldCall(cls, mce->m_value)) != nullptr) &&
                 !isMagicCall)) {
        Stats::inc(Stats::TgtCache_MethodHit, func != nullptr);
        isMagicCall = false;
      } else {
        Class* ctx = arGetContextClass((ActRec*)ar->m_savedRbp);
        Stats::inc(Stats::TgtCache_MethodMiss);
        TRACE(2, "MethodCache: miss class %p name %s!\n", cls, name->data());
        auto const& objMethod = MethodLookup::CallType::ObjMethod;
        func = g_vmContext->lookupMethodCtx(cls, name, ctx, objMethod, false);
        if (UNLIKELY(!func)) {
          isMagicCall = true;
          func = cls->lookupMethod(s_call.get());
          if (UNLIKELY(!func)) {
            // Do it again, but raise the error this time.
            (void) g_vmContext->lookupMethodCtx(cls, name, ctx, objMethod,
                                                true);
            NOT_REACHED();
          }
        } else {
          isMagicCall = false;
        }
      }

      isStatic = func->attrs() & AttrStatic;

      mce->m_key = uintptr_t(cls) | (uintptr_t(isStatic) << 1) |
        uintptr_t(isMagicCall);
      mce->m_value = func;
    }

    assert(func);
    func->validate();
    ar->m_func = func;

    if (UNLIKELY(isStatic && !func->isClosureBody())) {
      decRefObj(ar->getThis());
      if (debug) ar->setThis(nullptr); // suppress assert in setClass
      ar->setClass(cls);
    }

    assert(!ar->hasVarEnv() && !ar->hasInvName());
    if (UNLIKELY(isMagicCall)) {
      ar->setInvName(name);
      assert(name->isStatic()); // No incRef needed.
    }
  } catch (...) {
    /*
     * Barf.
     *
     * If the slow lookup fails, we're going to rewind to the state
     * before the FPushObjMethodD that dumped us here. In this state,
     * the object is still on the stack, but for efficiency reasons,
     * we've smashed this TypedValue* with the ActRec we were trying
     * to push.
     *
     * Reconstitute the virtual object before rethrowing.
     */
    TypedValue* shouldBeObj = reinterpret_cast<TypedValue*>(ar) +
      kNumActRecCells - 1;
    ObjectData* arThis = ar->getThis();
    shouldBeObj->m_type = KindOfObject;
    shouldBeObj->m_data.pobj = arThis;

    // There used to be a half-built ActRec on the stack that we need the
    // unwinder to ignore. We overwrote 1/3 of it with the code above, but
    // because of the emitMarker() in LdObjMethod we need the other two slots
    // to not have any TypedValues.
    tvWriteNull(shouldBeObj - 1);
    tvWriteNull(shouldBeObj - 2);

    throw;
  }
}

//=============================================================================
// *SPropCache
//

RDS::Handle SPropCache::alloc(const StringData* sd) {
  assert(sd->isStatic());
  return RDS::bind<SPropCache>(
    RDS::StaticProp { sd }
  ).handle();
}

template<bool raiseOnError>
TypedValue*
SPropCache::lookupSProp(const Class *cls, const StringData *name, Class* ctx) {
  bool visible, accessible;
  TypedValue* val;
  val = cls->getSProp(ctx, name, visible, accessible);
  if (UNLIKELY(!visible || !accessible)) {
    if (!raiseOnError) return NULL;
    std::string propertyName;
    Util::string_printf(propertyName, "%s::%s",
                  cls->name()->data(), name->data());
    raise_error("Invalid static property access: %s", propertyName.c_str());
  }
  return val;
}

template TypedValue* SPropCache::lookupSProp<true>(const Class *cls,
                                                   const StringData *name,
                                                   Class* ctx);

template TypedValue* SPropCache::lookupSProp<false>(const Class *cls,
                                                    const StringData *name,
                                                    Class* ctx);

template<bool raiseOnError>
TypedValue*
SPropCache::lookup(RDS::Handle handle, const Class *cls, const StringData *name,
                   Class* ctx) {
  // The fast path is in-TC. If we get here, we have already missed.
  auto const thiz = handleToPtr<SPropCache>(handle);
  Stats::inc(Stats::TgtCache_SPropMiss);
  Stats::inc(Stats::TgtCache_SPropHit, -1);
  assert(cls && name);
  assert(!thiz->m_tv);
  TRACE(3, "SPropCache miss: %s::$%s\n", cls->name()->data(),
        name->data());
  TypedValue* val = lookupSProp<raiseOnError>(cls, name, ctx);
  if (!val) {
    assert(!raiseOnError);
    return NULL;
  }
  thiz->m_tv = val;
  TRACE(3, "SPropCache::lookup(\"%s::$%s\") %p -> %p t%d\n",
        cls->name()->data(),
        name->data(),
        val,
        val->m_data.pref,
        val->m_type);
  assert(val->m_type >= MinDataType && val->m_type < MaxNumDataTypes);
  return val;
}

template TypedValue* SPropCache::lookup<true>(RDS::Handle handle,
                                              const Class *cls,
                                              const StringData *name,
                                              Class* ctx);

template TypedValue* SPropCache::lookup<false>(RDS::Handle handle,
                                               const Class *cls,
                                               const StringData *name,
                                               Class* ctx);

//=============================================================================
// StaticMethodCache
//

static const StringData* mangleSmcName(const StringData* cls,
                                       const StringData* meth,
                                       const char* ctx) {
  // Implementation detail of FPushClsMethodD/F: we use "C::M:ctx" as
  // the key for invoking static method "M" on class "C". This
  // composes such a key. "::" is semi-arbitrary, though whatever we
  // choose must delimit possible class and method names, so we might
  // as well ape the source syntax
  return
    makeStaticString(String(cls->data()) + String("::") +
                     String(meth->data()) + String(":") +
                     String(ctx));
}

RDS::Handle StaticMethodCache::alloc(const StringData* clsName,
                                const StringData* methName,
                                const char* ctxName) {
  return RDS::bind<StaticMethodCache>(
    RDS::StaticMethod { mangleSmcName(clsName, methName, ctxName) }
  ).handle();
}

RDS::Handle StaticMethodFCache::alloc(const StringData* clsName,
                                 const StringData* methName,
                                 const char* ctxName) {
  return RDS::bind<StaticMethodFCache>(
    RDS::StaticMethodF { mangleSmcName(clsName, methName, ctxName) }
  ).handle();
}

const Func*
StaticMethodCache::lookupIR(RDS::Handle handle, const NamedEntity *ne,
                            const StringData* clsName,
                            const StringData* methName, TypedValue* vmfp,
                            TypedValue* vmsp) {
  StaticMethodCache* thiz = static_cast<StaticMethodCache*>
    (handleToPtr(handle));
  Stats::inc(Stats::TgtCache_StaticMethodMiss);
  Stats::inc(Stats::TgtCache_StaticMethodHit, -1);
  TRACE(1, "miss %s :: %s caller %p\n",
        clsName->data(), methName->data(), __builtin_return_address(0));

  ActRec* ar = reinterpret_cast<ActRec*>(vmsp - kNumActRecCells);
  const Func* f;
  VMExecutionContext* ec = g_vmContext;
  const Class* cls = Unit::loadClass(ne, clsName);
  if (UNLIKELY(!cls)) {
    raise_error(Strings::UNKNOWN_CLASS, clsName->data());
  }
  LookupResult res = ec->lookupClsMethod(f, cls, methName,
                                         nullptr, // there may be an active this,
                                               // but we can just fall through
                                               // in that case.
                                         arGetContextClass((ActRec*)vmfp),
                                         false /*raise*/);
  if (LIKELY(res == LookupResult::MethodFoundNoThis &&
             !f->isAbstract() &&
             f->isStatic())) {
    f->validate();
    TRACE(1, "fill %s :: %s -> %p\n", clsName->data(),
          methName->data(), f);
    // Do the | here instead of on every call.
    thiz->m_cls = (Class*)(uintptr_t(cls) | 1);
    thiz->m_func = f;
    ar->setClass(const_cast<Class*>(cls));
    return f;
  }
  assert(res != LookupResult::MethodFoundWithThis); // Not possible: no this.

  // Indicate to the IR that it should take even slower path
  return nullptr;
}

const Func*
StaticMethodCache::lookup(RDS::Handle handle, const NamedEntity *ne,
                          const StringData* clsName,
                          const StringData* methName) {
  StaticMethodCache* thiz = static_cast<StaticMethodCache*>
    (handleToPtr(handle));
  Stats::inc(Stats::TgtCache_StaticMethodMiss);
  Stats::inc(Stats::TgtCache_StaticMethodHit, -1);
  TRACE(1, "miss %s :: %s caller %p\n",
        clsName->data(), methName->data(), __builtin_return_address(0));
  Transl::VMRegAnchor _; // needed for lookupClsMethod.

  ActRec* ar = reinterpret_cast<ActRec*>(vmsp() - kNumActRecCells);
  const Func* f;
  VMExecutionContext* ec = g_vmContext;
  const Class* cls = Unit::loadClass(ne, clsName);
  if (UNLIKELY(!cls)) {
    raise_error(Strings::UNKNOWN_CLASS, clsName->data());
  }
  LookupResult res = ec->lookupClsMethod(f, cls, methName,
                                         nullptr, // there may be an active this,
                                               // but we can just fall through
                                               // in that case.
                                         arGetContextClass(ec->getFP()),
                                         false /*raise*/);
  if (LIKELY(res == LookupResult::MethodFoundNoThis &&
             !f->isAbstract() &&
             f->isStatic())) {
    f->validate();
    TRACE(1, "fill %s :: %s -> %p\n", clsName->data(),
          methName->data(), f);
    // Do the | here instead of on every call.
    thiz->m_cls = (Class*)(uintptr_t(cls) | 1);
    thiz->m_func = f;
    ar->setClass(const_cast<Class*>(cls));
    return f;
  }
  assert(res != LookupResult::MethodFoundWithThis); // Not possible: no this.
  // We've already sync'ed regs; this is some hard case, we might as well
  // just let the interpreter handle this entirely.
  assert(toOp(*vmpc()) == OpFPushClsMethodD);
  Stats::inc(Stats::Instr_InterpOneFPushClsMethodD);
  Stats::inc(Stats::Instr_TC, -1);
  ec->opFPushClsMethodD();
  // Return whatever func the instruction produced; if nothing was
  // possible we'll either have fataled or thrown.
  assert(ar->m_func);
  ar->m_func->validate();
  // Don't update the cache; this case was too scary to memoize.
  TRACE(1, "unfillable miss %s :: %s -> %p\n", clsName->data(),
        methName->data(), ar->m_func);
  // Indicate to the caller that there is no work to do.
  return nullptr;
}

const Func*
StaticMethodFCache::lookupIR(RDS::Handle handle, const Class* cls,
                             const StringData* methName, TypedValue* vmfp) {
  assert(cls);
  StaticMethodFCache* thiz = static_cast<StaticMethodFCache*>
    (handleToPtr(handle));
  Stats::inc(Stats::TgtCache_StaticMethodFMiss);
  Stats::inc(Stats::TgtCache_StaticMethodFHit, -1);

  const Func* f;
  VMExecutionContext* ec = g_vmContext;
  LookupResult res = ec->lookupClsMethod(f, cls, methName,
                                         nullptr,
                                         arGetContextClass((ActRec*)vmfp),
                                         false /*raise*/);
  assert(res != LookupResult::MethodFoundWithThis); // Not possible: no this.
  if (LIKELY(res == LookupResult::MethodFoundNoThis && !f->isAbstract())) {
    // We called lookupClsMethod with a NULL this and got back a
    // method that may or may not be static. This implies that
    // lookupClsMethod, given the same class and the same method name,
    // will never return MagicCall*Found or MethodNotFound. It will
    // always return the same f and if we do give it a this it will
    // return MethodFoundWithThis iff (this->instanceof(cls) &&
    // !f->isStatic()). this->instanceof(cls) is always true for
    // FPushClsMethodF because it is only used for self:: and parent::
    // calls. So, if we store f and its staticness we can handle calls
    // with and without this completely in assembly.
    f->validate();
    thiz->m_func = f;
    thiz->m_static = f->isStatic();
    TRACE(1, "fill staticfcache %s :: %s -> %p\n",
          cls->name()->data(), methName->data(), f);
    Stats::inc(Stats::TgtCache_StaticMethodFFill);
    return f;
  }

  return nullptr;
}

//////////////////////////////////////////////////////////////////////

}}

