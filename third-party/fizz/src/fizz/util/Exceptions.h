/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/record/Alerts.h>
#include <folly/Optional.h>

namespace fizz {
class FizzException : public std::runtime_error {
 public:
  FizzException(const std::string& msg, folly::Optional<AlertDescription> alert)
      : std::runtime_error(msg), alert_(std::move(alert)) {}

  folly::Optional<AlertDescription> getAlert() const {
    return alert_;
  }

 private:
  folly::Optional<AlertDescription> alert_;
};

class FizzVerificationException : public FizzException {
 public:
  FizzVerificationException(
      const std::string& msg,
      folly::Optional<AlertDescription> alert)
      : FizzException(msg, std::move(alert)) {}
};

// Used to indicate to the Decrypter that extension expansion failed (which is
// a hard error)
class OuterExtensionsError : public std::runtime_error {
 public:
  explicit OuterExtensionsError(const std::string& what)
      : std::runtime_error(what) {}
};
} // namespace fizz
