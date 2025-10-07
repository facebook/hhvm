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

#include <thrift/lib/python/server/RequestContextExtractor.h>

#include <folly/python/import.h>
#include <thrift/lib/python/server/request_context_api.h>

namespace {
bool ensure_module_imported() {
  static folly::python::import_cache_nocapture import(
      import_thrift__python__server_impl__request_context);
  return import();
}
} // namespace

namespace apache::thrift::python {

/* static */
folly::Expected<apache::thrift::Cpp2RequestContext*, std::string_view>
RequestContextExtractor::extractCppRequestContext(PyObject* obj) {
  if (!ensure_module_imported()) {
    std::string_view error = "Failed to load request context Cython module";
    return folly::Unexpected(error);
  }

  apache::thrift::Cpp2RequestContext* ctx_ptr =
      extract_cpp_request_context(obj);

  if (!ctx_ptr) {
    DCHECK(PyErr_Occurred());
    const std::string_view error =
        "Failed to extract C++ request context via extract_cpp_request_context";
    return folly::Unexpected(error);
  }

  return ctx_ptr;
}

/* static */
folly::Expected<apache::thrift::Cpp2ConnContext*, std::string_view>
RequestContextExtractor::extractCppConnectionContext(PyObject* obj) {
  if (!ensure_module_imported()) {
    std::string_view error = "Failed to load request context Cython module";
    return folly::Unexpected(error);
  }

  apache::thrift::Cpp2ConnContext* ctx_ptr =
      extract_cpp_connection_context(obj);

  if (!ctx_ptr) {
    DCHECK(PyErr_Occurred());
    const std::string_view error =
        "Failed to extract C++ connection context via extract_cpp_connection_context";
    return folly::Unexpected(error);
  }

  return ctx_ptr;
}

} // namespace apache::thrift::python
