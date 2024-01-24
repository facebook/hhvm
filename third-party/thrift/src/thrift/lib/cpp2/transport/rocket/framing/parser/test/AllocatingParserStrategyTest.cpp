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

  static constexpr size_t kTestFrameLength = 20;

  HeaderSerializer serializer(static_cast<uint8_t*>(buf), lenReturn);
  serializer.writeFrameOrMetadataSize(kTestFrameLength);
  parser.readDataAvailable(Serializer::kBytesForFrameOrMetadataLength);

  EXPECT_EQ(parser.getSize(), Serializer::kBytesForFrameOrMetadataLength);
  EXPECT_EQ(parser.getFrameLength(), kTestFrameLength);
  EXPECT_EQ(
      parser.getCurrentBufferSize(),
      kTestFrameLength + Serializer::kBytesForFrameOrMetadataLength);
  EXPECT_EQ(
      owner.memoryCounter_,
      kTestFrameLength + Serializer::kBytesForFrameOrMetadataLength);
  ASSERT_EQ(owner.frames_.size(), 0);

  parser.getReadBuffer(&buf, &lenReturn);
  EXPECT_EQ(lenReturn, kTestFrameLength);

  std::string b(kTestFrameLength, 'b');
  memcpy(static_cast<uint8_t*>(buf), b.data(), kTestFrameLength);

  parser.readDataAvailable(kTestFrameLength);

  EXPECT_EQ(parser.getSize(), 0);
  EXPECT_EQ(parser.getFrameLength(), 0);
  EXPECT_EQ(owner.memoryCounter_, 0);
  ASSERT_EQ(owner.frames_.size(), 1);

  const folly::IOBuf& frame = *owner.frames_[0];
  EXPECT_EQ(frame.length(), kTestFrameLength);
}

TEST(AllocatingParserStrategyTest, testAppendLessThanFullFrame) {
  FakeOwner owner;
  AllocatingParserStrategy<FakeOwner> parser(owner, alloc);

  void* buf;
  size_t lenReturn;
  parser.getReadBuffer(&buf, &lenReturn);

  static constexpr size_t kTestFrameLength = 20;

  HeaderSerializer serializer(static_cast<uint8_t*>(buf), lenReturn);
  serializer.writeFrameOrMetadataSize(kTestFrameLength);
  parser.readDataAvailable(Serializer::kBytesForFrameOrMetadataLength);

  std::string b(10, 'b');
  parser.getReadBuffer(&buf, &lenReturn);
  memcpy(static_cast<uint8_t*>(buf), b.data(), 10);

  parser.readDataAvailable(10);

  EXPECT_EQ(parser.getSize(), 10 + Serializer::kBytesForFrameOrMetadataLength);
  EXPECT_EQ(parser.getFrameLength(), kTestFrameLength);
  EXPECT_EQ(
      owner.memoryCounter_,
      kTestFrameLength + Serializer::kBytesForFrameOrMetadataLength);
  ASSERT_EQ(owner.frames_.size(), 0);
}

TEST(AllocatingParserStrategyTest, testAppendTwice) {
  FakeOwner owner;
  AllocatingParserStrategy<FakeOwner> parser(owner, alloc);

  void* buf;
  size_t lenReturn;
  parser.getReadBuffer(&buf, &lenReturn);

  static constexpr size_t kTestFrameLength = 20;

  HeaderSerializer serializer(static_cast<uint8_t*>(buf), lenReturn);
  serializer.writeFrameOrMetadataSize(kTestFrameLength);
  parser.readDataAvailable(Serializer::kBytesForFrameOrMetadataLength);

  std::string first(10, 'b');
  parser.getReadBuffer(&buf, &lenReturn);
  memcpy(static_cast<uint8_t*>(buf), first.data(), 10);
  parser.readDataAvailable(10);

  EXPECT_EQ(parser.getSize(), 10 + Serializer::kBytesForFrameOrMetadataLength);
  EXPECT_EQ(parser.getFrameLength(), kTestFrameLength);
  EXPECT_EQ(
      owner.memoryCounter_,
      kTestFrameLength + Serializer::kBytesForFrameOrMetadataLength);
  ASSERT_EQ(owner.frames_.size(), 0);

  std::string second(10, 'c');
  memcpy(&static_cast<uint8_t*>(buf)[10], second.data(), 10);
  parser.readDataAvailable(10);

  EXPECT_EQ(parser.getSize(), 0);
  EXPECT_EQ(parser.getFrameLength(), 0);
  EXPECT_EQ(owner.memoryCounter_, 0);
  ASSERT_EQ(owner.frames_.size(), 1);
}

