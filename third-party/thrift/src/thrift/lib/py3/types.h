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

#include <memory>
#include <string_view>
#include <unordered_map>

#include <Python.h>
#include <glog/logging.h>

#include <folly/Indestructible.h>
#include <folly/Range.h>
#include <folly/Traits.h>

#include <thrift/lib/cpp/Thrift.h>
#include <thrift/lib/cpp2/FieldRef.h>

namespace thrift::py3 {

template <typename T>
std::shared_ptr<T> constant_shared_ptr(const T& x) {
  return std::shared_ptr<T>(std::shared_ptr<T>{}, const_cast<T*>(&x));
}

template <typename T>
const T& deref_const(const std::unique_ptr<T>& ptr) {
  return *ptr;
}

template <typename T>
const T& deref_const(const std::shared_ptr<T>& ptr) {
  return *ptr;
}

template <typename T>
const T& deref_const(T& x) {
  return x;
}

template <typename T>
const T& deref_const(const T& x) {
  return x;
}

// the folly::remove_cvref_t conversions work around Cython's limitation on
// const/reference qualifier when custom cpp.type with those are defined
template <typename T, typename S>
std::shared_ptr<folly::remove_cvref_t<T>> reference_shared_ptr(
    const folly::remove_cvref_t<T>& ref, const std::shared_ptr<S>& owner) {
  using Type = folly::remove_cvref_t<T>;
  return std::shared_ptr<Type>(owner, const_cast<Type*>(&ref));
}

template <typename T>
const T& default_inst() {
  static const folly::Indestructible<T> inst{};
  return *inst;
}

template <typename T>
bool richcmp(const std::shared_ptr<T>& a, const std::shared_ptr<T>& b, int op) {
  switch (op) {
    case Py_LT:
      return *a < *b;
    case Py_LE:
      return *a <= *b;
    case Py_EQ:
      return *a == *b;
    case Py_NE:
      return *a != *b;
    case Py_GT:
      return *a > *b;
    case Py_GE:
      return *a >= *b;
    default:
      LOG(FATAL) << "Invalid op in richcmp " << op;
  }
}

template <typename T>
struct set_iter {
  set_iter() = default;
  explicit set_iter(const T& cpp_obj) : it{cpp_obj.begin()} {}
  typename T::const_iterator it;
  template <typename V>
  void genNextItem(V& out) {
    out = *it;
    ++it;
  }
  template <typename V>
  void genNextItem(std::shared_ptr<V>& out) {
    out = std::make_shared<V>(*it);
    ++it;
  }
};

template <typename T>
using cbegin_method_t = decltype(std::declval<T&>().cbegin());
template <typename T>
constexpr bool has_cbegin_v = folly::is_detected_v<cbegin_method_t, T>;

template <typename T, typename It = typename T::const_iterator>
It getConstIterator(const T& cpp_obj) {
  if constexpr (has_cbegin_v<T>) {
    return cpp_obj.cbegin();
  } else {
    return cpp_obj.begin();
  }
}

template <typename T>
struct map_iter {
  map_iter() = default;
  explicit map_iter(const T& cpp_obj) : it{getConstIterator(cpp_obj)} {}
  typename T::const_iterator it;
  template <typename K, typename V>
  void genNextKeyVal(K& key_out, V& value_out) {
    key_out = it->first;
    value_out = it->second;
    ++it;
  }
  template <typename K, typename V>
  void genNextKeyVal(K& key_out, std::shared_ptr<V>& value_out) {
    key_out = it->first;
    value_out = std::make_shared<V>(it->second);
    ++it;
  }
  template <typename K, typename V>
  void genNextKeyVal(std::shared_ptr<K>& key_out, V& value_out) {
    key_out = std::make_shared<K>(it->first);
    value_out = it->second;
    ++it;
  }
  template <typename K, typename V>
  void genNextKeyVal(
      std::shared_ptr<K>& key_out, std::shared_ptr<V>& value_out) {
    key_out = std::make_shared<K>(it->first);
    value_out = std::make_shared<V>(it->second);
    ++it;
  }
};

template <typename T>
void reset_field(T& obj, uint16_t index);

template <typename T>
struct PyStructTraits {
  using NamesMap = std::unordered_map<std::string_view, std::string_view>;
  static const NamesMap& namesmap();
};

template <typename T>
std::string_view get_field_name_by_index(size_t idx) {
  static const typename PyStructTraits<T>::NamesMap map =
      PyStructTraits<T>::namesmap();
  using storage = apache::thrift::TStructDataStorage<T>;
  const auto name = std::string_view(storage::fields_names.at(idx));
  auto found = map.find(name);
  return found == map.end() ? name : found->second;
}

// This is workaround of cython's limitation that function return isn't C++
// left-values. e.g. `foo.bar_ref() = make_unique[T]()` isn't valid cython code
// We need to write `assign_unique_ptr(foo.bar_ref(), make_unique[T]())` instead
template <typename T>
void assign_unique_ptr(std::unique_ptr<T>& x, std::unique_ptr<T> y) {
  x = std::move(y);
}

template <typename T>
void assign_shared_ptr(std::shared_ptr<T>& x, std::shared_ptr<T> y) {
  x = std::move(y);
}

template <typename T>
void assign_shared_const_ptr(
    std::shared_ptr<const T>& x, std::shared_ptr<const T> y) {
  x = std::move(y);
}

template <typename T, typename U>
T* get_union_field_value(apache::thrift::union_field_ref<U&> ref) {
  return &ref.value();
}

template <typename T>
PyObject* init_unicode_from_cpp(const T& str) {
  return PyUnicode_FromStringAndSize(str.data(), str.size());
}

} // namespace thrift::py3
