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

#include <thrift/lib/python/std_libcpp.h>

#include <folly/python/error.h>

namespace apache::thrift::python {

std::string_view extractStringViewFromBytes(PyObject* bytes) {
  Py_ssize_t size = 0;
  char* buffer = nullptr;
  if (PyBytes_AsStringAndSize(bytes, &buffer, &size) < 0) {
    folly::python::handlePythonError("extractStringViewFromBytes");
  }
  return std::string_view(buffer, size);
}

} // namespace apache::thrift::python
