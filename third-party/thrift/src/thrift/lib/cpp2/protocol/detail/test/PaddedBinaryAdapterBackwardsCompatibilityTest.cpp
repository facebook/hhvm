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
#include <folly/io/IOBuf.h>
#include <thrift/lib/cpp2/protocol/Serializer.h>
#include <thrift/lib/cpp2/protocol/detail/test/gen-cpp2/PaddedBinaryAdapter_types.h>

using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::test;

namespace {

TEST(PaddedBinaryAdapterBackwardsCompatibilityTest, OldToNew) {
  std::string testData = "Test data from old client";
  std::string testChecksum = "checksum123";

  OldRequest oldReq;
  oldReq.data_ref() = folly::IOBuf::copyBuffer(testData);
  oldReq.checksum_ref() = testChecksum;
  std::string serialized = BinarySerializer::serialize<std::string>(oldReq);

  NewRequest newReq = BinarySerializer::deserialize<NewRequest>(serialized);
  EXPECT_EQ(newReq.data_ref()->buf->to<std::string>(), testData);
  EXPECT_EQ(*newReq.checksum_ref(), testChecksum);
  EXPECT_EQ(newReq.data_ref()->paddingBytes, 0);
}

TEST(PaddedBinaryAdapterBackwardsCompatibilityTest, NewToOld) {
  std::string testData = "Test data from new client";
  std::string testChecksum = "checksum456";

  NewRequest newReq;
  newReq.data_ref() = PaddedBinaryData(0, folly::IOBuf::copyBuffer(testData));
  newReq.checksum_ref() = testChecksum;
  std::string serialized = BinarySerializer::serialize<std::string>(newReq);

  OldRequest oldReq = BinarySerializer::deserialize<OldRequest>(serialized);
  EXPECT_EQ((*oldReq.data_ref())->to<std::string>(), testData);
  EXPECT_EQ(*oldReq.checksum_ref(), testChecksum);
}

TEST(PaddedBinaryAdapterBackwardsCompatibilityTest, Identical) {
  std::string testData = "Byte equivalence test data";
  std::string testChecksum = "checksum789";

  OldRequest oldReq;
  oldReq.data_ref() = folly::IOBuf::copyBuffer(testData);
  oldReq.checksum_ref() = testChecksum;

  NewRequest newReq;
  newReq.data_ref() = PaddedBinaryData(0, folly::IOBuf::copyBuffer(testData));
  newReq.checksum_ref() = testChecksum;

  std::string oldSerialized = BinarySerializer::serialize<std::string>(oldReq);
  std::string newSerialized = BinarySerializer::serialize<std::string>(newReq);

  EXPECT_EQ(oldSerialized, newSerialized);
}

TEST(PaddedBinaryAdapterBackwardsCompatibilityTest, NoData) {
  std::string testChecksum = "checksumEmpty";

  OldRequest oldReq;
  oldReq.data_ref() = folly::IOBuf::create(0);
  oldReq.checksum_ref() = testChecksum;
  std::string serialized = BinarySerializer::serialize<std::string>(oldReq);

  NewRequest newReq = BinarySerializer::deserialize<NewRequest>(serialized);
  EXPECT_EQ(newReq.data_ref()->buf->computeChainDataLength(), 0);
  EXPECT_EQ(*newReq.checksum_ref(), testChecksum);
  EXPECT_EQ(newReq.data_ref()->paddingBytes, 0);
}

} // namespace
