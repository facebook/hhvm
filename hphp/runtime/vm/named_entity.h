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

#include "runtime/base/complex_types.h"

#include "util/atomic.h"

namespace HPHP { namespace VM {

class Class;
class Typedef;
class Func;

//////////////////////////////////////////////////////////////////////

/*
 * A NameDef is a definition of a type for a given name.  This
 * currently means it is either a Class*, a Typedef*, or null.
 */
struct NameDef : private boost::equality_comparable<NameDef> {
  explicit NameDef() : m_p(0) {}
  explicit NameDef(Class* c) : m_p(reinterpret_cast<uintptr_t>(c)) {
    assert(!(m_p & 0x1));
  }

  explicit NameDef(Typedef* t)
    : m_p(reinterpret_cast<uintptr_t>(t) | 0x1)
  {}

  Class* asClass() const {
    return m_p & 0x1 ? nullptr : reinterpret_cast<Class*>(m_p);
  }

  Typedef* asTypedef() const {
    return m_p & 0x1 ? reinterpret_cast<Typedef*>(m_p - 1) : nullptr;
  }

  explicit operator bool() const { return !!m_p; }

  bool operator==(NameDef other) const {
    return m_p == other.m_p;
  }

private:
  uintptr_t m_p;
};

/*
 * NamedEntity represents a user-defined name that may map to
 * different objects (NameDefs) in different requests.  Classes and
 * functions are in separate namespaces, so we have a targetcache
 * offset for resolving each.
 *
 * Classes and typedefs are in the same namespace when we're naming
 * types, but different namespaces at sites that allocate classes.  If
 * a typedef is defined for a given name, we'll cache a NameDef in
 * each request at m_cachedNameDefOffset.  Classes are always cached
 * at m_cachedClassOffset.  See TargetCache::allocNameDef.
 */
struct NamedEntity {
  explicit NamedEntity()
    : m_cachedClassOffset(0)
    , m_cachedFuncOffset(0)
    , m_cachedNameDefOffset(0)
    , m_clsList(nullptr)
  {}

  unsigned m_cachedClassOffset;
  unsigned m_cachedFuncOffset;
  unsigned m_cachedNameDefOffset;

  void setCachedFunc(Func *f);
  Func* getCachedFunc() const;

  void setCachedClass(Class* c);
  Class* getCachedClass() const;

  NameDef getCachedNameDef() const;
  void setCachedNameDef(NameDef);

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

typedef tbb::concurrent_unordered_map<
  const StringData*,
  NamedEntity,
  string_data_hash,
  string_data_isame
> NamedEntityMap;
typedef std::pair<const StringData*,const NamedEntity*> NamedEntityPair;

//////////////////////////////////////////////////////////////////////

}}

#endif
