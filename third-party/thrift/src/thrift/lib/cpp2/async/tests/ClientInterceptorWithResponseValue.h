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
#include <variant>
#include <folly/Unit.h>
#include <thrift/lib/cpp2/async/ClientInterceptor.h>
#include <thrift/lib/cpp2/util/TypeErasedRef.h>

namespace apache::thrift::test {

/**
 * A client interceptor that captures onResponse calls and makes a copy of the
 * data from responseInfo.result.
 *
 * This interceptor can be used to verify that the result field in responseInfo
 * contains the correct type and value for both successful calls and RPC
 * exceptions.
 */
class ClientInterceptorWithResponseValue
    : public apache::thrift::NamedClientInterceptor<folly::Unit> {
 public:
  using RequestState = folly::Unit;
  using ResultVariant =
      std::variant<std::monostate, std::string, int32_t, int64_t>;

  explicit ClientInterceptorWithResponseValue(std::string name)
      : NamedClientInterceptor(std::move(name)) {}

  std::optional<RequestState> onRequest(RequestInfo) override {
    return folly::unit;
  }

  void onResponse(RequestState*, ResponseInfo responseInfo) override {
    // Store the method name
    methodName = responseInfo.methodName;

    // Store whether there was an exception
    hadException = responseInfo.result.hasException();

    // Store the result if available
    if (hadException) {
      // Get the exception type
      const std::type_info& typeInfo = responseInfo.result.exception().type();

      // Store the type name
      resultType = folly::demangle(typeInfo);

      // Store the exception value
      exceptionMessageSeen =
          responseInfo.result.exception().what().toStdString();
    } else if (responseInfo.result.hasValue()) {
      // Get the type info from TypeErasedRef
      const std::type_info& typeInfo = responseInfo.result->type();

      // Store the type name
      resultType = folly::demangle(typeInfo);

      // Check the type and extract the value accordingly
      if (std::holds_alternative<std::string>(resultValue)) {
        resultValue = responseInfo.result->value<std::string>();
        hadResult = true;
      } else if (std::holds_alternative<int32_t>(resultValue)) {
        resultValue = responseInfo.result->value<int32_t>();
        hadResult = true;
      } else if (std::holds_alternative<int64_t>(resultValue)) {
        resultValue = responseInfo.result->value<int64_t>();
        hadResult = true;
      } else {
        // If the type is not one of the supported types, ignore it
        resultValue = std::monostate{};
        hadResult = false;
      }
    } else {
      // No result
      resultValue = std::monostate{};
      resultType = "none";
      hadResult = false;
    }
  }

  // Get the result value as a string (for compatibility with existing tests)
  std::string getResultValueAsString() const {
    if (std::holds_alternative<std::monostate>(resultValue)) {
      return "";
    } else if (std::holds_alternative<std::string>(resultValue)) {
      return std::get<std::string>(resultValue);
    } else if (std::holds_alternative<int32_t>(resultValue)) {
      return std::to_string(std::get<int32_t>(resultValue));
    } else if (std::holds_alternative<int64_t>(resultValue)) {
      return std::to_string(std::get<int64_t>(resultValue));
    }
    return "";
  }

  // Stored information about the response
  std::string methodName;
  bool hadException = false;
  bool hadResult = false;
  std::string resultType;
  std::string exceptionMessageSeen;
  ResultVariant resultValue = std::monostate{};
};

} // namespace apache::thrift::test
