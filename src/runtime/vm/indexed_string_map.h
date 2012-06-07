/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#ifndef incl_VM_INDEXED_STRING_MAP_H_
#define incl_VM_INDEXED_STRING_MAP_H_

#include <boost/mpl/if.hpp>

#include "runtime/base/string_data.h"
#include "runtime/vm/core_types.h"
#include "runtime/vm/fixed_string_map.h"

namespace HPHP { namespace VM {

//////////////////////////////////////////////////////////////////////

/*
 * Several VM data structures can be accessed with both a string-based
 * lookup and by an index, or "slot".  This map supports both types of
 * lookup.
 *
 * The number of elements must be fixed after initialization time
 * (since we use a FixedStringMap internally).
 */
template<class T, bool CaseSensitive>
struct IndexedStringMap {
  struct Builder;

  explicit IndexedStringMap() : m_vec(0), m_size(0) {}

  ~IndexedStringMap() {
    delete [] m_vec;
  }

  /*
   * Create an IndexedStringMap from the supplied builder.  See
   * builder below.
   */
  void create(Builder& b) {
    ASSERT(!m_size && "IndexedStringMap::create called more than once");
    m_map.init(b.size());
    m_vec = new T[b.size()];
    m_size = b.size();

    std::copy(b.m_list.begin(), b.m_list.end(), m_vec);
    for (typename Builder::iterator it = b.begin();
        it != b.end();
        ++it) {
      m_map.add(it->first, it->second);
    }
  }

  bool contains(const StringData* k) const { return m_map.find(k); }
  Slot size() const { return m_size; }
  const T* accessList() const { return m_vec; }

  // Find the slot for an entry by name.  Returns kInvalidSlot if
  // there is no entry with this name.
  Slot findSlot(const StringData* k) const {
    Slot* i = m_map.find(k);
    if (!i) return kInvalidSlot;
    return *i;
  }

  // Lookup entries in the map by string.  Returns the entry or the
  // supplied default value if it doesn't exist.
  T lookupDefault(const StringData* k, T def) const {
    Slot* i = m_map.find(k);
    if (!i) return def;
    return m_vec[*i];
  }

  // Lookup entries by index.  Index must be in range or you get
  // undefined behavior.
  T& operator[](Slot index) {
    ASSERT(index < size());
    return m_vec[index];
  }
  const T& operator[](Slot index) const {
    return (*const_cast<IndexedStringMap*>(this))[index];
  }

public:
  static size_t vecOff() { return offsetof(IndexedStringMap, m_vec); }
  static size_t sizeOff() { return offsetof(IndexedStringMap, m_size); }

private:
  typedef FixedStringMap<Slot,CaseSensitive> Map;

  IndexedStringMap(const IndexedStringMap&);
  IndexedStringMap& operator=(const IndexedStringMap&);

  T* m_vec;
  Slot m_size;
  Map m_map;
};

//////////////////////////////////////////////////////////////////////

/*
 * Builder object for creating IndexedStringMaps.  Fill one of these
 * up, and then pass it to IndexedStringMap::create.
 */
template<class T, bool CaseSensitive>
class IndexedStringMap<T,CaseSensitive>::Builder {
  typedef typename boost::mpl::if_c<
    CaseSensitive,
    string_data_same,
    string_data_isame
  >::type EqObject;
  typedef hphp_hash_map<const StringData*,Slot,
    string_data_hash,EqObject> Map;

public:
  typedef typename Map::iterator iterator;

  iterator find(const StringData* key) { return m_map.find(key); }
  iterator begin()                     { return m_map.begin(); }
  iterator end()                       { return m_map.end(); }
  Slot     size() const                { return m_list.size(); }

  T& operator[](Slot idx) {
    ASSERT(idx < m_list.size());
    return m_list[idx];
  }

  /*
   * Add an object to the slot on the end, that can be looked up by
   * `name'.
   */
  void add(const StringData* name, const T& t) {
    if (m_list.size() >= std::numeric_limits<Slot>::max()) {
      ASSERT(false && "IndexedStringMap::Builder overflowed");
      abort();
    }

    m_map[name] = m_list.size();
    m_list.push_back(t);
  }

  /*
   * Adds objects that occupy slots but can't be located by name.
   */
  void addUnnamed(const T& t) {
    m_list.push_back(t);
  }

private:
  friend class IndexedStringMap;
  std::vector<T> m_list;
  Map m_map;
};

//////////////////////////////////////////////////////////////////////

}}

#endif
