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

#include <thrift/conformance/cpp2/ThriftTypeInfo.h>

#include <folly/portability/GTest.h>
#include <thrift/test/gen-cpp2/AnyTest1_types.h>
#include <thrift/test/gen-cpp2/AnyTest2_types.h>
#include <thrift/test/gen-cpp2/AnyTest3_types.h>

namespace apache::thrift::test {
namespace {

TEST(ThrfitTypeInfoTest, GetGeneratedThriftTypeInfo) {
  EXPECT_EQ(
      conformance::getGeneratedThriftTypeInfo<AnyTestStruct>().uri(),
      "facebook.com/thrift/test/AnyTestStruct");
  EXPECT_EQ(
      conformance::getGeneratedThriftTypeInfo<AnyTestUnion>().uri(),
      "facebook.com/thrift/test/AnyTestUnion");
  // TypeInfo is still available, even though any is not enabled.
  EXPECT_EQ(
      conformance::getGeneratedThriftTypeInfo<AnyTestMissingAnyOption>().uri(),
      "facebook.com/thrift/test/AnyTestMissingAnyOption");
}

} // namespace
} // namespace apache::thrift::test
