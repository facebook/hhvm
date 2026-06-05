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

#include <string>

#include <folly/ExceptionWrapper.h>

#include <thrift/lib/cpp2/TrustedServerException.h>

namespace apache::thrift::python {

// Build a folly::exception_wrapper holding an appOverload
// TrustedServerException. The wrapper retains the concrete
// TrustedServerException type so the server's handler-error path preserves the
// ApplicationExceptionType (LOADSHEDDING) to the client.
//
// This lives in C++ because TrustedServerException has only private
// constructors (reachable through the static factories) and is not
// default-constructible, which Cython cannot express as a by-value temporary.
inline folly::exception_wrapper makeAppOverloadExceptionWrapper(
    const std::string& message) {
  return folly::exception_wrapper(
      TrustedServerException::appOverloadError(message));
}

} // namespace apache::thrift::python
