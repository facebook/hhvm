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

#include <folly/portability/GTest.h>

#include <folly/ExceptionWrapper.h>
#include <folly/Singleton.h>
#include <folly/io/IOBuf.h>
#include <folly/io/async/DelayedDestruction.h>
#include <thrift/lib/cpp2/transport/rocket/framing/Parser.h>

namespace apache {
namespace thrift {
namespace rocket {

class FakeOwner : public folly::DelayedDestruction {
 public:
  void handleFrame(std::unique_ptr<folly::IOBuf>) {}
  void close(folly::exception_wrapper) noexcept {}
  void scheduleTimeout(
      folly::HHWheelTimer::Callback*, const std::chrono::milliseconds&) {}
  bool incMemoryUsage(uint32_t) { return true; }
  void decMemoryUsage(uint32_t) {}
};

// TODO: This should be removed once the new buffer logic controlled by
// THRIFT_FLAG(rocket_parser_dont_hold_buffer_enabled) is stable.
TEST(ParserTest, resizeBufferTest) {
  FakeOwner owner;
  Parser<FakeOwner> parser(owner);
  parser.setReadBufferSize(Parser<FakeOwner>::kMaxBufferSize * 2);

  folly::IOBuf iobuf{folly::IOBuf::CreateOp(), parser.getReadBufferSize()};
  // pretend there is something written into the buffer
  iobuf.append(Parser<FakeOwner>::kMinBufferSize);

  parser.setReadBuffer(std::move(iobuf));
  parser.resizeBuffer();

  EXPECT_EQ(parser.getReadBufferSize(), Parser<FakeOwner>::kMaxBufferSize);
}

// TODO: This should be removed once the new buffer logic controlled by
// THRIFT_FLAG(rocket_parser_dont_hold_buffer_enabled) is stable.
TEST(ParserTest, noResizeBufferReadBufGtMaxTest) {
  FakeOwner owner;
  Parser<FakeOwner> parser(owner);
  parser.setReadBufferSize(Parser<FakeOwner>::kMaxBufferSize * 2);

  folly::IOBuf iobuf{folly::IOBuf::CreateOp(), parser.getReadBufferSize()};
  // pretend there is something written into the buffer, but with size > max
  iobuf.append(Parser<FakeOwner>::kMaxBufferSize * 1.5);

  parser.setReadBuffer(std::move(iobuf));
  parser.resizeBuffer();

  EXPECT_EQ(parser.getReadBufferSize(), Parser<FakeOwner>::kMaxBufferSize * 2);
}

// TODO: This should be removed once the new buffer logic controlled by
// THRIFT_FLAG(rocket_parser_dont_hold_buffer_enabled) is stable.
TEST(ParserTest, noResizeBufferReadBufEqMaxTest) {
  FakeOwner owner;
  Parser<FakeOwner> parser(owner);
  parser.setReadBufferSize(Parser<FakeOwner>::kMaxBufferSize * 2);

  folly::IOBuf iobuf{folly::IOBuf::CreateOp(), parser.getReadBufferSize()};
  // pretend there is something written into the buffer, but with size = max
  iobuf.append(Parser<FakeOwner>::kMaxBufferSize);

  parser.setReadBuffer(std::move(iobuf));
  parser.resizeBuffer();

  EXPECT_EQ(parser.getReadBufferSize(), Parser<FakeOwner>::kMaxBufferSize * 2);
}

// TODO: This should be removed once the new buffer logic controlled by
// THRIFT_FLAG(rocket_parser_dont_hold_buffer_enabled) is stable.
TEST(ParserTest, AlignmentTest) {
  std::string s = "1234567890";
  auto iobuf = folly::IOBuf::copyBuffer(s);
  auto res = alignTo4k(
      *iobuf, 4 /* 4 should be the first on 4k aligned address */, 1000);
  EXPECT_TRUE(res);
  EXPECT_EQ(reinterpret_cast<std::uintptr_t>(iobuf->data() + 4) % 4096u, 0);
  EXPECT_EQ(iobuf->length() + iobuf->tailroom(), 1000);
  EXPECT_EQ(folly::StringPiece(iobuf->coalesce()), s);

  iobuf = folly::IOBuf::copyBuffer(s);
  // it is possible the aligned part has not been received yet
  res = alignTo4k(*iobuf, 256, 1000);
  EXPECT_TRUE(res);
  EXPECT_EQ(reinterpret_cast<std::uintptr_t>(iobuf->data() + 256) % 4096u, 0);
  EXPECT_EQ(iobuf->length() + iobuf->tailroom(), 1000);
  EXPECT_EQ(folly::StringPiece(iobuf->coalesce()), s);

  iobuf = folly::IOBuf::copyBuffer(s);
  // it is also possible the frame is smaller than received data
  res = alignTo4k(*iobuf, 4, 7);
  EXPECT_EQ(reinterpret_cast<std::uintptr_t>(iobuf->data() + 4) % 4096u, 0);
  EXPECT_EQ(iobuf->length() + iobuf->tailroom(), 10);
  EXPECT_EQ(iobuf->tailroom(), 0);
  EXPECT_EQ(folly::StringPiece(iobuf->coalesce()), s);
}

TEST(ParserTest, AlignmentIOBufQueueTest) {
  std::string s = "1234567890";
  folly::IOBufQueue iobufQueue{folly::IOBufQueue::cacheChainLength()};
  iobufQueue.append(folly::IOBuf::copyBuffer(s));
  auto res = alignTo4kBufQueue(
      iobufQueue, 4 /* 4 should be the first on 4k aligned address */, 1000);
  EXPECT_TRUE(res);
  auto iobuf = iobufQueue.move();
  EXPECT_EQ(reinterpret_cast<std::uintptr_t>(iobuf->data() + 4) % 4096u, 0);
  EXPECT_EQ(iobuf->length() + iobuf->tailroom(), 1000);
  EXPECT_EQ(folly::StringPiece(iobuf->coalesce()), s);

  iobufQueue.reset();
  iobufQueue.append(folly::IOBuf::copyBuffer(s));
  // it is possible the aligned part has not been received yet
  res = alignTo4kBufQueue(iobufQueue, 256, 1000);
  EXPECT_TRUE(res);
  iobuf = iobufQueue.move();
  EXPECT_EQ(reinterpret_cast<std::uintptr_t>(iobuf->data() + 256) % 4096u, 0);
  EXPECT_EQ(iobuf->length() + iobuf->tailroom(), 1000);
  EXPECT_EQ(folly::StringPiece(iobuf->coalesce()), s);

  iobufQueue.reset();
  iobufQueue.append(folly::IOBuf::copyBuffer(s));
  // it is also possible the frame is smaller than received data
  res = alignTo4kBufQueue(iobufQueue, 4, 7);
  EXPECT_TRUE(res);
  iobuf = iobufQueue.move();
  EXPECT_EQ(reinterpret_cast<std::uintptr_t>(iobuf->data() + 4) % 4096u, 0);
  EXPECT_EQ(iobuf->length() + iobuf->tailroom(), 10);
  EXPECT_EQ(iobuf->tailroom(), 0);
  EXPECT_EQ(folly::StringPiece(iobuf->coalesce()), s);
}

TEST(ParserTest, BufferIsChainedAlignmentTest) {
  std::string s = "1234567890";
  auto iobuf = folly::IOBuf::copyBuffer(s);
  auto iobuf2 = folly::IOBuf::copyBuffer(s);
  iobuf->appendChain(std::move(iobuf2));
  EXPECT_TRUE(iobuf->isChained());

  folly::IOBufQueue iobufQueue{folly::IOBufQueue::cacheChainLength()};
  iobufQueue.append(std::move(iobuf));
  auto res = alignTo4kBufQueue(
      iobufQueue, 4 /* 4 should be the first on 4k aligned address */, 1000);
  EXPECT_TRUE(res);
  auto buf = iobufQueue.move();
  EXPECT_EQ(reinterpret_cast<std::uintptr_t>(buf->data() + 4) % 4096u, 0);
  EXPECT_EQ(buf->length() + buf->tailroom(), 1000);
  EXPECT_EQ(folly::StringPiece(buf->coalesce()), s + s);
}

TEST(ParserTest, PageAlignedBufferTest) {
  // total number of (usable) bytes to be allocated
  constexpr size_t kNumBytes = 32;
  // portion of kNumBytes to be placed before page boundary
  constexpr size_t kStartOffset = 13;
  // final length of the iobuf (must be <= kNumBytes)
  size_t trimLength = 0;
  auto buf = get4kAlignedBuf(kNumBytes, kStartOffset, trimLength);
  ASSERT_NE(nullptr, buf.get());
  EXPECT_EQ(
      reinterpret_cast<std::uintptr_t>(buf->data() + kStartOffset) % 4096u, 0);
  EXPECT_EQ(trimLength, buf->length());

  trimLength = kStartOffset;
  buf = get4kAlignedBuf(kNumBytes, kStartOffset, trimLength);
  ASSERT_NE(nullptr, buf.get());
  EXPECT_EQ(
      reinterpret_cast<std::uintptr_t>(buf->data() + kStartOffset) % 4096u, 0);
  EXPECT_EQ(trimLength, buf->length());
}

} // namespace rocket
} // namespace thrift
} // namespace apache

int main(int argc, char** argv) {
  // Enable glog logging to stderr by default.
  FLAGS_logtostderr = true;

  ::testing::InitGoogleTest(&argc, argv);
  folly::SingletonVault::singleton()->registrationComplete();

  return RUN_ALL_TESTS();
}
