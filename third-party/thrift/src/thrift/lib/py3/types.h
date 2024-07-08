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
#include <optional>
#include <string_view>
#include <unordered_map>

#include <Python.h>
#include <glog/logging.h>

#include <folly/Indestructible.h>
#include <folly/Range.h>

#include <thrift/lib/cpp/Thrift.h>
#include <thrift/lib/cpp2/FieldRef.h>

namespace thrift {
namespace py3 {

template <typename T>
std::shared_ptr<T> constant_shared_ptr(const T& x) {
  return std::shared_ptr<T>(std::shared_ptr<T>{}, const_cast<T*>(&x));
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
bool setcmp(const std::shared_ptr<T>& a, const std::shared_ptr<T>& b, int op) {
  auto sa = a->size();
  auto sb = b->size();
  // reduce to LE with additional size check
  switch (op) {
    case Py_LT: // subset
      return sa < sb && setcmp(a, b, Py_LE);
    case Py_EQ:
      return sa == sb && setcmp(a, b, Py_LE);
    case Py_NE:
      return sa != sb || !setcmp(a, b, Py_LE);
    case Py_GT: // strict superset
      return sa > sb && setcmp(b, a, Py_LE);
    case Py_GE: // superset
      return setcmp(b, a, Py_LE);
    case Py_LE:
      for (const auto& e : *a) {
        if (b->find(e) == b->end()) {
          return false;
        }
      }
      return true;
    default:
      LOG(FATAL) << "Invalid op in richcmp " << op;
  }
}

template <typename T>
std::shared_ptr<T> setand(
    const std::shared_ptr<T>& a, const std::shared_ptr<T>& b) {
  T out;
  for (const auto& e : *a) {
    if (b->find(e) != b->end()) {
      out.insert(e);
    }
  }
  return std::make_shared<T>(std::move(out));
}

template <typename T>
std::shared_ptr<T> setsub(
    const std::shared_ptr<T>& a, const std::shared_ptr<T>& b) {
  T out;
  for (const auto& e : *a) {
    if (b->find(e) == b->end()) {
      out.insert(e);
    }
  }
  return std::make_shared<T>(std::move(out));
}

template <typename T>
std::shared_ptr<T> setor(
    const std::shared_ptr<T>& a, const std::shared_ptr<T>& b) {
  T out;
  for (const auto& e : *a) {
    out.insert(e);
  }
  for (const auto& e : *b) {
    out.insert(e);
  }
  return std::make_shared<T>(std::move(out));
}

template <typename T>
std::shared_ptr<T> setxor(
    const std::shared_ptr<T>& a, const std::shared_ptr<T>& b) {
  T out;
  for (const auto& e : *a) {
    if (b->find(e) == b->end()) {
      out.insert(e);
    }
  }
  for (const auto& e : *b) {
    if (a->find(e) == a->end()) {
      out.insert(e);
    }
  }
  return std::make_shared<T>(std::move(out));
}

enum SetOp {
  AND,
  OR,
  SUB,
  XOR,
  REVSUB,
};

template <typename T>
std::shared_ptr<T> set_op(
    const std::shared_ptr<T>& a, const std::shared_ptr<T>& b, SetOp op) {
  switch (op) {
    case SetOp::AND:
      return setand(a, b);
    case SetOp::OR:
      return setor(a, b);
    case SetOp::SUB:
      return setsub(a, b);
    case SetOp::XOR:
      return setxor(a, b);
    case SetOp::REVSUB:
      return setsub(b, a);
  }
}

template <typename T>
struct set_iter {
  set_iter() = default;
  set_iter(const std::shared_ptr<T>& cpp_obj) : it{cpp_obj->begin()} {}
  typename T::iterator it;
  template <typename V>
  void genNext(const std::shared_ptr<T>& cpp_obj, std::shared_ptr<V>& out) {
    out = std::shared_ptr<V>(cpp_obj, const_cast<V*>(&*it));
    ++it;
  }
  template <typename V>
  void genNext(const std::shared_ptr<T>&, V& out) {
    out = *it;
    ++it;
  }
};

template <typename T>
bool map_contains(
    const std::shared_ptr<T>& cpp_obj, const typename T::key_type key) {
  return cpp_obj->find(key) != cpp_obj->end();
}

template <typename T, typename V>
void map_getitem(
    const std::shared_ptr<T>& cpp_obj,
    const typename T::key_type key,
    std::shared_ptr<V>& out) {
  out = std::shared_ptr<V>(cpp_obj, &cpp_obj->operator[](key));
}

template <typename T, typename V>
void map_getitem(
    const std::shared_ptr<T>& cpp_obj, const typename T::key_type key, V& out) {
  out = cpp_obj->operator[](key);
}

template <typename T>
struct map_iter {
  map_iter() = default;
  map_iter(const std::shared_ptr<T>& cpp_obj) : it{cpp_obj->begin()} {}
  typename T::iterator it;
  template <typename K>
  void genNextKey(const std::shared_ptr<T>& cpp_obj, std::shared_ptr<K>& out) {
    out = std::shared_ptr<K>(cpp_obj, const_cast<K*>(&it->first));
    ++it;
  }
  template <typename K>
  void genNextKey(const std::shared_ptr<T>&, K& out) {
    out = it->first;
    ++it;
  }
  template <typename V>
  void genNextValue(
      const std::shared_ptr<T>& cpp_obj, std::shared_ptr<V>& out) {
    out = std::shared_ptr<V>(cpp_obj, const_cast<V*>(&it->second));
    ++it;
  }
  template <typename V>
  void genNextValue(const std::shared_ptr<T>&, V& out) {
    out = it->second;
    ++it;
  }
  template <typename K, typename V>
  void genNextItem(const std::shared_ptr<T>&, K& key_out, V& value_out) {
    key_out = it->first;
    value_out = it->second;
    ++it;
  }
  template <typename K, typename V>
  void genNextItem(
      const std::shared_ptr<T>& cpp_obj,
      K& key_out,
      std::shared_ptr<V>& value_out) {
    key_out = it->first;
    value_out = std::shared_ptr<V>(cpp_obj, const_cast<V*>(&it->second));
    ++it;
  }
  template <typename K, typename V>
  void genNextItem(
      const std::shared_ptr<T>& cpp_obj,
      std::shared_ptr<K>& key_out,
      V& value_out) {
    key_out = std::shared_ptr<K>(cpp_obj, const_cast<K*>(&it->first));
    value_out = it->second;
    ++it;
  }

  template <typename K, typename V>
  void genNextItem(
      const std::shared_ptr<T>& cpp_obj,
      std::shared_ptr<K>& key_out,
      std::shared_ptr<V>& value_out) {
    key_out = std::shared_ptr<K>(cpp_obj, const_cast<K*>(&it->first));
    value_out = std::shared_ptr<V>(cpp_obj, const_cast<V*>(&it->second));
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

} // namespace py3
} // namespace thrift
