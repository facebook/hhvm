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

class PaddedBinaryAdapterTest : public testing::Test {};

TEST_F(PaddedBinaryAdapterTest, SerializedSize) {
  std::string testData = "Test data for single padded field";
  PaddedBinaryData paddedData(4, folly::IOBuf::copyBuffer(testData));
  PaddedBinaryData nullData(4, nullptr);
  PaddedBinaryData emptyData(4, folly::IOBuf::create(0));
  PaddedBinaryData zeroPaddingData(0, folly::IOBuf::copyBuffer(testData));

  BinaryProtocolWriter binaryWriter;
  uint32_t size = PaddedBinaryAdapter::serializedSize<false, std::string>(
      binaryWriter, paddedData);
  EXPECT_EQ(size, 4);
  size = PaddedBinaryAdapter::serializedSize<true, std::string>(
      binaryWriter, paddedData);
  EXPECT_EQ(size, 4);
  size = PaddedBinaryAdapter::serializedSize<false, std::string>(
      binaryWriter, nullData);
  EXPECT_EQ(size, 0);
  size = PaddedBinaryAdapter::serializedSize<true, std::string>(
      binaryWriter, nullData);
  EXPECT_EQ(size, 0);
  size = PaddedBinaryAdapter::serializedSize<false, std::string>(
      binaryWriter, emptyData);
  EXPECT_EQ(size, 4);
  size = PaddedBinaryAdapter::serializedSize<true, std::string>(
      binaryWriter, emptyData);
  EXPECT_EQ(size, 4);
  size = PaddedBinaryAdapter::serializedSize<false, std::string>(
      binaryWriter, zeroPaddingData);
  EXPECT_EQ(size, 37);
  size = PaddedBinaryAdapter::serializedSize<true, std::string>(
      binaryWriter, zeroPaddingData);
  EXPECT_EQ(size, 37);

  CompactProtocolWriter compactWriter;
  size = PaddedBinaryAdapter::serializedSize<false, std::string>(
      compactWriter, paddedData);
  EXPECT_EQ(size, 38);
  size = PaddedBinaryAdapter::serializedSize<true, std::string>(
      compactWriter, paddedData);
  EXPECT_EQ(size, 38);
}

TEST_F(PaddedBinaryAdapterTest, SinglePaddedField) {
  std::string testData = "Test data for single padded field";
  SingleAlignedBinary original;
  original.data() = PaddedBinaryData(4, folly::IOBuf::copyBuffer(testData));

  std::string serialized = BinarySerializer::serialize<std::string>(original);
  SingleAlignedBinary deserialized =
      BinarySerializer::deserialize<SingleAlignedBinary>(serialized);

  EXPECT_EQ(deserialized.data()->paddingBytes, 4);
  EXPECT_EQ(
      deserialized.data()->buf->computeChainDataLength(), testData.size());
  std::string deserializedData = deserialized.data()->buf->to<std::string>();
  EXPECT_EQ(deserializedData, testData);
}

TEST_F(PaddedBinaryAdapterTest, MultiplePaddedFields) {
  MultipleAlignedBinary original;
  original.data()->emplace_back(8, folly::IOBuf::copyBuffer("First field"));
  original.data()->emplace_back(32, folly::IOBuf::copyBuffer("Second field"));
  original.data()->emplace_back(64, folly::IOBuf::copyBuffer("Third field"));

  std::string serialized = BinarySerializer::serialize<std::string>(original);
  MultipleAlignedBinary deserialized =
      BinarySerializer::deserialize<MultipleAlignedBinary>(serialized);

  ASSERT_EQ(deserialized.data()->size(), 3);

  EXPECT_EQ(deserialized.data()->at(0).paddingBytes, 8);
  std::string data1Str = deserialized.data()->at(0).buf->to<std::string>();
  EXPECT_EQ(data1Str, "First field");

  EXPECT_EQ(deserialized.data()->at(1).paddingBytes, 32);
  std::string data2Str = deserialized.data()->at(1).buf->to<std::string>();
  EXPECT_EQ(data2Str, "Second field");

  EXPECT_EQ(deserialized.data()->at(2).paddingBytes, 64);
  std::string data3Str = deserialized.data()->at(2).buf->to<std::string>();
  EXPECT_EQ(data3Str, "Third field");
}

TEST_F(PaddedBinaryAdapterTest, LargePadding) {
  std::string testData = "Test data for large padding";
  SingleAlignedBinary original;
  original.data() = PaddedBinaryData(4096, folly::IOBuf::copyBuffer(testData));

  std::string serialized = BinarySerializer::serialize<std::string>(original);
  SingleAlignedBinary deserialized =
      BinarySerializer::deserialize<SingleAlignedBinary>(serialized);

  EXPECT_EQ(deserialized.data()->paddingBytes, 4096);
  EXPECT_EQ(
      deserialized.data()->buf->computeChainDataLength(), testData.size());
  std::string deserializedData = deserialized.data()->buf->to<std::string>();
  EXPECT_EQ(deserializedData, testData);
}

