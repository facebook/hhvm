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

#include <thrift/lib/cpp/protocol/TProtocolException.h>

#include <fmt/core.h>

namespace apache::thrift::protocol {

[[noreturn]] void TProtocolException::throwUnionMissingStop() {
  throw TProtocolException(
      TProtocolException::INVALID_DATA,
      "missing stop marker to terminate a union");
}

[[noreturn]] void TProtocolException::throwReportedTypeMismatch() {
  throw TProtocolException(
      TProtocolException::INVALID_DATA,
      "The reported type of thrift element does not match the serialized type");
}

[[noreturn]] void TProtocolException::throwNegativeSize() {
  throw TProtocolException(TProtocolException::NEGATIVE_SIZE);
}

[[noreturn]] void TProtocolException::throwExceededDepthLimit() {
  throw TProtocolException(TProtocolException::DEPTH_LIMIT);
}

[[noreturn]] void TProtocolException::throwExceededSizeLimit() {
  throw TProtocolException(TProtocolException::SIZE_LIMIT);
}

[[noreturn]] void TProtocolException::throwExceededSizeLimit(
    size_t size, size_t limit) {
  throw TProtocolException(
      TProtocolException::SIZE_LIMIT,
      fmt::format("TProtocolException: {} exceeds size limit {}", size, limit));
}

[[noreturn]] void TProtocolException::throwMissingRequiredField(
    folly::StringPiece field, folly::StringPiece type) {
  throw TProtocolException(
      TProtocolException::MISSING_REQUIRED_FIELD,
      fmt::format(
          "Required field '{}' was not found in serialized data! Struct: {}",
          field,
          type));
}

[[noreturn]] void TProtocolException::throwBoolValueOutOfRange(uint8_t value) {
  throw TProtocolException(
      TProtocolException::INVALID_DATA,
      fmt::format(
          "Attempt to interpret value {} as bool, probably the data is "
          "corrupted",
          value));
}

[[noreturn]] void TProtocolException::throwInvalidSkipType(TType type) {
  throw TProtocolException(
      TProtocolException::INVALID_DATA,
      fmt::format(
          "Encountered invalid field/element type ({}) during skipping",
          static_cast<uint8_t>(type)));
}

[[noreturn]] void TProtocolException::throwInvalidFieldData() {
  throw TProtocolException(
      TProtocolException::INVALID_DATA,
      "The field stream contains corrupted data");
}

[[noreturn]] void TProtocolException::throwTruncatedData() {
  throw TProtocolException(
      TProtocolException::INVALID_DATA,
      "Not enough bytes to read the entire message, the data appears to be "
      "truncated");
}
} // namespace apache::thrift::protocol
