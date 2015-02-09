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

#ifndef incl_HPHP_VM_NAMED_ENTITY_H_
#define incl_HPHP_VM_NAMED_ENTITY_H_

#include "hphp/runtime/base/rds.h"
#include "hphp/runtime/base/string-data.h"
#include "hphp/runtime/vm/type-alias.h"

#include "hphp/util/portability.h"

#include <folly/AtomicHashMap.h>

#include <atomic>
#include <utility>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class Class;
class Func;
class String;

///////////////////////////////////////////////////////////////////////////////

/*
 * StringData* comparison for AtomicHashMap entries, where -1, -2, and -3 are
 * used as magic values.
 */
struct ahm_string_data_isame {
  bool operator()(const StringData *s1, const StringData *s2) const {
    return int64_t(s1) > 0 && s1->isame(s2);
  }
};

///////////////////////////////////////////////////////////////////////////////

/*
 * NamedEntity represents a user-defined name that may map to different objects
 * in different requests.
 *
 * Classes and functions are in separate namespaces, so we have a target cache
 * offset for resolving each.
 *
 * Classes and typedefs are in the same namespace when we're naming types, but
 * different namespaces at sites that allocate classes.  If a typedef is
 * defined for a given name, we'll cache it in each request at m_cachedTypedef.
 * Classes are always cached at m_cachedClass.
 */
struct NamedEntity {

  /////////////////////////////////////////////////////////////////////////////
  // Types.

  /*
   * Global NamedEntity map type.
   *
   * We hold onto references to elements of this map.  If we use a different
   * map, we must use one that doesnt invalidate references to its elements
   * (unless they are deleted, which never happens here).  Any standard
   * associative container will meet this requirement.
   */
  typedef folly::AtomicHashMap<const StringData*,
                               NamedEntity,
                               string_data_hash,
                               ahm_string_data_isame> Map;

  /////////////////////////////////////////////////////////////////////////////
  // Constructors.

  explicit NamedEntity()
    : m_cachedClass(rds::kInvalidHandle)
    , m_cachedFunc(rds::kInvalidHandle)
    , m_cachedTypeAlias(rds::kInvalidHandle)
  {}

  NamedEntity(NamedEntity&& ne) noexcept;


  /////////////////////////////////////////////////////////////////////////////
  // Func cache.

  /*
   * Get the rds::Handle that caches this Func*, creating a (non-persistent)
   * one if it doesn't exist yet.
   */
  rds::Handle getFuncHandle() const;

  /*
   * Set and get the cached Func*.
   */
  void setCachedFunc(Func *f);
  Func* getCachedFunc() const;


  /////////////////////////////////////////////////////////////////////////////
  // Class cache.

  /*
   * Get the rds::Handle that caches this Class*, creating a (non-persistent)
   * one if it doesn't exist yet.
   */
  rds::Handle getClassHandle() const;

  /*
   * Set and get the cached Class*.
   */
  void setCachedClass(Class* c);
  Class* getCachedClass() const;


  /////////////////////////////////////////////////////////////////////////////
  // Type alias cache.

  /*
   * Is the cached type alias persistent?
   */
  bool isPersistentTypeAlias() const;

  /*
   * Set and get the cached TypeAliasReq.
   */
  void setCachedTypeAlias(const TypeAliasReq&);
  const TypeAliasReq* getCachedTypeAlias() const;


  /////////////////////////////////////////////////////////////////////////////
  // Class list.

  /*
   * Return the head of the Class* list.
   *
   * The list is chained together by Class::m_nextClass.
   */
  Class* clsList() const;

  /*
   * Add or remove Classes from the list.
   *
   * Should be called while holding g_classesMutex.
   */
  void pushClass(Class* cls);
  void removeClass(Class* goner);


  /////////////////////////////////////////////////////////////////////////////
  // Global table.                                                     [static]

  /*
   * Get the NamedEntity for `str' from the global table, or create it if it
   * doesn't exist and `allowCreate' is true.
   *
   * If `str' needs to be namespace-normalized, we pass the normalized result
   * out through `normalizedStr' if it is provided.
   */
  static NamedEntity* get(const StringData* str,
                          bool allowCreate = true,
                          String* normalizedStr = nullptr) FLATTEN;

  /*
   * The global NamedEntity table.
   *
   * TODO(#4717225) Get rid of this.
   */
  static Map* table();

  /*
   * Size of the global NamedEntity table.
   */
  static size_t tableSize();


  /////////////////////////////////////////////////////////////////////////////
  // Data members.

public:
  mutable rds::Link<Class*> m_cachedClass;
  mutable rds::Link<Func*> m_cachedFunc;
  mutable rds::Link<TypeAliasReq> m_cachedTypeAlias;

private:
  std::atomic<Class*> m_clsList{nullptr};
};

/*
 * Litstr and NamedEntity pair.
 */
using NamedEntityPair = std::pair<const StringData*, const NamedEntity*>;

///////////////////////////////////////////////////////////////////////////////
}

#define incl_HPHP_VM_NAMED_ENTITY_INL_H_
#include "hphp/runtime/vm/named-entity-inl.h"
#undef incl_HPHP_VM_NAMED_ENTITY_INL_H_

#endif // incl_HPHP_VM_NAMED_ENTITY_INL_H_
