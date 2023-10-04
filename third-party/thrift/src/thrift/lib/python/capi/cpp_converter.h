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

#include <folly/python/error.h>
#include <thrift/lib/python/capi/constructor.h>
#include <thrift/lib/python/capi/extractor.h>

namespace apache {
namespace thrift {
namespace python {
namespace capi {

/**
 * These apis allow conversion between C++ thrift objects and their
 * thrift-python equivalents. Their template parameters need to be
 * instantiated with specific thrift types in C++, then the instantiating
 * function can be bound with cython. Be careful with error handling when
 * binding these functions! See the docblock for each.
 *
 * This api is only recommended for power users. For a better devX, please
 * use polyglot toolkit, aka Ligen:
 * https://www.internalfb.com/intern/staticdocs/ligen/
 *
 * See //thrift/test/python_capi/cpp_conversion for an example.
 */

/**
 * Converts a C++ thrift object into corresponding thrift-python object.
 * Returns nullptr if conversion fails. When conversion fails, a python
 * error is raised by `Constructor`. Cython will handle properly because
 * `object` returns are implicitly `except nullptr`.
 */
template <typename CppThrift>
PyObject* cpp_to_python(const CppThrift& cppThrift) {
  return Constructor<CppThrift>{}(cppThrift);
}

/**
 * Converts a thrift-python object into corresponding C++ thrift object.
 * Ensures python error raised if conversion fails.
 * Returns default constructed C++ thrift object if conversion fails,
 * but this should be ignored because error raised.
 * Should be used with `except *` in cython to ensure proper error handling.
 */
template <typename CppThrift>
CppThrift python_to_cpp(PyObject* obj) {
  auto extracted = Extractor<CppThrift>{}(obj);
  if (extracted.hasValue()) {
    return std::move(*extracted);
  }
  if (!PyErr_Occurred()) {
    PyErr_SetString(PyExc_RuntimeError, extracted.error().data());
  }
  return CppThrift{};
}

/**
 * Converts a thrift-python object into corresponding C++ thrift object.
 * Throws std::runtime_error if conversion fails, capturing any current
 * python error (and clearing it) in the process using PyErr_Fetch.
 * Should be used with `except +` in cython to ensure proper error handling.
 */
template <typename CppThrift>
CppThrift python_to_cpp_throws(PyObject* obj) {
  auto extracted = Extractor<CppThrift>{}(obj);
  if (extracted.hasValue()) {
    return std::move(*extracted);
  }
  if (!PyErr_Occurred()) {
    PyErr_SetString(PyExc_RuntimeError, extracted.error().data());
  }
  folly::python::handlePythonError("python_to_cpp Extractor error: ");
}

} // namespace capi
} // namespace python
} // namespace thrift
} // namespace apache
