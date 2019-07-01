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

#include "hphp/runtime/vm/jit/target-cache.h"

#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/runtime-error.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/stats.h"
#include "hphp/runtime/base/strings.h"

#include "hphp/runtime/vm/jit/smashable-instr.h"
#include "hphp/runtime/vm/jit/tc-internal.h"
#include "hphp/runtime/vm/jit/tc.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/jit/translator-runtime.h"
#include "hphp/runtime/vm/jit/write-lease.h"

#include "hphp/runtime/vm/interp-helpers.h"
#include "hphp/runtime/vm/method-lookup.h"
#include "hphp/runtime/vm/treadmill.h"
#include "hphp/runtime/vm/unit-util.h"

#include "hphp/util/text-util.h"

#include <cassert>
#include <limits>
#include <mutex>
#include <string>
#include <vector>

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
static std::vector<rds::Link<FuncCache,rds::Mode::Normal>> funcCacheEntries;

rds::Handle FuncCache::alloc() {
  auto const link = rds::alloc<FuncCache,rds::Mode::Normal,sizeof(Pair)>();
  std::lock_guard<std::mutex> g(funcCacheMutex);
  funcCacheEntries.push_back(link);
  return link.handle();
}

const Func* FuncCache::lookup(rds::Handle handle,
                              StringData* sd,
                              ActRec* ar,
                              ActRec* fp) {
  assertx(ar->isDynamicCall());
  auto const thiz = rds::handleToPtr<FuncCache, rds::Mode::Normal>(handle);
  if (!rds::isHandleInit(handle, rds::NormalTag{})) {
    for (std::size_t i = 0; i < FuncCache::kNumLines; ++i) {
      thiz->m_pairs[i].m_key = nullptr;
      thiz->m_pairs[i].m_value = nullptr;
    }
    rds::initHandle(handle);
  }
  auto const pair = keyToPair(thiz, sd);
  const StringData* pairSd = pair->m_key;
  if (!stringMatches(pairSd, sd)) {
    // Miss. Does it actually exist?
    auto const* func = Unit::lookupFunc(sd);
    bool noEffects = true;
    try {
      if (LIKELY(func != nullptr)) {
        noEffects &= callerDynamicCallChecks(func);
        noEffects &= callerRxChecks(fp, func);
      } else {
        ObjectData *this_ = nullptr;
        Class* self_ = nullptr;
        StringData* inv = nullptr;
        ArrayData* reifiedGenerics = nullptr;
        bool dynamic = false;
        func = vm_decode_function(
          Variant{sd},
          fp,
          this_,
          self_,
          inv,
          dynamic,
          reifiedGenerics,
          DecodeFlags::NoWarn);
        if (!func) {
          raise_call_to_undefined(sd);
        }
        assertx(dynamic);
        noEffects &= callerDynamicCallChecks(func);
        noEffects &= callerRxChecks(fp, func);

        if (this_) {
          ar->m_func = func;
          ar->setThis(this_);
          this_->incRefCount();
          if (UNLIKELY(inv != nullptr)) ar->setMagicDispatch(inv);
          return func;
        }
        if (self_) {
          ar->m_func = func;
          ar->setClass(self_);
          if (UNLIKELY(inv != nullptr)) ar->setMagicDispatch(inv);
          return func;
        }
      }
    } catch (...) {
      *arPreliveOverwriteCells(ar) = make_tv<KindOfString>(sd);
      throw;
    }
    assertx(!func->implCls());
    func->validate();
    if (UNLIKELY(!noEffects)) {
      ar->m_func = func;
      ar->trashThis();
      return func;
    }
    pair->m_key =
      const_cast<StringData*>(func->displayName()); // use a static name
    pair->m_value = func;
  }
  ar->m_func = pair->m_value;
  ar->trashThis();
  assertx(stringMatches(pair->m_key, pair->m_value->displayName()));
  pair->m_value->validate();
  return pair->m_value;
}

void invalidateForRenameFunction(const StringData* name) {
  assertx(name);
  std::lock_guard<std::mutex> g(funcCacheMutex);
  for (auto& h : funcCacheEntries) {
    if (h.isInit()) h.markUninit();
  }
}

//////////////////////////////////////////////////////////////////////
// ClassCache

rds::Handle ClassCache::alloc() {
  return rds::alloc<ClassCache,rds::Mode::Normal,sizeof(Pair)>().handle();
}

