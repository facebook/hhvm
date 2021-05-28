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

#include "hphp/runtime/base/string-data.h"
#include "hphp/runtime/vm/fixed-string-map.h"

#include <folly/Range.h>

namespace HPHP {

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
         class Index,
         Index InvalidIndex = Index(-1)>
struct IndexedStringMap {
  struct Builder;

  explicit IndexedStringMap() {
    setSize(0);
  }

  ~IndexedStringMap() {
    clear();
  }

  IndexedStringMap(const IndexedStringMap&) = delete;
  IndexedStringMap& operator=(const IndexedStringMap&) = delete;

  void clear() {
    m_map.clear();
    setSize(0);
  }

  /*
   * Create an IndexedStringMap from the supplied builder.  See
   * builder below.
   */
  void create(const Builder& b) {
    assertx(!size() && "IndexedStringMap::create called more than once");
    setSize(b.size());
    m_map.init(b.size(), size() * sizeof(T));
    if (!b.size()) {
      // note that we have to initialize m_map even though its zero
      // sized (an empty FixedStringMap isn't quite empty - see
      // FixedStringMap::init).
      return;
    }

    std::copy(b.m_list.begin(), b.m_list.end(), mutableAccessList());
    m_map.addFrom(b.begin(), b.end());
  }

  const T* accessList() const {
    return static_cast<const T*>(m_map.extraData());
  }
  T* mutableAccessList() { return const_cast<T*>(accessList()); }

  bool contains(const StringData* k) const { return m_map.find(k); }
  Index size() const { return m_map.extra(); }
  bool empty() const { return size() == 0; }

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
    return accessList()[*i];
  }

  // Lookup entries by index.  Index must be in range or you get
  // undefined behavior.
  T& operator[](Index index) {
    assertx(index < size());
    return mutableAccessList()[index];
  }
  const T& operator[](Index index) const {
    return (*const_cast<IndexedStringMap*>(this))[index];
  }

  folly::Range<const T*> range() const {
    return folly::range(accessList(), accessList() + size());
  }

  static constexpr ptrdiff_t vecOff() {
    return offsetof(IndexedStringMap, m_map) +
      FixedStringMap<Index,Index>::tableOff();
  }
  static constexpr ptrdiff_t sizeOff() {
    return offsetof(IndexedStringMap, m_map) +
      FixedStringMap<Index,Index>::sizeOff();
  }
  static constexpr size_t sizeSize() {
    return FixedStringMap<Index,Index>::sizeSize();
  }

private:
  uint32_t byteSize() const { return size() * sizeof(T); }
  void setSize(Index size) { m_map.extra() = size; }

  FixedStringMap<Index,Index> m_map;
};

//////////////////////////////////////////////////////////////////////

/*
 * Builder object for creating IndexedStringMaps.  Fill one of these
 * up, and then pass it to IndexedStringMap::create.
 */
template<class T, class Index, Index InvalidIndex>
struct IndexedStringMap<T,Index,InvalidIndex>::Builder {
private:
  using Map = hphp_hash_map<
    const StringData*,
    Index,
    string_data_hash,
    string_data_same
  >;

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

  auto& ordered_range() const { return m_list; }

  bool contains(const StringData* key) const { return m_map.count(key); }

  T& operator[](Index idx) {
    assertx(idx >= 0);
    assertx(size_t(idx) < m_list.size());
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
      assertx(false && "IndexedStringMap::Builder overflowed");
      abort();
    }

    if (!m_map.emplace(name, m_list.size()).second) {
      abort();
    }
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
  template<class SerDe, class F>
  typename std::enable_if<SerDe::deserializing>::type serde(SerDe& sd,
                                                            F lambda) {
    uint32_t size;
    sd(size);
    for (uint32_t i = 0; i < size; ++i) {
      T t;
      sd(t);

      auto name = lambda(t);
      if (name) {
        add(name, t);
      } else {
        addUnnamed(t);
      }
    }
  }

  // Serialization version.
  template<class SerDe, class F>
  typename std::enable_if<!SerDe::deserializing>::type serde(SerDe& sd,
                                                             F lambda) {
    sd(uint32_t(m_list.size()));
    for (uint32_t i = 0; i < m_list.size(); ++i) {
      sd(m_list[i]);
    }
  }

  const std::vector<T>& list() const {
    return m_list;
  }

private:
  friend struct IndexedStringMap;
  std::vector<T> m_list;
  Map m_map;
};

//////////////////////////////////////////////////////////////////////

}

