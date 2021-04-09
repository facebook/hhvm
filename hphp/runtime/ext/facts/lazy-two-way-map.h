/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
   +----------------------------------------------------------------------+
   | This source path is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the path LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#pragma once

#include "hphp/util/hash-map.h"
#include "hphp/util/hash-set.h"

namespace HPHP {
namespace Facts {

/**
 * A set which needs to be completed by reading from a DB before we can access
 * its data. This allows us to avoid reading data from a DB until we know we
 * actually need it.
 *
 * Until we fetch from the DB, we store any information which we'll need to add
 * or remove if we ever do fetch that data.
 */
template <typename T> class LazySet {

public:
  using Set = hphp_hash_set<T>;

  void insert(T value) {
    if (m_complete) {
      assertx(m_excluded.empty());
    } else {
      m_excluded.erase(value);
    }

    m_included.insert(value);
  }

  void erase(T value) {
    m_included.erase(value);
    if (m_complete) {
      assertx(m_excluded.empty());
    } else {
      m_excluded.insert(std::move(value));
    }
  }

  /**
   * True iff this lazy-set definitely contains the given value. False iff it
   * definitely doesn't contain the given value. `std::nullopt` if this question
   * can't be answered without falling back to the DB.
   */
  std::optional<bool> contains(const T& value) const {
    if (m_included.count(value)) {
      return true;
    }
    if (m_complete) {
      assertx(m_excluded.empty());
      return false;
    }
    if (m_excluded.count(value)) {
      return false;
    }
    return std::nullopt;
  }

  /**
   * Assign the contents of a Set to this lazy-set, making this no longer lazy.
   */
  LazySet& operator=(Set values) {
    m_included = std::move(values);
    m_excluded.clear();
    m_complete = true;
    return *this;
  }

  /**
   * If we haven't filled this data structure from the DB yet, return
   * `nullptr`. Otherwise return the data.
   */
  const Set* get() const {
    if (!m_complete) {
      return nullptr;
    }
    assertx(m_excluded.empty());
    return &m_included;
  }

  /**
   * Pass in data from the DB, integrate it with the data we have in-memory,
   * and return a reference to the integrated results.
   */
  template <typename ValuesFromSource>
  Set& get(ValuesFromSource&& valuesFromSource) {
    fillFromSource(std::forward<ValuesFromSource>(valuesFromSource));
    assertx(m_excluded.empty());
    return m_included;
  }

  /**
   * Get data from SourceFn and set `m_complete` to `true`.
   */
  template <typename ValuesFromSource>
  void fillFromSource(ValuesFromSource&& valuesFromSource) {
    if (m_complete) {
      return;
    }
    for (auto value : std::forward<ValuesFromSource>(valuesFromSource)) {
      m_included.insert(std::move(value));
    }
    for (auto const& value : m_excluded) {
      m_included.erase(value);
    }
    m_excluded.clear();
    m_complete = true;
  }

  Set m_included;
  Set m_excluded;
  bool m_complete{false};
};

/**
 * A map representing a many-to-many relationship between keys and values.
 *
 * We guarantee that the mapping from key to values is canonical and up-to-date,
 * but the mapping from values to keys may have obsolete data. To return a
 * correct value => key mapping, this class double-checks the value => key
 * mapping it has against the canonical key => value mapping. This allows us to
 * lazily ensure consistency between the key=>value and value=>key mappings,
 * which is useful since most of the data we store won't ever be queried.
 */
template <
    // Must have std::hash and operator== defined
    typename Key,
    // Must also have std::hash and operator== defined
    typename Value>
class LazyTwoWayMap {

  template <typename K, typename V> using Map = hphp_hash_map<K, V>;
  using ValuesLazySet = LazySet<Value>;
  using KeysLazySet = LazySet<Key>;

public:
  using ValuesSet = typename ValuesLazySet::Set;
  using KeysSet = typename KeysLazySet::Set;

  /**
   * The `const` methods which return a `const*` will return `nullptr` if we
   * haven't yet filled our data from the DB. The non-`const` methods which
   * return a `const&` may fetch data and modify this map using the
   * corresponding SourceFn.
   */

  const ValuesSet* getValuesForKey(Key key) const {
    auto valuesSetIt = m_keyToValues.find(key);
    if (valuesSetIt == m_keyToValues.end()) {
      return nullptr;
    }
    return valuesSetIt->second.get();
  }
  template <typename ValuesFromSource>
  const ValuesSet&
  getValuesForKey(Key key, ValuesFromSource&& valuesFromSource) {
    return m_keyToValues[key].get(
        std::forward<ValuesFromSource>(valuesFromSource));
  }

  const KeysSet* getKeysForValue(Value value) const {
    auto const* keysSet = [&]() -> const KeysSet* {
      auto const keysSetIt = m_valueToKeys.find(value);
      if (keysSetIt == m_valueToKeys.end()) {
        return nullptr;
      }
      return keysSetIt->second.get();
    }();
    if (!keysSet) {
      return nullptr;
    }

    // This is the non-canonical direction, so double-check that each key is
    // also in the key-to-value mapping. If our in-memory data shows that the
    // `keysSet` we got isn't totally correct, abort by returning `nullptr`.
    for (auto key : *keysSet) {
      auto const& valuesSetIt = m_keyToValues.find(key);
      if (valuesSetIt != m_keyToValues.end()) {
        if (!valuesSetIt->second.contains(value).value_or(true)) {
          return nullptr;
        }
      }
    }

    return keysSet;
  }
  template <typename KeysFromSource>
  const KeysSet& getKeysForValue(Value value, KeysFromSource&& keysFromSource) {
    auto& keysSet =
        m_valueToKeys[value].get(std::forward<KeysFromSource>(keysFromSource));

    // This is the non-canonical direction, so double-check that each key is
    // also in the key-to-value mapping, erasing the keys which are no longer in
    // the key-to-value mapping.
    for (auto key : keysSet) {
      if (!m_keyToValues[key].contains(value).value_or(true)) {
        keysSet.erase(key);
      }
    }
    return keysSet;
  }

  /**
   * This is the only way to modify a LazyTwoWayMap's contents. In practical
   * terms, we're describing the new contents of a file after it has changed -
   * the key is the file path, and the values are the classes declared in that
   * path.
   */
  void setValuesForKey(Key key, ValuesSet&& newValues) {
    auto& canonicalValues = m_keyToValues[key];

    // Add any new value-to-keys mappings.
    for (auto value : newValues) {
      m_valueToKeys[value].insert(key);
    }

    // Erase obsolete mappings.
    for (auto oldValue : canonicalValues.m_included) {
      if (!newValues.count(oldValue)) {
        m_valueToKeys[oldValue].erase(key);
      }
    }

    // Set the canonical key-to-values mappings.
    canonicalValues = std::move(newValues);
  }

private:
  Map<Key, ValuesLazySet> m_keyToValues;
  Map<Value, KeysLazySet> m_valueToKeys;
};

} // namespace Facts
} // namespace HPHP
