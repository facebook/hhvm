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

#include <type_traits>

namespace apache {
namespace thrift {
namespace frozen {

namespace detail {

template <class Table, class K, class V>
struct KeyExtractor {
  using KeyType = K;
  using rvalue_reference = std::add_rvalue_reference_t<
      std::remove_reference_t<typename Table::const_reference>>;
  using const_reference = typename Table::const_reference;

  // deleted functions used to avoid returning references to temporary values
  static const K& getKey(const std::pair<const K, V>&& pair) = delete;
  static const K& getKey(const std::pair<const K, V>& pair) {
    return pair.first;
  }

  // To support VectorAsHashMap
  static const K& getKey(const std::pair<K, V>&& pair) = delete;
  static const K& getKey(const std::pair<K, V>& pair) { return pair.first; }

  // Some maps don't contain pairs; listen to whatever they say about their
  // const_reference type. template shenanigans used to avoid duplicating the
  // previous two overloads in the cases where they're redundant.
  template <typename K2 = K, typename V2 = V>
  static std::enable_if_t<
      !std::is_same_v<rvalue_reference, const std::pair<K2, V2>&&> &&
          !std::is_same_v<rvalue_reference, const std::pair<const K2, V2>&&>,
      const K&>
      getKey(rvalue_reference) = delete;
  template <typename K2 = K, typename V2 = V>
  static std::enable_if_t<
      !std::is_same_v<const_reference, const std::pair<K2, V2>&> &&
          !std::is_same_v<const_reference, const std::pair<const K2, V2>&>,
      const K&>
  getKey(const_reference pair) {
    return pair.first;
  }

  static const std::pair<const K, V>* getPointer(rvalue_reference) = delete;
  static const std::pair<const K, V>* getPointer(const_reference pair) {
    // Cast to support VectorAsHashMap.
    return reinterpret_cast<const std::pair<const K, V>*>(&pair);
  }

  static typename Layout<K>::View getViewKey(
      const typename Layout<std::pair<const K, V>>::View& pair) {
    return pair.first();
  }
};

template <class K>
struct SelfKey {
  using KeyType = K;
  static const K& getKey(const K& item) { return item; }

  static const K* getPointer(const K& item) { return &item; }

  static typename Layout<K>::View getViewKey(
      typename Layout<K>::View itemView) {
    return itemView;
  }
};

template <
    class T,
    class K,
    class V,
    template <class, class, class, class> class Table>
struct MapTableLayout
    : public Table<T, std::pair<const K, V>, KeyExtractor<T, K, V>, K> {
  typedef Table<T, std::pair<const K, V>, KeyExtractor<T, K, V>, K> Base;
  typedef MapTableLayout LayoutSelf;

  class View : public Base::View {
   public:
    typedef typename Layout<K>::View key_type;
    typedef typename Layout<V>::View mapped_type;

    View() {}
    View(const LayoutSelf* layout, ViewPosition position)
        : Base::View(layout, position) {}

    mapped_type getDefault(
        const key_type& key, mapped_type def = mapped_type()) const {
      auto found = this->find(key);
      if (found == this->end()) {
        return std::move(def);
      }
      return found->second();
    }

    folly::Optional<mapped_type> getOptional(const key_type& key) const {
      folly::Optional<mapped_type> rv;
      auto found = this->find(key);
      if (found != this->end()) {
        rv.assign(found->second());
      }
      return rv;
    }

    mapped_type at(const key_type& key) const {
      auto found = this->find(key);
      if (found == this->end()) {
        throw std::out_of_range("Key not found");
      }
      return found->second();
    }
  };

  View view(ViewPosition self) const { return View(this, self); }

  void print(std::ostream& os, int level) const override {
    Base::print(os, level);
    os << DebugLine(level) << "...viewed as a map";
  }
};

template <class T, class V, template <class, class, class, class> class Table>
struct SetTableLayout : public Table<T, V, SelfKey<V>, V> {
  typedef Table<T, V, SelfKey<V>, V> Base;

