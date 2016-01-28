/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_USER_ATTRIBUTES_H_
#define incl_HPHP_USER_ATTRIBUTES_H_

#include <cstdlib>
#include <utility>

#include "hphp/util/copy-ptr.h"
#include "hphp/util/functional.h"
#include "hphp/util/hash-map-typedefs.h"

#include "hphp/runtime/base/string-data.h"
#include "hphp/runtime/base/typed-value.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

/*
 * User attributes on various runtime structures are stored in these maps.
 * Most things won't have attributes, so we've made the map optimize for the
 * case that it's empty (minimizing sizeof(UserAttributeMap)).
 */
class UserAttributeMap {
  using Map = hphp_hash_map<
    LowStringPtr,
    TypedValue,
    string_data_hash,
    string_data_isame
  >;

public:
  using mapped_type    = TypedValue;
  using key_type       = LowStringPtr;
  using value_type     = std::pair<key_type,mapped_type>;
  using size_type      = std::size_t;
  using iterator       = Map::iterator;
  using const_iterator = Map::const_iterator;

  mapped_type& operator[](const key_type& k) {
    return map()[k];
  }

  template<class... Args>
  std::pair<iterator,bool> insert(Args&&... args) {
    return map().insert(std::forward<Args>(args)...);
  }

  const_iterator find(const key_type& k) const {
    return map().find(k);
  }

  // Note: non-const iteration is not allowed, since we don't want to allocate
  // a map on accident.
  const_iterator begin() const  { return map().begin(); }
  const_iterator end() const    { return map().end(); }
  const_iterator cbegin() const { return map().cbegin(); }
  const_iterator cend() const   { return map().cend(); }

  size_type size() const { return m_map ? m_map->size() : 0; }
  bool empty() const     { return size() == 0; }

  size_type count(const key_type& k) const {
    if (!m_map) return 0;
    return m_map->count(k);
  }

  template<class SerDe>
  void serde(SerDe& sd) {
    if (SerDe::deserializing) {
      bool empty;
      sd(empty);
      if (empty) return;
      m_map.reset(new Map);
      sd(*m_map);
      return;
    }

    bool empty = !m_map.get();
    sd(empty);
    if (empty) return;
    sd(*m_map);
  }

private:
  Map& map() {
    if (!m_map) m_map.reset(new Map);
    return *m_map;
  }
  const Map& map() const {
    return !m_map ? s_empty_map : *m_map;
  }

private:
  static Map s_empty_map; // so our iterators can be normal Map iterators
  copy_ptr<Map> m_map;
};

//////////////////////////////////////////////////////////////////////

}

#endif
