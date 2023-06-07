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

#include <folly/Expected.h>
#include <thrift/lib/python/capi/constructor.h>
#include <thrift/lib/python/capi/extractor.h>
#include <thrift/lib/python/capi/types.h>

namespace apache {
namespace thrift {
namespace python {

using capi::Bytes;
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

inline PyObject* __make_unicode(PyObject* bytes) {
  auto cpp = capi::Extractor<Bytes>{}(bytes);
  if (cpp.hasValue()) {
    return capi::Constructor<String>{}(std::move(*cpp));
  }
  return nullptr;
}

} // namespace python
} // namespace thrift
} // namespace apache
