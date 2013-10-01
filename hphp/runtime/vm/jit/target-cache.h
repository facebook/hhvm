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
#ifndef incl_HPHP_RUNTIME_VM_JIT_TARGETCACHE_H_
#define incl_HPHP_RUNTIME_VM_JIT_TARGETCACHE_H_

#include "hphp/util/util.h"
#include "hphp/runtime/base/types.h"
#include "hphp/runtime/base/rds.h"

namespace HPHP {
  struct Func;
  struct ActRec;
  struct StringData;
  struct TypedValue;
  struct Class;
  struct NamedEntity;
}

namespace HPHP { namespace JIT {

//////////////////////////////////////////////////////////////////////

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
  int KNLines = 4,
  typename ReturnValue = Value>
class Cache {
public:
  static const int kNumLines = KNLines;

  struct Pair {
    Key   m_key;
    Value m_value;
  } m_pairs[kNumLines];

  inline Pair* keyToPair(Key k) {
    if (kNumLines == 1) {
      return &m_pairs[0];
    }
    assert(HPHP::Util::isPowerOfTwo(kNumLines));
    return m_pairs + (hashKey(k) & (kNumLines - 1));
  }

private:
  // Each instance needs to implement this
  static int hashKey(Key k);

public:
  typedef Key CacheKey;
  typedef LookupKey CacheLookupKey;
  typedef Value CacheValue;

  static RDS::Handle alloc() {
    return HPHP::RDS::alloc<Cache,sizeof(Pair)>().handle();
  }
  static ReturnValue lookup(RDS::Handle chand, LookupKey lookup,
                            const void* extraKey = nullptr);
};

struct StaticMethodCache {
  const Func* m_func;
  const Class* m_cls;
  static RDS::Handle alloc(const StringData* cls,
                      const StringData* meth,
                      const char* ctxName);
  static const Func* lookupIR(RDS::Handle chand,
                              const NamedEntity* ne, const StringData* cls,
                              const StringData* meth, TypedValue* vmfp,
                              TypedValue* vmsp);
  static const Func* lookup(RDS::Handle chand,
                            const NamedEntity* ne, const StringData* cls,
                            const StringData* meth);
};

struct StaticMethodFCache {
  const Func* m_func;
  int m_static;

  static RDS::Handle alloc(const StringData* cls,
                      const StringData* meth,
                      const char* ctxName);
  static const Func* lookupIR(RDS::Handle chand, const Class* cls,
                              const StringData* meth, TypedValue* vmfp);
};

typedef Cache<StringData*,const Class*,StringData*> ClassCache;
typedef Cache<const StringData*,const Func*,StringData*> FuncCache;

template<> RDS::Handle FuncCache::alloc();
template<> const Func* FuncCache::lookup(RDS::Handle,
  StringData*, const void* extraKey);
template<> const Class* ClassCache::lookup(RDS::Handle,
  StringData*, const void* extraKey);

/*
 * In order to handle fb_rename_function (when it is enabled), we need
 * to invalidate dynamic function call caches (the FuncCache).  This
 * hook is called when fb_rename_function is used.
 */
void invalidateForRenameFunction(const StringData* name);

/*
 * Static properties.
 *
 * We only cache statically known property name references from within
 * the class.  Current statistics shows in class references dominating
 * by 91.5% of all static property access.
 */
struct SPropCache {
  TypedValue* m_tv;  // public; it is used from TC and we assert the offset

  static RDS::Handle alloc(const StringData* sd);

  template<bool raiseOnError>
  static TypedValue* lookup(RDS::Handle handle, const Class* cls,
                            const StringData* nm, Class* ctx);

  template<bool raiseOnError>
  static TypedValue* lookupSProp(const Class *cls, const StringData *name,
                                 Class* ctx);
};

struct MethodCache {
  struct Pair {
    uintptr_t m_key;
    const Func* m_value;
  } m_pairs[1];

  inline Pair* keyToPair(uintptr_t k) {
    return &m_pairs[0];
  }

  static int hashKey(uintptr_t);

  static RDS::Handle alloc() {
    return ::HPHP::RDS::alloc<MethodCache,alignof(Pair)>()
      .handle();
  }
};

void methodCacheSlowPath(MethodCache::Pair* mce,
                         ActRec* ar,
                         StringData* name,
                         Class* cls);

//////////////////////////////////////////////////////////////////////

}}

#endif
