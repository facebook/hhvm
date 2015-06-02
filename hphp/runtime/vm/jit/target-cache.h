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
#ifndef incl_HPHP_RUNTIME_VM_JIT_TARGETCACHE_H_
#define incl_HPHP_RUNTIME_VM_JIT_TARGETCACHE_H_

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

namespace HPHP { namespace jit {

//////////////////////////////////////////////////////////////////////

/*
 * Per-callsite dynamic function name lookups (where the name of the
 * function isn't known at translation time).  4-way cache.
 */
struct FuncCache {
  static constexpr uint32_t kNumLines = 4;

  struct Pair {
    StringData*  m_key;
    const Func*  m_value;
  };

  static rds::Handle alloc();
  static const Func* lookup(rds::Handle, StringData* lookup);

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
  static constexpr uint32_t kNumLines = 4;

  struct Pair {
    StringData*  m_key;
    const Class* m_value;
  };

  static rds::Handle alloc();
  static const Class* lookup(rds::Handle, StringData* lookup);

  Pair m_pairs[kNumLines];
};

//////////////////////////////////////////////////////////////////////

struct StaticMethodCache {
  const Func* m_func;
  const Class* m_cls;

  static rds::Handle alloc(const StringData* cls,
                      const StringData* meth,
                      const char* ctxName);
  static const Func* lookup(rds::Handle chand,
                            const NamedEntity* ne, const StringData* cls,
                            const StringData* meth, TypedValue* vmfp);
};

struct StaticMethodFCache {
  const Func* m_func;
  int m_static;

  static rds::Handle alloc(const StringData* cls,
                      const StringData* meth,
                      const char* ctxName);
  static const Func* lookup(rds::Handle chand, const Class* cls,
                            const StringData* meth, TypedValue* vmfp);
};

//////////////////////////////////////////////////////////////////////

namespace MethodCache {

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
struct Entry {
  uintptr_t m_key;
  const Func* m_value;
};

template<bool fatal>
void handlePrimeCacheInit(Entry* mce,
                          ActRec* ar,
                          StringData* name,
                          Class* cls,
                          Class* ctx,
                          uintptr_t rawTarget);

} // namespace MethodCache

//////////////////////////////////////////////////////////////////////

}}

#endif
