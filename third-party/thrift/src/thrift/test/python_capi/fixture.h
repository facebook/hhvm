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

#include <folly/python/iobuf.h>
#include <thrift/lib/python/capi/constructor.h>
#include <thrift/lib/python/capi/extractor.h>
#include <thrift/lib/python/capi/iobuf.h>

#include <thrift/test/python_capi/gen-python-capi/containers/thrift_types_capi.h>
#include <thrift/test/python_capi/gen-python-capi/module/thrift_types_capi.h>
#include <thrift/test/python_capi/gen-python-capi/serialized_dep/thrift_types_capi.h>

namespace apache::thrift::test {

template <typename T, typename Py3Namespace>
PyObject* __shim__roundtrip(PyObject* obj) {
  auto cpp = python::capi::Extractor<
      python::capi::PythonNamespaced<T, Py3Namespace>>{}(obj);
  if (cpp.hasValue()) {
    return python::capi::Constructor<
        python::capi::PythonNamespaced<T, Py3Namespace>>{}(*cpp);
  }
  return nullptr;
}

template <typename T, typename Py3Namespace>
int __shim__typeCheck(PyObject* obj) {
  return python::capi::Extractor<
             python::capi::PythonNamespaced<T, Py3Namespace>>{}
      .typeCheck(obj);
}

template <typename T, typename Py3Namespace>
PyObject* __shim__marshal_to_iobuf(PyObject* obj) {
  auto cpp = python::capi::Extractor<
      python::capi::PythonNamespaced<T, Py3Namespace>>{}(obj);
  if (cpp.hasValue()) {
    return folly::python::make_python_iobuf(
        python::capi::detail::serialize_to_iobuf(*cpp));
  }
  return nullptr;
}

template <typename T>
PyObject* __shim__serialize_to_iobuf(PyObject* obj) {
  auto cpp = python::capi::detail::deserialize_iobuf<T>(
      folly::python::iobuf_ptr_from_python_iobuf(obj));
  return folly::python::make_python_iobuf(
      python::capi::detail::serialize_to_iobuf(cpp));
}

PyObject* __shim__gen_SerializedStruct(int64_t len) {
  ::thrift::test::python_capi::SerializedStruct s;
  s.s() = std::string(len, '1');
  return python::capi::Constructor<python::capi::PythonNamespaced<
      ::thrift::test::python_capi::SerializedStruct,
      thrift__test__python_capi__serialized_dep::NamespaceTag>>{}(s);
}

std::string serializeTemplateLists() noexcept;
PyObject* constructTemplateLists() noexcept;
std::string serializeTemplateSets() noexcept;
PyObject* constructTemplateSets() noexcept;
std::string serializeTemplateMaps() noexcept;
PyObject* constructTemplateMaps() noexcept;

template <typename T, typename Py3Namespace>
std::string extractAndSerialize(PyObject* obj) {
  auto cpp = python::capi::Extractor<
      python::capi::PythonNamespaced<T, Py3Namespace>>{}(obj);
  if (!cpp.hasValue()) {
    return "";
  }
  auto iobuf_ptr = python::capi::detail::serialize_to_iobuf(*cpp);
  std::string ret;
  iobuf_ptr->appendTo(ret);
  return ret;
}

} // namespace apache::thrift::test
