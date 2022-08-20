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

#include <thrift/conformance/if/gen-cpp2/rpc_types.h>

namespace apache::thrift::conformance {

inline bool equal(
    const ClientTestResult& actual, const ClientTestResult& expected) {
  switch (expected.getType()) {
    case ClientTestResult::Type::requestResponseUndeclaredException:
      if (actual.getType() !=
          ClientTestResult::Type::requestResponseUndeclaredException) {
        return false;
      }
      // return true iff expected exception message is a substring of actual
      // exception message
      return actual.requestResponseUndeclaredException_ref()
                 ->exceptionMessage()
                 ->find(*expected.requestResponseUndeclaredException_ref()
                             ->exceptionMessage()) != std::string::npos;
    default:
      return actual == expected;
  }
}

} // namespace apache::thrift::conformance
