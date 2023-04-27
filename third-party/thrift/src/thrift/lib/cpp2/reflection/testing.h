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
#include <folly/portability/GTest.h>

#include <thrift/lib/cpp2/reflection/debug.h>

namespace apache {
namespace thrift {

template <class T>
::testing::AssertionResult thriftEqualHelper(
    const char* left, const char* right, const T& a, const T& b) {
  ::testing::AssertionResult result(false);
  if (debug_equals(a, b, make_debug_output_callback(result, left, right))) {
    return ::testing::AssertionResult(true);
  } else {
    return result;
  }
}

template <class TC, class T>
::testing::AssertionResult thriftEqualHelperTC(
    const char*,
    const char* left,
    const char* right,
    const TC&,
    const T& a,
    const T& b) {
  ::testing::AssertionResult result(false);
  std::string path = "$";
  if (detail::debug_equals<TC>(
          path, a, b, make_debug_output_callback(result, left, right))) {
    return ::testing::AssertionResult(true);
  } else {
    return result;
  }
}

} // namespace thrift
} // namespace apache

#define EXPECT_THRIFT_EQ(a, b) \
  EXPECT_PRED_FORMAT2(::apache::thrift::thriftEqualHelper, a, b)

#define ASSERT_THRIFT_EQ(a, b) \
  ASSERT_PRED_FORMAT2(::apache::thrift::thriftEqualHelper, a, b)

#define EXPECT_THRIFT_TC_EQ(tc, a, b) \
  EXPECT_PRED_FORMAT3(::apache::thrift::thriftEqualHelperTC, tc, a, b)

#define ASSERT_THRIFT_TC_EQ(tc, a, b) \
  ASSERT_PRED_FORMAT3(::apache::thrift::thriftEqualHelperTC, tc, a, b)
