/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/util/Status.h>

namespace fizz {

FOLLY_PUSH_WARNING
FOLLY_GNU_ENABLE_ERROR("-Wswitch")
FOLLY_GNU_DISABLE_WARNING("-Wswitch-default")

void Error::throwException() const {
  const char* msg =
      msgType_ == Error::MessageType::Static ? staticMsg_ : dynamicMsg_.c_str();
  switch (source_) {
    case Error::Category::Fizz:
      throw FizzException(msg, alert_);
    case Error::Category::Verifier:
      throw FizzVerificationException(msg, alert_);
    case Error::Category::OuterExtensions:
      throw OuterExtensionsError(msg);
    case Error::Category::StdRuntime:
      throw std::runtime_error(msg);
    case Error::Category::StdOverFlow:
      throw std::overflow_error(msg);
    case Error::Category::StdLogic:
      throw std::logic_error(msg);
    case Error::Category::StdOutOfRange:
      throw std::out_of_range(msg);
  }
}
folly::exception_wrapper Error::toException() const {
  switch (source_) {
    case Error::Category::Fizz:
      return folly::make_exception_wrapper<FizzException>(msg(), alert());
    case Error::Category::Verifier:
      return folly::make_exception_wrapper<FizzVerificationException>(
          msg(), alert());
    case Error::Category::OuterExtensions:
      return folly::make_exception_wrapper<OuterExtensionsError>(msg());
    case Error::Category::StdRuntime:
      return folly::make_exception_wrapper<std::runtime_error>(msg());
    case Error::Category::StdOverFlow:
      return folly::make_exception_wrapper<std::overflow_error>(msg());
    case Error::Category::StdLogic:
      return folly::make_exception_wrapper<std::logic_error>(msg());
    case Error::Category::StdOutOfRange:
      return folly::make_exception_wrapper<std::out_of_range>(msg());
  }
  return {};
}
FOLLY_POP_WARNING
} // namespace fizz
