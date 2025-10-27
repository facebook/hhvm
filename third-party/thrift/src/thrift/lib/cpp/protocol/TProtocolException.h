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

#ifndef _THRIFT_PROTOCOL_TPROTOCOLEXCEPTION_H_
#define _THRIFT_PROTOCOL_TPROTOCOLEXCEPTION_H_ 1

#include <thrift/lib/cpp/Thrift.h>
#include <thrift/lib/cpp/protocol/TType.h>

#include <string>

namespace apache::thrift::protocol {

/**
 * Class to encapsulate all the possible types of protocol errors that may
 * occur in various protocol systems. This provides a sort of generic
 * wrapper around the shitty UNIX E_ error codes that lets a common code
 * base of error handling to be used for various types of protocols, i.e.
 * pipes etc.
 *
 */
class FOLLY_EXPORT TProtocolException
    : public apache::thrift::TLibraryException {
 public:
  /**
   * Error codes for the various types of exceptions.
   */
  enum TProtocolExceptionType {
    UNKNOWN = 0,
    INVALID_DATA = 1,
    NEGATIVE_SIZE = 2,
    SIZE_LIMIT = 3,
    BAD_VERSION = 4,
    NOT_IMPLEMENTED = 5,
    MISSING_REQUIRED_FIELD = 6,
    CHECKSUM_MISMATCH = 7,
    DEPTH_LIMIT = 8,
  };

  TProtocolException() : apache::thrift::TLibraryException(), type_(UNKNOWN) {}

  explicit TProtocolException(TProtocolExceptionType type)
      : apache::thrift::TLibraryException(), type_(type) {}

  explicit TProtocolException(const std::string& message)
      : apache::thrift::TLibraryException(message), type_(UNKNOWN) {}

  TProtocolException(TProtocolExceptionType type, const std::string& message)
      : apache::thrift::TLibraryException(message), type_(type) {}

  ~TProtocolException() noexcept override {}

  /**
   * Returns an error code that provides information about the type of error
   * that has occurred.
   *
   * @return Error code
   */
  TProtocolExceptionType getType() const { return type_; }

  const char* what() const noexcept override {
    if (message_.empty()) {
      switch (type_) {
        case UNKNOWN:
          return "TProtocolException: Unknown protocol exception";
        case INVALID_DATA:
          return "TProtocolException: Invalid data";
        case NEGATIVE_SIZE:
          return "TProtocolException: Negative size";
        case SIZE_LIMIT:
          return "TProtocolException: Exceeded size limit";
        case BAD_VERSION:
          return "TProtocolException: Invalid version";
        case NOT_IMPLEMENTED:
          return "TProtocolException: Not implemented";
        case MISSING_REQUIRED_FIELD:
          return "TProtocolException: Missing required field";
        case CHECKSUM_MISMATCH:
          return "TProtocolException: Checksum mismatch";
        case DEPTH_LIMIT:
          return "TProtocolException: Exceeded depth limit";
        default:
          return "TProtocolException: (Invalid exception type)";
      }
    } else {
      return message_.c_str();
    }
  }

  [[noreturn]] static void throwUnionMissingStop();
  [[noreturn]] static void throwReportedTypeMismatch();
  [[noreturn]] static void throwNegativeSize();
  [[deprecated("Use override with size and limit.")]] //
  [[noreturn]] static void throwExceededSizeLimit();
  [[noreturn]] static void throwExceededSizeLimit(size_t size, size_t limit);
  [[noreturn]] static void throwExceededDepthLimit();
  [[noreturn]] static void throwMissingRequiredField(
      folly::StringPiece field, folly::StringPiece type);
  [[noreturn]] static void throwBoolValueOutOfRange(uint8_t value);
  [[noreturn]] static void throwInvalidSkipType(TType type);
  [[noreturn]] static void throwInvalidFieldData();
  [[noreturn]] static void throwTruncatedData();

 protected:
  /**
   * Error code
   */
  TProtocolExceptionType type_;
};

} // namespace apache::thrift::protocol

#endif // #ifndef _THRIFT_PROTOCOL_TPROTOCOLEXCEPTION_H_
