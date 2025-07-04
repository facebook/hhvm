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
  auto stringContains = [](std::string_view a, std::string_view b) {
    // return true iff b is a substring of a
    return a.find(b) != std::string::npos;
  };
  switch (expected.getType()) {
    case ClientTestResult::Type::requestResponseUndeclaredException:
      if (actual.getType() !=
          ClientTestResult::Type::requestResponseUndeclaredException) {
        return false;
      }
      return stringContains(
          *actual.requestResponseUndeclaredException()->exceptionMessage(),
          *expected.requestResponseUndeclaredException()->exceptionMessage());
    case ClientTestResult::Type::streamUndeclaredException:
      if (actual.getType() !=
          ClientTestResult::Type::streamUndeclaredException) {
        return false;
      }
      return stringContains(
          *actual.streamUndeclaredException()->exceptionMessage(),
          *expected.streamUndeclaredException()->exceptionMessage());
    case ClientTestResult::Type::streamInitialUndeclaredException:
      if (actual.getType() !=
          ClientTestResult::Type::streamInitialUndeclaredException) {
        return false;
      }
      return stringContains(
          *actual.streamInitialUndeclaredException()->exceptionMessage(),
          *expected.streamInitialUndeclaredException()->exceptionMessage());
    default:
      return actual == expected;
  }
}

inline bool equal(
    const ServerTestResult& actual, const ServerTestResult& expected) {
  if (actual.getType() != expected.getType()) {
    return false;
  }
  switch (expected.getType()) {
    case ServerTestResult::Type::sinkUndeclaredException: {
      auto& actualMsg = *actual.sinkUndeclaredException()->exceptionMessage();
      auto& expectedMsg =
          *expected.sinkUndeclaredException()->exceptionMessage();
      return actualMsg.find(expectedMsg) != std::string::npos;
    }
    default:
      return actual == expected;
  }
}
} // namespace apache::thrift::conformance
