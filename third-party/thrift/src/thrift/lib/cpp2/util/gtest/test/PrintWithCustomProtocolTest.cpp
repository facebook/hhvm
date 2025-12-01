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

#include <thrift/lib/cpp2/protocol/DebugProtocol.h>
#include <thrift/lib/cpp2/util/gtest/Printer.h>
#include <thrift/lib/cpp2/util/gtest/test/gen-cpp2/Matcher_types.h>
#include <thrift/lib/cpp2/util/gtest/test/gen-cpp2/Matcher_types_custom_protocol.h>

#include <gtest/gtest.h>

TEST(
    ThriftDefaultPrint,
    Given_CustomProtocolHeaderIsIncluded_When_PrintTo_Then_DebugPrint) {
  auto r = apache::thrift::test::SameType();
  std::string debugPrint = apache::thrift::debugStringViaEncode(r);
  EXPECT_EQ(testing::PrintToString(r), debugPrint);
}

TEST(ThriftDefaultPrint, Enum_DebugPrint) {
  EXPECT_EQ(testing::PrintToString(apache::thrift::test::Color::RED), "1");
}
