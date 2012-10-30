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
#ifndef INCL_TARGETCACHE_H_
#define INCL_TARGETCACHE_H_

#include <runtime/vm/func.h>
#include <util/util.h>
#include <runtime/vm/translator/types.h>
#include <runtime/vm/translator/asm-x64.h>
#include <boost/static_assert.hpp>

namespace HPHP {
namespace VM {
namespace Transl {
namespace TargetCache {

void requestInit();
void requestExit();
void threadInit();
void threadExit();
void flush();

/*
 * The targetCaches are physically thread-private, but they share their
 * layout. So the memory is in tl_targetCaches, but we allocate it via the
 * global s_frontier. This is protected by the translator's write-lease.
 */
extern __thread void* tl_targetCaches;
extern size_t s_frontier;

static const int kConditionFlagsOff = 0;

/*
 * Some caches have different numbers of lines. This is our default.
 */
static const int kDefaultNumLines = 4;

/*
 * The lookup functions are called into from generated assembly and passed an
 * opaque handle into the request-private targetcache.
 */
typedef ptrdiff_t CacheHandle;

enum PHPNameSpace {
  NSCtor,
  NSFixedCall,
  NSDynFunction,
  NSStaticMethod,
  NSStaticMethodF,
  NSClass,
  NSClsInitProp,
  NSClsInitSProp,

  NumInsensitive, _NS_placeholder = NumInsensitive-1,

  NSConstant,
  NSClassConstant,
  NSGlobal,
  NSSProp,
  NSProperty,
  NSCnsBits,

  NumNameSpaces,
  NumCaseSensitive = NumNameSpaces - NumInsensitive,
  FirstCaseSensitive = NumInsensitive,

  NSInvalid = -1,
  NSPersistent = -2
};

template <bool sensitive>
CacheHandle namedAlloc(PHPNameSpace where, const StringData* name,
                       int numBytes, int align);

template<PHPNameSpace where>
CacheHandle namedAlloc(const StringData* name, int numBytes, int align) {
  return namedAlloc<(where >= FirstCaseSensitive)>(where, name,
                                                   numBytes, align);
}

size_t allocBit();
size_t allocCnsBit(const StringData* name);
CacheHandle bitOffToHandleAndMask(size_t bit, uint32 &mask);
bool testBit(CacheHandle handle, uint32 mask);
bool testBit(size_t bit);
bool testAndSetBit(CacheHandle handle, uint32 mask);
bool testAndSetBit(size_t bit);
bool isPersistentHandle(CacheHandle handle);

CacheHandle ptrToHandle(const void*);

TCA fcallHelper(ActRec* ar);

static inline void*
handleToPtr(CacheHandle h) {
  ASSERT(h < RuntimeOption::EvalJitTargetCacheSize);
  return (char*)tl_targetCaches + h;
}

template<class T>
T& handleToRef(CacheHandle h) {
  return *static_cast<T*>(handleToPtr(h));
}

inline ssize_t* conditionFlagsPtr() {
  return ((ssize_t*)handleToPtr(kConditionFlagsOff));
}

inline ssize_t loadConditionFlags() {
  return atomic_acquire_load(conditionFlagsPtr());
}

void invalidateForRename(const StringData* name);

/*
 * Some caches have a Lookup != k, because the TC passes a container
 * with other necessary info to Cache::lookup. E.g., when looking up
 * function preludes, we also need the number of arguments. Since
 * the current ActRec encapsulates both, we pass in an ActRec to
 * Cache::lookup even though CallCache maps Funcs to TCAs.
 *
 * KNLines must be a power of two.
 */
template<typename Key, typename Value, class LookupKey,
  PHPNameSpace NameSpace = NSInvalid,
  int KNLines = kDefaultNumLines,
  typename ReturnValue = Value>
class Cache {
public:
  typedef Cache<Key, Value, LookupKey, NameSpace, KNLines, ReturnValue> Self;
  static const int kNumLines = KNLines;

  struct Pair {
    Key   m_key;
    Value m_value;
  } m_pairs[kNumLines];

  static inline Self* cacheAtHandle(CacheHandle handle) {
    return (Self*)handleToPtr(handle);
  }

  inline Pair* keyToPair(Key k) {
    if (kNumLines == 1) {
      return &m_pairs[0];
    }
    ASSERT(HPHP::Util::isPowerOfTwo(kNumLines));
    return m_pairs + (hashKey(k) & (kNumLines - 1));
  }

protected:
  // Each instance needs to implement this
  static int hashKey(Key k);

public:
  typedef Key CacheKey;
  typedef LookupKey CacheLookupKey;
  typedef Value CacheValue;

