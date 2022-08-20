/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/portability/GMock.h>

#include <folly/Optional.h>
#include <proxygen/lib/http/HTTPHeaders.h>

namespace proxygen {

/**
 * A gmock matcher for HTTPHeader objects.
 *
 * This can be used for check for the existence of a header, or for a header
 * with a specific value. To check for multiple headers, or more complicated
 * predicate logic, use ::testing::AllOf() and friends from gmock.
 *
 * Use the factory functions below in tests rather than using this class
 * directly.
 */
class HasHTTPHeaderMatcherImpl
    : public ::testing::MatcherInterface<const HTTPHeaders&> {
 public:
  explicit HasHTTPHeaderMatcherImpl(std::string name) : name_(name) {
  }

  HasHTTPHeaderMatcherImpl(std::string name, std::string value)
      : name_(name), value_(value) {
  }

  bool MatchAndExplain(
      const HTTPHeaders& headers,
      ::testing::MatchResultListener* /*listener*/) const override {
    bool matches = false;
    headers.forEach([&](const std::string& name, const std::string& value) {
      if (name_ != name) {
        return;
      }

      matches = matches || !value_ || *value_ == value;
    });

    return matches;
  }

  void DescribeTo(::std::ostream* os) const override {
    if (!value_) {
      *os << "has the '" << name_ << "' header";
    } else {
      *os << "has the header '" << name_ << "' equal to '" << *value_ << "'";
    }
  }

  void DescribeNegationTo(::std::ostream* os) const override {
    if (!value_) {
      *os << "does not have the '" << name_ << "' header";
    } else {
      *os << "does not have the '" << name_ << "' header "
          << "equal to '" << *value_ << "'";
    }
  }

 private:
  const std::string name_;
  const folly::Optional<std::string> value_;
};

// Factory function for matching an HTTPHeaders that contains the given header
inline ::testing::Matcher<const HTTPHeaders&> HasHTTPHeader(std::string name) {
  return ::testing::MakeMatcher(new HasHTTPHeaderMatcherImpl(name));
}

// Factory function for matching an HTTPHeaders that contains the given header
// and has it set to the specified value
inline ::testing::Matcher<const HTTPHeaders&> HasHTTPHeader(std::string name,
                                                            std::string value) {
  return ::testing::MakeMatcher(new HasHTTPHeaderMatcherImpl(name, value));
}

} // namespace proxygen
