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

#include <optional>

#include <thrift/lib/cpp2/util/gtest/Printer.h>
#include <thrift/lib/cpp2/util/gtest/test/gen-cpp2/Matcher_types.h>

#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>

// portability/GTest must be imported before any other gtest header
#include <gmock/gmock.h>
#include <gtest/gtest-spi.h>

// Bring the function into the thrift type namespace
namespace apache::thrift::test {
using apache::thrift::PrintTo;
}

#if __cpp_concepts

TEST(
    ThriftDefaultPrint,
    Given_CustomProtocolHeaderNotIncluded_When_PrintTo_Then_PrintsTypeAndSuggestionForContent) {
  auto r = apache::thrift::test::SameType();
  EXPECT_THAT(
      testing::PrintToString(r),
      testing::AllOf(
          testing::HasSubstr("apache::thrift::test::SameType"),
          testing::HasSubstr("_types_custom_protocol.h")));
}

#endif
