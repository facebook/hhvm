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

#include <gtest/gtest.h>

#include <thrift/lib/cpp2/fast_thrift/thrift/server/util/ResponseMetadata.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

namespace apache::thrift::fast_thrift::thrift {

TEST(ResponseMetadataTest, FillSuccessResponseMetadataPopulatesResponseType) {
  apache::thrift::ResponseRpcMetadata md;
  fillSuccessResponseMetadata(md);

  ASSERT_TRUE(md.payloadMetadata().has_value());
  EXPECT_EQ(
      md.payloadMetadata()->getType(),
      apache::thrift::PayloadMetadata::Type::responseMetadata);
}

TEST(ResponseMetadataTest, FillAppErrorResponseMetadataHasBlame) {
  apache::thrift::ResponseRpcMetadata md;
  fillAppErrorResponseMetadata(
      md,
      "SomeException",
      "error occurred",
      apache::thrift::ErrorBlame::CLIENT);

  ASSERT_TRUE(md.payloadMetadata().has_value());
  EXPECT_EQ(
      md.payloadMetadata()->getType(),
      apache::thrift::PayloadMetadata::Type::exceptionMetadata);

  auto& exBase = md.payloadMetadata()->get_exceptionMetadata();
  ASSERT_TRUE(exBase.name_utf8().has_value());
  EXPECT_EQ(*exBase.name_utf8(), "SomeException");
  ASSERT_TRUE(exBase.what_utf8().has_value());
  EXPECT_EQ(*exBase.what_utf8(), "error occurred");

  ASSERT_TRUE(exBase.metadata().has_value());
  EXPECT_EQ(
      exBase.metadata()->getType(),
      apache::thrift::PayloadExceptionMetadata::Type::appUnknownException);
  auto& appEx = exBase.metadata()->get_appUnknownException();
  ASSERT_TRUE(appEx.errorClassification().has_value());
  ASSERT_TRUE(appEx.errorClassification()->blame().has_value());
  EXPECT_EQ(
      *appEx.errorClassification()->blame(),
      apache::thrift::ErrorBlame::CLIENT);
}

TEST(ResponseMetadataTest, FillDeclaredExceptionMetadataHasCorrectType) {
  apache::thrift::ErrorClassification classification;
  classification.blame() = apache::thrift::ErrorBlame::CLIENT;

  apache::thrift::ResponseRpcMetadata md;
  fillDeclaredExceptionMetadata(
      md, "MyException", "something went wrong", classification);

  ASSERT_TRUE(md.payloadMetadata().has_value());
  EXPECT_EQ(
      md.payloadMetadata()->getType(),
      apache::thrift::PayloadMetadata::Type::exceptionMetadata);

  auto& exBase = md.payloadMetadata()->get_exceptionMetadata();
  ASSERT_TRUE(exBase.name_utf8().has_value());
  EXPECT_EQ(*exBase.name_utf8(), "MyException");
  ASSERT_TRUE(exBase.what_utf8().has_value());
  EXPECT_EQ(*exBase.what_utf8(), "something went wrong");

  ASSERT_TRUE(exBase.metadata().has_value());
  EXPECT_EQ(
      exBase.metadata()->getType(),
      apache::thrift::PayloadExceptionMetadata::Type::declaredException);

  auto& declaredEx = exBase.metadata()->get_declaredException();
  ASSERT_TRUE(declaredEx.errorClassification().has_value());
  ASSERT_TRUE(declaredEx.errorClassification()->blame().has_value());
  EXPECT_EQ(
      *declaredEx.errorClassification()->blame(),
      apache::thrift::ErrorBlame::CLIENT);
}

} // namespace apache::thrift::fast_thrift::thrift
