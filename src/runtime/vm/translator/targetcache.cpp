/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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
#include <string>
#include <stdio.h>

#include <util/trace.h>
#include <util/base.h>
#include <runtime/base/complex_types.h>
#include <runtime/base/execution_context.h>
#include <runtime/base/types.h>
#include <runtime/base/tv_macros.h>
#include <runtime/vm/unit.h>
#include <runtime/vm/class.h>
#include <runtime/vm/runtime.h>
#include <runtime/vm/translator/targetcache.h>
#include <runtime/vm/translator/translator-inline.h>
#include <system/gen/sys/system_globals.h>

using namespace HPHP::MethodLookup;

/*
 * The targetcache module provides a set of per-request caches.
 */
namespace HPHP {
namespace VM {
namespace Transl {
namespace TargetCache {

TRACE_SET_MOD(targetcache);

static StaticString s___call(LITSTR_INIT("__call"));

// Shorthand.
typedef CacheHandle Handle;

// Targetcache memory. See the comment in targetcache.h
__thread HPHP::x64::DataBlock tl_targetCaches = {0, 0, kNumTargetCacheBytes};
size_t s_frontier;

// Mapping from names to targetcache locations. Protected by the translator
// write lease. NB: These are case-sensitive; this has less recall than
// possible for func, class, etc.
typedef hphp_hash_map<const StringData*, Handle, string_data_hash,
        string_data_same>
  HandleMap;

// handleMaps[NSConstant]['FOO'] is the cache associated with the constant
// FOO, eg. handleMaps is a rare instance of shared, mutable state across
// the request threads in the translator: it is essentially a lazily
// constructed link table for tl_targetCaches.
HandleMap handleMaps[NumNameSpaces];

// Vector of cache handles
typedef std::vector<Handle> HandleVector;

// Set of FuncCache handles for dynamic function callsites, used for
// invalidation when a function is renamed.
HandleVector funcHandles;

// RAII lock for handleMaps/funcHandles. Allow recursive acquisitions.
class HandleMutex {
  static pthread_mutex_t m_lock;
 public:
  HandleMutex() {
    pthread_mutex_lock(&m_lock);
  }
  ~HandleMutex() {
    pthread_mutex_unlock(&m_lock);
  }
};
pthread_mutex_t HandleMutex::m_lock = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;

inline Handle
ptrToHandle(const void* ptr) {
  ptrdiff_t retval = uintptr_t(ptr) - uintptr_t(tl_targetCaches.base);
  ASSERT(retval < kNumTargetCacheBytes);
  return retval;
}

inline void*
handleToPtr(Handle h) {
  ASSERT(h < kNumTargetCacheBytes);
  return tl_targetCaches.base + h;
}

// namedAlloc --
//   Many targetcache entries (Func, Class, Constant, ...) have
//   request-unique values. There is no reason to allocate more than
//   one item for all such calls in a request.
//
//   handleMaps acts as a de-facto dynamic link table that lives
//   across requests; the translator can write out code that assumes
//   that a given named entity's location in tl_targetCaches is
//   stable from request to request.
Handle
namedAlloc(PHPNameSpace where, const StringData* name, int numBytes,
           int align) {
  ASSERT(!name || (where >= 0 && where < NumNameSpaces));
  Handle retval;
  HandleMutex mtx;
  HandleMap& map = handleMaps[where];
  if (name && mapGet(map, name, &retval)) {
    TRACE(2, "TargetCache: hit \"%s\", %d\n", name->data(), int(retval));
    return retval;
  }
  void *mem = tl_targetCaches.allocAt(s_frontier, numBytes, align);
  retval = ptrToHandle(mem);
  if (name) {
    mapInsertUnique(map, StringData::GetStaticString(name->copy()), retval);
    TRACE(1, "TargetCache: inserted \"%s\", %d\n", name->data(), int(retval));
  } else if (where == NSDynFunction) {
    funcHandles.push_back(retval);
  }
  return retval;
}

void
invalidateFuncName(const StringData* name) {
  ASSERT(name);
  Handle handle;
  HandleMutex mtx;
  HandleMap& map = handleMaps[NSFunction];
  if (mapGet(map, name, &handle)) {
    TRACE(1, "TargetCaches: invalidating func mapping for %s\n", name->data());
    // OK, there's a targetcache for this name.
    FixedFuncCache::invalidate(handle);
  }

  for (HandleVector::iterator i = funcHandles.begin();
       i != funcHandles.end(); ++i) {
    FuncCache::invalidate(*i, name);
  }
}

// requestInit --
//   Per-request work.
void
requestInit() {
  TRACE(1, "TargetCaches: @%p\n", tl_targetCaches.base);
  if (!tl_targetCaches.base) {
    tl_targetCaches.init();
  }
  TRACE(1, "TargetCaches: bzeroing %zd bytes: %p\n", tl_targetCaches.size,
        tl_targetCaches.base);
  memset(tl_targetCaches.base, 0, tl_targetCaches.size);
}

static inline bool
stringMatches(const StringData* rowString, const StringData* sd) {
  return rowString &&
    (rowString == sd ||
     rowString->data() == sd->data() ||
     (rowString->hash() == sd->hash() &&
      rowString->same(sd)));

}

//=============================================================================
// CallCache
template<>
inline int
CallCache::hashKey(const Func* f) {
  pointer_hash<Func> h;
  return h(f);
}

template<>
TCA
CallCache::lookup(Handle handle, ActRec *ar, const void* offset) {
  CallCache* thiz = cacheAtHandle(handle);
  const Func *f = ar->m_func;
  int prologueFlags = Translator::FuncPrologueNormal;
  if (UNLIKELY(ar->hasInvName())) {
    prologueFlags |= Translator::FuncPrologueMagicCall;
  }
  if (UNLIKELY(intercept_data(ar) != NULL)) {
    prologueFlags |= Translator::FuncPrologueIntercepted;
  }

  // XXX: We could inline this into the caller.
  Pair* pair = thiz->keyToPair(f);
  if (pair->m_key == f) {
    ASSERT(pair->m_key->m_magic == Func::kMagic);
    return pair->m_value;
  }

  // No cache entry. Builtin?
  int numArgs = ar->m_numArgs;
  if (UNLIKELY(f->m_info != NULL)) {
    ASSERT(ar->m_func == f);
    ASSERT(ar->m_func->m_magic == Func::kMagic);
    ActRec* savedFp = g_context->m_fp;
    g_context->m_fp = ar;
    g_context->m_pc = savedFp->m_func->m_unit->at((uintptr_t)offset);
    if (f->m_preClass) {
      f->m_builtinClassFuncPtr(ar);
    } else {
      f->m_builtinFuncPtr(ar);
    }
    DynTracer::FunctionExit(ar);
    g_context->m_fp = savedFp;
    // Unlike the usual case, our work here is done. Return NULL
    // to indicate to the TC that there is no callee to resume
    // to.
    return NULL;
  }
  // Drat. Evict the occupant of our favored slot.
  TCA dest = Translator::Get()->funcPrologue(f, numArgs, prologueFlags);
  pair->m_key = f;
  pair->m_value = dest;
  return dest;
}

//=============================================================================
// FuncCache
template<>
inline int
FuncCache::hashKey(const StringData* sd) {
  return sd->hash();
}

template<>
const Func*
FuncCache::lookup(Handle handle, StringData *sd,
                  const void* /* ignored */) {
  FuncCache* thiz = cacheAtHandle(handle);
  Func* func;
  Pair* pair = thiz->keyToPair(sd);
  const StringData* pairSd = pair->m_key;
  if (!stringMatches(pairSd, sd)) {
    // Miss. Does it actually exist?
    func = g_context->lookupFunc(sd);
    ASSERT(func->m_magic == Func::kMagic);
    ASSERT_NOT_IMPLEMENTED(func != NULL);
    pair->m_key = func->m_name; // use a static name
    pair->m_value = func;
  }
  // DecRef the string here; more compact than doing so in
  // callers.
  if (sd->decRefCount() == 0) {
    sd->release();
  }
  ASSERT(stringMatches(pair->m_key, pair->m_value->m_name));
  ASSERT(pair->m_value->m_magic == Func::kMagic);
  return pair->m_value;
}

//=============================================================================
// FixedFuncCache

const Func*
FixedFuncCache::lookup(Handle handle, StringData *sd) {
  ASSERT(sd->isStatic());

  FixedFuncCache* thiz = cacheAtHandle(handle);
  ASSERT(thiz->m_func == NULL);
  Func* func = g_context->lookupFunc(sd);
  ASSERT_NOT_IMPLEMENTED(func != NULL);
  ASSERT(func->m_magic == Func::kMagic);

  thiz->m_func = func;
  return func;
}

//=============================================================================
// MethodCache

template<>
inline int
MethodCache::hashKey(const Class* c) {
  pointer_hash<Class> h;
  return h(c);
}

template<>
MethodCacheEntry
MethodCache::lookup(Handle handle, ActRec *ar, const void* extraKey) {
  StringData* name = (StringData*)extraKey;
  ASSERT(ar->hasThis());
  ObjectData* obj = ar->getThis();
  Class* c = obj->getVMClass();
  ASSERT(c);
  MethodCache* thiz = MethodCache::cacheAtHandle(handle);
  Pair* pair = thiz->keyToPair(c);
  const Func* func;
  bool isMagicCall;
  bool useThis;
  if (pair->m_key == c) {
    TRACE(5, "MethodCache: hit class %p name %s!\n", c, name->data());
    func = pair->m_value.getFunc();
    isMagicCall = pair->m_value.isMagicCall();
    useThis = !(func->m_attrs & AttrStatic);
  } else {
    LookupResult res = g_context->lookupObjMethod(func, c, name, true);
    ASSERT(res == MethodFoundWithThis || res == MethodFoundNoThis ||
           res == MagicCallFound);
    isMagicCall = (res == MagicCallFound);
    useThis = (res != MethodFoundNoThis);
    TRACE(2, "MethodCache: miss class %p name %s!\n", c, name->data());
    pair->m_key = c;
    pair->m_value.set(func, isMagicCall);
  }
  ASSERT(pair->m_key == obj->getVMClass());
  ASSERT(func);
  ASSERT(func->m_magic == Func::kMagic);

  if (func->m_needsStaticLocalCtx) {
    ar->m_staticLocalCtx = obj->getVMClass()->getStaticLocals();
  }
  ar->m_func = func;
  if (!useThis) {
    // Drop the ActRec's reference to the current instance
    if (obj->decRefCount() == 0) {
      obj->release();
    }
    ar->setThis(NULL);
    // Set the ActRec's class (needed for late static binding)
    ar->setClass(c);
  }
  ASSERT(!ar->hasVarEnv() && !ar->hasInvName());
  if (isMagicCall) {
    ar->setInvName(name);
    name->incRefCount();
  }
  return pair->m_value;
}

//=============================================================================
// GlobalCache
//  | - BoxedGlobalCache

static inline HphpArray*
getGlobArray() {
  SystemGlobals *g = (SystemGlobals*)get_global_variables();
  return
    dynamic_cast<HphpArray*>(g->hg_global_storage.getArrayData());
}

template<bool isBoxed>
inline TypedValue*
GlobalCache::lookupImpl(StringData *name) {
  bool hit ATTRIBUTE_UNUSED;
  if (UNLIKELY(m_globals == NULL)) {
    m_globals = getGlobArray();
    TRACE(1, "%sGlobalCache %p initializing m_globals %p\n",
          isBoxed ? "Boxed" : "",
          this, m_globals);
    m_hint = 0;
  } else {
    TRACE(1, "%sGlobalCache %p cbo %d m_globals %p real globals %p\n",
          isBoxed ? "Boxed" : "",
          this, (int)cacheHandle(), m_globals, getGlobArray());
    ASSERT(m_globals == getGlobArray());
  }
  TypedValue* retval;
  // We have a flat array of indices into globals. Simply try them
  // all.
  retval = m_globals->nvGet(name, m_hint, &m_hint);
  if (!retval) {
    hit = false;
    VarEnv* ve = HPHP::g_context->m_varEnvs.front();
    retval = ve->lookup(name);
    if (UNLIKELY(retval == NULL)) {
      TypedValue tv;
      TV_WRITE_NULL(&tv);
      ve->set(name, &tv);
      retval = ve->lookup(name);
    }
  } else {
    hit = true;
  }
  if (isBoxed && retval->m_type != KindOfVariant) {
    tvBox(retval);
    ASSERT(retval->m_type == KindOfVariant);
  }
  if (!isBoxed && retval->m_type == KindOfVariant) {
    retval = retval->m_data.ptv;
  }
  ASSERT(retval);
  ASSERT(!isBoxed || retval->m_type == KindOfVariant);
  ASSERT(!IS_REFCOUNTED_TYPE(retval->m_type) || retval->_count >= 0);
  if (isBoxed || IS_REFCOUNTED_TYPE(retval->m_type)) {
    // This value is destined for the stack.
    tvIncRef(retval);
  }
  // decRef the name: we're consuming it from the stack.
  if (name->decRefCount() == 0) { name->release(); }
  TRACE(5, "%sGlobalCache::lookup(\"%s\") %p -> (%s) %p t%d\n",
        isBoxed ? "Boxed" : "",
        name->data(),
        retval,
        hit ? "hit" : "miss",
        retval->m_data.ptv,
        retval->m_type);
  // Careful! KindOfVariant in-register values are the pointers to
  // inner items.
  return isBoxed ? retval->m_data.ptv : retval;
}

TypedValue*
BoxedGlobalCache::lookup(Handle handle, StringData *name) {
  BoxedGlobalCache* thiz = (BoxedGlobalCache*)
    BoxedGlobalCache::cacheAtHandle(handle);
  TypedValue* retval = thiz->lookupImpl<true>(name);
  // The assert is confusing-looking, but accurate: the enregistered
  // representation of a Var is the Var's value, which points to an
  // inner type.
  ASSERT(retval->m_type != KindOfVariant);
  return retval;
}

TypedValue*
GlobalCache::lookup(Handle handle, StringData *name) {
  GlobalCache* thiz = (GlobalCache*)GlobalCache::cacheAtHandle(handle);
  TypedValue* retval = thiz->lookupImpl<false>(name);
  ASSERT(retval->m_type != KindOfVariant);
  return retval;
}

//=============================================================================
// ClassCache

template<>
inline int
ClassCache::hashKey(StringData* sd) {
  return sd->hash();
}

template<>
const Class*
ClassCache::lookup(Handle handle, StringData *name,
                   const void* unused) {
  ClassCache* thiz = cacheAtHandle(handle);
  Pair *pair = thiz->keyToPair(name);
  const StringData* pairSd = pair->m_key;
  if (!stringMatches(pairSd, name)) {
    Class *c = g_context->lookupClass(name);
    TRACE(1, "ClassCache miss: %s\n", name->data());
    if (!c) {
      // XXX: try to autoload; share this code with bytecode.cpp
      raise_error("Class undefined: %s", name->data());
    }
    if (pair->m_key &&
        pair->m_key->decRefCount() == 0) {
      pair->m_key->release();
    }
    pair->m_key = name;
    name->incRefCount();
    pair->m_value = c;
  } else {
    TRACE(1, "ClassCache hit: %s\n", name->data());
  }
  return pair->m_value;
}

/*
 * Constants are raw TypedValues read from TLS storage by emitted code.
 * We must represent the undefined value as KindOfUninit == 0. Constant
 * definition is hooked in the runtime to allocate and update these
 * structures.
 */
CacheHandle allocConstant(StringData* name) {
  BOOST_STATIC_ASSERT(KindOfUninit == 0);
  return namedAlloc(NSConstant, name, sizeof(TypedValue), sizeof(TypedValue));
}

void
fillConstant(StringData* name) {
  HandleMutex mtx;
  ASSERT(name);
  Handle ch = allocConstant(name);
  ASSERT(mapContains(handleMaps[NSConstant], name));
  mapGet(handleMaps[NSConstant], name, &ch);
  TypedValue *val = g_context->getCns(name);
  ASSERT(val);
  if (val->m_type == KindOfString && !val->m_data.pstr->isStatic()) {
    // The only case where constants' values need refcounting is if the value is
    // a non-static string, which should be dynamically rare. However, the value
    // of a constant will live until the end of the request, so it can
    // effectively be treated as static. Thus, if we're defining a constant to
    // have a non-static string value, make a static copy and keep that as the
    // value. That way, we don't have to refcount when looking up constants.
    StringData* copy = val->m_data.pstr->copy();
    copy->setStatic();
    tvDecRef(val);
    val->m_data.pstr = copy;
  }
  ASSERT(!IS_REFCOUNTED_TYPE(val->m_type) ||
         (val->m_type == KindOfString && val->m_data.pstr->isStatic()));
  *(TypedValue*)handleToPtr(ch) = *val;
}

//=============================================================================
// *SPropCache
//

TypedValue*
SPropCache::lookup(Handle handle, const Class *cls, const StringData *name) {
  SPropCache* thiz = cacheAtHandle(handle);
  ASSERT(cls && name);
  ASSERT(!thiz->m_tv);
  TRACE(3, "SPropCache miss: %s::$%s\n", cls->m_preClass->m_name->data(),
        name->data());

  // This is valid only if the lookup comes from an in-class method
  PreClass *ctx = arGetContextPreClass((ActRec*)vmfp());
  ASSERT(ctx == cls->m_preClass.get());
  bool visible, accessible;
  TypedValue* val = cls->getSProp(ctx, name, visible, accessible);
  if (!visible) {
    raise_error("Invalid static property access: %s::%s",
                cls->m_preClass->m_name->data(), name->data());
  }
  // We only cache in class references, thus we can always cache them
  // once the property is known to exist
  ASSERT(accessible);
  thiz->m_tv = val;
  TRACE(3, "SPropCache::lookup(\"%s::$%s\") %p -> %p t%d\n",
        cls->m_preClass->m_name->data(),
        name->data(),
        val,
        val->m_data.ptv,
        val->m_type);
  return val;
}

} } } } // HPHP::VM::Transl::TargetCache
