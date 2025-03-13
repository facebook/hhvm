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
#include <folly/String.h>
#include <folly/io/IOBuf.h>
#include <thrift/lib/cpp2/server/ThriftServer.h>
#include <thrift/lib/cpp2/transport/rocket/framing/Frames.h>
#include <thrift/lib/cpp2/transport/rocket/framing/parser/AlignedParserStrategy.h>
#include <thrift/lib/cpp2/transport/rocket/framing/parser/test/TestUtil.h>

namespace apache::thrift::rocket {

using State = detail::aligned_parser::State;

TEST(AlignedParserStrategyTest, AppendEmptyFrame) {
  FakeOwner owner;
  AlignedParserStrategy<FakeOwner> parser(owner);
  CancelFrame cancelFrame(StreamId(1));
  auto iobuf = std::move(cancelFrame).serialize();
  const folly::ByteRange cancelFrameBuf = iobuf->coalesce();

  void* buf;
  size_t lenReturn;
  parser.getReadBuffer(&buf, &lenReturn);

  EXPECT_EQ(Serializer::kMinimumFrameHeaderLength, lenReturn);

  std::memcpy(
      buf, cancelFrameBuf.data(), Serializer::kMinimumFrameHeaderLength);

  parser.readDataAvailable(Serializer::kMinimumFrameHeaderLength);

  EXPECT_EQ(owner.frames_.size(), 1);

  parser.getReadBuffer(&buf, &lenReturn);

  EXPECT_EQ(Serializer::kMinimumFrameHeaderLength, lenReturn);
  EXPECT_EQ(parser.state(), detail::aligned_parser::State::AwaitingHeader);
}

TEST(AlignedParserStrategyTest, AppendRequestFrameWithDataAndMetadata) {
  FakeOwner owner;
  AlignedParserStrategy<FakeOwner> parser(owner);
  auto data = folly::IOBuf::copyBuffer("test");
  auto metadata = folly::IOBuf::copyBuffer("metadata");
  RequestResponseFrame requestResponseFrame(
      StreamId(1),
      Payload::makeFromMetadataAndData(std::move(metadata), std::move(data)));
  auto iobuf = std::move(requestResponseFrame).serialize();
  const folly::ByteRange requestResponseBuf = iobuf->coalesce();

  void* buf;
  size_t lenReturn;
  parser.getReadBuffer(&buf, &lenReturn);

  EXPECT_EQ(Serializer::kMinimumFrameHeaderLength, lenReturn);

  std::memcpy(
      buf, requestResponseBuf.data(), Serializer::kMinimumFrameHeaderLength);

  parser.readDataAvailable(Serializer::kMinimumFrameHeaderLength);

  EXPECT_EQ(owner.frames_.size(), 0);
  EXPECT_EQ(
      parser.state(), detail::aligned_parser::State::AwaitingMetadataLength);

  parser.getReadBuffer(&buf, &lenReturn);

  EXPECT_EQ(Serializer::kBytesForFrameOrMetadataLength, lenReturn);

  EXPECT_EQ(
      parser.remainingHeader(), Serializer::kBytesForFrameOrMetadataLength);

  std::cout << "requestResponseBuf: \n"
            << folly::hexDump(
                   requestResponseBuf.data(), requestResponseBuf.size())
            << std::endl;

  std::cout << "metadata length: "
            << readFrameOrMetadataSize(
                   requestResponseBuf.data() +
                   Serializer::kMinimumFrameHeaderLength)
            << std::endl;

  std::memcpy(
      buf,
      requestResponseBuf.data() + Serializer::kMinimumFrameHeaderLength,
      Serializer::kBytesForFrameOrMetadataLength);

  std::cout << "written metadata length: "
            << readFrameOrMetadataSize(reinterpret_cast<uint8_t*>(buf))
            << std::endl;

  parser.readDataAvailable(Serializer::kBytesForFrameOrMetadataLength);

  EXPECT_EQ(parser.state(), detail::aligned_parser::State::AwaitingMetadata);

  EXPECT_EQ(parser.remainingHeader(), 0);
  EXPECT_EQ(parser.remainingMetadata(), 8);

  parser.getReadBuffer(&buf, &lenReturn);

  EXPECT_EQ(8, lenReturn);

  std::memcpy(
      buf,
      requestResponseBuf.data() + Serializer::kMinimumFrameHeaderLength +
          Serializer::kBytesForFrameOrMetadataLength,
      4);

  parser.readDataAvailable(4);

  EXPECT_EQ(parser.state(), detail::aligned_parser::State::AwaitingMetadata);
  EXPECT_EQ(parser.remainingMetadata(), 4);

  parser.getReadBuffer(&buf, &lenReturn);

  EXPECT_EQ(4, lenReturn);

  std::memcpy(
      buf,
      requestResponseBuf.data() + Serializer::kMinimumFrameHeaderLength +
          Serializer::kBytesForFrameOrMetadataLength + 4,
      4);

  parser.readDataAvailable(4);

  EXPECT_EQ(parser.remainingMetadata(), 0);
  EXPECT_EQ(parser.state(), detail::aligned_parser::State::AwaitingData);
  EXPECT_EQ(parser.remainingData(), 4);

  parser.getReadBuffer(&buf, &lenReturn);

  EXPECT_EQ(4, lenReturn);

  std::memcpy(
      buf,
      requestResponseBuf.data() + Serializer::kMinimumFrameHeaderLength +
          Serializer::kBytesForFrameOrMetadataLength + 8,
      4);

  parser.readDataAvailable(4);

  EXPECT_EQ(parser.remainingData(), 0);
  EXPECT_EQ(parser.state(), detail::aligned_parser::State::AwaitingHeader);
  EXPECT_EQ(owner.frames_.size(), 1);

  auto ownerBuf = owner.frames_[0]->clone();

  std::cout << "RequestResponseFrame Data after Parser: \n"
            << folly::hexDump(
                   ownerBuf->cloneCoalesced()->data(),
                   ownerBuf->computeChainDataLength())
            << std::endl;

  RequestResponseFrame ownerFrame(std::move(ownerBuf));
  auto& payload = ownerFrame.payload();
  auto md_size = payload.metadataSize();
  auto data_size = payload.dataSize();

  EXPECT_EQ(md_size, 8);
  EXPECT_EQ(data_size, 4);
}

TEST(AlignedParserStrategyTest, AppendMetadataPushFrame) {
  FakeOwner owner;
  AlignedParserStrategy<FakeOwner> parser(owner);
  auto metadata = folly::IOBuf::copyBuffer("metadata");
  MetadataPushFrame metadataPushFrame =
      MetadataPushFrame::makeFromMetadata(std::move(metadata));
  auto iobuf = std::move(metadataPushFrame).serialize();
  const folly::ByteRange metadatPushBuf = iobuf->coalesce();

  std::cout << "metadatPushBuf: \n"
            << folly::hexDump(metadatPushBuf.data(), metadatPushBuf.size());

  void* buf;
  size_t lenReturn;
  parser.getReadBuffer(&buf, &lenReturn);

  EXPECT_EQ(Serializer::kMinimumFrameHeaderLength, lenReturn);

  std::memcpy(
      buf, metadatPushBuf.data(), Serializer::kMinimumFrameHeaderLength);

  parser.readDataAvailable(Serializer::kMinimumFrameHeaderLength);

  EXPECT_EQ(parser.state(), detail::aligned_parser::State::AwaitingNonAligned);

  EXPECT_EQ(parser.remainingHeader(), 0);
  EXPECT_EQ(parser.remainingMetadata(), 0);
  EXPECT_EQ(parser.remainingUnaligned(), 8);

  parser.getReadBuffer(&buf, &lenReturn);

  EXPECT_EQ(8, lenReturn);

  std::memcpy(
      buf,
      metadatPushBuf.data() + Serializer::kMinimumFrameHeaderLength +
          Serializer::kBytesForFrameOrMetadataLength,
      8);

  parser.readDataAvailable(8);

  EXPECT_EQ(parser.remainingUnaligned(), 0);
  EXPECT_EQ(parser.state(), detail::aligned_parser::State::AwaitingHeader);
  EXPECT_EQ(owner.frames_.size(), 1);
}

TEST(AlignedParserStrategyTest, AppendRequestFrameWithData) {
  FakeOwner owner;
  AlignedParserStrategy<FakeOwner> parser(owner);
  auto data = folly::IOBuf::copyBuffer("test");
  RequestResponseFrame requestResponseFrame(
      StreamId(1), Payload::makeFromData(std::move(data)));
  auto iobuf = std::move(requestResponseFrame).serialize();
  const folly::ByteRange requestResponseBuf = iobuf->coalesce();

  std::cout << "RequestResponseFrame Data Before Parser: \n"
            << folly::hexDump(
                   iobuf->cloneCoalesced()->data() +
                       Serializer::kBytesForFrameOrMetadataLength,
                   iobuf->computeChainDataLength())
            << std::endl;

  void* buf;
  size_t lenReturn;

  // Write the frame length / header
  parser.getReadBuffer(&buf, &lenReturn);
  EXPECT_EQ(Serializer::kMinimumFrameHeaderLength, lenReturn);
  std::memcpy(buf, requestResponseBuf.data(), lenReturn);
  parser.readDataAvailable(lenReturn);

  EXPECT_EQ(parser.remainingHeader(), 0);
  EXPECT_EQ(owner.frames_.size(), 0);
  EXPECT_EQ(parser.state(), detail::aligned_parser::State::AwaitingData);

  parser.getReadBuffer(&buf, &lenReturn);

  EXPECT_EQ(parser.remainingData(), lenReturn);

  std::memcpy(
      buf,
      requestResponseBuf.data() + Serializer::kMinimumFrameHeaderLength,
      lenReturn);

  parser.readDataAvailable(4);

  EXPECT_EQ(parser.remainingData(), 0);
  EXPECT_EQ(parser.state(), detail::aligned_parser::State::AwaitingHeader);
  EXPECT_EQ(owner.frames_.size(), 1);

  auto ownerBuf = owner.frames_[0]->clone();

  std::cout << "RequestResponseFrame Data after Parser: \n"
            << folly::hexDump(
                   ownerBuf->cloneCoalesced()->data(),
                   ownerBuf->computeChainDataLength())
            << std::endl;

  RequestResponseFrame ownerFrame(std::move(ownerBuf));
  auto& payload = ownerFrame.payload();
  auto md_size = payload.metadataSize();
  auto data_size = payload.dataSize();

  EXPECT_EQ(md_size, 0);
  EXPECT_EQ(data_size, 4);
}

TEST(AlignedParserStrategyTest, AppendSetupFrame) {
  FakeOwner owner;
  AlignedParserStrategy<FakeOwner> parser(owner);
  auto data = folly::IOBuf::copyBuffer("test");
  SetupFrame setupFrame(Payload::makeFromData(std::move(data)), false);
  auto iobuf = std::move(setupFrame).serialize();
  const folly::ByteRange requestResponseBuf = iobuf->coalesce();

  std::cout << "SetupFrame Data Before Parser: \n"
            << folly::hexDump(
                   iobuf->cloneCoalesced()->data() +
                       Serializer::kBytesForFrameOrMetadataLength,
                   iobuf->computeChainDataLength())
            << std::endl;
  std::cout << "SetupFrame total size including length: " << iobuf->length()
            << std::endl
            << std::endl;
  void* buf;
  size_t lenReturn;
  size_t read = 0;

  // Write the frame length
  parser.getReadBuffer(&buf, &lenReturn);
  EXPECT_EQ(Serializer::kMinimumFrameHeaderLength, lenReturn);
  std::memcpy(buf, requestResponseBuf.data(), lenReturn);
  parser.readDataAvailable(lenReturn);
  read += lenReturn;

  // Expect the parser to be in the awaiting non aligned buffer
  EXPECT_EQ(owner.frames_.size(), 0);
  EXPECT_EQ(parser.state(), detail::aligned_parser::State::AwaitingNonAligned);

  // Write all the remaining data
  parser.getReadBuffer(&buf, &lenReturn);

  std::cout << "remainging data length after header: " << lenReturn << std::endl
            << std::endl;

  EXPECT_EQ(
      parser.remainingUnaligned(), iobuf->computeChainDataLength() - read);
  EXPECT_EQ(parser.remainingUnaligned(), lenReturn);

  std::memcpy(
      buf,
      requestResponseBuf.data() + Serializer::kMinimumFrameHeaderLength,
      lenReturn);
  read += lenReturn;

  EXPECT_EQ(read, requestResponseBuf.size());

  parser.readDataAvailable(lenReturn);

  EXPECT_EQ(parser.remainingUnaligned(), 0);
  EXPECT_EQ(parser.state(), detail::aligned_parser::State::AwaitingHeader);
  EXPECT_EQ(owner.frames_.size(), 1);

  auto ownerBuf = owner.frames_[0]->clone();

  std::cout << "SetupFrame Data after Parser: \n"
            << folly::hexDump(
                   ownerBuf->cloneCoalesced()->data(),
                   ownerBuf->computeChainDataLength())
            << std::endl;

  SetupFrame frame(std::move(ownerBuf));
  EXPECT_EQ(setupFrame.frameType(), frame.frameType());
  EXPECT_EQ(setupFrame.frameHeaderSize(), frame.frameHeaderSize());
  EXPECT_EQ(
      setupFrame.hasResumeIdentificationToken(),
      frame.hasResumeIdentificationToken());
  EXPECT_EQ(
      setupFrame.encodeMetadataUsingBinary(),
      frame.encodeMetadataUsingBinary());

  auto& payload = frame.payload();
  auto md_size = payload.metadataSize();
  auto data_size = payload.dataSize();

  EXPECT_EQ(md_size, 0);
  EXPECT_EQ(data_size, 4);
}

TEST(AlignedParserStrategyTest, AppendRequestNFrame) {
  FakeOwner owner;
  AlignedParserStrategy<FakeOwner> parser(owner);
  RequestNFrame requestNFrame(StreamId(1), 5);
  auto iobuf = std::move(requestNFrame).serialize();
  const folly::ByteRange requestNBuf = iobuf->coalesce();

  std::cout << "RequestNFrame Data Before Parser: \n"
            << folly::hexDump(
                   iobuf->cloneCoalesced()->data() +
                       Serializer::kBytesForFrameOrMetadataLength,
                   iobuf->computeChainDataLength())
            << std::endl;

  void* buf;
  size_t lenReturn;

  // Write the frame length / header
  parser.getReadBuffer(&buf, &lenReturn);
  EXPECT_EQ(Serializer::kMinimumFrameHeaderLength, lenReturn);
  std::memcpy(buf, requestNBuf.data(), lenReturn);
  parser.readDataAvailable(lenReturn);

  EXPECT_EQ(parser.remainingHeader(), 0);
  EXPECT_EQ(owner.frames_.size(), 0);
  EXPECT_EQ(parser.state(), detail::aligned_parser::State::AwaitingNonAligned);

  parser.getReadBuffer(&buf, &lenReturn);

  EXPECT_EQ(parser.remainingUnaligned(), lenReturn);

  std::memcpy(
      buf,
      requestNBuf.data() + Serializer::kMinimumFrameHeaderLength,
      lenReturn);

  parser.readDataAvailable(lenReturn);

  EXPECT_EQ(parser.remainingData(), 0);
  EXPECT_EQ(parser.state(), detail::aligned_parser::State::AwaitingHeader);
  EXPECT_EQ(owner.frames_.size(), 1);

  auto ownerBuf = owner.frames_[0]->clone();

  std::cout << "RequestNFrame Data after Parser: \n"
            << folly::hexDump(
                   ownerBuf->cloneCoalesced()->data(),
                   ownerBuf->computeChainDataLength())
            << std::endl;

  RequestNFrame frame(std::move(ownerBuf));
  EXPECT_EQ(requestNFrame.frameType(), frame.frameType());
  EXPECT_EQ(requestNFrame.requestN(), frame.requestN());
}

} // namespace apache::thrift::rocket
