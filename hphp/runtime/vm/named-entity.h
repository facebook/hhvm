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
#include "hphp/runtime/base/string-data.h"
#include "hphp/runtime/vm/type-alias.h"

#include "hphp/util/portability.h"
#include "hphp/util/low-ptr.h"
#include "hphp/util/alloc.h"

#include <folly/AtomicHashMap.h>

#include <atomic>
#include <utility>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

struct Func;
struct String;

///////////////////////////////////////////////////////////////////////////////

/*
 * StringData* comparison for AtomicHashMap entries, where -1, -2, and -3 are
 * used as magic values. Optimized for comparisons between static strings.
 */
struct ahm_string_data_isame {
  bool operator()(const StringData *s1, const StringData *s2) const {
    assertx(int64_t(s2) > 0);  // RHS is never a magic value.
    return s1 == s2 || (int64_t(s1) > 0 && s1->isame(s2));
  }
};

///////////////////////////////////////////////////////////////////////////////

/*
 * NamedType represents a user-defined type that may map to different objects
 * in different requests.
 *
 * Classes and type aliases are in the same namespace when we're naming types,
 * but different namespaces at sites that allocate classes. If a type alias is
 * defined for a given name, we cache it in each request at m_cachedTypeAlias.
 * Classes are always cached at m_cachedClass.
 */
struct NamedType {

  /////////////////////////////////////////////////////////////////////////////
  // Types.

  /*
   * Global NamedType map type.
   *
   * We hold onto references to elements of this map.  If we use a different
   * map, we must use one that doesnt invalidate references to its elements
   * (unless they are deleted, which never happens here).  Any standard
   * associative container will meet this requirement.
   */
  using Map = folly::AtomicHashMap<const StringData*,
                               NamedType,
                               string_data_hash,
                               ahm_string_data_isame,
                               LowAllocator<char>>;

  /////////////////////////////////////////////////////////////////////////////
  // Constructors.

  explicit NamedType() {}

  NamedType(NamedType&& ne) noexcept;
  NamedType& operator=(NamedType&&) = delete;

  /////////////////////////////////////////////////////////////////////////////
  // Class cache.

  /*
   * Get the rds::Handle that caches this Class*, creating a (non-persistent)
   * one if it doesn't exist yet.
   */
  rds::Handle getClassHandle(const StringData* name) const;

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
   * Set and get the cached TypeAlias.
   */
  void setCachedTypeAlias(const TypeAlias&);
  const TypeAlias* getCachedTypeAlias() const;

  /////////////////////////////////////////////////////////////////////////////
  // Reified generic cache.

  /*
   * Set and get the cached ReifiedGenerics.
   */
  void setCachedReifiedGenerics(ArrayData*);
  ArrayData* getCachedReifiedGenerics() const;


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
   * Get the NamedType for `str' from the global table, or create it if it
   * doesn't exist and `allowCreate' is true.
   *
   * If `str' needs to be namespace-normalized, we pass the normalized result
   * out through `normalizedStr' if it is provided.
   */
  static NamedType* get(const StringData* str,
                        bool allowCreate = true,
                        String* normalizedStr = nullptr) FLATTEN;

  /*
   * Visitors that traverse the named type table
   */
  template<class Fn> static void foreach_class(Fn fn);
  template<class Fn> static void foreach_cached_class(Fn fn);
  template<class Fn> static void foreach_name(Fn);

  template<class T>
  const char* checkSameName();

private:
  static Map* types();

  /////////////////////////////////////////////////////////////////////////////
  // Data members.

public:
  mutable rds::Link<LowPtr<Class>, rds::Mode::NonLocal> m_cachedClass;
  union {
    mutable rds::Link<TypeAlias, rds::Mode::NonLocal> m_cachedTypeAlias{};
    mutable rds::Link<ArrayData*, rds::Mode::NonLocal> m_cachedReifiedGenerics;
  };

  template<class T>
  using ListType = AtomicLowPtr<T, std::memory_order_acquire,
                                   std::memory_order_release>;
private:
  ListType<Class> m_clsList{nullptr};
};

/*
 * NamedFunc represents a user-defined function that may map to different funcs
 * in different requests.
 */
struct NamedFunc {

  /////////////////////////////////////////////////////////////////////////////
  // Types.

  /*
   * Global NamedFunc map type.
   *
   * We hold onto references to elements of this map.  If we use a different
   * map, we must use one that doesnt invalidate references to its elements
   * (unless they are deleted, which never happens here).  Any standard
   * associative container will meet this requirement.
   */
  using Map = folly::AtomicHashMap<const StringData*,
                               NamedFunc,
                               string_data_hash,
                               ahm_string_data_isame,
                               LowAllocator<char>>;

  /////////////////////////////////////////////////////////////////////////////
  // Constructors & Assignment.

  explicit NamedFunc() {}

  NamedFunc(NamedFunc&& ne) noexcept;
  NamedFunc& operator=(NamedFunc&&) = delete;

  /////////////////////////////////////////////////////////////////////////////
  // Func cache.

  /*
   * Get the rds::Handle that caches this Func*, creating a (non-persistent)
   * one if it doesn't exist yet.
   */
  rds::Handle getFuncHandle(const StringData* name) const;

  /*
   * Set and get the cached Func*.
   */
  void setCachedFunc(Func *f);
  Func* getCachedFunc() const;

  /////////////////////////////////////////////////////////////////////////////
  // Global table.                                                     [static]

  /*
   * Get the NamedFunc for `str' from the global table, or create it if it
   * doesn't exist and `allowCreate' is true.
   *
   * If `str' needs to be namespace-normalized, we pass the normalized result
   * out through `normalizedStr' if it is provided.
   */
  static NamedFunc* get(const StringData* str,
                        bool allowCreate = true,
                        String* normalizedStr = nullptr) FLATTEN;

  template<class Fn> static void foreach_cached_func(Fn fn);
  template<class Fn> static void foreach_name(Fn);

private:
  static Map* funcs();

  /////////////////////////////////////////////////////////////////////////////
  // Data members.

public:
  mutable rds::Link<LowPtr<Func>, rds::Mode::NonLocal> m_cachedFunc;
};

using NamedTypePair = std::pair<LowStringPtr,LowPtr<const NamedType>>;
using NamedFuncPair = std::pair<LowStringPtr,LowPtr<const NamedFunc>>;

/*
 * Size of the combined NamedType and NamedFunc tables.
 */
size_t namedEntityTableSize();

/*
 * Various stats about the combined NamedType and NamedFunc tables,
 * excluding their size.
 */
std::vector<std::pair<const char*, int64_t>> namedEntityStats();

}

#define incl_HPHP_VM_NAMED_ENTITY_INL_H_
#include "hphp/runtime/vm/named-entity-inl.h"
#undef incl_HPHP_VM_NAMED_ENTITY_INL_H_
