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

#include <iostream>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>
#include <utility>

#include <folly/Utility.h>
#include <thrift/lib/cpp/util/VarintUtils.h>
#include <thrift/lib/cpp2/Adapter.h>
#include <thrift/lib/cpp2/op/detail/BasePatch.h>

namespace apache {
namespace thrift {
namespace op {
namespace detail {

class ListPatchIndex
    : public type::detail::EqWrap<ListPatchIndex, std::int32_t> {
 private:
  using Base = type::detail::EqWrap<ListPatchIndex, std::int32_t>;

 public:
  ListPatchIndex() = default;
  explicit ListPatchIndex(size_t pos) : Base(util::toI32ZigZagOrdinal(pos)) {}
  size_t position() const { return util::fromI32ZigZagOrdinal(toThrift()); }

 private:
  template <class>
  friend struct ::apache::thrift::InlineAdapter;
  template <class>
  friend struct ::std::hash;
  using Base::Base;
  using Base::empty;
  using Base::reset;
  using Base::toThrift;
};

template <typename C1, typename C2>
void erase_all(C1& container, const C2& values) {
  for (auto itr = values.begin(); itr != values.end() && !container.empty();
       ++itr) {
    container.erase(*itr);
  }
}

template <typename C1, typename C2>
void remove_all_values(C1& container, const C2& values) {
  container.erase(
      std::remove_if(
          container.begin(),
          container.end(),
          [&](const auto& element) {
            return std::find(values.begin(), values.end(), element) !=
                values.end();
          }),
      container.end());
}

/// Patch for a Thrift list.
///
/// The `Patch` template parameter must be a Thrift struct with the following
/// fields:
/// * `optional list<T> assign`
/// * `terse bool clear`
/// * `terse list<T> append`
/// * `terse list<T> prepend`
template <typename Patch>
class ListPatch : public BaseContainerPatch<Patch, ListPatch<Patch>> {
  using Base = BaseContainerPatch<Patch, ListPatch>;
  using T = typename Base::value_type;
  using VPMap = folly::remove_cvref_t<decltype(*std::declval<Patch>().patch())>;
  using VP = typename VPMap::mapped_type;

 public:
  using Base::apply;
  using Base::Base;
  using Base::operator=;

  /// Creates a new patch with appended list.
  template <typename C = T>
  static ListPatch createAppend(C&& values) {
    ListPatch result;
    *result.data_.append() = std::forward<C>(values);
    return result;
  }

  /// Creates a new patch with prepended list.
  template <typename C = T>
  static ListPatch createPrepend(C&& values) {
    ListPatch result;
    *result.data_.prepend() = std::forward<C>(values);
    return result;
  }

  /// Appends a list.
  template <typename C = T>
  void append(C&& rhs) {
    auto& lhs = assignOr(*data_.append());
    lhs.insert(lhs.end(), rhs.begin(), rhs.end());
  }
  /// Emplaces and appends a new element.
  template <typename... Args>
  void emplace_back(Args&&... args) {
    assignOr(*data_.append()).emplace_back(std::forward<Args>(args)...);
  }
  /// Appends the given element value.
  template <typename U = typename T::value_type>
  void push_back(U&& val) {
    assignOr(*data_.append()).push_back(std::forward<U>(val));
  }

  /// Prepends a list.
  template <typename C = T>
  void prepend(C&& lhs) {
    auto& rhs = assignOr(*data_.prepend());
    rhs.insert(rhs.begin(), lhs.begin(), lhs.end());
  }
  /// Emplaces and prepends a new element.
  template <typename... Args>
  void emplace_front(Args&&... args) {
    // TODO(afuller): Switch prepend to a std::forward_list.
    auto& prepend = assignOr(*data_.prepend());
    prepend.emplace(prepend.begin(), std::forward<Args>(args)...);
  }
  /// Prepends the given element value.
  template <typename U = typename T::value_type>
  void push_front(U&& val) {
    // TODO(afuller): Switch prepend to a std::forward_list.
    auto& prepend = assignOr(*data_.prepend());
    prepend.insert(prepend.begin(), std::forward<U>(val));
  }