const Class* ClassCache::lookup(rds::Handle handle, StringData* name) {
  auto const thiz = rds::handleToPtr<ClassCache, rds::Mode::Normal>(handle);
  if (!rds::isHandleInit(handle, rds::NormalTag{})) {
    for (std::size_t i = 0; i < ClassCache::kNumLines; ++i) {
      thiz->m_pairs[i].m_key = nullptr;
      thiz->m_pairs[i].m_value = nullptr;
    }
    rds::initHandle(handle);
  }
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

NEVER_INLINE
uintptr_t lookup(const Class* cls, const StringData* name, const Class* ctx) {
  auto const func = lookupMethodCtx(cls, name, ctx, CallType::ObjMethod, false);
  if (LIKELY(func != nullptr)) {
    if (UNLIKELY(func->isStaticInPrologue())) {
      throw_has_this_need_static(func);
    }
    return reinterpret_cast<uintptr_t>(func);
  }

  auto const magicFunc = cls->lookupMethod(s_call.get());
  if (LIKELY(magicFunc != nullptr)) {
    assertx(!magicFunc->isStatic());
    return reinterpret_cast<uintptr_t>(magicFunc) | 0x1u;
  }

  lookupMethodCtx(cls, name, ctx, CallType::ObjMethod, true /* raise */);
  not_reached();
}

void smashImmediate(TCA movAddr, const Class* cls, uintptr_t funcVal) {
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
  //     handleStaticCall currently assumes we've ruled this out.
  //
  // It's ok to store into the inline cache even if there are low bits
  // set in mce->m_key.  In that case we'll always just miss the in-TC
  // fast path.  We still need to clear the bit so handleStaticCall can
  // tell it was smashed, though.
  //
  // If the situation is not cacheable, we just put a value into the
  // immediate that will cause it to always call out to handleStaticCall.
  assertx(cls && funcVal);
  auto const clsVal = reinterpret_cast<uintptr_t>(cls);
  bool const cacheable =
    RuntimeOption::RepoAuthoritative &&
    !(funcVal & 0x1) &&
    clsVal < std::numeric_limits<uint32_t>::max() &&
    funcVal < std::numeric_limits<uint32_t>::max();

  if (cacheable) {
    assertx(!(funcVal & 0x1));
    smashMovq(movAddr, funcVal << 32 | clsVal);
  } else {
    smashMovq(movAddr, 0x1);
  }
}

///////////////////////////////////////////////////////////////////////////////
}

EXTERNALLY_VISIBLE uintptr_t
handleDynamicCall(const Class* cls, const StringData* name, const Class* ctx) {
  // Perform lookup without any caching.
  return lookup(cls, name, ctx);
}

