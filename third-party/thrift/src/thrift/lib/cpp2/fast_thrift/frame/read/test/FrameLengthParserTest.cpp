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

#include <cstring>
#include <vector>

#include <gtest/gtest.h>

#include <folly/io/IOBuf.h>
#include <folly/io/IOBufQueue.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/frame/FrameType.h>
#include <thrift/lib/cpp2/fast_thrift/frame/read/FrameLengthParser.h>

namespace apache::thrift::fast_thrift::frame::read {

using apache::thrift::fast_thrift::channel_pipeline::BytesPtr;
using apache::thrift::fast_thrift::channel_pipeline::Result;

namespace {

void writeFrameLength(uint8_t* buf, size_t length) {
  buf[0] = static_cast<uint8_t>((length >> 16) & 0xFF);
  buf[1] = static_cast<uint8_t>((length >> 8) & 0xFF);
  buf[2] = static_cast<uint8_t>(length & 0xFF);
}

} // namespace

class FrameLengthParserTest : public ::testing::Test {
 protected:
  static BytesPtr buildFrame(size_t payloadSize) {
    auto buf = folly::IOBuf::create(kMetadataLengthSize + payloadSize);
    writeFrameLength(buf->writableData(), payloadSize);
    std::memset(buf->writableData() + kMetadataLengthSize, 'x', payloadSize);
    buf->append(kMetadataLengthSize + payloadSize);
    return buf;
  }

  static BytesPtr buildHeader(size_t payloadSize) {
    auto buf = folly::IOBuf::create(kMetadataLengthSize);
    writeFrameLength(buf->writableData(), payloadSize);
    buf->append(kMetadataLengthSize);
    return buf;
  }

  static BytesPtr buildPayload(size_t size) {
    auto buf = folly::IOBuf::create(size);
    std::memset(buf->writableData(), 'x', size);
    buf->append(size);
    return buf;
  }

  // Collects emitted frames and replays whatever Result the test asked for.
  auto sink() noexcept {
    return [this](BytesPtr&& frame) noexcept {
      frames_.push_back(std::move(frame));
      return sinkResult_;
    };
  }

  Result feed(BytesPtr buf) {
    return parser_.consumeBuffer(std::move(buf), sink());
  }

  // Drives the socket-facing path: ask for a buffer, fill it, hand back the
  // length, exactly as AsyncSocket would.
  Result feedViaReadBuffer(const BytesPtr& bytes) {
    auto len = bytes->computeChainDataLength();
    void* buf = nullptr;
    size_t avail = 0;
    parser_.getReadBuffer(&buf, &avail);
    EXPECT_GE(avail, len);
    size_t offset = 0;
    for (const auto& range : *bytes) {
      std::memcpy(
          static_cast<uint8_t*>(buf) + offset, range.data(), range.size());
      offset += range.size();
    }
    return parser_.consume(len, sink());
  }

