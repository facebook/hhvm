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

#include <stdexcept>

#include <folly/CppAttributes.h>
#include <folly/Portability.h>
#include <thrift/lib/python/util.h>
#include <thrift/lib/python/util_api.h> // @manual

#if FOLLY_HAS_COROUTINES

namespace thrift {
namespace python {

namespace {

void do_import() {
  if (0 != import_thrift__python__util()) {
    throw std::runtime_error("import_thrift__python__util__cancel failed");
  }
}

} // namespace

void cancelPythonIterator(PyObject* iter) {
  [[maybe_unused]] static bool done = (do_import(), false);
  cancelAsyncGenerator(iter);
}

} // namespace python
} // namespace thrift
#endif
