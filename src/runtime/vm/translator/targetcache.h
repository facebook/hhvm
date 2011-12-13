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
#include <runtime/vm/translator/asm-x64.h>
#include <boost/static_assert.hpp>

namespace HPHP {
namespace VM {
namespace Transl {
namespace TargetCache {

void requestInit(void);

/*
 * The targetCaches are physically thread-private, but they share their
 * layout. So the memory is in tl_targetCaches, but we allocate it via the
 * global s_frontier. This is protected by the translator's write-lease.
 */
extern __thread HPHP::x64::DataBlock tl_targetCaches;
extern size_t s_frontier;

/*
 * Some caches have different numbers of lines. This is our default.
 */
static const int kDefaultNumLines = 4;

/*
 * The lookup functions are called into from generated assembly and passed an
 * opaque handle into the request-private targetcache.
 */
typedef ptrdiff_t CacheHandle;
static const ptrdiff_t kNumTargetCacheBytes = (1 << 24);

enum PHPNameSpace {
  NSFunction,
  NSDynFunction,
  NSClass,
  NSConstant,
  NSGlobal,
  NSSProp,

  NumNameSpaces,
  NSInvalid = -1
};

CacheHandle namedAlloc(PHPNameSpace where, const StringData* name,
                       int numBytes, int align = 16);
CacheHandle ptrToHandle(const void*);
void* handleToPtr(CacheHandle h);
void invalidateFuncName(const StringData* name);

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
  int KNLines = kDefaultNumLines, int KAlign=64>
class Cache {
public:
  typedef Cache<Key, Value, LookupKey, NameSpace, KNLines, KAlign> Self;
protected:
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

  // Each instance needs to implement this
  static int hashKey(Key k);

public:
  typedef Key CacheKey;
  typedef LookupKey CacheLookupKey;
  typedef Value CacheValue;

  static CacheHandle alloc(const StringData* name = NULL) {
    return namedAlloc(NameSpace, name, sizeof(Self), KAlign);
  }
  inline CacheHandle cacheHandle() const {
    return ptrToHandle(this);
  }
  static void invalidate(CacheHandle chand, Key lookup) {
    Pair* pair = cacheAtHandle(chand)->keyToPair(lookup);
    memset(pair, 0, sizeof(Pair));
  }
  static Value lookup(CacheHandle chand, LookupKey lookup,
                      const void* extraKey = NULL);
};

class FixedFuncCache {
public:
  const Func* m_func;

  static inline FixedFuncCache* cacheAtHandle(CacheHandle handle) {
    return (FixedFuncCache*)handleToPtr(handle);
  }

  static CacheHandle alloc(const StringData* name) {
    return namedAlloc(NSFunction, name,
                      sizeof(FixedFuncCache), sizeof(FixedFuncCache));
  }

  static void invalidate(CacheHandle handle) {
    FixedFuncCache* thiz = cacheAtHandle(handle);
    thiz->m_func = NULL;
  }

  static const Func* lookup(CacheHandle chand, StringData* sd);
};

struct MethodCacheEntry {
  intptr_t m_data;
  void set(const Func* func, bool isMagicCall) {
    ASSERT(func);
    ASSERT((intptr_t(func) & 1) == 0);
    m_data = intptr_t(func) | intptr_t(isMagicCall);
  }
  bool isMagicCall() const {
    return (m_data & 1);
  }
  const Func* getFunc() const {
    return (const Func*)(m_data & ~1);
  }
};

typedef Cache<const StringData*, const Func*, StringData*, NSDynFunction>
  FuncCache;
typedef Cache<const Class*, MethodCacheEntry, ActRec*> MethodCache;
typedef Cache<const Func*, TCA, ActRec*> CallCache;
typedef Cache<StringData*, const Class*, StringData*, NSClass> ClassCache;

/*
 * GlobalCache --
 *
 *   Records offsets into the current global array.
 */
class GlobalCache {
  HphpArray* m_globals;
  int m_hint;

  void checkGlobals();

protected:
  static inline GlobalCache* cacheAtHandle(CacheHandle handle) {
    return (GlobalCache*)(uintptr_t(tl_targetCaches.base) + handle);
  }

public:
  inline CacheHandle cacheHandle() const {
    return ptrToHandle(this);
  }

  static CacheHandle alloc(const StringData* sd = NULL) {
    return namedAlloc(NSGlobal, sd, sizeof(GlobalCache));
  }

  template<bool isBoxed>
  TypedValue* lookupImpl(StringData *name);

  static TypedValue* lookup(CacheHandle handle, StringData* nm);
};

class BoxedGlobalCache : public GlobalCache {
public:
  static TypedValue* lookup(CacheHandle handle, StringData* nm);
};

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

/*
 * Static properties.  We only cache statically known property name
 * refernces from within the class.  Current statistics shows in
 * class references dominating by 91.5% of all static property access.
 */

class SPropCache {
private:
  static inline SPropCache* cacheAtHandle(CacheHandle handle) {
    return (SPropCache*)(uintptr_t(tl_targetCaches.base) + handle);
  }
public:
  TypedValue* m_tv;  // public; it is used from TC and we assert the offset
  static CacheHandle alloc(const StringData* sd = NULL) {
    return namedAlloc(NSSProp, sd, sizeof(SPropCache));
  }
  static TypedValue* lookup(CacheHandle handle, const Class* cls,
                            const StringData* nm);
};

} } } }

#endif
