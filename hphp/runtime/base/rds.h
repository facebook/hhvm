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
#ifndef incl_HPHP_RUNTIME_RDS_H_
#define incl_HPHP_RUNTIME_RDS_H_

#include "hphp/runtime/vm/func.h"
#include "hphp/util/util.h"
#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/unwind-x64.h"
#include "hphp/util/asm-x64.h"

namespace HPHP { namespace RDS {

//////////////////////////////////////////////////////////////////////

/*
 * The RDS (Request Data Segment) is a region of memory quickly
 * accessible to each hhvm thread that is running a PHP request.
 *
 * Essentially this is a per-thread memory region, along with an
 * internal dynamic link table to give the segment the same layout for
 * each thread as new data is allocated.
 *
 * The RDS starts with a small header that is statically layed out,
 * followed by the main segment, which is initialized to zero at the
 * start of each request.  The next section, contains "persistent"
 * data, which is data that retains the same values across requests.
 *
 * The persistent segment is implemented       RDS Layout:
 * by mapping the same physical pages to
 * different virtual addresses, so they          +------------+ <-- tl_base
 * are all accessible from the                   |  Header    |
 * per-thread RDS base.  The normal              +------------+
 * region is perhaps analogous to .bss,          |            |
 * while the persistent region is                |  Normal    |
 * analagous to .rodata.                         |    region  |
 *                                               |            |
 * When we're running in C++, the base           +------------+
 * of RDS is available via a thread              | Persistent | higher
 * local exported from this module               |     region |   addresses
 * (tl_base).  When running in                   +------------+
 * JIT-compiled code, a machine register
 * is reserved to always point at the base of RDS.
 *
 * There are several different types of data stored here; documented
 * in line below on the various entry points (TODO).  A common theme
 * is mapping some kind of PHP-level identifier to a runtime
 * structure, where the identifier may mean different things in
 * different requests, but once bound in any given request will retain
 * meaning until the end.  (E.g. class names to Class*.)
 *
 * Side note: this module originally only contained caches for things
 * like method call targets, so it was referred to as "target cache".
 * There are probably still a few references to that name still
 * around, so it seems worth mentioning ...
 */

//////////////////////////////////////////////////////////////////////

/*
 * Lifetime-related hooks, exported to be called at the appropriate
 * times.
 */
void requestInit();
void requestExit();
void threadInit();
void threadExit();

/*
 * Flushing RDS means to madvise the memory away.  Should only be done
 * while a request is not in flight on this thread.
 *
 * This is done to conserve memory if a particular thread is unlikely
 * to need to serve another PHP request for a while.
 */
void flush();

/*
 * Return the number of bytes that have been allocated from either
 * persistent or non-persistent RDS.
 */
size_t usedBytes();
size_t usedPersistentBytes();

/*
 * The thread-local pointer to the base of RDS.
 */
extern __thread void* tl_base;

//////////////////////////////////////////////////////////////////////

/*
 * Statically layed-out header that goes at the front of RDS.
 */
struct Header {
  /*
   * Surprise flags.  May be written by other threads.  At various
   * points, the runtime will check whether this word is non-zero, and
   * if so go to a slow path to handle unusual conditions (e.g. OOM).
   */
  ssize_t conditionFlags;

  /*
   * Used to pass values between unwinder code and catch traces.
   */
  int64_t unwinderScratch;
  TypedValue unwinderTv;
  bool doSideExit;
};

Header* header();

constexpr ptrdiff_t kConditionFlagsOff   = offsetof(Header, conditionFlags);
constexpr ptrdiff_t kUnwinderScratchOff  = offsetof(Header, unwinderScratch);
constexpr ptrdiff_t kUnwinderSideExitOff = offsetof(Header, doSideExit);
constexpr ptrdiff_t kUnwinderTvOff       = offsetof(Header, unwinderTv);

//////////////////////////////////////////////////////////////////////

/*
 * Values for dynamically defined constants are stored as key value
 * pairs in an array, accessible here.
 */
Array& s_constants();

enum PHPNameSpace {
  NSCtor,
  NSFixedCall,
  NSDynFunction,
  NSStaticMethod,
  NSStaticMethodF,
  NSClass,
  NSClsInitProp,
  NSClsInitSProp,

