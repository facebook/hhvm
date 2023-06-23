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

#include <type_traits>

#include <Python.h>
#include <folly/CppAttributes.h>
#include <folly/Traits.h>
#include <folly/Utility.h>
#include <thrift/lib/python/capi/types.h>

namespace apache {
namespace thrift {
namespace python {
namespace capi {
/**
 * Series of functions to convert a native type T into a Python object
 *
 * Some important notes:
 * 1. The construct function returns a 'new reference'. i.e. you own the object
 *    it returns.
 * 2: The constructor will leave the python indicator set if a python
 *    exception is encountered during the extraction. Be sure to check the error
 *    indicator before calling back into python!
 */
template <typename T>
struct Constructor {
  template <typename U>
  struct Unspecialized : std::false_type {};
  static_assert(Unspecialized<T>{}, "No constructor defined");
  /* Target */ PyObject* operator()(T&&);
};

#define SPECIALIZE_SCALAR(type)                  \
  template <>                                    \
  struct Constructor<type> {                     \
    PyObject* FOLLY_NULLABLE operator()(type&&); \
  }

SPECIALIZE_SCALAR(int8_t);
SPECIALIZE_SCALAR(int16_t);
SPECIALIZE_SCALAR(int32_t);
SPECIALIZE_SCALAR(int64_t);
SPECIALIZE_SCALAR(uint32_t);
SPECIALIZE_SCALAR(uint64_t);
SPECIALIZE_SCALAR(bool);
SPECIALIZE_SCALAR(float);
SPECIALIZE_SCALAR(double);

#undef SPECIALIZE_SCALAR
#define SPECIALIZE_STR(cpp_type, py_type)            \
  template <>                                        \
  struct Constructor<py_type> {                      \
    PyObject* FOLLY_NULLABLE operator()(cpp_type&&); \
  }

SPECIALIZE_STR(std::string, Bytes);
SPECIALIZE_STR(std::string, String);

#undef SPECIALIZE_STR

template <typename ElemT, typename CppT>
struct Constructor<list<ElemT, CppT>> {
  PyObject* FOLLY_NULLABLE operator()(CppT&& val) {
    const size_t size = val.size();
    StrongRef list(PyTuple_New(size));
    if (!list) {
      return nullptr;
    }
    Constructor<ElemT> ctor{};
    for (size_t i = 0; i < size; ++i) {
      // PyTuple_SET_ITEM steals, so don't use StrongRef
      PyObject* elem(ctor(std::move(val[i])));
      if (elem == nullptr) {
        // StrongRef DECREFs the list tuple on scope exit
        return nullptr;
      }
      PyTuple_SET_ITEM(*list, i, elem);
    }
    return std::move(list).release();
  }
};

template <typename C>
using iter_t = decltype(std::declval<C&>().begin());
template <typename C>
using extract_method_t =
    decltype(std::declval<C&>().extract(std::declval<iter_t<C>>()));
template <typename C>
constexpr bool has_extract_v =
    folly::is_detected_v<extract_method_t, std::remove_reference_t<C>>;

template <typename ElemT, typename CppT>
struct Constructor<set<ElemT, CppT>> {
  PyObject* FOLLY_NULLABLE operator()(CppT&& val) {
    StrongRef set_obj(PyFrozenSet_New(nullptr));
    if (!set_obj) {
      return nullptr;
    }
    Constructor<ElemT> ctor{};
    auto it = val.begin();
    while (it != val.end()) {
      StrongRef elem;
      if constexpr (has_extract_v<CppT>) {
        auto node_handle = val.extract(it++);
        elem = StrongRef(ctor(std::move(node_handle.value())));
      } else {
        // If the container doesn't support extract, have to copy
        // because set iterator is const :(
        elem = StrongRef(ctor(folly::copy(*it)));
        ++it;
      }
      if (*elem == nullptr) {
        // StrongRef DECREFs the frozenset on scope exit
        return nullptr;
      }
      // Not stolen, so let StrongRef DECREF on exit
      if (PySet_Add(*set_obj, *elem) == -1) {
        // Should only fail for MemoryError (OOM)
        return nullptr;
      }
    }
    return std::move(set_obj).release();
  }
};

template <typename KeyT, typename ValT, typename CppT>
struct Constructor<map<KeyT, ValT, CppT>> {
  PyObject* FOLLY_NULLABLE operator()(CppT&& cpp_map) {
    StrongRef dict(PyTuple_New(cpp_map.size()));
    if (!dict) {
      return nullptr;
    }
    Constructor<KeyT> key_ctor{};
    Constructor<ValT> val_ctor{};
    auto it = cpp_map.begin();
    size_t idx = 0;
    while (it != cpp_map.end()) {
      StrongRef key;
      StrongRef val;
      if constexpr (has_extract_v<CppT>) {
        auto node_handle = cpp_map.extract(it++);
        key = StrongRef(key_ctor(std::move(node_handle.key())));
        if (!key) {
          // StrongRef DECREFs the dict on scope exit
          return nullptr;
        }
        val = StrongRef(val_ctor(std::move(node_handle.mapped())));
      } else {
        // If the container doesn't support extract, have to copy
        // because set iterator is const :(
        key = StrongRef(key_ctor(folly::copy(it->first)));
        if (!key) {
          // StrongRef DECREFs the dict on scope exit
          return nullptr;
        }
        val = StrongRef(val_ctor(std::move(it->second)));
        ++it;
      }
      if (!val) {
        // StrongRef DECREFs the dict and key on scope exit
        return nullptr;
      }
      PyObject* kv_tuple = PyTuple_New(2);
      if (kv_tuple == nullptr) {
        return nullptr;
      }
      // PyTuple_SET_ITEM steals, so have to release StrongRefs
      PyTuple_SET_ITEM(kv_tuple, 0, std::move(key).release());
      PyTuple_SET_ITEM(kv_tuple, 1, std::move(val).release());
      PyTuple_SET_ITEM(*dict, idx++, kv_tuple);
    }
    return std::move(dict).release();
  }
};

} // namespace capi
} // namespace python
} // namespace thrift
} // namespace apache