TEST(AllocatingParserStrategyTest, testAppendMultiple) {
  FakeOwner owner;
  AllocatingParserStrategy<FakeOwner> parser(owner, alloc);

  void* buf;
  size_t lenReturn;
  parser.getReadBuffer(&buf, &lenReturn);

  static constexpr size_t kTestFrameLength = 20;

  HeaderSerializer serializer(static_cast<uint8_t*>(buf), lenReturn);
  serializer.writeFrameOrMetadataSize(kTestFrameLength);
  parser.readDataAvailable(Serializer::kBytesForFrameOrMetadataLength);

  std::string first(10, 'b');
  parser.getReadBuffer(&buf, &lenReturn);
  memcpy(static_cast<uint8_t*>(buf), first.data(), 10);
  parser.readDataAvailable(10);

  EXPECT_EQ(parser.getSize(), 13);
  EXPECT_EQ(parser.getFrameLength(), kTestFrameLength);
  EXPECT_EQ(
      owner.memoryCounter_,
      kTestFrameLength + Serializer::kBytesForFrameOrMetadataLength);
  ASSERT_EQ(owner.frames_.size(), 0);

  std::string second(5, 'c');
  memcpy(&static_cast<uint8_t*>(buf)[10], second.data(), 5);
  parser.readDataAvailable(5);

  EXPECT_EQ(
      parser.getSize(), 10 + 5 + Serializer::kBytesForFrameOrMetadataLength);
  EXPECT_EQ(parser.getFrameLength(), kTestFrameLength);
  EXPECT_EQ(
      owner.memoryCounter_,
      kTestFrameLength + Serializer::kBytesForFrameOrMetadataLength);
  ASSERT_EQ(owner.frames_.size(), 0);

  std::string third(5, 'd');
  memcpy(&static_cast<uint8_t*>(buf)[10 + 5], third.data(), 5);
  parser.readDataAvailable(5);

  EXPECT_EQ(parser.getSize(), 0);
  EXPECT_EQ(parser.getFrameLength(), 0);
  EXPECT_EQ(owner.memoryCounter_, 0);
  ASSERT_EQ(owner.frames_.size(), 1);
}

