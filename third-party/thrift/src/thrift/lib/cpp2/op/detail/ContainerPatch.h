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

#include <utility>

#include <thrift/lib/cpp2/op/detail/BasePatch.h>

namespace apache {
namespace thrift {
namespace op {
namespace detail {

template <typename C1, typename C2>
void erase_all(C1& container, const C2& values) {
  for (auto itr = values.begin(); itr != values.end() && !container.empty();
       ++itr) {
    container.erase(*itr);
  }
}

// Patch must have the following fields:
//   optional list<T> assign;
//   bool clear;
//   list<T> append;
//   list<T> prepend;
template <typename Patch>
class ListPatch : public BaseContainerPatch<Patch, ListPatch<Patch>> {
  using Base = BaseContainerPatch<Patch, ListPatch>;
  using T = typename Base::value_type;

 public:
  using Base::apply;
  using Base::Base;
  using Base::operator=;

  template <typename C = T>
  static ListPatch createAppend(C&& values) {
    ListPatch result;
    *result.data_.append() = std::forward<C>(values);
    return result;
  }

  template <typename C = T>
  static ListPatch createPrepend(C&& values) {
    ListPatch result;
    *result.data_.prepend() = std::forward<C>(values);
    return result;
  }

  template <typename C = T>
  void append(C&& rhs) {
    auto& lhs = assignOr(*data_.append());
    lhs.insert(lhs.end(), rhs.begin(), rhs.end());
  }
  template <typename... Args>
  void emplace_back(Args&&... args) {
    assignOr(*data_.append()).emplace_back(std::forward<Args>(args)...);
  }
  template <typename U = typename T::value_type>
  void push_back(U&& val) {
    assignOr(*data_.append()).push_back(std::forward<U>(val));
  }

  template <typename C = T>
  void prepend(C&& lhs) {
    auto& rhs = assignOr(*data_.prepend());
    rhs.insert(rhs.begin(), lhs.begin(), lhs.end());
  }
  template <typename... Args>
  void emplace_front(Args&&... args) {
    // TODO(afuller): Switch prepend to a std::forward_list.
    auto& prepend = assignOr(*data_.prepend());
    prepend.emplace(prepend.begin(), std::forward<Args>(args)...);
  }
  template <typename U = typename T::value_type>
  void push_front(U&& val) {
    // TODO(afuller): Switch prepend to a std::forward_list.
    auto& prepend = assignOr(*data_.prepend());
    prepend.insert(prepend.begin(), std::forward<U>(val));
  }

  void apply(T& val) const {
    if (!applyAssignOrClear(val)) {
      val.insert(val.begin(), data_.prepend()->begin(), data_.prepend()->end());
      val.insert(val.end(), data_.append()->begin(), data_.append()->end());
    }
  }

  template <typename U>
  void merge(U&& next) {
    if (!mergeAssignAndClear(std::forward<U>(next))) {
      // TODO(afuller): Optimize the r-value reference case.
      if (!next.toThrift().prepend()->empty()) {
        decltype(auto) rhs = *std::forward<U>(next).toThrift().prepend();
        data_.prepend()->insert(
            data_.prepend()->begin(), rhs.begin(), rhs.end());
      }
      if (!next.toThrift().append()->empty()) {
        decltype(auto) rhs = *std::forward<U>(next).toThrift().append();
        data_.append()->reserve(data_.append()->size() + rhs.size());
        auto inserter = std::back_inserter(*data_.append());
        std::copy_n(rhs.begin(), rhs.size(), inserter);
      }
    }
  }

 private:
  using Base::applyAssignOrClear;
  using Base::assignOr;
  using Base::data_;
  using Base::mergeAssignAndClear;
};

// Patch must have the following fields:
//   optional set<T> assign;
//   bool clear;
//   set<T> add;
//   set<T> remove;
template <typename Patch>
class SetPatch : public BaseContainerPatch<Patch, SetPatch<Patch>> {
  using Base = BaseContainerPatch<Patch, SetPatch>;
  using T = typename Base::value_type;

 public:
  using Base::apply;
  using Base::Base;
  using Base::operator=;

  template <typename C = T>
  static SetPatch createAdd(C&& keys) {
    SetPatch result;
    *result.data_.add() = std::forward<C>(keys);
    return result;
  }

  template <typename C = T>
  static SetPatch createRemove(C&& keys) {
    SetPatch result;
    *result.data_.remove() = std::forward<C>(keys);
    return result;
  }

  template <typename C = T>
  void add(C&& keys) {
    erase_all(*data_.remove(), keys);
    assignOr(*data_.add()).insert(keys.begin(), keys.end());
  }
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
  template <typename U = typename T::value_type>
  void insert(U&& val) {
    if (data_.assign().has_value()) {
      data_.assign()->insert(std::forward<U>(val));
      return;
    }
    data_.remove()->erase(val);
    data_.add()->insert(std::forward<U>(val));
  }

  template <typename C = T>
  void remove(C&& keys) {
    if (data_.assign().has_value()) {
      erase_all(*data_.assign(), keys);
      return;
    }
    erase_all(*data_.add(), keys);
    data_.remove()->insert(keys.begin(), keys.end());
  }
  template <typename U = typename T::value_type>
  void erase(U&& val) {
    assignOr(*data_.add()).erase(val);
    assignOr(*data_.remove()).insert(std::forward<U>(val));
  }

  void apply(T& val) const {
    if (applyAssignOrClear(val)) {
      return;
    }

    erase_all(val, *data_.remove());
    val.insert(data_.add()->begin(), data_.add()->end());
  }

  template <typename U>
  void merge(U&& next) {
    if (!mergeAssignAndClear(std::forward<U>(next))) {
      remove(*std::forward<U>(next).toThrift().remove());
      add(*std::forward<U>(next).toThrift().add());
    }
  }

 private:
  using Base::applyAssignOrClear;
  using Base::assignOr;
  using Base::data_;
  using Base::mergeAssignAndClear;
};

// Patch must have the following fields:
//   optional map<K, V> assign;
//   bool clear;
//   map<K, V> put;
template <typename Patch>
class MapPatch : public BaseContainerPatch<Patch, MapPatch<Patch>> {
  using Base = BaseContainerPatch<Patch, MapPatch>;
  using T = typename Base::value_type;

 public:
  using Base::apply;
  using Base::Base;
  using Base::operator=;

  template <typename C = T>
  static MapPatch createPut(C&& entries) {
    MapPatch result;
    *result.data_.put() = std::forward<C>(entries);
    return result;
  }

  template <typename C = T>
  void put(C&& entries) {
    auto& field = assignOr(*data_.put());
    for (auto&& entry : entries) {
      field.insert_or_assign(
          std::forward<decltype(entry)>(entry).first,
          std::forward<decltype(entry)>(entry).second);
    }
  }
  template <typename K, typename V>
  void insert_or_assign(K&& key, V&& value) {
    assignOr(*data_.put())
        .insert_or_assign(std::forward<K>(key), std::forward<V>(value));
  }

  void apply(T& val) const {
    if (applyAssignOrClear(val)) {
      return;
    }
    for (const auto& entry : *data_.put()) {
      val.insert_or_assign(entry.first, entry.second);
    }
  }

  template <typename U>
  void merge(U&& next) {
    if (!mergeAssignAndClear(std::forward<U>(next))) {
      put(*std::forward<U>(next).toThrift().put());
    }
  }

 private:
  using Base::applyAssignOrClear;
  using Base::assignOr;
  using Base::data_;
  using Base::mergeAssignAndClear;
};

} // namespace detail
} // namespace op
} // namespace thrift
} // namespace apache
