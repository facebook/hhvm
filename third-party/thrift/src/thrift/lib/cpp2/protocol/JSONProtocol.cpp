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

#include <thrift/lib/cpp2/protocol/JSONProtocol.h>

#include <folly/Conv.h>

namespace apache {
namespace thrift {

namespace detail {
namespace json {

[[noreturn]] void throwNegativeSize(const int64_t size) {
  throw TProtocolException(
      TProtocolException::NEGATIVE_SIZE, folly::to<std::string>(size, " < 0"));
}

[[noreturn]] void throwExceededSizeLimit(
    const int64_t size, const int64_t sizeMax) {
  throw TProtocolException(
      TProtocolException::SIZE_LIMIT,
      folly::to<std::string>(size, " is too large (", sizeMax, ")"));
}

[[noreturn]] void throwUnrecognizedType() {
  throw TProtocolException(
      TProtocolException::NOT_IMPLEMENTED, "Unrecognized type");
}
} // namespace json
} // namespace detail

[[noreturn]] void JSONProtocolReader::throwUnrecognizableAsBoolean(
    const int8_t byte) {
  throw TProtocolException(
      TProtocolException::INVALID_DATA,
      folly::to<std::string>(byte, " is not a valid bool"));
}
} // namespace thrift
} // namespace apache
