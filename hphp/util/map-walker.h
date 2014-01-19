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

#ifndef incl_HPHP_UTIL_MAPWALKER_H_
#define incl_HPHP_UTIL_MAPWALKER_H_

namespace HPHP {

/*
 * MapWalker is designed to iterate over an ordered map and an ordered list of
 * keys in parallel. Once created with a reference to a map, hasNext(key) will
 * return true iff the next item in the map matches the given key. If it
 * returns true, next() will return a reference to that element and advance its
 * internal iterator. hasNext() can be used to check if the internal iterator
 * is at the end of the map.
 */
template<typename Map>
struct MapWalker {
  typedef typename Map::key_type Key;
  typedef typename Map::mapped_type Value;
  typedef typename Map::const_iterator ConstIter;

  explicit MapWalker(const Map& m)
    : m_map(m)
    , m_iter(m.begin())
  {}

  bool hasNext() const {
    return m_iter != m_map.end();
  }

  bool hasNext(const Key& key) const {
    assert(m_iter == m_map.end() || key <= m_iter->first);
    return m_iter != m_map.end() && m_iter->first == key;
  }

  const Value& next() {
    assert(m_iter != m_map.end());
    auto const& val = m_iter->second;
    ++m_iter;
    return val;
  }

 private:
  const Map& m_map;
  ConstIter m_iter;
};

template<typename Map>
MapWalker<Map> makeMapWalker(const Map& m) {
  return MapWalker<Map>(m);
}

}

#endif