  /// Removes an element.
  template <typename U = typename T::value_type>
  void erase(U&& val) {
    if (hasAssign()) {
      auto& assign = *data_.assign();
      auto it = std::find(assign.begin(), assign.end(), val);
      if (it != assign.end()) {
        assign.erase(it);
      }
    }
    data_.remove()->push_back(std::forward<U>(val));
  }

  /// Removes all elements in the list.
  template <typename C = std::unordered_set<typename T::value_type>>
  void remove(C&& entries) {
    if (hasAssign()) {
      auto& assign = *data_.assign();
      remove_all_values(assign, entries);
    }
    data_.remove()->insert(
        data_.remove()->end(), entries.begin(), entries.end());
  }

  /// Returns the patch that for the element in a given position.
  FOLLY_NODISCARD VP& patchAt(size_t pos) {
    if (*data_.clear()) {
      folly::throw_exception<bad_patch_access>();
    }

    return data_.patch()->operator[](ListPatchIndex(pos));
  }

  template <class Visitor>
  void customVisit(Visitor&& v) const {
    if (false) {
      // Test whether the required methods exist in Visitor
      v.assign(T{});
      v.clear();
      v.patchIfSet(std::unordered_map<ListPatchIndex, VP>{});
      v.remove(std::vector<typename T::value_type>{});
      v.prepend(T{});
      v.append(T{});
    }

    if (Base::template customVisitAssignAndClear(v)) {
      return;
    }

    v.patchIfSet(*data_.patch());
    v.remove(*data_.remove());
    v.prepend(*data_.prepend());
    v.append(*data_.append());
  }

  void apply(T& val) const {
    struct Visitor {
      T& v;
      void assign(const T& t) { v = t; }
      void clear() { v.clear(); }
      void patchIfSet(const VPMap& patches) {
        for (const auto& ep : patches) {
          auto idx = ep.first.position();
          if (idx < v.size()) {
            ep.second.apply(v[idx]);
          }
        }
      }
      void remove(const std::vector<typename T::value_type>& t) {
        remove_all_values(v, t);
      }
      void prepend(const T& t) { v.insert(v.begin(), t.begin(), t.end()); }
      void append(const T& t) { v.insert(v.end(), t.begin(), t.end()); }
    };

    return customVisit(Visitor{val});
  }

 private:
  using Base::applyAssignOrClear;
  using Base::assignOr;
  using Base::data_;
  using Base::hasAssign;
  using Base::mergeAssignAndClear;

  // Needed for merge(...). We can consider making this a public API.
  void patchIfSet(const VPMap& patches) {
    for (const auto& ep : patches) {
      patchAt(ep.first.position()).merge(ep.second);
    }
  }
};

/// Patch for a Thrift set.
///
/// The `Patch` template parameter must be a Thrift struct with the following
/// fields:
/// * `optional set<T> assign`
/// * `terse bool clear`
/// * `terse set<T> add`
/// * `terse set<T> remove`
template <typename Patch>
class SetPatch : public BaseContainerPatch<Patch, SetPatch<Patch>> {
  using Base = BaseContainerPatch<Patch, SetPatch>;
  using T = typename Base::value_type;

 public:
  using Base::apply;
  using Base::Base;
  using Base::operator=;

  /// Creates a new patch that adds keys.
  template <typename C = T>
  static SetPatch createAdd(C&& keys) {
    SetPatch result;
    *result.data_.add() = std::forward<C>(keys);
    return result;
  }

  /// Creates a new patch that removes keys.
  template <typename C = T>
  static SetPatch createRemove(C&& keys) {
    SetPatch result;
    *result.data_.remove() = std::forward<C>(keys);
    return result;
  }