EXTERNALLY_VISIBLE uintptr_t
handleStaticCall(const Class* cls, const StringData* name, const Class* ctx,
                 rds::Handle mceHandle, uintptr_t mcePrime) {
  assertx(name->isStatic());
  assertx(cls);
  auto& mce = rds::handleToRef<Entry, rds::Mode::Normal>(mceHandle);
  if (!rds::isHandleInit(mceHandle, rds::NormalTag{})) {
    // If the low bit is set in mcePrime, we have not yet smashed the immediate
    // into the TC, or the value was not cacheable.
    if (UNLIKELY(mcePrime & 0x1)) {
      // First fill the request local cache for this call.
      auto const func = lookup(cls, name, ctx);
      mce = Entry { cls, func };
      rds::initHandle(mceHandle);
      if (mcePrime != 0x1) {
        // Smash the prime value in TC; mcePrime >> 1 is the smash target.
        smashImmediate(TCA(mcePrime >> 1), cls, func);
      }
      return func;
    }

    // Otherwise, use TC's mcePrime as a starting point for request local cache.
    mce = Entry { nullptr, mcePrime >> 32 };
    rds::initHandle(mceHandle);
  } else if (LIKELY(mce.m_key == cls)) {
    // Fast path -- hit in the request local cache.
    return mce.m_value;
  } else if (UNLIKELY(mce.m_value & 0x1)) {
    // Snail path -- the cache missed func is magic, can't use it.
    auto const func = lookup(cls, name, ctx);
    mce = Entry { cls, func };
    return func;
  }

  assertx(mce.m_value && !(mce.m_value & 0x1));
  auto const oldFunc = reinterpret_cast<const Func*>(mce.m_value);
  assertx(!oldFunc->isStaticInPrologue());

  // Note: if you manually CSE oldFunc->methodSlot() here, gcc 4.8
  // will strangely generate two loads instead of one.
  if (UNLIKELY(cls->numMethods() <= oldFunc->methodSlot())) {
    auto const func = lookup(cls, name, ctx);
    mce = Entry { cls, func };
    return func;
  }
  auto const cand = cls->getMethod(oldFunc->methodSlot());

  // If this class has the same func at the same method slot we're
  // good to go.  No need to recheck permissions, since we already
  // checked them first time around.
  //
  // This case occurs when the current target class `cls' and the
  // class we saw last time have some shared ancestor that defines
  // the method, but neither overrode the method.
  if (LIKELY(cand == oldFunc)) {
    mce.m_key = cls;
    return reinterpret_cast<uintptr_t>(oldFunc);
  }

  // If the previously called function (oldFunc) was private, then
  // the current context class must be oldFunc->cls(), since we
  // called it last time.  So if the new class in `cls' derives from
  // oldFunc->cls(), its the same function that would be picked.
  // Note that we can only get this case if there is a same-named
  // (private or not) function deeper in the class hierarchy.
  //
  // In this case, we can do a fast subtype check using the classVec,
  // because we know oldCls can't be an interface (because we observed
  // an instance of it last time).
  if (UNLIKELY(oldFunc->attrs() & AttrPrivate)) {
    auto const oldCls = oldFunc->cls();
    assertx(!(oldCls->attrs() & AttrInterface));
    if (cls->classVecLen() >= oldCls->classVecLen() &&
        cls->classVec()[oldCls->classVecLen() - 1] == oldCls) {
      // cls <: oldCls -- choose the same function as last time.
      mce.m_key = cls;
      return reinterpret_cast<uintptr_t>(oldFunc);
    }
  }

  // If the candidate has the same name, its probably the right
  // function.  Try to prove it.
  //
  // We can use the invoked name `name' to compare with cand, but note
  // that function names are case insensitive, so it's not necessarily
  // true that oldFunc->name() == name bitwise.
  assertx(oldFunc->name()->isame(name));
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
      if (UNLIKELY(cand->isStaticInPrologue())) {
        throw_has_this_need_static(cand);
      }

      auto const func = reinterpret_cast<uintptr_t>(cand);
      mce = Entry { cls, func };
      return func;
    }

    // If the candidate function and the old function are originally
    // declared on the same class, then we have mce.m_key and `cls' as
    // related class types, and they are inheriting this (non-public)
    // function from some shared ancestor, but have different
    // implementations (since we already know oldFunc != cand).
    //
    // Since the current context class could call it last time, we can
    // call the new implementation too.  We also know the new function
    // can't be static, because the last one wasn't.
    if (LIKELY(cand->baseCls() == oldFunc->baseCls())) {
      assertx(!cand->isStaticInPrologue());
      auto const func = reinterpret_cast<uintptr_t>(cand);
      mce = Entry { cls, func };
      return func;
    }
  }

  auto const func = lookup(cls, name, ctx);
  mce = Entry { cls, func };
  return func;
}

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
  return rds::bind<StaticMethodCache, rds::Mode::Normal>(
    rds::StaticMethod { mangleSmcName(clsName, methName, ctxName) }
  ).handle();
}

rds::Handle StaticMethodFCache::alloc(const StringData* clsName,
                                      const StringData* methName,
                                      const char* ctxName) {
  return rds::bind<StaticMethodFCache, rds::Mode::Normal>(
    rds::StaticMethodF { mangleSmcName(clsName, methName, ctxName) }
  ).handle();
}

const Func*
StaticMethodCache::lookup(rds::Handle handle, const NamedEntity *ne,
                          const StringData* clsName,
                          const StringData* methName, TypedValue* vmfp) {
  assertx(rds::isNormalHandle(handle));
  auto thiz = rds::handleToPtr<StaticMethodCache, rds::Mode::Normal>(handle);
  Stats::inc(Stats::TgtCache_StaticMethodMiss);
  TRACE(1, "miss %s :: %s caller %p\n",
        clsName->data(), methName->data(), __builtin_return_address(0));

  const Func* f;
  auto const cls = Unit::loadClass(ne, clsName);
  if (UNLIKELY(!cls)) {
    raise_error(Strings::UNKNOWN_CLASS, clsName->data());
  }

  // After this call, it's a post-condition that the RDS entry for `cls' is
  // initialized, so make sure it has been as a side-effect of
  // Unit::loadClass().
  assertx(cls == ne->getCachedClass());

  LookupResult res = lookupClsMethod(f, cls, methName,
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
    rds::initHandle(handle);
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
  assertx(rds::isNormalHandle(handle));
  auto thiz = rds::handleToPtr<StaticMethodFCache, rds::Mode::Normal>(handle);
  Stats::inc(Stats::TgtCache_StaticMethodFMiss);
  Stats::inc(Stats::TgtCache_StaticMethodFHit, -1);

  const Func* f;
  LookupResult res = lookupClsMethod(f, cls, methName,
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
    rds::initHandle(handle);
    TRACE(1, "fill staticfcache %s :: %s -> %p\n",
          cls->name()->data(), methName->data(), f);
    Stats::inc(Stats::TgtCache_StaticMethodFFill);
    return f;
  }

  return nullptr;
}

//////////////////////////////////////////////////////////////////////

}}
