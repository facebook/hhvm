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

#include <thrift/lib/python/capi/constructor.h>
#include <thrift/lib/python/capi/extractor.h>

namespace apache {
namespace thrift {
namespace python {
namespace capi {

template <typename CppThrift>
PyObject* py3_to_python(std::shared_ptr<CppThrift> cppThrift) {
  if (cppThrift) {
    return Constructor<CppThrift>{}(*cppThrift);
  }
  return nullptr;
}

template <typename CppThrift>
std::shared_ptr<CppThrift> python_to_py3(PyObject* obj) {
  auto extracted = Extractor<CppThrift>{}(obj);
  if (extracted.hasValue()) {
    return std::make_shared<CppThrift>(std::move(*extracted));
  }
  if (!PyErr_Occurred()) {
    PyErr_SetString(PyExc_RuntimeError, extracted.error().data());
  }
  return nullptr;
}

} // namespace capi
} // namespace python
} // namespace thrift
} // namespace apache
