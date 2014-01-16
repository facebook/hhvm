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
#include "hphp/runtime/vm/jit/types.h"

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
 * Per-callsite dynamic function name lookups (where the name of the
 * function isn't known at translation time).  4-way cache.
 */
struct FuncCache {
  static constexpr int kNumLines = 4;

  struct Pair {
    StringData*  m_key;
    const Func*  m_value;
  };

  static RDS::Handle alloc();
  static const Func* lookup(RDS::Handle, StringData* lookup);

  Pair m_pairs[kNumLines];
};

/*
 * In order to handle fb_rename_function (when it is enabled), we need
 * to invalidate dynamic function call caches (the FuncCache).  This
 * hook is called when fb_rename_function is used.
 */
void invalidateForRenameFunction(const StringData* name);

//////////////////////////////////////////////////////////////////////

/*
 * Per-callsite dynamic class name lookups (where the name of the
 * class isn't known at translation time).  4-way cache.
 */
struct ClassCache {
  static constexpr int kNumLines = 4;

  struct Pair {
    StringData*  m_key;
    const Class* m_value;
  };

  static RDS::Handle alloc();
  static const Class* lookup(RDS::Handle, StringData* lookup);

  Pair m_pairs[kNumLines];
};

//////////////////////////////////////////////////////////////////////

struct StaticMethodCache {
  const Func* m_func;
  const Class* m_cls;

  static RDS::Handle alloc(const StringData* cls,
                      const StringData* meth,
                      const char* ctxName);
  static const Func* lookup(RDS::Handle chand,
                            const NamedEntity* ne, const StringData* cls,
                            const StringData* meth, TypedValue* vmfp);
};

struct StaticMethodFCache {
  const Func* m_func;
  int m_static;

  static RDS::Handle alloc(const StringData* cls,
                      const StringData* meth,
                      const char* ctxName);
  static const Func* lookup(RDS::Handle chand, const Class* cls,
                            const StringData* meth, TypedValue* vmfp);
};

//////////////////////////////////////////////////////////////////////

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

//////////////////////////////////////////////////////////////////////

/*
 * Method cache entries cache the dispatch target for a function call.
 * The key is a Class*, but the low bits are reused for other
 * purposes.  The fast path in the TC doesn't have to check these
 * bits---it just checks if m_key is bitwise equal to the candidate
 * Class* it has, and if so it accepts m_value.
 *
 * The MethodCache line consists of a Class* key (stored as a
 * uintptr_t) and a Func*.  The low bit of the key is set if the
 * function call is a magic call (in which case the cached Func* is
 * the __call function).  The second lowest bit of the key is set if
 * the cached Func has AttrStatic.
 */
struct MethodCache {
  uintptr_t m_key;
  const Func* m_value;
};

/*
 * When we first create method cache entries, we need some information
 * for pmethodCacheMissPath to set up an immediate for the first
 * dispatched function.  A pointer to one of these is passed in
 * pdataRaw to pmethodCacheMissPath.
 */
struct MethodCachePrimeData {
  JIT::TCA smashImmAddr;
  JIT::TCA retAddr;
};

template<bool fatal>
void methodCacheSlowPath(MethodCache* mce,
                         ActRec* ar,
                         StringData* name,
                         Class* cls,
                         uintptr_t mcePrime);

template<bool fatal>
void pmethodCacheMissPath(MethodCache* mce,
                          ActRec* ar,
                          StringData* name,
                          Class* cls,
                          uintptr_t pdataRaw);

//////////////////////////////////////////////////////////////////////

}}

#endif
