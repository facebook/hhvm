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

#ifndef _THRIFT_TRANSPORT_TTRANSPORTEXCEPTION_H_
#define _THRIFT_TRANSPORT_TTRANSPORTEXCEPTION_H_ 1

#include <folly/Conv.h>
#include <folly/io/async/AsyncSocketException.h>

#include <thrift/lib/cpp/Thrift.h>

namespace apache {
namespace thrift {
namespace transport {

/**
 * Class to encapsulate all the possible types of transport errors that may
 * occur in various transport systems. This provides a sort of generic
 * wrapper around the shitty UNIX E_ error codes that lets a common code
 * base of error handling to be used for various types of transports, i.e.
 * pipes etc.
 *
 */
class FOLLY_EXPORT TTransportException
    : public apache::thrift::TLibraryException {
 public:
  /**
   * Error codes for the various types of exceptions.
   */
  enum TTransportExceptionType {
    UNKNOWN = 0,
    NOT_OPEN = 1,
    ALREADY_OPEN = 2,
    TIMED_OUT = 3,
    END_OF_FILE = 4,
    INTERRUPTED = 5,
    BAD_ARGS = 6,
    CORRUPTED_DATA = 7,
    INTERNAL_ERROR = 8,
    NOT_SUPPORTED = 9,
    INVALID_STATE = 10,
    INVALID_FRAME_SIZE = 11,
    SSL_ERROR = 12,
    COULD_NOT_BIND = 13,
    NETWORK_ERROR = 15,
    EARLY_DATA_REJECTED = 16,
    STREAMING_CONTRACT_VIOLATION = 17,
    // Remember to update TTransportExceptionTypeSize if you add an entry here
  };

  using TTransportExceptionTypeSize = std::integral_constant<std::size_t, 18>;

  TTransportException()
      : apache::thrift::TLibraryException(),
        type_(UNKNOWN),
        errno_(0),
        options_(0) {}

  explicit TTransportException(TTransportExceptionType type)
      : apache::thrift::TLibraryException(getDefaultMessage(type, "")),
        type_(type),
        errno_(0),
        options_(0) {}

  explicit TTransportException(const std::string& message)
      : apache::thrift::TLibraryException(message),
        type_(UNKNOWN),
        errno_(0),
        options_(0) {}

  TTransportException(TTransportExceptionType type, std::string message)
      : apache::thrift::TLibraryException(
            getDefaultMessage(type, std::move(message))),
        type_(type),
        errno_(0),
        options_(0) {}

  TTransportException(
      TTransportExceptionType type, std::string message, int errno_copy)
      : apache::thrift::TLibraryException(getMessage(
            getDefaultMessage(type, std::move(message)), errno_copy)),
        type_(type),
        errno_(errno_copy),
        options_(0) {}

  explicit TTransportException(const folly::AsyncSocketException& ex)
      : TTransportException(
            TTransportExceptionType(ex.getType()), ex.what(), ex.getErrno()) {}

  ~TTransportException() noexcept override {}

  /**
   * Returns an error code that provides information about the type of error
   * that has occurred.
   *
   * @return Error code
   */
  TTransportExceptionType getType() const noexcept { return type_; }

  enum Options {
    // Channel is still valid (this is a high-level error, not a TCP level
    // error, so we know that framing is still good)
    CHANNEL_IS_VALID = 1 << 0,
  };

  int getOptions() const { return options_; }
  void setOptions(int options) { options_ = options; }

  const char* what() const noexcept override {
    if (message_.empty()) {
      switch (type_) {
        case UNKNOWN:
          return "TTransportException: Unknown transport exception";
        case NOT_OPEN:
          return "TTransportException: Transport not open";
        case ALREADY_OPEN:
          return "TTransportException: Transport already open";
        case TIMED_OUT:
          return "TTransportException: Timed out";
        case END_OF_FILE:
          return "TTransportException: End of file";
        case INTERRUPTED:
          return "TTransportException: Interrupted";
        case BAD_ARGS:
          return "TTransportException: Invalid arguments";
        case CORRUPTED_DATA:
          return "TTransportException: Corrupted Data";
        case INTERNAL_ERROR:
          return "TTransportException: Internal error";
        case NOT_SUPPORTED:
          return "TTransportException: Not supported";
        case INVALID_STATE:
          return "TTransportException: Invalid state";
        case COULD_NOT_BIND:
          return "TTransportException: Could not bind";
        case INVALID_FRAME_SIZE:
          return "TTransportException: Invalid frame size";
        case SSL_ERROR:
          return "TTransportException: SSL error";
        case NETWORK_ERROR:
          return "TTransportException: Network Error";
        case EARLY_DATA_REJECTED:
          return "TTransportException: Early data rejected";
        case STREAMING_CONTRACT_VIOLATION:
          return "TTransportException: Streaming contract violation";
        default:
          return "TTransportException: (Invalid exception type)";
      }
    } else {
      return message_.c_str();
    }
  }

  int getErrno() const { return errno_; }

 protected:
  /** Just like strerror_r but returns a C++ string object. */
  std::string strerror_s(int errno_copy);

  /** Return a message based on the input. */
  static std::string getMessage(std::string&& message, int errno_copy) {
    if (errno_copy != 0) {
      return message + ": " + TOutput::strerror_s(errno_copy);
    } else {
      return std::move(message);
    }
  }

  static std::string getDefaultMessage(
      TTransportExceptionType type, std::string&& message) {
    if (message.empty() &&
        static_cast<size_t>(type) >= TTransportExceptionTypeSize::value) {
      return "TTransportException: (Invalid exception type '" +
          folly::to<std::string>(type) + "')";
    } else {
      return std::move(message);
    }
  }

  /** Error code */
  TTransportExceptionType type_;

  /** A copy of the errno. */
  int errno_;

  int options_;
};

} // namespace transport
} // namespace thrift
} // namespace apache

#endif // #ifndef _THRIFT_TRANSPORT_TTRANSPORTEXCEPTION_H_
