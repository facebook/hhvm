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

#ifndef incl_HPHP_VM_NAMED_ENTITY_H_
#define incl_HPHP_VM_NAMED_ENTITY_H_

#include <tbb/concurrent_unordered_map.h>
#include <boost/operators.hpp>

#include "folly/AtomicHashMap.h"

#include "hphp/util/atomic.h"
#include "hphp/runtime/base/rds.h"
#include "hphp/runtime/vm/type-alias.h"

namespace HPHP {

class Class;
class Func;

//////////////////////////////////////////////////////////////////////

/*
 * NamedEntity represents a user-defined name that may map to
 * different objects in different requests.  Classes and functions are
 * in separate namespaces, so we have a targetcache offset for
 * resolving each.
 *
 * Classes and typedefs are in the same namespace when we're naming
 * types, but different namespaces at sites that allocate classes.  If
 * a typedef is defined for a given name, we'll cache it in each
 * request at m_cachedTypedef.  Classes are always cached at
 * m_cachedClass.
 */
struct NamedEntity {
  explicit NamedEntity()
    : m_cachedClass(RDS::kInvalidHandle)
    , m_cachedFunc(RDS::kInvalidHandle)
    , m_cachedTypeAlias(RDS::kInvalidHandle)
    , m_clsList(nullptr)
  {}

  mutable RDS::Link<Class*> m_cachedClass;
  mutable RDS::Link<Func*> m_cachedFunc;
  mutable RDS::Link<TypeAliasReq> m_cachedTypeAlias;

  /*
   * Get the RDS::Handle that caches this Class*, creating a
   * (non-persistent) one if it doesn't exist yet.
   */
  RDS::Handle getClassHandle() const {
    m_cachedClass.bind();
    return m_cachedClass.handle();
  }

  /*
   * Get the RDS::Handle that caches this Func*, creating a
   * (non-persistent) one if it doesn't exist yet.
   */
  RDS::Handle getFuncHandle() const {
    m_cachedFunc.bind();
    return m_cachedFunc.handle();
  }

  void setCachedFunc(Func *f);
  Func* getCachedFunc() const;

  void setCachedClass(Class* c);
  Class* getCachedClass() const;

  const TypeAliasReq* getCachedTypeAlias() const;
  void setCachedTypeAlias(const TypeAliasReq&);

  Class* clsList() const {
    return m_clsList;
  }

  // Call while holding s_classesMutex.  Add or remove classes from
  // the list.
  void pushClass(Class* cls);
  void removeClass(Class* goner);

private:
  Class* m_clsList;
};

//////////////////////////////////////////////////////////////////////

/*
 * Lookup a TypeAliasReq for the supplied NamedEntity (which must be
 * the NamedEntity for `name'), if necessary invoking autoload for
 * types but not classes.
 */
const TypeAliasReq* getTypeAliasWithAutoload(const NamedEntity* ne,
                                             const StringData* name);

//////////////////////////////////////////////////////////////////////

struct ahm_string_data_isame {
  bool operator()(const StringData *s1, const StringData *s2) const {
    // ahm uses -1, -2, -3 as magic values
    return int64_t(s1) > 0 && s1->isame(s2);
  }
};

typedef folly::AtomicHashMap<
  const StringData*,
  NamedEntity,
  string_data_hash,
  ahm_string_data_isame
> NamedEntityMap;
typedef std::pair<const StringData*,const NamedEntity*> NamedEntityPair;

//////////////////////////////////////////////////////////////////////

}

#endif
