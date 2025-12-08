/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/record/Types.h>
#include <folly/Optional.h>

namespace fizz {

/*
 * Some build environments mandate using 'default' even though all cases are
 * covered. These macros bypass the restriction, allowing a switch statement
 * without 'default' as long as all cases are covered.
 */
FOLLY_PUSH_WARNING
FOLLY_GNU_ENABLE_ERROR("-Wswitch")
FOLLY_GNU_DISABLE_WARNING("-Wswitch-default")

enum class [[nodiscard]] Status : uint8_t { Fail, Success };

#define FIZZ_RETURN_ON_ERROR(expr) \
  do {                             \
    if ((expr) == Status::Fail) {  \
      return Status::Fail;         \
    }                              \
  } while (0)

/*
 * The Error class is used to record error information when a function returns
 * Status::Fail. It stores an error message, alert description, and the source
 * of the error. We use two types of strings to store error messages in Error:
 * For literal strings, we use const char*.
 * For other cases, we use std::string.
 * This approach minimizes the overhead associated with std::string assignment
 * and destruction. While we considered using std::variant for a simpler and
 * more flexible design, it introduces additional costs. Specifically,
 * std::variant requires iterating through all possible types and destroying
 * the currently active member, which can be inefficient and increase the
 * binary size.
 */
class Error {
 public:
  /**
   * Category represents the "exception type".
   *
   * `Unknown` - A catch all, roughly analagous to any `std::exception` being
   * thrown.
   * `Verifier` - The error was a result of invoking a user supplied
   * certificate verifier.
   * `Fizz` - Analagous to throwing a `FizzException`.
   * These are generally conditions that the TLS implementation _explicitly_
   * checks for (e.g. argument, protocol validations. etc.).
   */
  // clang-format off
  enum class Category : uint8_t {
    Unknown,
    Verifier,
    Fizz
  };
  // clang-format on
  Error() = default;
  // For removing the ambiguity of const char* and std::string
  Status error(
      const std::string& msg,
      folly::Optional<AlertDescription> alertIn,
      Category type = Category::Fizz) = delete;

  // Record the error with a literal string
  Status error(
      const char* msg,
      folly::Optional<AlertDescription> alertIn,
      Category type = Category::Fizz) {
    msgType_ = MessageType::Static;
    staticMsg_ = msg;
    alert_ = std::move(alertIn);
    source_ = type;
    return Status::Fail;
  }
  // Record the error with a dynamically constructed string
  Status error(
      std::string&& msg,
      folly::Optional<AlertDescription> alertIn,
      Category type = Category::Fizz) {
    msgType_ = MessageType::Dynamic;
    dynamicMsg_ = std::move(msg);
    alert_ = std::move(alertIn);
    source_ = type;
    return Status::Fail;
  }
  const char* msg() const {
    switch (msgType_) {
      case MessageType::Static:
        return staticMsg_;
      case MessageType::Dynamic:
        return dynamicMsg_.c_str();
      case MessageType::None:
        return nullptr;
    }
    return nullptr;
  }
  const folly::Optional<AlertDescription>& alert() const {
    return alert_;
  }
  Category errorType() const {
    return source_;
  }

 private:
  enum class MessageType { None, Static, Dynamic };
  MessageType msgType_ = MessageType::None;
  const char* staticMsg_{};
  std::string dynamicMsg_;
  folly::Optional<AlertDescription> alert_ = folly::none;
  Category source_ = Category::Fizz;
};
FOLLY_POP_WARNING
} // namespace fizz
