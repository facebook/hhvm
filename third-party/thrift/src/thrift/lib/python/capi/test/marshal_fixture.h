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

#include <Python.h>

#include <type_traits>
#include <folly/Expected.h>
#include <folly/container/F14Map.h>
#include <folly/container/F14Set.h>
#include <thrift/lib/python/capi/constructor.h>
#include <thrift/lib/python/capi/extractor.h>
#include <thrift/lib/python/capi/types.h>

namespace apache::thrift::python {

using capi::Bytes;
using capi::FallibleString;
using capi::list;
using capi::map;
using capi::set;
using capi::String;

template <typename T>
PyObject* __roundtrip_pyobject(PyObject* obj) {
  auto cpp = capi::Extractor<T>{}(obj);
  if (cpp.hasValue()) {
    return capi::Constructor<T>{}(std::move(*cpp));
  }
  return nullptr;
}

inline PyObject* __roundtrip_bytes(PyObject* obj) {
  return __roundtrip_pyobject<Bytes>(obj);
}

inline PyObject* __roundtrip_unicode(PyObject* obj) {
  return __roundtrip_pyobject<String>(obj);
}

inline PyObject* __roundtrip_fallible_unicode(PyObject* obj) {
  return __roundtrip_pyobject<FallibleString>(obj);
}

template <typename T>
inline PyObject* __make_numeric(PyObject* obj) {
  if constexpr (std::is_integral_v<T>) {
    auto cpp = capi::Extractor<int64_t>{}(obj);
    if (cpp.hasValue()) {
      return capi::Constructor<T>{}(static_cast<T>(*cpp));
    }
  } else if constexpr (std::is_floating_point_v<T>) {
    auto cpp = capi::Extractor<double>{}(obj);
    if (cpp.hasValue()) {
      return capi::Constructor<T>{}(static_cast<T>(*cpp));
    }
  } else {
    static_assert(folly::always_false<T>, "Use for numerics only");
  }
}

inline PyObject* __make_unicode(PyObject* bytes) {
  auto cpp = capi::Extractor<Bytes>{}(bytes);
  if (cpp.hasValue()) {
    return capi::Constructor<String>{}(std::move(*cpp));
  }
  return nullptr;
}

template <typename T>
inline PyObject* __roundtrip_list(PyObject* obj) {
  return __roundtrip_pyobject<capi::list<T>>(obj);
}

inline PyObject* __roundtrip_bytes_list(PyObject* obj) {
  return __roundtrip_pyobject<list<Bytes>>(obj);
}

inline PyObject* __roundtrip_unicode_list(PyObject* obj) {
  return __roundtrip_pyobject<list<String>>(obj);
}

inline PyObject* __make_unicode_list(PyObject* obj) {
  auto cpp = capi::Extractor<list<Bytes>>{}(obj);
  if (cpp.hasValue()) {
    return capi::Constructor<list<String>>{}(std::move(*cpp));
  }
  return nullptr;
}

template <typename T>
inline PyObject* __roundtrip_set(PyObject* obj) {
  return __roundtrip_pyobject<set<T>>(obj);
}

inline PyObject* __roundtrip_bytes_set(PyObject* obj) {
  return __roundtrip_pyobject<set<Bytes, folly::F14FastSet<std::string>>>(obj);
}

inline PyObject* __roundtrip_unicode_set(PyObject* obj) {
  return __roundtrip_pyobject<set<String>>(obj);
}

inline PyObject* __make_unicode_set(PyObject* obj) {
  auto cpp = capi::Extractor<set<Bytes>>{}(obj);
  if (cpp.hasValue()) {
    return capi::Constructor<set<String>>{}(std::move(*cpp));
  }
  return nullptr;
}

template <typename K, typename V>
inline PyObject* __roundtrip_map(PyObject* obj) {
  return __roundtrip_pyobject<map<K, V>>(obj);
}

template <typename V>
inline PyObject* __roundtrip_bytes_key_map(PyObject* obj) {
  return __roundtrip_pyobject<map<Bytes, V, folly::F14FastMap<std::string, V>>>(
      obj);
}

template <typename K>
inline PyObject* __roundtrip_bytes_val_map(PyObject* obj) {
  return __roundtrip_pyobject<map<K, Bytes>>(obj);
}

template <typename V>
inline PyObject* __roundtrip_unicode_key_map(PyObject* obj) {
  return __roundtrip_pyobject<map<String, V>>(obj);
}

template <typename K>
inline PyObject* __roundtrip_unicode_val_map(PyObject* obj) {
  return __roundtrip_pyobject<
      map<K, String, folly::F14FastMap<K, std::string>>>(obj);
}

inline PyObject* __make_unicode_key_map(PyObject* obj) {
  auto cpp = capi::Extractor<map<Bytes, int64_t>>{}(obj);
  if (cpp.hasValue()) {
    return capi::Constructor<map<String, int64_t>>{}(std::move(*cpp));
  }
  return nullptr;
}

inline PyObject* __make_unicode_val_map(PyObject* obj) {
  auto cpp = capi::Extractor<map<int64_t, Bytes>>{}(obj);
  if (cpp.hasValue()) {
    return capi::Constructor<map<int64_t, String>>{}(std::move(*cpp));
  }
  return nullptr;
}

} // namespace apache::thrift::python
