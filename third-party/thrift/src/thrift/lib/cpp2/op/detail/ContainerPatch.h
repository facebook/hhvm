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

#include <folly/Range.h>
#include <folly/Utility.h>
#include <thrift/lib/cpp/util/VarintUtils.h>
#include <thrift/lib/cpp2/Adapter.h>
#include <thrift/lib/cpp2/op/detail/BasePatch.h>

namespace apache {
namespace thrift {
namespace op {
namespace detail {

// Replace with `std::views::single` in C++20
template <class T>
auto single(T&& t) {
  return folly::Range{&t, 1};
}

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

  /// Returns the patch that for the element in a given position.
  FOLLY_NODISCARD VP& patchAt(size_t pos) {
    if (*data_.clear()) {
      folly::throw_exception<bad_patch_access>();
    }

    return data_.patch()->operator[](ListPatchIndex(pos));
  }

  /// @copybrief AssignPatch::customVisit
  ///
  /// Users should provide a visitor with the following methods
  ///
  ///     struct Visitor {
  ///       void assign(const List&);
  ///       void clear();
  ///       void prepend(const List&);
  ///       void append(const List&);
  ///       void patchIfSet(const std::unordered_map<ListPatchIndex,
  ///                                                ElementPatch>&);
  ///     }
  ///
  /// For example:
  ///
  ///     ListPatch<ListI32Patch> patch;
  ///     patch.patchAt(10).add(20);
  ///     patch.push_front(30);
  ///
  /// `patch.customVisit(v)` will invoke the following methods
  ///
  ///     v.patchIfSet({{10, I32Patch::createAdd(20)}});
  ///     v.prepend({30});
  template <class Visitor>
  void customVisit(Visitor&& v) const {
    if (false) {
      // Test whether the required methods exist in Visitor
      v.assign(T{});
      v.clear();
      v.patchIfSet(std::unordered_map<ListPatchIndex, VP>{});
      v.prepend(T{});
      v.append(T{});
    }

    if (Base::template customVisitAssignAndClear(v)) {
      return;
    }

    v.patchIfSet(*data_.patch());
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
      void prepend(const T& t) { v.insert(v.begin(), t.begin(), t.end()); }
      void append(const T& t) { v.insert(v.end(), t.begin(), t.end()); }
    };

    return customVisit(Visitor{val});
  }

 private:
  using Base::assignOr;
  using Base::data_;
  using Base::hasAssign;

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

  /// Adds keys.
  template <typename C = T>
  void add(C&& keys) {
    erase_all(*data_.remove(), keys);
    assignOr(*data_.add()).insert(keys.begin(), keys.end());
  }
  /// Emplaces the set.
  template <typename... Args>
  void emplace(Args&&... args) {
    insert({std::forward<Args>(args)...});
  }
  /// Adds a key.
  template <typename U = typename T::value_type>
  void insert(U&& val) {
    add(single(std::forward<U>(val)));
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

  /// @copybrief AssignPatch::customVisit
  ///
  /// Users should provide a visitor with the following methods
  ///
  ///     struct Visitor {
  ///       void assign(const Set&);
  ///       void clear();
  ///       void remove(const Set&);
  ///       void add(const Set&);
  ///     }
  ///
  /// For example:
  ///
  ///     SetPatch<SetI32Patch> patch;
  ///     patch.remove({20, 30});
  ///     patch.add({10, 20});
  ///
  /// `patch.customVisit(v)` will invoke the following methods
  ///
  ///     v.add({10, 20});
  ///     v.remove({30});
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
  using Base::assignOr;
  using Base::data_;
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
    put(single(std::pair<K&&, V&&>(key, value)));
  }

  /// Inserts entries. Ignore entries that already exist.
  template <typename C = T>
  void add(C&& entries) {
    assignOr(*data_.add()).insert(entries.begin(), entries.end());
    for (const auto& entry : entries) {
      if (data_.remove()->erase(entry.first)) {
        // If it was already removed, we should re-assign the entity.
        auto& added = (*data_.add())[entry.first];
        (*data_.patch())[entry.first] = std::move(added);
        added = {};
      }
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
    remove(single(std::forward<K>(key)));
  }

  /// Returns the patch that for the entry.
  template <typename K = typename T::key_type>
  FOLLY_NODISCARD VP& patchByKey(K&& key) {
    ensurePatchable();
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

  /// @copybrief AssignPatch::customVisit
  ///
  /// Users should provide a visitor with the following methods
  ///
  ///     struct Visitor {
  ///       void assign(const Map&);
  ///       void clear();
  ///       void add(const Map&);
  ///       void put(const Map&);
  ///       void remove(const std::unordered_set<Key>&);
  ///       void patchIfSet(const std::unordered_map<Key, ValuePatch>&);
  ///     }
  ///
  /// For example:
  ///
  ///     MapPatch<MapI32StringPatch> patch;
  ///     patch.add({{10, "10"}});
  ///     patch.ensureAndPatchByKey(20).append("_");
  ///
  /// `patch.customVisit(v)` will invoke the following methods
  ///
  ///     v.add({{10, "10"}, {20, ""}});
  ///     v.patchIfSet({{20, StringPatch::createAppend("_")}});
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

  // If field has assign, we need to replace it with clear && add
  void ensurePatchable() {
    if (hasAssign()) {
      data_.clear() = true;
      data_.add() = std::move(*data_.assign());
      data_.assign().reset();
    }
  }

  using Base::assignOr;
  using Base::data_;
  using Base::hasAssign;

  VP dummy_;
};

} // namespace detail
} // namespace op
} // namespace thrift
} // namespace apache

FBTHRIFT_STD_HASH_WRAP_DATA(apache::thrift::op::detail::ListPatchIndex)