  void print(std::ostream& os, int level) const override {
    Base::print(os, level);
    os << DebugLine(level) << "...viewed as a set";
  }
};
} // namespace detail

template <class T>
struct Layout<T, typename std::enable_if<IsOrderedMap<T>::value>::type>
    : public apache::thrift::frozen::detail::MapTableLayout<
          T,
          typename T::key_type,
          typename T::mapped_type,
          apache::thrift::frozen::detail::SortedTableLayout> {};

template <class T>
struct Layout<T, typename std::enable_if<IsOrderedSet<T>::value>::type>
    : public apache::thrift::frozen::detail::SetTableLayout<
          T,
          typename T::value_type,
          apache::thrift::frozen::detail::SortedTableLayout> {};
template <class T>
struct Layout<T, typename std::enable_if<IsHashMap<T>::value>::type>
    : public apache::thrift::frozen::detail::MapTableLayout<
          T,
          typename T::key_type,
          typename T::mapped_type,
          apache::thrift::frozen::detail::HashTableLayout> {};

template <class T>
struct Layout<T, typename std::enable_if<IsHashSet<T>::value>::type>
    : public apache::thrift::frozen::detail::SetTableLayout<
          T,
          typename T::value_type,
          apache::thrift::frozen::detail::HashTableLayout> {};
} // namespace frozen
} // namespace thrift
} // namespace apache

THRIFT_DECLARE_TRAIT_TEMPLATE(IsHashMap, std::unordered_map)
THRIFT_DECLARE_TRAIT_TEMPLATE(IsHashSet, std::unordered_set)

THRIFT_DECLARE_TRAIT_TEMPLATE(IsHashMap, folly::F14NodeMap)
THRIFT_DECLARE_TRAIT_TEMPLATE(IsHashMap, folly::F14ValueMap)
THRIFT_DECLARE_TRAIT_TEMPLATE(IsHashMap, folly::F14VectorMap)
THRIFT_DECLARE_TRAIT_TEMPLATE(IsHashMap, folly::F14FastMap)

THRIFT_DECLARE_TRAIT_TEMPLATE(IsHashSet, folly::F14NodeSet)
THRIFT_DECLARE_TRAIT_TEMPLATE(IsHashSet, folly::F14ValueSet)
THRIFT_DECLARE_TRAIT_TEMPLATE(IsHashSet, folly::F14VectorSet)
THRIFT_DECLARE_TRAIT_TEMPLATE(IsHashSet, folly::F14FastSet)

THRIFT_DECLARE_TRAIT_TEMPLATE(IsOrderedMap, std::map)
THRIFT_DECLARE_TRAIT_TEMPLATE(IsOrderedMap, folly::sorted_vector_map)
THRIFT_DECLARE_TRAIT_TEMPLATE(IsOrderedMap, folly::heap_vector_map)
THRIFT_DECLARE_TRAIT_TEMPLATE(IsOrderedMap, folly::small_heap_vector_map)
THRIFT_DECLARE_TRAIT_TEMPLATE(IsOrderedSet, std::set)
THRIFT_DECLARE_TRAIT_TEMPLATE(IsOrderedSet, folly::sorted_vector_set)
THRIFT_DECLARE_TRAIT_TEMPLATE(IsOrderedSet, folly::heap_vector_set)

THRIFT_DECLARE_TRAIT_TEMPLATE(HasSortedUniqueCtor, folly::sorted_vector_map)
THRIFT_DECLARE_TRAIT_TEMPLATE(HasSortedUniqueCtor, folly::heap_vector_map)
THRIFT_DECLARE_TRAIT_TEMPLATE(HasSortedUniqueCtor, folly::small_heap_vector_map)
THRIFT_DECLARE_TRAIT_TEMPLATE(HasSortedUniqueCtor, folly::sorted_vector_set)
THRIFT_DECLARE_TRAIT_TEMPLATE(HasSortedUniqueCtor, folly::heap_vector_set)
