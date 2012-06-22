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
#include <boost/utility/enable_if.hpp>

#include "runtime/base/string_data.h"
#include "runtime/vm/fixed_string_map.h"

namespace HPHP { namespace VM {

//////////////////////////////////////////////////////////////////////

/*
 * Several VM data structures can be accessed with both a string-based
 * lookup and by an index.  This map supports both types of lookup.
 * (The index type is a template parameter in order to allow different
 * uses of this map to have different sized indexes).
 *
 * The number of elements must be fixed after initialization time
 * (since we use a FixedStringMap internally).
 */
template<class T,
         bool CaseSensitive,
         class Index,
         Index InvalidIndex = Index(-1)>
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
  void create(const Builder& b) {
    ASSERT(!m_size && "IndexedStringMap::create called more than once");
    m_map.init(b.size());
    m_vec = new T[b.size()];
    m_size = b.size();

    std::copy(b.m_list.begin(), b.m_list.end(), m_vec);
    for (typename Builder::const_iterator it = b.begin();
        it != b.end();
        ++it) {
      m_map.add(it->first, it->second);
    }
  }

  bool contains(const StringData* k) const { return m_map.find(k); }
  Index size() const { return m_size; }
  const T* accessList() const { return m_vec; }

  // Find the index for an entry by name.  Returns InvalidIndex if
  // there is no entry with this name.
  Index findIndex(const StringData* k) const {
    Index* i = m_map.find(k);
    if (!i) return InvalidIndex;
    return *i;
  }

  // Lookup entries in the map by string.  Returns the entry or the
  // supplied default value if it doesn't exist.
  T lookupDefault(const StringData* k, T def) const {
    Index* i = m_map.find(k);
    if (!i) return def;
    return m_vec[*i];
  }

  // Lookup entries by index.  Index must be in range or you get
  // undefined behavior.
  T& operator[](Index index) {
    ASSERT(index < size());
    return m_vec[index];
  }
  const T& operator[](Index index) const {
    return (*const_cast<IndexedStringMap*>(this))[index];
  }

public:
  static size_t vecOff() { return offsetof(IndexedStringMap, m_vec); }
  static size_t sizeOff() { return offsetof(IndexedStringMap, m_size); }

private:
  typedef FixedStringMap<Index,CaseSensitive> Map;

  IndexedStringMap(const IndexedStringMap&);
  IndexedStringMap& operator=(const IndexedStringMap&);

  T* m_vec;
  Index m_size;
  Map m_map;
};

//////////////////////////////////////////////////////////////////////

/*
 * Builder object for creating IndexedStringMaps.  Fill one of these
 * up, and then pass it to IndexedStringMap::create.
 */
template<class T, bool CaseSensitive, class Index, Index InvalidIndex>
class IndexedStringMap<T,CaseSensitive,Index,InvalidIndex>::Builder {
  typedef typename boost::mpl::if_c<
    CaseSensitive,
    string_data_same,
    string_data_isame
  >::type EqObject;
  typedef hphp_hash_map<const StringData*,Index,
    string_data_hash,EqObject> Map;

public:
  typedef typename Map::const_iterator const_iterator;
  typedef typename Map::iterator iterator;

  iterator find(const StringData* key) { return m_map.find(key); }
  iterator begin()                     { return m_map.begin(); }
  iterator end()                       { return m_map.end(); }
  Index    size() const                { return m_list.size(); }

  const_iterator find(const StringData* key) const {
    return m_map.find(key);
  }
  const_iterator begin() const { return m_map.begin(); }
  const_iterator end()   const { return m_map.end(); }

  T& operator[](Index idx) {
    ASSERT(idx >= 0);
    ASSERT(size_t(idx) < m_list.size());
    return m_list[idx];
  }

  const T& operator[](Index idx) const {
    return (*const_cast<Builder*>(this))[idx];
  }

  /*
   * Add an object to the position on the end, and allow lookup by
   * `name'.
   */
  void add(const StringData* name, const T& t) {
    if (m_list.size() >= size_t(std::numeric_limits<Index>::max())) {
      ASSERT(false && "IndexedStringMap::Builder overflowed");
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
   * Implement custom separate versions of (de)serialization code to
   * be more compact about what we put on disk.  (Instead of just
   * pumping through our m_list and m_map.)
   */

  // Deserialization version.
  template<class SerDe>
  typename boost::enable_if_c<SerDe::deserializing>::type
  serde(SerDe& sd) {
    uint32_t size;
    sd(size);
    for (uint32_t i = 0; i < size; ++i) {
      const StringData* name;
      T t;
      sd(name)(t);

      if (name) {
        add(name, t);
      } else {
        addUnnamed(t);
      }
    }
  }

  // Serialization version.
  template<class SerDe>
  typename boost::disable_if_c<SerDe::deserializing>::type
  serde(SerDe& sd) {
    std::vector<const StringData*> names(m_list.size());
    for (typename Map::const_iterator it = m_map.begin();
        it != m_map.end();
        ++it) {
      names[it->second] = it->first;
    }

    sd(uint32_t(m_list.size()));
    for (uint32_t i = 0; i < m_list.size(); ++i) {
      sd(names[i])(m_list[i]);
    }
  }

private:
  friend class IndexedStringMap;
  std::vector<T> m_list;
  Map m_map;
};

//////////////////////////////////////////////////////////////////////

}}

#endif