  static CacheHandle alloc(const StringData* name = NULL) {
    // Each lookup should access exactly one Pair so there's no point
    // in making sure the entire cache fits on one cache line.
    return namedAlloc<NameSpace>(name, sizeof(Self), sizeof(Pair));
  }
  inline CacheHandle cacheHandle() const {
    return ptrToHandle(this);
  }
  static void invalidate(CacheHandle chand, Key lookup) {
    Pair* pair = cacheAtHandle(chand)->keyToPair(lookup);
    memset(pair, 0, sizeof(Pair));
  }
  static void invalidate(CacheHandle chand) {
    Self *thiz = cacheAtHandle(chand);
    memset(thiz, 0, sizeof(*thiz));
  }
  static ReturnValue lookup(CacheHandle chand, LookupKey lookup,
                            const void* extraKey = NULL);
};

struct FixedFuncCache {
  const Func* m_func;

  static inline FixedFuncCache* cacheAtHandle(CacheHandle handle) {
    return (FixedFuncCache*)handleToPtr(handle);
  }

  static void invalidate(CacheHandle handle) {
    FixedFuncCache* thiz = cacheAtHandle(handle);
    thiz->m_func = NULL;
  }

  static void lookupFailed(StringData* name);
};

struct StaticMethodCache {
  const Func* m_func;
  const Class* m_cls;
  static CacheHandle alloc(const StringData* cls, const StringData* meth,
                           const char* ctxName);
  static const Func* lookupIR(CacheHandle chand,
                              const NamedEntity* ne, const StringData* cls,
                              const StringData* meth);
  static const Func* lookup(CacheHandle chand,
                            const NamedEntity* ne, const StringData* cls,
                            const StringData* meth);
};

struct StaticMethodFCache {
  const Func* m_func;
  int m_static;

  static CacheHandle alloc(const StringData* cls, const StringData* meth,
                           const char* ctxName);
  static const Func* lookup(CacheHandle chand, const Class* cls,
                            const StringData* meth);
};

struct MethodCacheEntry {
  intptr_t m_data;
  void set(const Func* func, bool isMagicCall, bool isStatic) {
    ASSERT(func);
    ASSERT((intptr_t(func) & 0x3) == 0);
    m_data = intptr_t(func) | intptr_t(isMagicCall) | (intptr_t(isStatic) << 1);
  }
  bool isMagicCall() const {
    return (m_data & 1);
  }
  bool isStatic() const {
    return (m_data & 2);
  }
  const Func* getFunc() const {
    return (const Func*)(m_data & ~3);
  }
};

typedef Cache<const StringData*, const Func*, StringData*, NSDynFunction>
  FuncCache;
typedef Cache<const Class*, MethodCacheEntry, ActRec*, NSInvalid, 1, void>
  MethodCache;
typedef Cache<StringData*, const Class*, StringData*, NSClass> ClassCache;

/**
 * PropCache --
 *
 *   Records offets into ObjectData for declared properties.
 */
static const int kPropCacheLines = 8;

typedef void (*pcb_lookup_func_t)(CacheHandle handle, ObjectData* base,
                                  StringData* name, TypedValue* stackPtr,
                                  ActRec* fp);
typedef void (*pcb_set_func_t)(CacheHandle ch, ObjectData* base,
                               StringData* name, int64 val,
                               DataType type, ActRec* fp);

template<typename Key, PHPNameSpace ns = NSInvalid>
class PropCacheBase : public Cache<Key, uintptr_t, ObjectData*, ns,
                                   kPropCacheLines> {
public:
  typedef Cache<Key, uintptr_t, ObjectData*, ns, kPropCacheLines> Parent;

  template<bool baseIsLocal>
  static void lookup(CacheHandle handle, ObjectData* base,
                     StringData* name, TypedValue* stackPtr,
                     ActRec* fp);

  template<bool baseIsLocal>
  static void set(CacheHandle ch, ObjectData* base,
                  StringData* name, int64 val,
                  DataType type, ActRec* fp);

  static inline void incStat();
};

enum NameState {
  STATIC_NAME,
  DYN_NAME,
};

// These functions allocate a CacheHandle of the appropriate type and
// return a pointer to the C++ helper to call.
pcb_lookup_func_t propLookupPrep(CacheHandle& ch, const StringData* name,
                                 bool decRefBase, NameState ns);
pcb_set_func_t propSetPrep(CacheHandle& ch, const StringData* name,
                           bool decRefBase, NameState ns);

struct PropKey {
  Class* cls;
  bool operator==(const PropKey& other) {
    return cls == other.cls;
  }
  PropKey(Class* cls, StringData* name)
      : cls(cls) {}
  void destroy() {}
};

struct PropNameKey {
  Class* cls;
  StringData* name;
  PropNameKey(Class* cls, StringData* name)
      : cls(cls), name(name) {}
  bool operator==(const PropNameKey& other) {
    return cls == other.cls && name->same(other.name);
  }