  NumInsensitive, NS_placeholder = NumInsensitive-1,

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
Handle namedAlloc(PHPNameSpace where, const StringData* name,
                  int numBytes, int align);

template<PHPNameSpace where>
Handle namedAlloc(const StringData* name, int numBytes, int align) {
  return namedAlloc<(where >= FirstCaseSensitive)>(where, name,
                                                   numBytes, align);
}

size_t allocBit();
Handle bitOffToHandleAndMask(size_t bit, uint8_t &mask);
bool testBit(Handle handle, uint32_t mask);
bool testBit(size_t bit);
bool testAndSetBit(Handle handle, uint32_t mask);
bool testAndSetBit(size_t bit);
bool isPersistentHandle(Handle handle);
bool classIsPersistent(const Class* cls);

Handle ptrToHandle(const void*);

template<typename T = void>
static inline T*
handleToPtr(Handle h) {
  assert(h < RuntimeOption::EvalJitTargetCacheSize);
  return (T*)((char*)tl_base + h);
}

template<class T>
T& handleToRef(Handle h) {
  return *static_cast<T*>(handleToPtr(h));
}

inline ssize_t* conditionFlagsPtr() {
  return &header()->conditionFlags;
}

inline ssize_t loadConditionFlags() {
  return atomic_acquire_load(conditionFlagsPtr());
}

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
  int KNLines = 4,
  typename ReturnValue = Value>
class Cache {
public:
  static const int kNumLines = KNLines;

  struct Pair {
    Key   m_key;
    Value m_value;
  } m_pairs[kNumLines];

  static inline Cache* cacheAtHandle(Handle handle) {
    return (Cache*)handleToPtr(handle);
  }

  inline Pair* keyToPair(Key k) {
    if (kNumLines == 1) {
      return &m_pairs[0];
    }
    assert(HPHP::Util::isPowerOfTwo(kNumLines));
    return m_pairs + (hashKey(k) & (kNumLines - 1));
  }

protected:
  // Each instance needs to implement this
  static int hashKey(Key k);

public:
  typedef Key CacheKey;
  typedef LookupKey CacheLookupKey;
  typedef Value CacheValue;

  static Handle alloc(const StringData* name = nullptr) {
    // Each lookup should access exactly one Pair so there's no point
    // in making sure the entire cache fits on one cache line.
    return namedAlloc<NameSpace>(name, sizeof(Cache), sizeof(Pair));
  }
  inline Handle handle() const {
    return ptrToHandle(this);
  }
  static void invalidate(Handle chand, Key lookup) {
    Pair* pair = cacheAtHandle(chand)->keyToPair(lookup);
    memset(pair, 0, sizeof(Pair));
  }
  static void invalidate(Handle chand) {
    auto const thiz = cacheAtHandle(chand);
    memset(thiz, 0, sizeof *thiz);
  }
  static ReturnValue lookup(Handle chand, LookupKey lookup,
                            const void* extraKey = nullptr);
};

struct FixedFuncCache {
  const Func* m_func;

  static inline FixedFuncCache* cacheAtHandle(Handle handle) {
    return (FixedFuncCache*)handleToPtr(handle);
  }

  static void invalidate(Handle handle) {
    FixedFuncCache* thiz = cacheAtHandle(handle);
    thiz->m_func = nullptr;
  }

  static const Func* lookupUnknownFunc(StringData* name);
};

struct StaticMethodCache {
  const Func* m_func;
  const Class* m_cls;
  static Handle alloc(const StringData* cls, const StringData* meth,
                           const char* ctxName);
  static const Func* lookupIR(Handle chand,
                              const NamedEntity* ne, const StringData* cls,
                              const StringData* meth, TypedValue* vmfp,
                              TypedValue* vmsp);
  static const Func* lookup(Handle chand,
                            const NamedEntity* ne, const StringData* cls,
                            const StringData* meth);
};

struct StaticMethodFCache {
  const Func* m_func;
  int m_static;

