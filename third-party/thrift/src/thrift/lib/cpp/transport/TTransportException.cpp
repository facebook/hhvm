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

#include <thrift/lib/cpp/transport/TTransportException.h>

#include <fmt/core.h>

#include <folly/String.h>

namespace apache::thrift::transport {

std::string TTransportException::getMessage(
    std::string&& message, int errno_copy) {
  if (errno_copy != 0) {
    return message + ": " + folly::errnoStr(errno_copy);
  } else {
    return std::move(message);
  }
}

std::string TTransportException::getDefaultMessage(
    TTransportExceptionType type, std::string&& message) {
  if (message.empty() &&
      static_cast<size_t>(type) >= TTransportExceptionTypeSize::value) {
    return fmt::format(
        "TTransportException: (Invalid exception type '{}')",
        folly::to_underlying(type));
  } else {
    return std::move(message);
  }
}

} // namespace apache::thrift::transport
