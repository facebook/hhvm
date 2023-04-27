/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <tuple>
#include <utility>
#include <vector>

#include <thrift/lib/cpp2/frozen/Traits.h>

namespace apache {
namespace thrift {
namespace frozen {
/*
 * Vector-backed associative types without support for lookup, strictly for
 * building large maps and sets with minimal memory. Uniqueness assumed.
 */
template <class V>
class VectorAsSet : public std::vector<V> {
  typedef std::vector<V> Base;

 public:
  using Base::Base;
  /**
   * Insert value into the set at unspecified location.
   */
  void insert(const V& value) { this->push_back(value); }
  void insert(V&& value) { this->push_back(std::move(value)); }

  template <class Iterator>
  void insert(Iterator begin, Iterator end) {
    for (; begin != end; ++begin) {
      this->insert(*begin);
    }
  }

  template <class T>
  void insert(T&& value) {
    this->emplace_back(std::forward<T>(value));
  }

  /**
   * Insert value into the set with a specified hint location.
   * Note: Hint is ignored, insertion always adds to the end.
   */
  void insert(typename Base::iterator /* hint */, const V& value) {
    this->emplace_back(value);
  }

  void insert(typename Base::iterator /* hint */, V&& value) {
    this->emplace_back(std::move(value));
  }

  template <class T>
  void emplace(typename Base::iterator /* hint */, T&& value) {
    this->emplace_back(std::forward<T>(value));
  }
};

template <class V>
class VectorAsHashSet : public VectorAsSet<V> {
  using VectorAsSet<V>::VectorAsSet;
};

template <class K, class V>
class VectorAsMap : public VectorAsSet<std::pair<K, V>> {
 public:
  using VectorAsSet<std::pair<K, V>>::VectorAsSet;

  typedef K key_type;
  typedef V mapped_type;
  V& operator[](K key) {
    this->emplace_back(
        std::piecewise_construct,
        std::tuple<K&&>(std::move(key)),
        std::tuple<>());
    return this->back().second;
  }
};

template <class K, class V>
class VectorAsHashMap : public VectorAsMap<K, V> {
  using VectorAsMap<K, V>::VectorAsMap;
};

} // namespace frozen
} // namespace thrift
} // namespace apache

THRIFT_DECLARE_TRAIT_TEMPLATE(
    IsHashMap, apache::thrift::frozen::VectorAsHashMap)
THRIFT_DECLARE_TRAIT_TEMPLATE(
    IsHashSet, apache::thrift::frozen::VectorAsHashSet)
THRIFT_DECLARE_TRAIT_TEMPLATE(IsOrderedMap, apache::thrift::frozen::VectorAsMap)
THRIFT_DECLARE_TRAIT_TEMPLATE(IsOrderedSet, apache::thrift::frozen::VectorAsSet)