  static Handle alloc(const StringData* cls, const StringData* meth,
                           const char* ctxName);
  static const Func* lookupIR(Handle chand, const Class* cls,
                              const StringData* meth, TypedValue* vmfp);
};

typedef Cache<uintptr_t, const Func*, ActRec*, NSInvalid, 1, void>
  MethodCache;
typedef Cache<StringData*, const Class*, StringData*, NSClass> ClassCache;

typedef Cache<const StringData*, const Func*, StringData*, NSDynFunction>
  FuncCache;

/*
 * In order to handle fb_rename_function (when it is enabled), we need
 * to invalidate dynamic function call caches (the FuncCache).  This
 * hook is called when fb_rename_function is used.
 */
void invalidateForRenameFunction(const StringData* name);

/*
 * Classes.
 *
 * The request-private Class* for a given class name. This is used when
 * the class name is known at translation time.
 */
Handle allocKnownClass(const Class* name);
Handle allocKnownClass(const NamedEntity* name, bool persistent);
Handle allocKnownClass(const StringData* name);
typedef Class* (*lookupKnownClass_func_t)(Class** cache,
                                          const StringData* clsName,
                                          bool isClass);
template<bool checkOnly>
Class* lookupKnownClass(Class** cache, const StringData* clsName,
                        bool isClass);
Handle allocClassInitProp(const StringData* name);
Handle allocClassInitSProp(const StringData* name);

/*
 * Functions.
 */
Handle allocFixedFunction(const NamedEntity* ne, bool persistent);
Handle allocFixedFunction(const StringData* name);

/*
 * Type aliases.
 *
 * Request-private values for type aliases (typedefs).  When a typedef
 * is defined, the entry for it is cached.  This reserves enough space
 * for a TypedefReq struct.
 */
Handle allocTypedef(const NamedEntity* name);

/*
 * Constants.
 *
 * The request-private value of a constant.
 */
Handle allocConstant(uint32_t* handlep, bool persistent);

Handle allocClassConstant(StringData* name);
TypedValue lookupClassConstantTv(TypedValue* cache,
                                 const NamedEntity* ne,
                                 const StringData* cls,
                                 const StringData* cns);

/*
 * Non-scalar class constants are stored in RDS slots as
 * Arrays.
 */
Handle allocNonScalarClassConstantMap(unsigned* handleOut);

/*
 * Static locals.
 *
 * For normal functions, static locals are allocated as RefData's that
 * live in RDS.  Note that we don't put closures or
 * generatorFromClosure locals here because they are per-instance.
 */
Handle allocStaticLocal(const Func*, const StringData*);

/*
 * Static properties.  We only cache statically known property name
 * references from within the class.  Current statistics shows in
 * class references dominating by 91.5% of all static property access.
 */
class SPropCache {
private:
  static inline SPropCache* cacheAtHandle(Handle handle) {
    return (SPropCache*)(uintptr_t(tl_base) + handle);
  }
public:
  TypedValue* m_tv;  // public; it is used from TC and we assert the offset
  static Handle alloc(const StringData* sd = nullptr) {
    return namedAlloc<NSSProp>(sd, sizeof(SPropCache), sizeof(SPropCache));
  }

  template<bool raiseOnError>
  static TypedValue* lookup(Handle handle, const Class* cls,
                            const StringData* nm, Class* ctx);

  template<bool raiseOnError>
  static TypedValue* lookupSProp(const Class *cls, const StringData *name,
                                 Class* ctx);
};

void methodCacheSlowPath(MethodCache::Pair* mce,
                         ActRec* ar,
                         StringData* name,
                         Class* cls);

//////////////////////////////////////////////////////////////////////

}}

#include "hphp/runtime/base/rds-inl.h"

#endif
