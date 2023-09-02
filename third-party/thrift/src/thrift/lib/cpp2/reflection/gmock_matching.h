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

#include <folly/portability/GMock.h>

#include <thrift/lib/cpp2/debug_thrift_data_difference/debug.h>

// This is a gmock Matcher for thrift object equality. On mismatch this uses the
// facilities from debug.h to provide a more-usable description of the
// difference between the two objects than the gmock `Eq` matcher.
//
// Example use:
//
//   using apache::thrift::ThriftEq;
//   EXPECT_CALL(mock_object, method(ThriftEq(expectedObject)));

namespace apache {
namespace thrift {

template <typename T>
struct ThriftEqMatcher : testing::MatcherInterface<T> {
  constexpr explicit ThriftEqMatcher(T const* expected) : expected_(expected) {}

// TODO(T69712535): Remove old googletest code
#if defined(MOCK_METHOD)
  using GMockT = T;
#else
  using GMockT = T const&;
#endif
  bool MatchAndExplain(
      GMockT actual, testing::MatchResultListener* listener) const override {
    return facebook::thrift::debug_thrift_data_difference(
        *expected_,
        actual,
        facebook::thrift::make_debug_output_callback(
            *listener, "expected", "actual"));
  }

  void DescribeTo(std::ostream* os) const override {
    *os << "matches expected value";
  }

  void DescribeNegationTo(std::ostream* os) const override {
    *os << "does not match expected value";
  }

 private:
  T const* expected_;
};

template <typename T>
testing::Matcher<T> ThriftEq(T const& expected) {
  return testing::MakeMatcher(new ThriftEqMatcher<T>(&expected));
}

} // namespace thrift
} // namespace apache
