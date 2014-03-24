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

#ifndef incl_HPHP_FIXED_STRING_MAP_H_
#define incl_HPHP_FIXED_STRING_MAP_H_

#include <cstdint>

#include "hphp/runtime/base/string-data.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

template<class V, bool case_sensitive, class ExtraType = int32_t>
struct FixedStringMap {
  explicit FixedStringMap(int num) : m_table(0) { init(num); }
  FixedStringMap() : m_mask(0), m_table(0) {}
  ~FixedStringMap() { clear(); }

  FixedStringMap(const FixedStringMap&) = delete;
  const FixedStringMap& operator=(const FixedStringMap&) = delete;

  void clear();
  void init(int num, uint32_t numExtraBytes = 0);
  void add(const StringData* s, const V& v);
  V* find(const StringData* s) const;

  void* extraData() { return m_table; }
  const void* extraData() const { return m_table; }
  ExtraType& extra() { return m_extra; }
  const ExtraType& extra() const { return m_extra; }

  static ptrdiff_t tableOff() { return offsetof(FixedStringMap, m_table); }
  ExtraType size() const { return m_extra; }

private:
  struct Elm {
    LowStringPtr sd;
    V data;
  };

  /*
   * The fields we need here are just m_mask and m_table.  This would
   * leave 4 byte padding hole, though, which some users
   * (e.g. IndexedStringMap) might have a use for, so we expose it as
   * a slot for our users.
   */
  uint32_t  m_mask;
  ExtraType m_extra;  // not ours
  Elm*      m_table;
};

template<class T, class V, bool case_sensitive, class ExtraType = int32_t>
struct FixedStringMapBuilder {
  typedef typename boost::mpl::if_c<
    case_sensitive,
    string_data_same,
    string_data_isame
  >::type EqObject;
  typedef hphp_hash_map<const StringData*,V,
    string_data_hash,EqObject> Map;
  typedef FixedStringMap<V, case_sensitive, ExtraType> FSMap;

  typedef typename Map::const_iterator const_iterator;
  typedef typename Map::iterator iterator;

  iterator find(const StringData* key) { return m_map.find(key); }
  iterator begin()                     { return m_map.begin(); }
  iterator end()                       { return m_map.end(); }
  V        size() const                { return m_list.size(); }

  const_iterator find(const StringData* key) const {
    return m_map.find(key);
  }
  const_iterator begin() const { return m_map.begin(); }
  const_iterator end()   const { return m_map.end(); }

  T& operator[](V idx) {
    assert(idx >= 0);
    assert(size_t(idx) < m_list.size());
    return m_list[idx];
  }

  const T& operator[](V idx) const {
    return (*const_cast<FixedStringMapBuilder*>(this))[idx];
  }

  /*
   * Add an object to the position on the end, and allow lookup by
   * `name'.
   */
  void add(const StringData* name, const T& t) {
    if (m_list.size() >= size_t(std::numeric_limits<V>::max())) {
      assert(false && "FixedStringMap::Builder overflowed");
      abort();
    }

    m_map[name] = m_list.size();
    m_list.push_back(t);
  }

  /*
   * Adds objects that occupy indexes but can't be located by name.
   */
  void addUnnamed(const T& t) {
    m_list.push_back(t);
  }

  /*
   * Create a FixedStringMap from the builder.
   */
  void create(FSMap& map) {
    map.extra() = size();
    map.init(size(), 0);
    if (!size()) {
      return;
    }
    for (const_iterator it = begin(); it != end(); ++it) {
      map.add(it->first, it->second);
    }
  }

private:
  std::vector<T> m_list;
  Map m_map;
};

//////////////////////////////////////////////////////////////////////

}

#endif