  std::vector<BytesPtr> frames_;
  Result sinkResult_{Result::Success};
  FrameLengthParser parser_;
};

TEST_F(FrameLengthParserTest, SingleCompleteFrame) {
  EXPECT_EQ(feed(buildFrame(20)), Result::Success);
  EXPECT_EQ(parser_.size(), 0);
  EXPECT_EQ(parser_.frameLength(), 0);
  EXPECT_EQ(parser_.frameLengthAndFieldSize(), 0);
  ASSERT_EQ(frames_.size(), 1);
  EXPECT_EQ(frames_[0]->computeChainDataLength(), 20);
}

TEST_F(FrameLengthParserTest, PartialHeader) {
  auto buf = folly::IOBuf::create(2);
  buf->writableData()[0] = 0x00;
  buf->writableData()[1] = 0x00;
  buf->append(2);

  EXPECT_EQ(feed(std::move(buf)), Result::Success);
  EXPECT_EQ(parser_.size(), 2);
  EXPECT_EQ(parser_.frameLength(), 0);
  EXPECT_EQ(frames_.size(), 0);
}

TEST_F(FrameLengthParserTest, HeaderThenPayload) {
  EXPECT_EQ(feed(buildHeader(20)), Result::Success);
  EXPECT_EQ(parser_.size(), 3);
  EXPECT_EQ(parser_.frameLength(), 20);
  EXPECT_EQ(parser_.frameLengthAndFieldSize(), 23);
  EXPECT_EQ(frames_.size(), 0);

  EXPECT_EQ(feed(buildPayload(20)), Result::Success);
  EXPECT_EQ(parser_.size(), 0);
  EXPECT_EQ(parser_.frameLength(), 0);
  ASSERT_EQ(frames_.size(), 1);
  EXPECT_EQ(frames_[0]->computeChainDataLength(), 20);
}

TEST_F(FrameLengthParserTest, MultipleFramesInOneBuffer) {
  folly::IOBufQueue queue{folly::IOBufQueue::cacheChainLength()};
  for (int i = 0; i < 3; ++i) {
    queue.append(buildFrame(20));
  }

  EXPECT_EQ(feed(queue.move()), Result::Success);
  EXPECT_EQ(parser_.size(), 0);
  ASSERT_EQ(frames_.size(), 3);
  for (const auto& frame : frames_) {
    EXPECT_EQ(frame->computeChainDataLength(), 20);
  }
}

TEST_F(FrameLengthParserTest, MultipleFramesSeparately) {
  for (int i = 0; i < 3; ++i) {
    EXPECT_EQ(feed(buildFrame(20)), Result::Success);
    EXPECT_EQ(parser_.size(), 0);
    EXPECT_EQ(frames_.size(), static_cast<size_t>(i + 1));
  }
}

TEST_F(FrameLengthParserTest, ChainedIOBuf) {
  auto header = buildHeader(20);
  header->appendToChain(buildPayload(20));
  EXPECT_EQ(header->computeChainDataLength(), 23);

  EXPECT_EQ(feed(std::move(header)), Result::Success);
  EXPECT_EQ(parser_.size(), 0);
  ASSERT_EQ(frames_.size(), 1);
  EXPECT_EQ(frames_[0]->computeChainDataLength(), 20);
}

TEST_F(FrameLengthParserTest, BackpressureStopsProcessing) {
  sinkResult_ = Result::Backpressure;

  folly::IOBufQueue queue{folly::IOBufQueue::cacheChainLength()};
  for (int i = 0; i < 3; ++i) {
    queue.append(buildFrame(20));
  }

  EXPECT_EQ(feed(queue.move()), Result::Backpressure);
  // Backpressure means "accepted, but slow down": the first frame landed.
  ASSERT_EQ(frames_.size(), 1);
  EXPECT_EQ(frames_[0]->computeChainDataLength(), 20);
  EXPECT_GT(parser_.size(), 0);
}

TEST_F(FrameLengthParserTest, ErrorStopsProcessing) {
  sinkResult_ = Result::Error;

  EXPECT_EQ(feed(buildFrame(20)), Result::Error);
  // The frame was handed over before the sink refused it.
  EXPECT_EQ(frames_.size(), 1);
}

TEST_F(FrameLengthParserTest, EmptyFrame) {
  EXPECT_EQ(feed(buildFrame(0)), Result::Success);
  ASSERT_EQ(frames_.size(), 1);
  EXPECT_EQ(frames_[0]->computeChainDataLength(), 0);
}

TEST_F(FrameLengthParserTest, LargeFrame) {
  constexpr size_t kFrameSize = 65536;

  EXPECT_EQ(feed(buildFrame(kFrameSize)), Result::Success);
  ASSERT_EQ(frames_.size(), 1);
  EXPECT_EQ(frames_[0]->computeChainDataLength(), kFrameSize);
}

TEST_F(FrameLengthParserTest, IncrementalLargeFrame) {
  constexpr size_t kFrameSize = 65536;
  constexpr size_t kChunkSize = 4096;

  EXPECT_EQ(feed(buildHeader(kFrameSize)), Result::Success);
  EXPECT_EQ(parser_.frameLength(), kFrameSize);
  EXPECT_EQ(frames_.size(), 0);

  for (size_t remaining = kFrameSize; remaining > 0;) {
    size_t toSend = std::min(kChunkSize, remaining);
    EXPECT_EQ(feed(buildPayload(toSend)), Result::Success);
    remaining -= toSend;
  }

  ASSERT_EQ(frames_.size(), 1);
  EXPECT_EQ(frames_[0]->computeChainDataLength(), kFrameSize);
}

TEST_F(FrameLengthParserTest, BackpressureThenResume) {
  sinkResult_ = Result::Backpressure;

  folly::IOBufQueue queue{folly::IOBufQueue::cacheChainLength()};
  queue.append(buildFrame(20));
  queue.append(buildFrame(30));

  EXPECT_EQ(feed(queue.move()), Result::Backpressure);
  ASSERT_EQ(frames_.size(), 1);
  EXPECT_EQ(frames_[0]->computeChainDataLength(), 20);
  EXPECT_GT(parser_.size(), 0);

  sinkResult_ = Result::Success;
  EXPECT_EQ(feed(buildFrame(40)), Result::Success);

  ASSERT_EQ(frames_.size(), 3);
  EXPECT_EQ(frames_[1]->computeChainDataLength(), 30);
  EXPECT_EQ(frames_[2]->computeChainDataLength(), 40);
  EXPECT_EQ(parser_.size(), 0);
}

TEST_F(FrameLengthParserTest, ResetDropsBufferedState) {
  EXPECT_EQ(feed(buildHeader(20)), Result::Success);
  EXPECT_GT(parser_.size(), 0);

  parser_.reset();

  EXPECT_EQ(parser_.size(), 0);
  EXPECT_EQ(parser_.frameLength(), 0);
  EXPECT_EQ(parser_.frameLengthAndFieldSize(), 0);
}

// --- Socket-facing buffer management ---

TEST_F(FrameLengthParserTest, ReadBufferIsReusedWhileTailroomRemains) {
  void* first = nullptr;
  size_t len = 0;
  parser_.getReadBuffer(&first, &len);
  ASSERT_NE(first, nullptr);
  EXPECT_GE(len, FrameLengthParser::kDefaultMaxBufferSize);

  // Nothing consumed, so the same tail must come back rather than a new
  // allocation — this reuse is the whole point of the parser owning the buffer.
  void* second = nullptr;
  parser_.getReadBuffer(&second, &len);
  EXPECT_EQ(second, first);
}

TEST_F(FrameLengthParserTest, ReadBufferAdvancesWhileAFrameIsIncomplete) {
  // A frame split across reads is the case reuse matters for: the partial
  // bytes stay put and the next read continues into the same allocation
  // instead of allocating again mid-frame.
  void* first = nullptr;
  size_t len = 0;
  parser_.getReadBuffer(&first, &len);

  EXPECT_EQ(feedViaReadBuffer(buildHeader(20)), Result::Success);
  EXPECT_EQ(frames_.size(), 0);
  EXPECT_EQ(parser_.frameLength(), 20);

  void* second = nullptr;
  parser_.getReadBuffer(&second, &len);
  EXPECT_EQ(second, static_cast<uint8_t*>(first) + kMetadataLengthSize);

  EXPECT_EQ(feedViaReadBuffer(buildPayload(20)), Result::Success);
  ASSERT_EQ(frames_.size(), 1);
  EXPECT_EQ(frames_[0]->computeChainDataLength(), 20);
}

TEST_F(FrameLengthParserTest, DoesNotPreallocateAnAnnouncedLargeFrame) {
  constexpr size_t kFrameSize = 512 * 1024;
  ASSERT_GT(kFrameSize, FrameLengthParser::kDefaultMaxBufferSize);

  EXPECT_EQ(feedViaReadBuffer(buildHeader(kFrameSize)), Result::Success);
  EXPECT_EQ(parser_.frameLength(), kFrameSize);

  // A peer can announce any length up to maxFrameSize, so the announcement
  // alone must not size the buffer — the payload accumulates across reads as
  // it actually arrives. Otherwise a silent peer pins maxFrameSize per
  // connection just by sending a 3-byte prefix.
  void* buf = nullptr;
  size_t len = 0;
  parser_.getReadBuffer(&buf, &len);
  EXPECT_LE(len, FrameLengthParser::kDefaultMaxBufferSize);
}

TEST_F(FrameLengthParserTest, RejectsFrameOverMaxFrameSize) {
  FrameLengthParser parser{
      FrameLengthParser::kDefaultMinBufferSize,
      FrameLengthParser::kDefaultMaxBufferSize,
      /*maxFrameSize=*/1024};

  auto result = parser.consumeBuffer(buildHeader(2048), sink());

  EXPECT_EQ(result, Result::Error);
  EXPECT_EQ(frames_.size(), 0);
}

TEST_F(FrameLengthParserTest, AllocationsGoThroughTheInstalledFactory) {
  size_t allocations = 0;
  folly::IOBufFactory factory = [&allocations](size_t capacity) {
    ++allocations;
    return folly::IOBuf::create(capacity);
  };
  parser_.setIOBufFactory(&factory);

  void* buf = nullptr;
  size_t len = 0;
  parser_.getReadBuffer(&buf, &len);

  EXPECT_EQ(allocations, 1);
}

} // namespace apache::thrift::fast_thrift::frame::read