TEST(AllocatingParserStrategyTest, testManyFrames) {
  FakeOwner owner;
  AllocatingParserStrategy<FakeOwner> parser(owner, alloc);

  {
    void* buf;
    size_t lenReturn;
    parser.getReadBuffer(&buf, &lenReturn);
    static constexpr size_t kTestFrameLength = 20;

    HeaderSerializer serializer(static_cast<uint8_t*>(buf), lenReturn);
    serializer.writeFrameOrMetadataSize(kTestFrameLength);
    parser.readDataAvailable(Serializer::kBytesForFrameOrMetadataLength);
    parser.getReadBuffer(&buf, &lenReturn);
    std::string b(kTestFrameLength, 'b');
    memcpy(static_cast<uint8_t*>(buf), b.data(), kTestFrameLength);

    parser.readDataAvailable(kTestFrameLength);

    EXPECT_EQ(parser.getSize(), 0);
    EXPECT_EQ(parser.getFrameLength(), 0);
    EXPECT_EQ(owner.memoryCounter_, 0);
    ASSERT_EQ(owner.frames_.size(), 1);
  }

  {
    void* buf;
    size_t lenReturn;
    parser.getReadBuffer(&buf, &lenReturn);
    static constexpr size_t kTestFrameLength = 20;

    HeaderSerializer serializer(static_cast<uint8_t*>(buf), lenReturn);
    serializer.writeFrameOrMetadataSize(kTestFrameLength);
    parser.readDataAvailable(Serializer::kBytesForFrameOrMetadataLength);
    parser.getReadBuffer(&buf, &lenReturn);
    std::string b(kTestFrameLength, 'b');
    memcpy(static_cast<uint8_t*>(buf), b.data(), kTestFrameLength);

    parser.readDataAvailable(kTestFrameLength);

    EXPECT_EQ(parser.getSize(), 0);
    EXPECT_EQ(parser.getFrameLength(), 0);
    EXPECT_EQ(owner.memoryCounter_, 0);
    ASSERT_EQ(owner.frames_.size(), 2);
  }

  {
    void* buf;
    size_t lenReturn;
    parser.getReadBuffer(&buf, &lenReturn);
    static constexpr size_t kTestFrameLength = 20;

    HeaderSerializer serializer(static_cast<uint8_t*>(buf), lenReturn);
    serializer.writeFrameOrMetadataSize(kTestFrameLength);
    parser.readDataAvailable(Serializer::kBytesForFrameOrMetadataLength);
    parser.getReadBuffer(&buf, &lenReturn);
    std::string b(kTestFrameLength, 'b');
    memcpy(static_cast<uint8_t*>(buf), b.data(), kTestFrameLength);

    parser.readDataAvailable(kTestFrameLength);

    EXPECT_EQ(parser.getSize(), 0);
    EXPECT_EQ(parser.getFrameLength(), 0);
    EXPECT_EQ(owner.memoryCounter_, 0);
    ASSERT_EQ(owner.frames_.size(), 3);
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

  static constexpr size_t kTestFrameLength = 12;

  HeaderSerializer serializer(static_cast<uint8_t*>(buf), lenReturn);
  serializer.writeFrameOrMetadataSize(kTestFrameLength);
  parser.readDataAvailable(Serializer::kBytesForFrameOrMetadataLength);

  EXPECT_EQ(parser.getSize(), Serializer::kBytesForFrameOrMetadataLength);
  EXPECT_EQ(parser.getFrameLength(), kTestFrameLength);
  EXPECT_EQ(parser.getCurrentBufferSize(), parser.getMinBufferSize());
  EXPECT_EQ(
      owner.memoryCounter_,
      kTestFrameLength + Serializer::kBytesForFrameOrMetadataLength);
  ASSERT_EQ(owner.frames_.size(), 0);

  parser.getReadBuffer(&buf, &lenReturn);
  EXPECT_EQ(lenReturn, parser.getMinBufferSize() - 3);

  std::string b(kTestFrameLength, 'b');
  memcpy(static_cast<uint8_t*>(buf), b.data(), kTestFrameLength);

  // The follow call to readDataAvailable() should trigger parser to handover
  // buffer to owner via handleFrame call even current buffer is not full.
  parser.readDataAvailable(kTestFrameLength);

  EXPECT_EQ(parser.getSize(), 0);
  EXPECT_EQ(parser.getFrameLength(), 0);
  EXPECT_EQ(owner.memoryCounter_, 0);
  ASSERT_EQ(owner.frames_.size(), 1);

  const folly::IOBuf& frame = *owner.frames_[0];
  EXPECT_EQ(frame.length(), kTestFrameLength);

  parser.getReadBuffer(&buf, &lenReturn);
  EXPECT_EQ(lenReturn, 16); // a new buffer is allocated by parser

  // Not enough bytes to trigger frameLength computation.
  static_cast<uint8_t*>(buf)[0] = 0;
  parser.readDataAvailable(1);

  EXPECT_EQ(parser.getSize(), 1);
  EXPECT_EQ(parser.getFrameLength(), 0);
  EXPECT_EQ(owner.memoryCounter_, 0);
  ASSERT_EQ(owner.frames_.size(), 1);
}

TEST(AllocatingParserStrategyTest, testTinyFrame) {
  FakeOwner owner;
  AllocatingParserStrategy<FakeOwner> parser(owner, alloc);

  void* buf;
  size_t lenReturn;
  parser.getReadBuffer(&buf, &lenReturn);
  EXPECT_EQ(lenReturn, parser.getMinBufferSize());
  EXPECT_EQ(parser.getCurrentBufferSize(), parser.getMinBufferSize());

  static constexpr size_t kTestFrameLength = 8;

  HeaderSerializer serializer1(static_cast<uint8_t*>(buf), lenReturn);
  serializer1.writeFrameOrMetadataSize(kTestFrameLength);
  parser.readDataAvailable(Serializer::kBytesForFrameOrMetadataLength);

  EXPECT_EQ(parser.getSize(), Serializer::kBytesForFrameOrMetadataLength);
  EXPECT_EQ(parser.getFrameLength(), kTestFrameLength);
  EXPECT_EQ(parser.getCurrentBufferSize(), parser.getMinBufferSize());
  EXPECT_EQ(owner.memoryCounter_, kTestFrameLength + 3);
  ASSERT_EQ(owner.frames_.size(), 0);

  parser.getReadBuffer(&buf, &lenReturn);
  EXPECT_EQ(
      lenReturn,
      parser.getMinBufferSize() - Serializer::kBytesForFrameOrMetadataLength);

  std::string b(kTestFrameLength, 'b');
  memcpy(static_cast<uint8_t*>(buf), b.data(), kTestFrameLength);

  // The follow call to readDataAvailable() should trigger parser to handover
  // buffer to owner via handleFrame call even current buffer is not full.
  parser.readDataAvailable(kTestFrameLength);

  EXPECT_EQ(parser.getSize(), 0);
  EXPECT_EQ(parser.getFrameLength(), 0);
  EXPECT_EQ(owner.memoryCounter_, 0);
  ASSERT_EQ(owner.frames_.size(), 1);

  parser.getReadBuffer(&buf, &lenReturn);
  EXPECT_EQ(lenReturn, parser.getMinBufferSize());

  static constexpr size_t kNewTestFrameLength = 29;
  HeaderSerializer serializer2(static_cast<uint8_t*>(buf), lenReturn);
  serializer2.writeFrameOrMetadataSize(kNewTestFrameLength);

  // frameLength compututation should be triggered.q
  parser.readDataAvailable(Serializer::kBytesForFrameOrMetadataLength);

  EXPECT_EQ(parser.getSize(), Serializer::kBytesForFrameOrMetadataLength);
  EXPECT_EQ(parser.getFrameLength(), kNewTestFrameLength);
  EXPECT_EQ(
      parser.getCurrentBufferSize(),
      kNewTestFrameLength + Serializer::kBytesForFrameOrMetadataLength);
  EXPECT_EQ(
      owner.memoryCounter_,
      kNewTestFrameLength + Serializer::kBytesForFrameOrMetadataLength);
  ASSERT_EQ(owner.frames_.size(), 1);

  const folly::IOBuf& frame = *owner.frames_[0];
  EXPECT_EQ(frame.length(), kTestFrameLength);

  parser.getReadBuffer(&buf, &lenReturn);
  EXPECT_EQ(lenReturn, kNewTestFrameLength);
  std::string c(2, 'c');
  memcpy(static_cast<uint8_t*>(buf), b.data(), 2);
  parser.readDataAvailable(2);

  EXPECT_EQ(parser.getSize(), 2 + Serializer::kBytesForFrameOrMetadataLength);
  EXPECT_EQ(parser.getFrameLength(), kNewTestFrameLength);
  EXPECT_EQ(owner.memoryCounter_, kNewTestFrameLength + 3);
  ASSERT_EQ(owner.frames_.size(), 1);
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

  static constexpr size_t kTestFrameLen1 = 7;
  static constexpr size_t kTestFrameLen2 = 19;
  static constexpr size_t kTestFrameLen3 = 12;

  HeaderSerializer serializer1(static_cast<uint8_t*>(buf), lenReturn);
  serializer1.writeFrameOrMetadataSize(kTestFrameLen1);
  size_t bytesWritten = Serializer::kBytesForFrameOrMetadataLength;
  std::string t1(kTestFrameLen1, 'b');
  memcpy(&static_cast<uint8_t*>(buf)[bytesWritten], t1.data(), kTestFrameLen1);
  bytesWritten += kTestFrameLen1;

  HeaderSerializer serializer2(
      &static_cast<uint8_t*>(buf)[bytesWritten], lenReturn);
  serializer2.writeFrameOrMetadataSize(kTestFrameLen2);
  bytesWritten += Serializer::kBytesForFrameOrMetadataLength;
  std::string t2(kTestFrameLen2, 'c');
  memcpy(&static_cast<uint8_t*>(buf)[bytesWritten], t2.data(), kTestFrameLen2);
  bytesWritten += kTestFrameLen2;

  HeaderSerializer serializer3(
      &static_cast<uint8_t*>(buf)[bytesWritten], lenReturn);
  serializer3.writeFrameOrMetadataSize(kTestFrameLen3);
  bytesWritten += Serializer::kBytesForFrameOrMetadataLength;
  std::string t3(kTestFrameLen3, 'd');
  memcpy(&static_cast<uint8_t*>(buf)[bytesWritten], t3.data(), kTestFrameLen3);
  bytesWritten += kTestFrameLen3;

  parser.readDataAvailable(bytesWritten);

  EXPECT_EQ(parser.getSize(), 0);
  EXPECT_EQ(parser.getFrameLength(), 0);
  EXPECT_EQ(owner.memoryCounter_, 0);
  ASSERT_EQ(owner.frames_.size(), 3);

  const folly::IOBuf& frame1 = *owner.frames_[0];
  EXPECT_EQ(frame1.length(), kTestFrameLen1);
  const folly::IOBuf& frame2 = *owner.frames_[1];
  EXPECT_EQ(frame2.length(), kTestFrameLen2);
  const folly::IOBuf& frame3 = *owner.frames_[2];
  EXPECT_EQ(frame3.length(), kTestFrameLen3);
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

  static constexpr size_t kTestFrameLen1 = 7;
  static constexpr size_t kTestFrameLen2 = 19;
  static constexpr size_t kTestFrameLen3 = 12;
  static constexpr size_t kTestFrameLen4 = 183;

  HeaderSerializer serializer1(static_cast<uint8_t*>(buf), lenReturn);
  serializer1.writeFrameOrMetadataSize(kTestFrameLen1);
  size_t bytesWritten = Serializer::kBytesForFrameOrMetadataLength;
  std::string t1(kTestFrameLen1, 'b');
  memcpy(&static_cast<uint8_t*>(buf)[bytesWritten], t1.data(), kTestFrameLen1);
  bytesWritten += kTestFrameLen1;

  HeaderSerializer serializer2(
      &static_cast<uint8_t*>(buf)[bytesWritten], lenReturn - bytesWritten);
  serializer2.writeFrameOrMetadataSize(kTestFrameLen2);
  bytesWritten += Serializer::kBytesForFrameOrMetadataLength;
  std::string t2(kTestFrameLen2, 'c');
  memcpy(&static_cast<uint8_t*>(buf)[bytesWritten], t2.data(), kTestFrameLen2);
  bytesWritten += kTestFrameLen2;

  HeaderSerializer serializer3(
      &static_cast<uint8_t*>(buf)[bytesWritten], lenReturn - bytesWritten);
  serializer3.writeFrameOrMetadataSize(kTestFrameLen3);
  bytesWritten += Serializer::kBytesForFrameOrMetadataLength;
  std::string t3(kTestFrameLen3, 'd');
  memcpy(&static_cast<uint8_t*>(buf)[bytesWritten], t3.data(), kTestFrameLen3);
  bytesWritten += kTestFrameLen3;

  HeaderSerializer serializer4(
      &static_cast<uint8_t*>(buf)[bytesWritten], lenReturn - bytesWritten);
  serializer4.writeFrameOrMetadataSize(kTestFrameLen4);
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
  EXPECT_EQ(parser.getFrameLength(), kTestFrameLen4);
  EXPECT_EQ(
      owner.memoryCounter_,
      kTestFrameLen4 + Serializer::kBytesForFrameOrMetadataLength);
  ASSERT_EQ(owner.frames_.size(), 3);

  const folly::IOBuf& frame1 = *owner.frames_[0];
  EXPECT_EQ(frame1.length(), kTestFrameLen1);
  const folly::IOBuf& frame2 = *owner.frames_[1];
  EXPECT_EQ(frame2.length(), kTestFrameLen2);
  const folly::IOBuf& frame3 = *owner.frames_[2];
  EXPECT_EQ(frame3.length(), kTestFrameLen3);
}

#if FOLLY_HAS_MEMORY_RESOURCE
TEST(AllocatingParserStrategyTest, testWithAllocatorFromSharedPtr) {
  char poolBuf[10'000];
  folly::detail::std_pmr::monotonic_buffer_resource pool(
      poolBuf, sizeof(poolBuf));
  ParserAllocatorType alloc(&pool);
  FakeOwner owner;
  AllocatingParserStrategy<FakeOwner> parser(owner, alloc);

  void* buf;
  size_t lenReturn;
  parser.getReadBuffer(&buf, &lenReturn);
  EXPECT_EQ(lenReturn, parser.getMinBufferSize());
  EXPECT_EQ(parser.getCurrentBufferSize(), parser.getMinBufferSize());

  static constexpr size_t kTestFrameLength = 20;

  HeaderSerializer serializer(static_cast<uint8_t*>(buf), lenReturn);
  serializer.writeFrameOrMetadataSize(kTestFrameLength);
  parser.readDataAvailable(Serializer::kBytesForFrameOrMetadataLength);

  EXPECT_EQ(parser.getSize(), Serializer::kBytesForFrameOrMetadataLength);
  EXPECT_EQ(parser.getFrameLength(), kTestFrameLength);
  EXPECT_EQ(
      parser.getCurrentBufferSize(),
      kTestFrameLength + Serializer::kBytesForFrameOrMetadataLength);
  EXPECT_EQ(
      owner.memoryCounter_,
      kTestFrameLength + Serializer::kBytesForFrameOrMetadataLength);
  EXPECT_EQ(owner.frames_.size(), 0);

  parser.getReadBuffer(&buf, &lenReturn);
  EXPECT_EQ(lenReturn, kTestFrameLength);

  std::string b(kTestFrameLength, 'b');
  memcpy(static_cast<uint8_t*>(buf), b.data(), kTestFrameLength);

  parser.readDataAvailable(kTestFrameLength);

  EXPECT_EQ(parser.getSize(), 0);
  EXPECT_EQ(parser.getFrameLength(), 0);
  EXPECT_EQ(owner.memoryCounter_, 0);
  ASSERT_EQ(owner.frames_.size(), 1);

  const folly::IOBuf& frame = *owner.frames_[0];
  EXPECT_EQ(frame.length(), kTestFrameLength);
}
#endif

} // namespace rocket
} // namespace thrift
} // namespace apache