  void destroy() {
    if (name && name->decRefCount() == 0) {
      name->release();
    }
  }
};

typedef PropCacheBase<PropKey, NSProperty> PropCache;
typedef PropCacheBase<PropNameKey> PropNameCache;

/*
 * GlobalCache --
 *
 *   Records offsets into the current global array.
 *
 *   For both GlobalCache and BoxedGlobalCache, the lookup routine may
 *   return NULL, but lookupCreate will create new entries with
 *   KindOfNull if the global didn't exist.
 *
 *   Both routines will decRef the name on behalf of the caller, but
 *   only if the lookup was successful (or a global was created).
 */
class GlobalCache {
  TypedValue* m_tv;

protected:
  static inline GlobalCache* cacheAtHandle(CacheHandle handle) {
    return (GlobalCache*)(uintptr_t(tl_targetCaches) + handle);
  }

  template<bool isBoxed>
  TypedValue* lookupImpl(StringData *name, bool allowCreate);

public:
  inline CacheHandle cacheHandle() const {
    return ptrToHandle(this);
  }

  static CacheHandle alloc(const StringData* sd) {
    ASSERT(sd);
    return namedAlloc<NSGlobal>(sd, sizeof(GlobalCache), sizeof(GlobalCache));
  }

  static TypedValue* lookup(CacheHandle handle, StringData* nm);
  static TypedValue* lookupCreate(CacheHandle handle, StringData* nm);
  static TypedValue* lookupCreateAddr(void* cacheAddr, StringData* nm);
};

class BoxedGlobalCache : public GlobalCache {
public:
  /*
   * Note: the returned pointer is a pointer to the outer variant.
   * You'll need to incref (or whatever) it yourself (if desired) and
   * emitDeref if you are going to put it in a register associated
   * with some vm location.  (Note that KindOfRef in-register
   * values are the pointers to inner items.)
   */
  static TypedValue* lookup(CacheHandle handle, StringData* nm);
  static TypedValue* lookupCreate(CacheHandle handle, StringData* nm);
};

/*
 * Classes.
 *
 * The request-private Class* for a given class name. This is used when
 * the class name is known at translation time.
 */
CacheHandle allocKnownClass(const Class* name);
CacheHandle allocKnownClass(const NamedEntity* name, bool persistent);
CacheHandle allocKnownClass(const StringData* name);
typedef Class* (*lookupKnownClass_func_t)(Class** cache,
                                          const StringData* clsName,
                                          bool isClass);
template<bool checkOnly>
Class* lookupKnownClass(Class** cache, const StringData* clsName,
                        bool isClass);
CacheHandle allocClassInitProp(const StringData* name);
CacheHandle allocClassInitSProp(const StringData* name);

CacheHandle allocFixedFunction(const NamedEntity* ne, bool persistent);
CacheHandle allocFixedFunction(const StringData* name);

/*
 * Constants.
 *
 * The request-private value of a constant. This one is a bit
 * different: we don't record a key, per se, expecting the translation
 * to remember it for us. When the targetcache area gets reset, the
 * translator must call fillConstant again.
 */
CacheHandle allocConstant(StringData* name);
void fillConstant(StringData* name);

CacheHandle allocClassConstant(StringData* name);
TypedValue* lookupClassConstant(TypedValue* cache,
                                const NamedEntity* ne,
                                const StringData* cls,
                                const StringData* cns);

/*
 * Static locals. Each StaticLocInit we translate gets its own soft
 * reference to the variant where it resides.
 */
CacheHandle allocStatic();

/*
 * Static properties.  We only cache statically known property name
 * references from within the class.  Current statistics shows in
 * class references dominating by 91.5% of all static property access.
 */

class SPropCache {
private:
  static inline SPropCache* cacheAtHandle(CacheHandle handle) {
    return (SPropCache*)(uintptr_t(tl_targetCaches) + handle);
  }
  CacheHandle allocConstantLocked(StringData* name);
public:
  TypedValue* m_tv;  // public; it is used from TC and we assert the offset
  static CacheHandle alloc(const StringData* sd = NULL) {
    return namedAlloc<NSSProp>(sd, sizeof(SPropCache), sizeof(SPropCache));
  }
  static TypedValue* lookup(CacheHandle handle, const Class* cls,
                            const StringData* nm);
};

} } } }

#endif