  /// Adds keys.
  template <typename C = T>
  void add(C&& keys) {
    erase_all(*data_.remove(), keys);
    assignOr(*data_.add()).insert(keys.begin(), keys.end());
  }
  /// Emplaces the set.
  template <typename... Args>
  void emplace(Args&&... args) {
    if (data_.assign().has_value()) {
      data_.assign()->emplace(std::forward<Args>(args)...);
      return;
    }
    auto result = data_.add()->emplace(std::forward<Args>(args)...);
    if (result.second) {
      data_.remove()->erase(*result.first);
    }
  }
  /// Adds a key.
  template <typename U = typename T::value_type>
  void insert(U&& val) {
    if (data_.assign().has_value()) {
      data_.assign()->insert(std::forward<U>(val));
      return;
    }
    data_.remove()->erase(val);
    data_.add()->insert(std::forward<U>(val));
  }

  /// Removes keys.
  template <typename C = T>
  void remove(C&& keys) {
    if (data_.assign().has_value()) {
      erase_all(*data_.assign(), keys);
      return;
    }
    erase_all(*data_.add(), keys);
    data_.remove()->insert(keys.begin(), keys.end());
  }
  /// Remove a key.
  template <typename U = typename T::value_type>
  void erase(U&& val) {
    assignOr(*data_.add()).erase(val);
    assignOr(*data_.remove()).insert(std::forward<U>(val));
  }

  template <class Visitor>
  void customVisit(Visitor&& v) const {
    if (false) {
      // Test whether the required methods exist in Visitor
      v.assign(T{});
      v.clear();
      v.remove(T{});
      v.add(T{});
    }

    if (Base::template customVisitAssignAndClear(v)) {
      return;
    }

    v.remove(*data_.remove());
    v.add(*data_.add());
  }

  void apply(T& val) const {
    struct Visitor {
      T& v;
      void assign(const T& t) { v = t; }
      void clear() { v.clear(); }
      void remove(const T& t) { erase_all(v, t); }
      void add(const T& t) { v.insert(t.begin(), t.end()); }
    };

    return customVisit(Visitor{val});
  }

 private:
  using Base::applyAssignOrClear;
  using Base::assignOr;
  using Base::data_;
  using Base::mergeAssignAndClear;
};

/// Patch for a Thrift map.
///
/// The `Patch` template parameter must be a Thrift struct with the following
/// fields:
/// * `optional map<K, V> assign`
/// * `terse bool clear`
/// * `terse map<K, V> put`
template <typename Patch>
class MapPatch : public BaseContainerPatch<Patch, MapPatch<Patch>> {
  using Base = BaseContainerPatch<Patch, MapPatch>;
  using T = typename Base::value_type;
  using P = folly::remove_cvref_t<decltype(*std::declval<Patch>().patch())>;
  using VP = typename P::mapped_type;

 public:
  using Base::apply;
  using Base::Base;
  using Base::operator=;

  /// Creates a patch that inserts entries. Override entries if exists.
  template <typename C = T>
  static MapPatch createPut(C&& entries) {
    MapPatch result;
    *result.data_.put() = std::forward<C>(entries);
    return result;
  }

  /// Inserts entries. Override entries if exists.
  template <typename C = T>
  void put(C&& entries) {
    auto& field = assignOr(*data_.put());
    auto& patchPrior = *data_.patchPrior();
    for (auto&& entry : entries) {
      auto key = std::forward<decltype(entry)>(entry).first;
      field.insert_or_assign(key, std::forward<decltype(entry)>(entry).second);
      data_.add()->erase(key);
      data_.remove()->erase(key);
      patchPrior.erase(key);
    }
  }
  /// Inserts entries. Override entries if exists.
  template <typename K, typename V>
  void insert_or_assign(K&& key, V&& value) {
    assignOr(*data_.put()).insert_or_assign(key, std::forward<V>(value));
    data_.add()->erase(key);
    data_.remove()->erase(key);
    data_.patchPrior()->erase(key);
  }

