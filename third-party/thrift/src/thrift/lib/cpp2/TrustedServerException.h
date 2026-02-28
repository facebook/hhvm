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

#include <exception>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

#include <folly/Portability.h>

#include <thrift/lib/cpp/TApplicationException.h>

namespace apache::thrift {

/**
 * A class that can throw a "trusted" TApplicationException, in that its
 * TApplicationExceptionType will be preserved in the response.
 *
 * Exceptions thrown by user code are "untrusted" by default. If a service
 * handler throws a TApplicationException, only its what() is preserved and its
 * type is discarded. This is because a rogue handler can easily invalidate
 * invariants assumed by the client on which error codes to expect.
 *
 * This class offers a backdoor that is intended to be only be used internally
 * in Thrift, primarily by generated handler code.
 *
 * NOTE: To propagate the exception type over Rocket, the error code must be
 * mapped to a ResponseRpcErrorCode on the server side, which must be mapped
 * back to TApplicationExceptionType on the client side.
 */
class FOLLY_EXPORT TrustedServerException final : public std::runtime_error {
 private:
  template <typename Str>
  explicit TrustedServerException(
      TApplicationException::TApplicationExceptionType type,
      const Str& message,
      std::string_view errorCode)
      : std::runtime_error{message},
        type_(type),
        errorCode_(std::move(errorCode)) {}

 public:
  static TrustedServerException requestParsingError(const char* message);
  static TrustedServerException unimplementedMethodError(const char* message);
  static TrustedServerException appOverloadError(const std::string& message);
  static TrustedServerException badInteractionState(const std::string& message);

  TApplicationException toApplicationException() const {
    return TApplicationException(type_, std::string(what()));
  }

  std::string_view errorCode() const noexcept { return errorCode_; }

 private:
  TApplicationException::TApplicationExceptionType type_;
  std::string_view errorCode_;
};

} // namespace apache::thrift
