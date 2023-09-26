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
#pragma once

#include "hphp/runtime/base/rds.h"
#include "hphp/runtime/vm/jit/types.h"

namespace HPHP {
struct ActRec;
struct ArrayData;
struct Class;
struct Func;
struct NamedType;
struct PackageInfo;
struct StringData;
struct TypedValue;
}

namespace HPHP::jit {

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

/*
 * Per-callsite dynamic type structure class lookups (where the type structure
 * isn't known at translation time).  4-way cache.
 */
struct TSClassCache {
  static constexpr uint32_t kNumLines = 4;

  struct Pair {
    ArrayData*  m_key;
    LowPtr<const Class> m_value;
  };

  static LowPtr<const Class> write(rds::Handle, ArrayData* lookup);

  Pair m_pairs[kNumLines];
};


//////////////////////////////////////////////////////////////////////

struct StaticMethodCache {
  LowPtr<const Func> m_func;
  LowPtr<const Class> m_cls;

  static rds::Handle alloc(const StringData* cls,
                      const StringData* meth,
                      const StringData* ctxName);
  static const Func* lookup(rds::Handle chand,
                            const NamedType* ne, const StringData* cls,
                            const StringData* meth, const Class* ctx,
                            const Func* callerFunc, const PackageInfo& packageInfo);
};

struct StaticMethodFCache {
  LowPtr<const Func> m_func;
  int m_static;

  static rds::Handle alloc(const StringData* cls,
                      const StringData* meth,
                      const StringData* ctxName);
  static const Func* lookup(rds::Handle chand, const Class* cls,
                            const StringData* meth, const Class* ctx,
                            const Func* callerFunc, const PackageInfo& packageInfo);
};

//////////////////////////////////////////////////////////////////////

namespace MethodCache {

/*
 * One-way request-local cache for object method lookups.
 *
 * MethodCache entries cache the dispatch target for an object method call.
 * Each line consists of a Class* key and a Func* (stored as a uintptr_t).
 */
struct Entry {
  const Class* m_key;
  const Func* m_value;
};

const Func* handleDynamicCall(const Class* cls,
                              const StringData* name,
                              const Class* ctx,
                              const Func* callerFunc);

const Func* handleStaticCall(const Class* cls,
                             const StringData* name,
                             const Class* ctx,
                             const Func* callerFunc,
                             rds::Handle mce_handle,
                             uintptr_t mcePrime);

} // namespace MethodCache

//////////////////////////////////////////////////////////////////////

}