  /// Inserts entries. Ignore entries that already exists.
  template <typename C = T>
  void add(C&& entries) {
    auto& field = assignOr(*data_.add());
    for (auto&& entry : entries) {
      auto key = std::forward<decltype(entry)>(entry).first;
      field.insert_or_assign(key, std::forward<decltype(entry)>(entry).second);
    }
  }

  /// Removes keys.
  template <typename C = std::unordered_set<typename T::key_type>>
  void remove(C&& keys) {
    auto& field = assignOr(*data_.add());
    auto& patchPrior = *data_.patchPrior();
    auto& patch = *data_.patch();
    for (auto&& key : keys) {
      field.erase(key);
      data_.remove()->insert(key);
      patchPrior.erase(key);
      patch.erase(key);
    }
  }

  /// Removes a key.
  template <typename K = typename T::key_type>
  void erase(K&& key) {
    assignOr(*data_.add()).erase(key);
    data_.remove()->insert(key);
    data_.patchPrior()->erase(key);
    data_.patch()->erase(key);
  }

  /// Returns the patch that for the entry.
  template <typename K = typename T::key_type>
  FOLLY_NODISCARD VP& patchByKey(K&& key) {
    if (data_.remove()->count(key)) {
      // We are going to delete key, thus patchByKey is no-op and we return a
      // dummy patch.
      return dummy_;
    }
    return isKeyModified(key) ? data_.patch()->operator[](key)
                              : data_.patchPrior()->operator[](key);
  }

  /// Ensures that key exists and patches the entry.
  template <typename K = typename T::key_type>
  FOLLY_NODISCARD VP& ensureAndPatchByKey(K&& key) {
    return (data_.add().value()[key], data_.patch()->operator[](key));
  }

  template <class Visitor>
  void customVisit(Visitor&& v) const {
    if (false) {
      // Test whether the required methods exist in Visitor
      v.assign(T{});
      v.clear();
      v.patchIfSet(P{});
      v.add(T{});
      v.put(T{});
      v.remove(std::unordered_set<typename T::key_type>{});
    }

    if (Base::template customVisitAssignAndClear(v)) {
      return;
    }

    v.patchIfSet(*data_.patchPrior());
    v.add(*data_.add());
    v.remove(*data_.remove());
    v.put(*data_.put());
    v.patchIfSet(*data_.patch());
  }

  void apply(T& val) const {
    struct Visitor {
      T& v;
      void assign(const T& t) { v = t; }
      void clear() { v.clear(); }
      void patchIfSet(const P& kv) {
        for (const auto& p : kv) {
          auto it = v.find(p.first);
          if (it != v.end()) {
            p.second.apply(it->second);
          }
        }
      }
      void remove(const std::unordered_set<typename T::key_type>& keys) {
        erase_all(v, keys);
      }
      void add(const T& t) { v.insert(t.begin(), t.end()); }
      void put(const T& t) {
        for (const auto& entry : t) {
          v.insert_or_assign(entry.first, entry.second);
        }
      }
    };

    return customVisit(Visitor{val});
  }

 private:
  template <typename K = typename T::key_type>
  bool isKeyModified(K&& key) {
    return data_.add()->find(key) != data_.add()->end() ||
        data_.put()->find(key) != data_.put()->end();
  }

  void applyPatch(T& val, const P& patch) const {
    for (const auto& p : patch) {
      auto it = val.find(p.first);
      if (it != val.end()) {
        p.second.apply(it->second);
      }
    }
  }

  // Needed for merge(...). We can consider making this a public API.
  void patchIfSet(const P& patches) {
    for (auto&& ep : patches) {
      patchByKey(ep.first).merge(ep.second);
    }
  }

  using Base::applyAssignOrClear;
  using Base::assignOr;
  using Base::data_;
  using Base::mergeAssignAndClear;

  VP dummy_;
};

} // namespace detail
} // namespace op
} // namespace thrift
} // namespace apache

FBTHRIFT_STD_HASH_WRAP_DATA(apache::thrift::op::detail::ListPatchIndex)
