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

#include <algorithm>
#include <memory>
#include <vector>

#include "hphp/util/hash-map.h"
#include "hphp/util/optional.h"

namespace HPHP {
namespace Facts {

template <typename Key, typename VersionKey>
const VersionKey& getVersionKey(const Key& key);

template <typename VersionKey>
struct LazyTwoWayMapVersionProvider {
  void bumpVersion(VersionKey key) noexcept {
    auto& version = ++m_versionMap[key];
    always_assert(version != 0);
  }

  std::uint64_t getVersion(VersionKey key) const noexcept {
    auto const versionIt = m_versionMap.find(key);
    if (versionIt == m_versionMap.end()) {
      return 0;
    }
    return versionIt->second;
  }

  hphp_hash_map<VersionKey, std::uint64_t> m_versionMap;
};

/**
 * A map representing a many-to-many relationship between keys and values.
 *
 * This data structure overlays a source DB which contains stale data.
 */
template <
    typename Key, // Must have std::hash and operator== defined.
    typename VersionKey,
    typename Value // Must also have std::hash and operator== defined
    >
struct LazyTwoWayMap {
  using Keys = std::vector<Key>;
  using Values = std::vector<Value>;

  struct VersionedKeys {
    bool m_complete = false;
    hphp_hash_map<Key, uint64_t> m_keys;
  };

  explicit LazyTwoWayMap(
      std::shared_ptr<LazyTwoWayMapVersionProvider<VersionKey>> versions)
      : m_versions{std::move(versions)} {}

  /**
   * The `const` methods which return an `Optional` will return `std::nullopt`
   * if we haven't yet filled our data from the DB.
   *
   * Call the `const` overload first, and if it returns a non-`std::nullopt`
   * value then that value is correct and up-to-date. If it returns
   * `std::nullopt`, get data from the DB and pass that data into the
   * non-`const` overload. The non-const overload will store the data you gave
   * it, remove stale values, and give you a correct answer.
   */

  Optional<Values> getValuesForKey(const Key& key) const noexcept {
    auto const valuesIt = m_keyToValues.find(key);
    if (valuesIt == m_keyToValues.end()) {
      return {};
    }
    return valuesIt->second;
  }

  // Gets the set of values associated for a key.  If the values have not been
  // previously populated, they will be initialized with the provided values
  // from the source.  The source is always considered to be more stale than
  // values that are already in the map.
  Values getValuesForKey(Key key, const Values& valuesFromSource) noexcept {
    if (getVersion(key) == 0) {
      setValuesForKey(key, valuesFromSource);
    } else {
      auto values = getValuesForKey(key);
      if (values) {
        return *values;
      }
    }
    return valuesFromSource;
  }

  Optional<Keys> getKeysForValue(Value value) const noexcept {
    auto const versionedKeysIt = m_valueToKeys.find(value);
    if (versionedKeysIt == m_valueToKeys.end()) {
      return {};
    }
    const VersionedKeys& versionedKeys = versionedKeysIt->second;

    // Bail out if we haven't filled our keys from the DB yet
    if (!versionedKeys.m_complete) {
      return {};
    }

    // Return only the up-to-date keys
    Keys keys;
    for (auto const& [key, version] : versionedKeys.m_keys) {
      if (version == getVersion(key)) {
        keys.push_back(key);
      }
    }
    return keys;
  }

  Keys getKeysForValue(Value value, const Keys& keysFromSource) noexcept {
    VersionedKeys* versionedKeys;
    auto const it = m_valueToKeys.find(value);
    if (it != m_valueToKeys.end()) {
      versionedKeys = &it->second;
    } else if (!keysFromSource.empty()) {
      versionedKeys = &m_valueToKeys[value];
    } else {
      // Do not create an empty placeholder for a missing symbol
      return {};
    }

    // Data in the DB is always considered to be more stale than data in the
    // map, and gets a version number of 0.
    for (auto const& key : keysFromSource) {
      versionedKeys->m_keys.insert({key, 0});
    }

    // Mark our list of keys as filled from the DB
    versionedKeys->m_complete = true;

    // Remove keys from older versions of files past
    removeStaleKeys(*versionedKeys);

    Keys keys;
    keys.reserve(versionedKeys->m_keys.size());
    for (auto const& [key, _] : versionedKeys->m_keys) {
      keys.push_back(key);
    }
    return keys;
  }

  /**
   * This is how we modify a LazyTwoWayMap's contents with fresh data. In
   * practical terms, we're describing the new contents of a file after it has
   * changed - the key is the file path, and the values are the classes declared
   * in that path.
   */
  void setValuesForKey(const Key& key, Values newValues) noexcept {
    // Erase old mappings.
    auto& existingValues = m_keyToValues[key];
    for (const Value& value : existingValues) {
      auto& versionedKeys = m_valueToKeys[value];
      versionedKeys.m_keys.erase(key);
    }

    // Add the value-to-keys mappings.
    uint64_t version = getVersion(key);
    for (const Value& value : newValues) {
      VersionedKeys& versionedKeys = m_valueToKeys[value];

      // Two keys with differently uppercased/lowercased characters can still be
      // equal due to case-insensitive equality. `insert_or_assign` never
      // changes keys, so we need to explicitly erase the key here.
      versionedKeys.m_keys.erase(key);
      versionedKeys.m_keys.insert_or_assign(key, version);
    }

    // Set the key-to-values mappings.
    existingValues = std::move(newValues);
  }

 private:
  uint64_t getVersion(const Key& key) const noexcept {
    return m_versions->getVersion(getVersionKey<Key, VersionKey>(key));
  }

  void removeStaleKeys(VersionedKeys& versionedKeys) const noexcept {
    auto it = versionedKeys.m_keys.begin();
    while (it != versionedKeys.m_keys.end()) {
      auto& [key, version] = *it;
      if (version != getVersion(key)) {
        it = versionedKeys.m_keys.erase(it);
      } else {
        ++it;
      }
    }
  }

  hphp_hash_map<Key, Values> m_keyToValues;
  hphp_hash_map<Value, VersionedKeys> m_valueToKeys;
  std::shared_ptr<LazyTwoWayMapVersionProvider<VersionKey>> m_versions;
};

} // namespace Facts
} // namespace HPHP
