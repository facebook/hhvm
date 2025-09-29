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

#include <gtest/gtest.h>
#include <thrift/conformance/cpp2/Testing.h>

namespace apache::thrift::conformance {
namespace {

TEST(ThriftTypeInfoTest, ValidateThriftTypeInfo) {
  const auto bad = "foo.com:42/my/typeInfo";
  const auto good1 = "foo.com/my/typeInfo";
  const auto good2 = "foo.com/my/other_typeInfo";
  ThriftTypeInfo typeInfo;
  EXPECT_THROW(validateThriftTypeInfo(typeInfo), std::invalid_argument);
  typeInfo.uri() = good1;
  validateThriftTypeInfo(typeInfo);
  typeInfo.altUris()->emplace(good2);
  validateThriftTypeInfo(typeInfo);
  typeInfo.typeHashBytes() = kMinTypeHashBytes;
  validateThriftTypeInfo(typeInfo);
  typeInfo.typeHashBytes() = 32;
  validateThriftTypeInfo(typeInfo);
  typeInfo.typeHashBytes() = 64;
  validateThriftTypeInfo(typeInfo);

  {
    ThriftTypeInfo badType(typeInfo);
    badType.uri() = bad;
    EXPECT_THROW(validateThriftTypeInfo(badType), std::invalid_argument);
  }

  {
    ThriftTypeInfo badType(typeInfo);
    badType.altUris()->emplace(good1); // Duplicate uri.
    EXPECT_THROW(validateThriftTypeInfo(badType), std::invalid_argument);
  }
  {
    ThriftTypeInfo badType(typeInfo);
    badType.altUris()->emplace(bad);
    EXPECT_THROW(validateThriftTypeInfo(badType), std::invalid_argument);
  }

  {
    ThriftTypeInfo badType(typeInfo);
    badType.typeHashBytes() = 1;
    EXPECT_THROW(validateThriftTypeInfo(badType), std::invalid_argument);
  }
}

TEST(ThriftTypeInfoTest, GetGeneratedThriftTypeInfo) {
  // You can get type info about ThriftTypeInfo.
  EXPECT_EQ(
      getGeneratedThriftTypeInfo<ThriftTypeInfo>().uri(),
      "facebook.com/thrift/ThriftTypeInfo");
}

} // namespace
} // namespace apache::thrift::conformance
