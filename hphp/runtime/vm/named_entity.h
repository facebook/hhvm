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

#include "tbb/concurrent_unordered_map.h"
#include <boost/operators.hpp>

#include "hphp/util/atomic.h"
#include "folly/AtomicHashMap.h"

namespace HPHP {

class Class;
struct Typedef;
struct TypedefReq;
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
 * request at m_cachedTypedefOffset.  Classes are always cached at
 * m_cachedClassOffset.
 */
struct NamedEntity {
  explicit NamedEntity()
    : m_cachedClassOffset(0)
    , m_cachedFuncOffset(0)
    , m_cachedTypedefOffset(0)
    , m_clsList(nullptr)
  {}

  // Assigning these fields is protected by the targetcache lock.  We
  // read them without locks.
  unsigned m_cachedClassOffset;
  unsigned m_cachedFuncOffset;
  unsigned m_cachedTypedefOffset;

  void setCachedFunc(Func *f);
  Func* getCachedFunc() const;

  void setCachedClass(Class* c);
  Class* getCachedClass() const;

  const TypedefReq* getCachedTypedef() const;
  void setCachedTypedef(const TypedefReq&);

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
 * Lookup a Typedef* for the supplied NamedEntity (which must be the
 * NamedEntity for `name'), if necessary invoking autoload for types
 * but not classes.
 */
const TypedefReq* getTypedefWithAutoload(const NamedEntity* ne,
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
