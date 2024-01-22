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
#include <thrift/lib/cpp2/transport/rocket/framing/parser/AllocatingParserStrategy.h>

namespace apache {
namespace thrift {
namespace rocket {

class FakeOwner {
 public:
  void handleFrame(std::unique_ptr<folly::IOBuf> buf) {
    frames_.push_back(std::move(buf));
  }
  bool incMemoryUsage(uint32_t n) {
    memoryCounter_ += n;
    return true;
  }
  void decMemoryUsage(uint32_t n) { memoryCounter_ -= n; }

  std::vector<std::unique_ptr<folly::IOBuf>> frames_{};

  uint32_t memoryCounter_ = 0;
};

ParserAllocatorType alloc = ParserAllocatorType();

TEST(AllocatingParserStrategyTest, testAppendFrame) {
  FakeOwner owner;
  AllocatingParserStrategy<FakeOwner> parser(owner, alloc);

  void* buf;
  size_t lenReturn;
  parser.getReadBuffer(&buf, &lenReturn);
  EXPECT_EQ(lenReturn, parser.getMinBufferSize());
  EXPECT_EQ(parser.getCurrentBufferSize(), parser.getMinBufferSize());

  size_t testFrameLength = 20;

  HeaderSerializer serializer(static_cast<uint8_t*>(buf), lenReturn);
  serializer.writeFrameOrMetadataSize(testFrameLength);
  parser.readDataAvailable(Serializer::kBytesForFrameOrMetadataLength);

  EXPECT_EQ(parser.getSize(), Serializer::kBytesForFrameOrMetadataLength);
  EXPECT_EQ(parser.getFrameLength(), testFrameLength);
  EXPECT_EQ(
      parser.getCurrentBufferSize(),
      testFrameLength + Serializer::kBytesForFrameOrMetadataLength);
  EXPECT_EQ(
      owner.memoryCounter_,
      testFrameLength + Serializer::kBytesForFrameOrMetadataLength);
  EXPECT_EQ(owner.frames_.size(), 0);

  parser.getReadBuffer(&buf, &lenReturn);
  EXPECT_EQ(lenReturn, testFrameLength);

  std::string b(testFrameLength, 'b');
  memcpy(static_cast<uint8_t*>(buf), b.data(), testFrameLength);

  parser.readDataAvailable(testFrameLength);

  EXPECT_EQ(parser.getSize(), 0);
  EXPECT_EQ(parser.getFrameLength(), 0);
  EXPECT_EQ(owner.memoryCounter_, 0);
  EXPECT_EQ(owner.frames_.size(), 1);

  auto frame = std::move(owner.frames_[0]);
  EXPECT_EQ(frame->length(), testFrameLength);
}

TEST(AllocatingParserStrategyTest, testAppendLessThanFullFrame) {
  FakeOwner owner;
  AllocatingParserStrategy<FakeOwner> parser(owner, alloc);

  void* buf;
  size_t lenReturn;
  parser.getReadBuffer(&buf, &lenReturn);

  size_t testFrameLength = 20;

  HeaderSerializer serializer(static_cast<uint8_t*>(buf), lenReturn);
  serializer.writeFrameOrMetadataSize(testFrameLength);
  parser.readDataAvailable(Serializer::kBytesForFrameOrMetadataLength);

  std::string b(10, 'b');
  parser.getReadBuffer(&buf, &lenReturn);
  memcpy(static_cast<uint8_t*>(buf), b.data(), 10);

  parser.readDataAvailable(10);

  EXPECT_EQ(parser.getSize(), 10 + Serializer::kBytesForFrameOrMetadataLength);
  EXPECT_EQ(parser.getFrameLength(), testFrameLength);
  EXPECT_EQ(
      owner.memoryCounter_,
      testFrameLength + Serializer::kBytesForFrameOrMetadataLength);
  EXPECT_EQ(owner.frames_.size(), 0);
}

TEST(AllocatingParserStrategyTest, testAppendTwice) {
  FakeOwner owner;
  AllocatingParserStrategy<FakeOwner> parser(owner, alloc);

  void* buf;
  size_t lenReturn;
  parser.getReadBuffer(&buf, &lenReturn);

  size_t testFrameLength = 20;

  HeaderSerializer serializer(static_cast<uint8_t*>(buf), lenReturn);
  serializer.writeFrameOrMetadataSize(testFrameLength);
  parser.readDataAvailable(Serializer::kBytesForFrameOrMetadataLength);

  std::string first(10, 'b');
  parser.getReadBuffer(&buf, &lenReturn);
  memcpy(static_cast<uint8_t*>(buf), first.data(), 10);
  parser.readDataAvailable(10);

  EXPECT_EQ(parser.getSize(), 10 + Serializer::kBytesForFrameOrMetadataLength);
  EXPECT_EQ(parser.getFrameLength(), testFrameLength);
  EXPECT_EQ(
      owner.memoryCounter_,
      testFrameLength + Serializer::kBytesForFrameOrMetadataLength);
  EXPECT_EQ(owner.frames_.size(), 0);

  std::string second(10, 'c');
  memcpy(&static_cast<uint8_t*>(buf)[10], second.data(), 10);
  parser.readDataAvailable(10);

  EXPECT_EQ(parser.getSize(), 0);
  EXPECT_EQ(parser.getFrameLength(), 0);
  EXPECT_EQ(owner.memoryCounter_, 0);
  EXPECT_EQ(owner.frames_.size(), 1);
}

TEST(AllocatingParserStrategyTest, testAppendMultiple) {
  FakeOwner owner;
  AllocatingParserStrategy<FakeOwner> parser(owner, alloc);

  void* buf;
  size_t lenReturn;
  parser.getReadBuffer(&buf, &lenReturn);

  size_t testFrameLength = 20;

  HeaderSerializer serializer(static_cast<uint8_t*>(buf), lenReturn);
  serializer.writeFrameOrMetadataSize(testFrameLength);
  parser.readDataAvailable(Serializer::kBytesForFrameOrMetadataLength);

  std::string first(10, 'b');
  parser.getReadBuffer(&buf, &lenReturn);
  memcpy(static_cast<uint8_t*>(buf), first.data(), 10);
  parser.readDataAvailable(10);

  EXPECT_EQ(parser.getSize(), 13);
  EXPECT_EQ(parser.getFrameLength(), testFrameLength);
  EXPECT_EQ(
      owner.memoryCounter_,
      testFrameLength + Serializer::kBytesForFrameOrMetadataLength);
  EXPECT_EQ(owner.frames_.size(), 0);

  std::string second(5, 'c');
  memcpy(&static_cast<uint8_t*>(buf)[10], second.data(), 5);
  parser.readDataAvailable(5);

  EXPECT_EQ(
      parser.getSize(), 10 + 5 + Serializer::kBytesForFrameOrMetadataLength);
  EXPECT_EQ(parser.getFrameLength(), testFrameLength);
  EXPECT_EQ(
      owner.memoryCounter_,
      testFrameLength + Serializer::kBytesForFrameOrMetadataLength);
  EXPECT_EQ(owner.frames_.size(), 0);

  std::string third(5, 'd');
  memcpy(&static_cast<uint8_t*>(buf)[10 + 5], third.data(), 5);
  parser.readDataAvailable(5);

  EXPECT_EQ(parser.getSize(), 0);
  EXPECT_EQ(parser.getFrameLength(), 0);
  EXPECT_EQ(owner.memoryCounter_, 0);
  EXPECT_EQ(owner.frames_.size(), 1);
}

TEST(AllocatingParserStrategyTest, testManyFrames) {
  FakeOwner owner;
  AllocatingParserStrategy<FakeOwner> parser(owner, alloc);

  {
    void* buf;
    size_t lenReturn;
    parser.getReadBuffer(&buf, &lenReturn);
    size_t testFrameLength = 20;

    HeaderSerializer serializer(static_cast<uint8_t*>(buf), lenReturn);
    serializer.writeFrameOrMetadataSize(testFrameLength);
    parser.readDataAvailable(Serializer::kBytesForFrameOrMetadataLength);
    parser.getReadBuffer(&buf, &lenReturn);
    std::string b(testFrameLength, 'b');
    memcpy(static_cast<uint8_t*>(buf), b.data(), testFrameLength);

    parser.readDataAvailable(testFrameLength);

    EXPECT_EQ(parser.getSize(), 0);
    EXPECT_EQ(parser.getFrameLength(), 0);
    EXPECT_EQ(owner.memoryCounter_, 0);
    EXPECT_EQ(owner.frames_.size(), 1);
  }

  {
    void* buf;
    size_t lenReturn;
    parser.getReadBuffer(&buf, &lenReturn);
    size_t testFrameLength = 20;

    HeaderSerializer serializer(static_cast<uint8_t*>(buf), lenReturn);
    serializer.writeFrameOrMetadataSize(testFrameLength);
    parser.readDataAvailable(Serializer::kBytesForFrameOrMetadataLength);
    parser.getReadBuffer(&buf, &lenReturn);
    std::string b(testFrameLength, 'b');
    memcpy(static_cast<uint8_t*>(buf), b.data(), testFrameLength);

    parser.readDataAvailable(testFrameLength);

    EXPECT_EQ(parser.getSize(), 0);
    EXPECT_EQ(parser.getFrameLength(), 0);
    EXPECT_EQ(owner.memoryCounter_, 0);
    EXPECT_EQ(owner.frames_.size(), 2);
  }

  {
    void* buf;
    size_t lenReturn;
    parser.getReadBuffer(&buf, &lenReturn);
    size_t testFrameLength = 20;

    HeaderSerializer serializer(static_cast<uint8_t*>(buf), lenReturn);
    serializer.writeFrameOrMetadataSize(testFrameLength);
    parser.readDataAvailable(Serializer::kBytesForFrameOrMetadataLength);
    parser.getReadBuffer(&buf, &lenReturn);
    std::string b(testFrameLength, 'b');
    memcpy(static_cast<uint8_t*>(buf), b.data(), testFrameLength);

    parser.readDataAvailable(testFrameLength);

    EXPECT_EQ(parser.getSize(), 0);
    EXPECT_EQ(parser.getFrameLength(), 0);
    EXPECT_EQ(owner.memoryCounter_, 0);
    EXPECT_EQ(owner.frames_.size(), 3);
  }
}

TEST(AllocatingParserStrategyTest, testSmallFrame) {
  FakeOwner owner;
  AllocatingParserStrategy<FakeOwner> parser(owner, alloc);

  void* buf;
  size_t lenReturn;
  parser.getReadBuffer(&buf, &lenReturn);
  EXPECT_EQ(lenReturn, parser.getMinBufferSize());
  EXPECT_EQ(parser.getCurrentBufferSize(), parser.getMinBufferSize());

  size_t testFrameLength = 12;

  HeaderSerializer serializer(static_cast<uint8_t*>(buf), lenReturn);
  serializer.writeFrameOrMetadataSize(testFrameLength);
  parser.readDataAvailable(Serializer::kBytesForFrameOrMetadataLength);

  EXPECT_EQ(parser.getSize(), Serializer::kBytesForFrameOrMetadataLength);
  EXPECT_EQ(parser.getFrameLength(), testFrameLength);
  EXPECT_EQ(parser.getCurrentBufferSize(), parser.getMinBufferSize());
  EXPECT_EQ(
      owner.memoryCounter_,
      testFrameLength + Serializer::kBytesForFrameOrMetadataLength);
  EXPECT_EQ(owner.frames_.size(), 0);

  parser.getReadBuffer(&buf, &lenReturn);
  EXPECT_EQ(lenReturn, parser.getMinBufferSize() - 3);

  std::string b(testFrameLength, 'b');
  memcpy(static_cast<uint8_t*>(buf), b.data(), testFrameLength);

  // The follow call to readDataAvailable() should trigger parser to handover
  // buffer to owner via handleFrame call even current buffer is not full.
  parser.readDataAvailable(testFrameLength);

  EXPECT_EQ(parser.getSize(), 0);
  EXPECT_EQ(parser.getFrameLength(), 0);
  EXPECT_EQ(owner.memoryCounter_, 0);
  EXPECT_EQ(owner.frames_.size(), 1);

  auto frame = std::move(owner.frames_[0]);
  EXPECT_EQ(frame->length(), testFrameLength);

  parser.getReadBuffer(&buf, &lenReturn);
  EXPECT_EQ(lenReturn, 16); // a new buffer is allocated by parser

  // Not enough bytes to trigger frameLength computation.
  static_cast<uint8_t*>(buf)[0] = 0;
  parser.readDataAvailable(1);

  EXPECT_EQ(parser.getSize(), 1);
  EXPECT_EQ(parser.getFrameLength(), 0);
  EXPECT_EQ(owner.memoryCounter_, 0);
  EXPECT_EQ(owner.frames_.size(), 1);
}

TEST(AllocatingParserStrategyTest, testTinyFrame) {
  FakeOwner owner;
  AllocatingParserStrategy<FakeOwner> parser(owner, alloc);

  void* buf;
  size_t lenReturn;
  parser.getReadBuffer(&buf, &lenReturn);
  EXPECT_EQ(lenReturn, parser.getMinBufferSize());
  EXPECT_EQ(parser.getCurrentBufferSize(), parser.getMinBufferSize());

  size_t testFrameLength = 8;

  HeaderSerializer serializer1(static_cast<uint8_t*>(buf), lenReturn);
  serializer1.writeFrameOrMetadataSize(testFrameLength);
  parser.readDataAvailable(Serializer::kBytesForFrameOrMetadataLength);

  EXPECT_EQ(parser.getSize(), Serializer::kBytesForFrameOrMetadataLength);
  EXPECT_EQ(parser.getFrameLength(), testFrameLength);
  EXPECT_EQ(parser.getCurrentBufferSize(), parser.getMinBufferSize());
  EXPECT_EQ(owner.memoryCounter_, testFrameLength + 3);
  EXPECT_EQ(owner.frames_.size(), 0);

  parser.getReadBuffer(&buf, &lenReturn);
  EXPECT_EQ(
      lenReturn,
      parser.getMinBufferSize() - Serializer::kBytesForFrameOrMetadataLength);

  std::string b(testFrameLength, 'b');
  memcpy(static_cast<uint8_t*>(buf), b.data(), testFrameLength);

  // The follow call to readDataAvailable() should trigger parser to handover
  // buffer to owner via handleFrame call even current buffer is not full.
  parser.readDataAvailable(testFrameLength);

  EXPECT_EQ(parser.getSize(), 0);
  EXPECT_EQ(parser.getFrameLength(), 0);
  EXPECT_EQ(owner.memoryCounter_, 0);
  EXPECT_EQ(owner.frames_.size(), 1);

  parser.getReadBuffer(&buf, &lenReturn);
  EXPECT_EQ(lenReturn, parser.getMinBufferSize());

  size_t newTestFrameLength = 29;
  HeaderSerializer serializer2(static_cast<uint8_t*>(buf), lenReturn);
  serializer2.writeFrameOrMetadataSize(newTestFrameLength);

  // frameLength compututation should be triggered.q
  parser.readDataAvailable(Serializer::kBytesForFrameOrMetadataLength);

  EXPECT_EQ(parser.getSize(), Serializer::kBytesForFrameOrMetadataLength);
  EXPECT_EQ(parser.getFrameLength(), newTestFrameLength);
  EXPECT_EQ(
      parser.getCurrentBufferSize(),
      newTestFrameLength + Serializer::kBytesForFrameOrMetadataLength);
  EXPECT_EQ(
      owner.memoryCounter_,
      newTestFrameLength + Serializer::kBytesForFrameOrMetadataLength);
  EXPECT_EQ(owner.frames_.size(), 1);

  auto frame = std::move(owner.frames_[0]);
  EXPECT_EQ(frame->length(), testFrameLength);

  parser.getReadBuffer(&buf, &lenReturn);
  EXPECT_EQ(lenReturn, newTestFrameLength);
  std::string c(2, 'c');
  memcpy(static_cast<uint8_t*>(buf), b.data(), 2);
  parser.readDataAvailable(2);

  EXPECT_EQ(parser.getSize(), 2 + Serializer::kBytesForFrameOrMetadataLength);
  EXPECT_EQ(parser.getFrameLength(), newTestFrameLength);
  EXPECT_EQ(owner.memoryCounter_, newTestFrameLength + 3);
  EXPECT_EQ(owner.frames_.size(), 1);
}

TEST(AllocatingParserStrategyTest, testManyTinyFrame) {
  THRIFT_FLAG_SET_MOCK(rocket_allocating_parser_min_buffer_size, 64);
  FakeOwner owner;
  AllocatingParserStrategy<FakeOwner> parser(owner, alloc);

  void* buf;
  size_t lenReturn;
  parser.getReadBuffer(&buf, &lenReturn);
  EXPECT_EQ(lenReturn, parser.getMinBufferSize());
  EXPECT_EQ(parser.getCurrentBufferSize(), parser.getMinBufferSize());

  size_t testFrameLen1 = 7;
  size_t testFrameLen2 = 19;
  size_t testFrameLen3 = 12;

  HeaderSerializer serializer1(static_cast<uint8_t*>(buf), lenReturn);
  serializer1.writeFrameOrMetadataSize(testFrameLen1);
  size_t bytesWritten = Serializer::kBytesForFrameOrMetadataLength;
  std::string t1(testFrameLen1, 'b');
  memcpy(&static_cast<uint8_t*>(buf)[bytesWritten], t1.data(), testFrameLen1);
  bytesWritten += testFrameLen1;

  HeaderSerializer serializer2(
      &static_cast<uint8_t*>(buf)[bytesWritten], lenReturn);
  serializer2.writeFrameOrMetadataSize(testFrameLen2);
  bytesWritten += Serializer::kBytesForFrameOrMetadataLength;
  std::string t2(testFrameLen2, 'c');
  memcpy(&static_cast<uint8_t*>(buf)[bytesWritten], t2.data(), testFrameLen2);
  bytesWritten += testFrameLen2;

  HeaderSerializer serializer3(
      &static_cast<uint8_t*>(buf)[bytesWritten], lenReturn);
  serializer3.writeFrameOrMetadataSize(testFrameLen3);
  bytesWritten += Serializer::kBytesForFrameOrMetadataLength;
  std::string t3(testFrameLen3, 'd');
  memcpy(&static_cast<uint8_t*>(buf)[bytesWritten], t3.data(), testFrameLen3);
  bytesWritten += testFrameLen3;

  parser.readDataAvailable(bytesWritten);

  EXPECT_EQ(parser.getSize(), 0);
  EXPECT_EQ(parser.getFrameLength(), 0);
  EXPECT_EQ(owner.memoryCounter_, 0);
  EXPECT_EQ(owner.frames_.size(), 3);

  auto frame1 = std::move(owner.frames_[0]);
  EXPECT_EQ(frame1->length(), testFrameLen1);
  auto frame2 = std::move(owner.frames_[1]);
  EXPECT_EQ(frame2->length(), testFrameLen2);
  auto frame3 = std::move(owner.frames_[2]);
  EXPECT_EQ(frame3->length(), testFrameLen3);
}

TEST(AllocatingParserStrategyTest, testManyTinyFrameWithIncompleteFrame) {
  THRIFT_FLAG_SET_MOCK(rocket_allocating_parser_min_buffer_size, 64);
  FakeOwner owner;
  AllocatingParserStrategy<FakeOwner> parser(owner, alloc);

  void* buf;
  size_t lenReturn;
  parser.getReadBuffer(&buf, &lenReturn);
  EXPECT_EQ(lenReturn, parser.getMinBufferSize());
  EXPECT_EQ(parser.getCurrentBufferSize(), parser.getMinBufferSize());

  size_t testFrameLen1 = 7;
  size_t testFrameLen2 = 19;
  size_t testFrameLen3 = 12;
  size_t testFrameLen4 = 183;

  HeaderSerializer serializer1(static_cast<uint8_t*>(buf), lenReturn);
  serializer1.writeFrameOrMetadataSize(testFrameLen1);
  size_t bytesWritten = Serializer::kBytesForFrameOrMetadataLength;
  std::string t1(testFrameLen1, 'b');
  memcpy(&static_cast<uint8_t*>(buf)[bytesWritten], t1.data(), testFrameLen1);
  bytesWritten += testFrameLen1;

  HeaderSerializer serializer2(
      &static_cast<uint8_t*>(buf)[bytesWritten], lenReturn - bytesWritten);
  serializer2.writeFrameOrMetadataSize(testFrameLen2);
  bytesWritten += Serializer::kBytesForFrameOrMetadataLength;
  std::string t2(testFrameLen2, 'c');
  memcpy(&static_cast<uint8_t*>(buf)[bytesWritten], t2.data(), testFrameLen2);
  bytesWritten += testFrameLen2;

  HeaderSerializer serializer3(
      &static_cast<uint8_t*>(buf)[bytesWritten], lenReturn - bytesWritten);
  serializer3.writeFrameOrMetadataSize(testFrameLen3);
  bytesWritten += Serializer::kBytesForFrameOrMetadataLength;
  std::string t3(testFrameLen3, 'd');
  memcpy(&static_cast<uint8_t*>(buf)[bytesWritten], t3.data(), testFrameLen3);
  bytesWritten += testFrameLen3;

  HeaderSerializer serializer4(
      &static_cast<uint8_t*>(buf)[bytesWritten], lenReturn - bytesWritten);
  serializer4.writeFrameOrMetadataSize(testFrameLen4);
  bytesWritten += Serializer::kBytesForFrameOrMetadataLength;
  size_t incompleteTestDataLen = 4;
  std::string t4Incomplete(incompleteTestDataLen, 'e');
  memcpy(
      &static_cast<uint8_t*>(buf)[bytesWritten],
      t4Incomplete.data(),
      incompleteTestDataLen);
  bytesWritten += incompleteTestDataLen;

  parser.readDataAvailable(bytesWritten);

  EXPECT_EQ(
      parser.getSize(),
      incompleteTestDataLen + Serializer::kBytesForFrameOrMetadataLength);
  EXPECT_EQ(parser.getFrameLength(), testFrameLen4);
  EXPECT_EQ(
      owner.memoryCounter_,
      testFrameLen4 + Serializer::kBytesForFrameOrMetadataLength);
  EXPECT_EQ(owner.frames_.size(), 3);

  auto frame1 = std::move(owner.frames_[0]);
  EXPECT_EQ(frame1->length(), testFrameLen1);
  auto frame2 = std::move(owner.frames_[1]);
  EXPECT_EQ(frame2->length(), testFrameLen2);
  auto frame3 = std::move(owner.frames_[2]);
  EXPECT_EQ(frame3->length(), testFrameLen3);
}

} // namespace rocket
} // namespace thrift
} // namespace apache
