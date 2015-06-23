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

#ifndef incl_HPHP_ITERATORS_H_
#define incl_HPHP_ITERATORS_H_

#include "hphp/runtime/vm/class.h"
#include "hphp/runtime/vm/named-entity.h"

#include <boost/iterator/iterator_facade.hpp>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/*
 * Iterator over all defined Classes.
 *
 * This iterator excludes subclasses of Closure that are created dynamically
 * through Closure::bind() and friends.
 */
struct class_iterator : boost::iterator_facade<class_iterator,
                                               Class,
                                               boost::forward_traversal_tag> {

  class_iterator() : class_iterator(NamedEntity::table()->end()) {}

  explicit class_iterator(const NamedEntity::Map::iterator& it)
    : m_iter{it}
    , m_cls{live() ? m_iter->second.clsList() : nullptr}
  {
    next_bound_name_if_null();
  }

private:
  friend class boost::iterator_core_access;

  /////////////////////////////////////////////////////////////////////////////
  // boost::iterator_facade methods.

private:
  /*
   * Access.
   *
   * Dereference null if the iterator is not live.
   */
  Class& dereference() const {
    assert(m_cls);
    auto const clones_end = m_cls->scopedClones().end();
    return *(m_clone != clones_end ? m_clone->second.get() : m_cls);
  }

  /*
   * Increment.
   *
   * No-op if the iterator is not live.
   */
  void increment() {
    if (!live()) return;
    assert(m_cls);

    // Bump the cloned closure table iterator, then return.  We don't change
    // m_cls because we always iterate the clones before the template class.
    if (m_clone != m_cls->scopedClones().end()) {
      ++m_clone;
      return;
    }

    // Bump the class chain pointer, then, if it's null, bump the NamedEntity
    // table iterator until we find a bound name.
    m_cls = m_cls->m_nextClass;
    next_bound_name_if_null();
  }

  /*
   * Equality.
   */
  bool equal(const class_iterator& o) const {
    return m_iter == o.m_iter &&
           m_cls == o.m_cls &&
           (m_cls == nullptr || m_clone == o.m_clone);
  }

  /////////////////////////////////////////////////////////////////////////////

private:
  /*
   * Whether the iterator has hit the end of the NamedEntity table.
   */
  bool live() const {
    return m_iter != NamedEntity::table()->end();
  }

  /*
   * Bump the NamedEntity table iterator until a valid class is found,
   * resetting the subordinate iterators appropriately.
   */
  void next_bound_name_if_null() {
    while (!m_cls && live()) {
      ++m_iter;
      if (live()) {
        m_cls = m_iter->second.clsList();
      }
    }

    if (live()) {
      m_clone = m_cls->scopedClones().begin();
    } else {
      m_cls = nullptr;
    }
  }

  /////////////////////////////////////////////////////////////////////////////
  // Data members.

private:
  /*
   * Three levels of iterators:
   *  - m_iter:   NamedEntity table entries
   *  - m_cls:    Class*'s chained to a particular NamedEntity
   *  - m_clone:  Cloned classes for closure scopings
   */
  NamedEntity::Map::iterator m_iter;
  Class* m_cls;
  Class::ScopedClonesMap::const_iterator m_clone;
};

/*
 * Range over all defined Classes.
 */
struct all_classes {
  class_iterator begin() const {
    return class_iterator(NamedEntity::table()->begin());
  }
  class_iterator end() const {
    return class_iterator(NamedEntity::table()->end());
  }
};

///////////////////////////////////////////////////////////////////////////////
}

#endif
