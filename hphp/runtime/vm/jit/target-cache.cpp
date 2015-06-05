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
#include "hphp/runtime/vm/jit/target-cache.h"

#include <cassert>
#include <string>
#include <vector>
#include <mutex>
#include <limits>

#include "hphp/runtime/base/strings.h"
#include "hphp/runtime/base/stats.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/runtime-error.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/jit/translator-runtime.h"
#include "hphp/runtime/vm/jit/mc-generator.h"
#include "hphp/runtime/vm/jit/back-end-x64.h"
#include "hphp/runtime/vm/jit/write-lease.h"
#include "hphp/runtime/vm/treadmill.h"
#include "hphp/util/text-util.h"

namespace HPHP { namespace jit {

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
T* handleToPtr(rds::Handle h) {
  return (T*)((char*)rds::tl_base + h);
}

template<class Cache>
typename Cache::Pair* keyToPair(Cache* cache, const StringData* k) {
  assertx(folly::isPowTwo(Cache::kNumLines));
  return cache->m_pairs + (k->hash() & (Cache::kNumLines - 1));
}

}

//////////////////////////////////////////////////////////////////////
// FuncCache

// Set of FuncCache handles for dynamic function callsites, used for
// invalidation when a function is renamed.
static std::mutex funcCacheMutex;
static std::vector<rds::Link<FuncCache> > funcCacheEntries;

rds::Handle FuncCache::alloc() {
  auto const link = rds::alloc<FuncCache,sizeof(Pair)>();
  std::lock_guard<std::mutex> g(funcCacheMutex);
  funcCacheEntries.push_back(link);
  return link.handle();
}

const Func* FuncCache::lookup(rds::Handle handle, StringData* sd) {
  Func* func;
  auto const thiz = handleToPtr<FuncCache>(handle);
  auto const pair = keyToPair(thiz, sd);
  const StringData* pairSd = pair->m_key;
  if (!stringMatches(pairSd, sd)) {
    // Miss. Does it actually exist?
    func = Unit::lookupFunc(sd);
    if (UNLIKELY(!func)) {
      VMRegAnchor _;
      func = Unit::loadFunc(sd);
      if (!func) {
        raise_error("Call to undefined function %s()", sd->data());
      }
    }
    func->validate();
    pair->m_key = const_cast<StringData*>(func->name()); // use a static name
    pair->m_value = func;
  }
  // DecRef the string here; more compact than doing so in callers.
  decRefStr(sd);
  assertx(stringMatches(pair->m_key, pair->m_value->name()));
  pair->m_value->validate();
  return pair->m_value;
}

void invalidateForRenameFunction(const StringData* name) {
  assertx(name);
  std::lock_guard<std::mutex> g(funcCacheMutex);
  for (auto& h : funcCacheEntries) {
    memset(h.get(), 0, sizeof *h);
  }
}

//////////////////////////////////////////////////////////////////////
// ClassCache

rds::Handle ClassCache::alloc() {
  return rds::alloc<ClassCache,sizeof(Pair)>().handle();
}

const Class* ClassCache::lookup(rds::Handle handle, StringData* name) {
  auto const thiz = handleToPtr<ClassCache>(handle);
  auto const pair = keyToPair(thiz, name);
  const StringData* pairSd = pair->m_key;
  if (!stringMatches(pairSd, name)) {
    TRACE(1, "ClassCache miss: %s\n", name->data());
    Class* c = Unit::loadClass(name);
    if (UNLIKELY(!c)) {
      raise_error(Strings::UNKNOWN_CLASS, name->data());
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

namespace MethodCache {

namespace {
///////////////////////////////////////////////////////////////////////////////

NEVER_INLINE __attribute__((__noreturn__))
void raiseFatal(ActRec* ar, Class* cls, StringData* name, Class* ctx) {
  try {
    g_context->lookupMethodCtx(
      cls,
      name,
      ctx,
      CallType::ObjMethod,
      true // raise error
    );
    not_reached();
  } catch (...) {
    auto const obj = ar->getThis();
    *arPreliveOverwriteCells(ar) = make_tv<KindOfObject>(obj);
    throw;
  }
}

NEVER_INLINE
void nullFunc(ActRec* ar, StringData* name) {
  try {
    raise_warning("Invalid argument: function: method '%s' not found",
                  name->data());
    ar->m_func = SystemLib::s_nullFunc;
  } catch (...) {
    auto const obj = ar->getThis();
    *arPreliveOverwriteCells(ar) = make_tv<KindOfObject>(obj);
    throw;
  }
}

template<bool fatal>
NEVER_INLINE
void lookup(Entry* mce, ActRec* ar, StringData* name, Class* cls, Class* ctx) {
  auto func = g_context->lookupMethodCtx(
    cls,
    name,
    ctx,
    CallType::ObjMethod,
    false // raise error
  );

  if (UNLIKELY(!func)) {
    func = cls->lookupMethod(s_call.get());
    if (UNLIKELY(!func)) {
      if (fatal) return raiseFatal(ar, cls, name, ctx);
      return nullFunc(ar, name);
    }
    ar->setMagicDispatch(name);
    assert(!(func->attrs() & AttrStatic));
    ar->m_func   = func;
    mce->m_key   = reinterpret_cast<uintptr_t>(cls) | 0x1u;
    mce->m_value = func;
    return;
  }

  bool const isStatic = func->attrs() & AttrStatic;
  mce->m_key   = reinterpret_cast<uintptr_t>(cls) | uintptr_t{isStatic} << 1;
  mce->m_value = func;
  ar->m_func   = func;

  if (UNLIKELY(isStatic && !func->isClosureBody())) {
    auto const obj = ar->getThis();
    if (debug) ar->setThis(nullptr); // suppress assert
    ar->setClass(cls);
    decRefObj(obj);
  }
}

template<bool fatal>
NEVER_INLINE
void readMagicOrStatic(Entry* mce,
                       ActRec* ar,
                       StringData* name,
                       Class* cls,
                       Class* ctx,
                       uintptr_t mceKey) {
  auto const storedClass = reinterpret_cast<Class*>(mceKey & ~0x3u);
  if (storedClass != cls) {
    return lookup<fatal>(mce, ar, name, cls, ctx);
  }

  auto const mceValue = mce->m_value;
  ar->m_func = mceValue;

  auto const isMagic = mceKey & 0x1u;
  if (UNLIKELY(isMagic)) {
    ar->setMagicDispatch(name);
    assertx(!(mceKey & 0x2u));
    return;
  }

  assertx(mceKey & 0x2u);
  if (LIKELY(!mceValue->isClosureBody())) {
    auto const obj = ar->getThis();
    if (debug) ar->setThis(nullptr); // suppress assert in setClass
    ar->setClass(cls);
    decRefObj(obj);
  }
}

template<bool fatal>
NEVER_INLINE
void readPublicStatic(Entry* mce,
                      ActRec* ar,
                      Class* cls,
                      const Func* cand) {
  mce->m_key = reinterpret_cast<uintptr_t>(cls) | 0x2u;
  if (LIKELY(!cand->isClosureBody())) {
    auto const obj = ar->getThis();
    if (debug) ar->setThis(nullptr); // suppress assert in setClass
    ar->setClass(cls);
    decRefObj(obj);
  }
}

template<bool fatal>
void handleSlowPath(Entry* mce,
                    ActRec* ar,
                    StringData* name,
                    Class* cls,
                    Class* ctx,
                    uintptr_t mcePrime) {
  assertx(ar->hasThis());
  assertx(ar->getThis()->getVMClass() == cls);
  assertx(IMPLIES(mce->m_key, mce->m_value));
  assertx(name->isStatic());

  // Check for a hit in the request local cache---since we've failed
  // on the immediate smashed in the TC.
  auto const mceKey = mce->m_key;
  if (LIKELY(mceKey == reinterpret_cast<uintptr_t>(cls))) {
    ar->m_func = mce->m_value;
    return;
  }

  // If the request local cache isn't filled, try to use the Func*
  // from the TC's mcePrime as a starting point.
  const Func* mceValue;
  if (UNLIKELY(!mceKey)) {
    // If the low bit is set in mcePrime, we're in the middle of
    // smashing immediates into the TC from the handlePrimeCacheInit,
    // and the upper bits is not yet a valid Func*.
    //
    // We're assuming that writes to executable code may be seen out
    // of order (i.e. it may call this function with the old
    // immediate), so we check this bit to ensure we don't try to
    // treat the immediate as a real Func* if it isn't yet.
    if (mcePrime & 0x1) {
      return lookup<fatal>(mce, ar, name, cls, ctx);
    }
    mceValue = reinterpret_cast<const Func*>(mcePrime >> 32);
    if (UNLIKELY(!mceValue)) {
      // The inline Func* might be null if it was uncacheable (not
      // low-malloced).
      return lookup<fatal>(mce, ar, name, cls, ctx);
    }
    mce->m_value = mceValue; // below assumes this is already in local cache
  } else {
    if (UNLIKELY(mceKey & 0x3)) {
      return readMagicOrStatic<fatal>(mce, ar, name, cls, ctx, mceKey);
    }
    mceValue = mce->m_value;
  }
  assertx(!(mceValue->attrs() & AttrStatic));

  // Note: if you manually CSE mceValue->methodSlot() here, gcc 4.8
  // will strangely generate two loads instead of one.
  if (UNLIKELY(cls->numMethods() <= mceValue->methodSlot())) {
    return lookup<fatal>(mce, ar, name, cls, ctx);
  }
  auto const cand = cls->getMethod(mceValue->methodSlot());

  // If this class has the same func at the same method slot we're
  // good to go.  No need to recheck permissions, since we already
  // checked them first time around.
  //
  // This case occurs when the current target class `cls' and the
  // class we saw last time in mceKey have some shared ancestor that
  // defines the method, but neither overrode the method.
  if (LIKELY(cand == mceValue)) {
    ar->m_func = cand;
    mce->m_key = reinterpret_cast<uintptr_t>(cls);
    return;
  }

  // If the previously called function (mceValue) was private, then
  // the current context class must be mceValue->cls(), since we
  // called it last time.  So if the new class in `cls' derives from
  // mceValue->cls(), its the same function that would be picked.
  // Note that we can only get this case if there is a same-named
  // (private or not) function deeper in the class hierarchy.
  //
  // In this case, we can do a fast subtype check using the classVec,
  // because we know oldCls can't be an interface (because we observed
  // an instance of it last time).
  if (UNLIKELY(mceValue->attrs() & AttrPrivate)) {
    auto const oldCls = mceValue->cls();
    assertx(!(oldCls->attrs() & AttrInterface));
    if (cls->classVecLen() >= oldCls->classVecLen() &&
        cls->classVec()[oldCls->classVecLen() - 1] == oldCls) {
      // cls <: oldCls -- choose the same function as last time.
      ar->m_func = mceValue;
      mce->m_key = reinterpret_cast<uintptr_t>(cls);
      return;
    }
  }

  // If the candidate has the same name, its probably the right
  // function.  Try to prove it.
  //
  // We can use the invoked name `name' to compare with cand, but note
  // that function names are case insensitive, so it's not necessarily
  // true that mceValue->name() == name bitwise.
  assertx(mceValue->name()->isame(name));
  if (LIKELY(cand->name() == name)) {
    if (LIKELY(cand->attrs() & AttrPublic)) {
      // If the candidate function is public, then it has to be the
      // right function.  There can be no other function with this
      // name on `cls', and we already ruled out the case where
      // dispatch should've gone to a private function with the same
      // name, above.
      //
      // The normal case here is an overridden public method.  But this
      // case can also occur on unrelated classes that happen to have
      // a same-named function at the same method slot, which means we
      // still have to check whether the new function is static.
      // Bummer.
      ar->m_func   = cand;
      mce->m_value = cand;
      if (UNLIKELY(cand->attrs() & AttrStatic)) {
        return readPublicStatic<fatal>(mce, ar, cls, cand);
      }
      mce->m_key = reinterpret_cast<uintptr_t>(cls);
      return;
    }

    // If the candidate function and the old function are originally
    // declared on the same class, then we have mceKey and `cls' as
    // related class types, and they are inheriting this (non-public)
    // function from some shared ancestor, but have different
    // implementations (since we already know mceValue != cand).
    //
    // Since the current context class could call it last time, we can
    // call the new implementation too.  We also know the new function
    // can't be static, because the last one wasn't.
    if (LIKELY(cand->baseCls() == mceValue->baseCls())) {
      assertx(!(cand->attrs() & AttrStatic));
      ar->m_func   = cand;
      mce->m_value = cand;
      mce->m_key   = reinterpret_cast<uintptr_t>(cls);
      return;
    }
  }

  return lookup<fatal>(mce, ar, name, cls, ctx);
}

///////////////////////////////////////////////////////////////////////////////
}

template<bool fatal>
void handlePrimeCacheInit(Entry* mce,
                          ActRec* ar,
                          StringData* name,
                          Class* cls,
                          Class* ctx,
                          uintptr_t rawTarget) {
  // If rawTarget doesn't have the flag bit we must have a smash in flight, but
  // the call is still pointed at us.  Just do a lookup.
  if (!(rawTarget & 0x1)) {
    return lookup<fatal>(mce, ar, name, cls, ctx);
  }

  // We should be able to use DECLARE_FRAME_POINTER here,
  // but that fails inside templates.
  // Fortunately, this code is very x86 specific anyway...
#if defined(__x86_64__)
  ActRec* framePtr;
  asm volatile("mov %%rbp, %0" : "=r" (framePtr) ::);
#else
  ActRec* framePtr = ar;
  always_assert(false);
#endif

  TCA toSmash =
    mcg->backEnd().smashableCallFromReturn(TCA(framePtr->m_savedRip));
  TCA movAddr = TCA(rawTarget >> 1);

  // First fill the request local method cache for this call.
  lookup<fatal>(mce, ar, name, cls, ctx);

  // If we fail to get the write lease, just let it stay unsmashed for now.
  // We are using the write lease + whether the code is already smashed to
  // determine which thread should free the SmashLoc---after getting the
  // lease, we need to re-check if someone else smashed it first.
  LeaseHolder writer(Translator::WriteLease());
  if (!writer) return;

  auto smashMov = [&] (TCA addr, uintptr_t value) -> bool {
    always_assert(mcg->backEnd().isSmashable(addr, x64::kMovLen));
    //XX these assume the immediate move was to r10
    //assertx(addr[0] == 0x49 && addr[1] == 0xba);
    auto const ptr = reinterpret_cast<uintptr_t*>(addr + x64::kMovImmOff);
    if (!(*ptr & 1)) {
      return false;
    }
    *ptr = value;
    return true;
  };

  // The inline cache is a 64-bit immediate, and we need to atomically
  // set both the Func* and the Class*.  We also can only cache these
  // values if the Func* and Class* can't be deallocated, so this is
  // limited to:
  //
  //   - Both Func* and Class* must fit in 32-bit value (i.e. be
  //     low-malloced).
  //
  //   - We must be in RepoAuthoritative mode.  It is ok to cache a
  //     non-AttrPersistent class here, because if it isn't loaded in
  //     the request we'll never hit the TC fast path.  But we can't
  //     do it if the Class* or Func* might be freed.
  //
  //   - The call must not be magic or static.  The code path in
  //     handleSlowPath currently assumes we've ruled this out.
  //
  // It's ok to store into the inline cache even if there are low bits
  // set in mce->m_key.  In that case we'll always just miss the in-TC
  // fast path.  We still need to clear the bit so handleSlowPath can
  // tell it was smashed, though.
  //
  // If the situation is not cacheable, we just put a value into the
  // immediate that will cause it to always call out to handleSlowPath.
  auto const fval = reinterpret_cast<uintptr_t>(mce->m_value);
  auto const cval = mce->m_key;
  bool const cacheable =
    RuntimeOption::RepoAuthoritative &&
    cval && !(cval & 0x3) &&
    fval < std::numeric_limits<uint32_t>::max() &&
    cval < std::numeric_limits<uint32_t>::max();

  uintptr_t imm = 0x2; /* not a Class, but clear low bit */
  if (cacheable) {
    assertx(!(mce->m_value->attrs() & AttrStatic));
    imm = fval << 32 | cval;
  }
  if (!smashMov(movAddr, imm)) {
    // Someone beat us to it.  Bail early.
    return;
  }

  // Regardless of whether the inline cache was populated, smash the
  // call to start doing real dispatch.
  mcg->backEnd().smashCall(toSmash,
                           reinterpret_cast<TCA>(handleSlowPath<fatal>));
}

template
void handlePrimeCacheInit<false>(Entry*, ActRec*, StringData*,
                                 Class*, Class*, uintptr_t);

template
void handlePrimeCacheInit<true>(Entry*, ActRec*, StringData*,
                                Class*, Class*, uintptr_t);

} // namespace MethodCache

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

rds::Handle StaticMethodCache::alloc(const StringData* clsName,
                                     const StringData* methName,
                                     const char* ctxName) {
  return rds::bind<StaticMethodCache>(
    rds::StaticMethod { mangleSmcName(clsName, methName, ctxName) }
  ).handle();
}

rds::Handle StaticMethodFCache::alloc(const StringData* clsName,
                                      const StringData* methName,
                                      const char* ctxName) {
  return rds::bind<StaticMethodFCache>(
    rds::StaticMethodF { mangleSmcName(clsName, methName, ctxName) }
  ).handle();
}

const Func*
StaticMethodCache::lookup(rds::Handle handle, const NamedEntity *ne,
                          const StringData* clsName,
                          const StringData* methName, TypedValue* vmfp) {
  StaticMethodCache* thiz = static_cast<StaticMethodCache*>
    (handleToPtr(handle));
  Stats::inc(Stats::TgtCache_StaticMethodMiss);
  Stats::inc(Stats::TgtCache_StaticMethodHit, -1);
  TRACE(1, "miss %s :: %s caller %p\n",
        clsName->data(), methName->data(), __builtin_return_address(0));

  const Func* f;
  auto const ec = g_context.getNoCheck();
  auto const cls = Unit::loadClass(ne, clsName);
  if (UNLIKELY(!cls)) {
    raise_error(Strings::UNKNOWN_CLASS, clsName->data());
  }
  LookupResult res = ec->lookupClsMethod(f, cls, methName,
                                         nullptr, // there may be an active
                                                  // this, but we can just fall
                                                  // through in that case.
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
    return f;
  }
  assertx(res != LookupResult::MethodFoundWithThis); // Not possible: no this.

  // Indicate to the IR that it should take even slower path
  return nullptr;
}

const Func*
StaticMethodFCache::lookup(rds::Handle handle, const Class* cls,
                           const StringData* methName, TypedValue* vmfp) {
  assertx(cls);
  StaticMethodFCache* thiz = static_cast<StaticMethodFCache*>
    (handleToPtr(handle));
  Stats::inc(Stats::TgtCache_StaticMethodFMiss);
  Stats::inc(Stats::TgtCache_StaticMethodFHit, -1);

  const Func* f;
  auto const ec = g_context.getNoCheck();
  LookupResult res = ec->lookupClsMethod(f, cls, methName,
                                         nullptr,
                                         arGetContextClass((ActRec*)vmfp),
                                         false /*raise*/);
  assertx(res != LookupResult::MethodFoundWithThis); // Not possible: no this.
  if (LIKELY(res == LookupResult::MethodFoundNoThis && !f->isAbstract())) {
    // We called lookupClsMethod with a NULL this and got back a method that
    // may or may not be static. This implies that lookupClsMethod, given the
    // same class and the same method name, will never return MagicCall*Found
    // or MethodNotFound. It will always return the same f and if we do give it
    // a this it will return MethodFoundWithThis iff (this->instanceof(cls) &&
    // !f->isStatic()). this->instanceof(cls) is always true for
    // FPushClsMethodF because it is only used for self:: and parent::
    // calls. So, if we store f and its staticness we can handle calls with and
    // without this completely in assembly.
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