TEST_F(PaddedBinaryAdapterTest, ArbitraryPadding) {
  std::string testData = "Test data for arbitrary padding";
  SingleAlignedBinary original;
  original.data() = PaddedBinaryData(133, folly::IOBuf::copyBuffer(testData));

  std::string serialized = BinarySerializer::serialize<std::string>(original);
  SingleAlignedBinary deserialized =
      BinarySerializer::deserialize<SingleAlignedBinary>(serialized);

  EXPECT_EQ(deserialized.data()->paddingBytes, 133);
  EXPECT_EQ(
      deserialized.data()->buf->computeChainDataLength(), testData.size());
  std::string deserializedData = deserialized.data()->buf->to<std::string>();
  EXPECT_EQ(deserializedData, testData);
}

TEST_F(PaddedBinaryAdapterTest, NullData) {
  SingleAlignedBinary original;
  original.data() = PaddedBinaryData(16, nullptr);

  std::string serialized = BinarySerializer::serialize<std::string>(original);
  ASSERT_EQ(serialized.size(), /* struct */ 4 + /* zero for data */ 4);
  SingleAlignedBinary deserialized =
      BinarySerializer::deserialize<SingleAlignedBinary>(serialized);

  EXPECT_EQ(deserialized.data()->paddingBytes, 0);
  EXPECT_EQ(deserialized.data()->buf->computeChainDataLength(), 0);
}

TEST_F(PaddedBinaryAdapterTest, EmptyData) {
  SingleAlignedBinary original;
  original.data() = PaddedBinaryData(16, nullptr);

  std::string serialized = BinarySerializer::serialize<std::string>(original);
  ASSERT_EQ(serialized.size(), /* struct */ 4 + /* zero for data */ 4);
  SingleAlignedBinary deserialized =
      BinarySerializer::deserialize<SingleAlignedBinary>(serialized);

  EXPECT_EQ(deserialized.data()->paddingBytes, 0);
  EXPECT_EQ(deserialized.data()->buf->computeChainDataLength(), 0);
}

TEST_F(PaddedBinaryAdapterTest, ZeroPadding) {
  SingleAlignedBinary original;
  std::string testData = "Test data for zero padding";
  original.data() = PaddedBinaryData(0, folly::IOBuf::copyBuffer(testData));

  std::string serialized = BinarySerializer::serialize<std::string>(original);

  // We should just decode the data as non-padded.
  SingleAlignedBinary deserialized =
      BinarySerializer::deserialize<SingleAlignedBinary>(serialized);

  EXPECT_EQ(deserialized.data()->paddingBytes, 0);
  EXPECT_EQ(
      deserialized.data()->buf->computeChainDataLength(), testData.size());
  std::string deserializedData = deserialized.data()->buf->to<std::string>();
  EXPECT_EQ(deserializedData, testData);
}

TEST_F(PaddedBinaryAdapterTest, SerializeNonPaddedDeserializePadded) {
  BinaryProtocolWriter writer;
  folly::IOBufQueue queue;
  writer.setOutput(&queue);

  std::string testData = "Test data for zero padding";
  writer.writeBinary(folly::IOBuf::copyBuffer(testData));
  std::unique_ptr<folly::IOBuf> serialized = queue.move();

  BinaryProtocolReader reader;
  reader.setInput(serialized.get());
  PaddedBinaryData paddedData;

  // We should just decode the data as non-padded.
  PaddedBinaryAdapter::decode<apache::thrift::type::binary_t>(
      reader, paddedData);

  EXPECT_EQ(paddedData.paddingBytes, 0);
  EXPECT_EQ(paddedData.buf->computeChainDataLength(), testData.size());
  std::string deserializedData = paddedData.buf->to<std::string>();
  EXPECT_EQ(deserializedData, testData);
}

TEST_F(PaddedBinaryAdapterTest, SerializePaddedDeserializeNonPadded) {
  BinaryProtocolWriter writer;
  folly::IOBufQueue queue;
  writer.setOutput(&queue);
  std::string testData = "Test data for zero padding";
  PaddedBinaryData paddedData(16, folly::IOBuf::copyBuffer(testData));
  uint32_t encodedSize =
      PaddedBinaryAdapter::encode<apache::thrift::type::binary_t>(
          writer, paddedData);
  ASSERT_EQ(
      encodedSize,
      PaddedBinaryData::kPaddingHeaderBytes + paddedData.paddingBytes +
          /* data size field */ 4 + testData.size());

  std::unique_ptr<folly::IOBuf> serialized = queue.move();
  BinaryProtocolReader reader;
  reader.setInput(serialized.get());
  std::unique_ptr<folly::IOBuf> deserialized;
  reader.readBinary(deserialized);
  ASSERT_EQ(
      deserialized->computeChainDataLength(), encodedSize - /*data size*/ 4);

  // Parse the deserialized data
  folly::io::Cursor cr(deserialized.get());
  ASSERT_EQ(cr.readBE<uint64_t>(), PaddedBinaryData::kMagic);
  uint32_t paddingBytes = cr.readBE<uint32_t>();
  ASSERT_EQ(paddingBytes, paddedData.paddingBytes);
  cr.skip(paddingBytes);
  std::string outData(
      reinterpret_cast<const char*>(cr.data()), testData.size());
  EXPECT_EQ(outData, testData);
}

} // namespace
