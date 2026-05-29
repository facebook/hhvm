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
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>
#include <thrift/lib/cpp2/fast_thrift/frame/FrameType.h>
#include <thrift/lib/cpp2/fast_thrift/frame/read/handler/FrameLengthParserHandler.h>
#include <thrift/lib/cpp2/fast_thrift/frame/test/MockContext.h>

namespace apache::thrift::fast_thrift::frame::read::handler {

using apache::thrift::fast_thrift::channel_pipeline::BytesPtr;
using apache::thrift::fast_thrift::channel_pipeline::erase_and_box;
using apache::thrift::fast_thrift::channel_pipeline::Result;
using apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox;
using test::MockContext;

namespace {

// Write 3-byte big-endian frame length
void writeFrameLength(uint8_t* buf, size_t length) {
  buf[0] = static_cast<uint8_t>((length >> 16) & 0xFF);
  buf[1] = static_cast<uint8_t>((length >> 8) & 0xFF);
  buf[2] = static_cast<uint8_t>(length & 0xFF);
}

} // namespace

class FrameLengthParserHandlerTest : public ::testing::Test {
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

  Result callOnRead(BytesPtr buf) {
    return handler_.onRead(ctx_, erase_and_box(std::move(buf)));
  }

  MockContext ctx_;
  FrameLengthParserHandler handler_;
};

TEST_F(FrameLengthParserHandlerTest, SingleCompleteFrame) {
  auto result = callOnRead(buildFrame(20));

  EXPECT_EQ(result, Result::Success);
  EXPECT_EQ(handler_.size(), 0);
  EXPECT_EQ(handler_.frameLength(), 0);
  EXPECT_EQ(handler_.frameLengthAndFieldSize(), 0);
  EXPECT_EQ(ctx_.frames().size(), 1);
  EXPECT_EQ(ctx_.frames()[0]->length(), 20);
}

TEST_F(FrameLengthParserHandlerTest, PartialHeader) {
  // Send only 2 bytes of header (need 3)
  auto buf = folly::IOBuf::create(2);
  buf->writableData()[0] = 0x00;
  buf->writableData()[1] = 0x00;
  buf->append(2);

  auto result = callOnRead(std::move(buf));

  EXPECT_EQ(result, Result::Success);
  EXPECT_EQ(handler_.size(), 2);
  EXPECT_EQ(handler_.frameLength(), 0);
  EXPECT_EQ(ctx_.frames().size(), 0);
}

TEST_F(FrameLengthParserHandlerTest, HeaderThenPayload) {
  auto result = callOnRead(buildHeader(20));

  EXPECT_EQ(result, Result::Success);
  EXPECT_EQ(handler_.size(), 3);
  EXPECT_EQ(handler_.frameLength(), 20);
  EXPECT_EQ(handler_.frameLengthAndFieldSize(), 23);
  EXPECT_EQ(ctx_.frames().size(), 0);

  result = callOnRead(buildPayload(20));

  EXPECT_EQ(result, Result::Success);
  EXPECT_EQ(handler_.size(), 0);
  EXPECT_EQ(handler_.frameLength(), 0);
  EXPECT_EQ(ctx_.frames().size(), 1);
  EXPECT_EQ(ctx_.frames()[0]->length(), 20);
}

TEST_F(FrameLengthParserHandlerTest, MultipleFramesInOneBuffer) {
  folly::IOBufQueue queue{folly::IOBufQueue::cacheChainLength()};
  for (int i = 0; i < 3; ++i) {
    queue.append(buildFrame(20));
  }

  auto result = callOnRead(queue.move());

  EXPECT_EQ(result, Result::Success);
  EXPECT_EQ(handler_.size(), 0);
  EXPECT_EQ(ctx_.frames().size(), 3);
  for (const auto& frame : ctx_.frames()) {
    EXPECT_EQ(frame->length(), 20);
  }
}

TEST_F(FrameLengthParserHandlerTest, MultipleFramesSeparately) {
  for (int i = 0; i < 3; ++i) {
    auto result = callOnRead(buildFrame(20));

    EXPECT_EQ(result, Result::Success);
    EXPECT_EQ(handler_.size(), 0);
    EXPECT_EQ(ctx_.frames().size(), i + 1);
  }
}

TEST_F(FrameLengthParserHandlerTest, ChainedIOBuf) {
  // Create a chained IOBuf: header in one, payload in another
  auto header = buildHeader(20);
  auto payload = buildPayload(20);
  header->appendToChain(std::move(payload));

  EXPECT_EQ(header->computeChainDataLength(), 23);

  auto result = callOnRead(std::move(header));

  EXPECT_EQ(result, Result::Success);
  EXPECT_EQ(handler_.size(), 0);
  EXPECT_EQ(ctx_.frames().size(), 1);
  EXPECT_EQ(ctx_.frames()[0]->computeChainDataLength(), 20);
}

TEST_F(FrameLengthParserHandlerTest, OnException) {
  auto ex = folly::make_exception_wrapper<std::runtime_error>("test error");
  handler_.onException(ctx_, ex);

  EXPECT_TRUE(ctx_.hasException());
}

TEST_F(FrameLengthParserHandlerTest, BackpressureStopsProcessing) {
  ctx_.setReturnBackpressure(true);

  folly::IOBufQueue queue{folly::IOBufQueue::cacheChainLength()};
  for (int i = 0; i < 3; ++i) {
    queue.append(buildFrame(20));
  }

  auto result = callOnRead(queue.move());

  EXPECT_EQ(result, Result::Backpressure);
  // First frame was accepted (Backpressure means "accepted but slow down")
  EXPECT_EQ(ctx_.frames().size(), 1);
  EXPECT_EQ(ctx_.frames()[0]->length(), 20);
  // Remaining 2 frames should still be buffered
  EXPECT_GT(handler_.size(), 0);
}

TEST_F(FrameLengthParserHandlerTest, ErrorStopsProcessing) {
  ctx_.setReturnError(true);

  auto result = callOnRead(buildFrame(20));

  EXPECT_EQ(result, Result::Error);
  EXPECT_EQ(ctx_.frames().size(), 0);
}

TEST_F(FrameLengthParserHandlerTest, EmptyFrame) {
  auto result = callOnRead(buildFrame(0));

  EXPECT_EQ(result, Result::Success);
  EXPECT_EQ(ctx_.frames().size(), 1);
  EXPECT_EQ(ctx_.frames()[0]->length(), 0);
}

TEST_F(FrameLengthParserHandlerTest, LargeFrame) {
  constexpr size_t frameSize = 65536;

  auto result = callOnRead(buildFrame(frameSize));

  EXPECT_EQ(result, Result::Success);
  EXPECT_EQ(ctx_.frames().size(), 1);
  EXPECT_EQ(ctx_.frames()[0]->computeChainDataLength(), frameSize);
}

TEST_F(FrameLengthParserHandlerTest, IncrementalLargeFrame) {
  constexpr size_t frameSize = 65536;

  auto result = callOnRead(buildHeader(frameSize));

  EXPECT_EQ(result, Result::Success);
  EXPECT_EQ(handler_.frameLength(), frameSize);
  EXPECT_EQ(ctx_.frames().size(), 0);

  size_t remaining = frameSize;
  constexpr size_t chunkSize = 4096;
  while (remaining > 0) {
    size_t toSend = std::min(chunkSize, remaining);
    result = callOnRead(buildPayload(toSend));
    EXPECT_EQ(result, Result::Success);
    remaining -= toSend;
  }

  EXPECT_EQ(ctx_.frames().size(), 1);
  EXPECT_EQ(ctx_.frames()[0]->computeChainDataLength(), frameSize);
}

TEST_F(FrameLengthParserHandlerTest, HandlerLifecycle) {
  EXPECT_EQ(callOnRead(buildHeader(20)), Result::Success);
  EXPECT_GT(handler_.size(), 0);

  handler_.handlerRemoved(ctx_);

  EXPECT_EQ(handler_.size(), 0);
  EXPECT_EQ(handler_.frameLength(), 0);
  EXPECT_EQ(handler_.frameLengthAndFieldSize(), 0);
}

TEST_F(FrameLengthParserHandlerTest, BackpressureThenResume) {
  // Send 2 frames in one buffer with backpressure enabled
  ctx_.setReturnBackpressure(true);

  folly::IOBufQueue queue{folly::IOBufQueue::cacheChainLength()};
  queue.append(buildFrame(20));
  queue.append(buildFrame(30));

  auto result = callOnRead(queue.move());

  // First frame extracted and accepted, but backpressure returned
  EXPECT_EQ(result, Result::Backpressure);
  EXPECT_EQ(ctx_.frames().size(), 1);
  EXPECT_EQ(ctx_.frames()[0]->length(), 20);
  // Second frame still buffered
  EXPECT_GT(handler_.size(), 0);

  // Clear backpressure and trigger processing of remaining data
  ctx_.setReturnBackpressure(false);
  result = callOnRead(buildFrame(40));

  EXPECT_EQ(result, Result::Success);
  // All frames should now be delivered
  EXPECT_EQ(ctx_.frames().size(), 3);
  EXPECT_EQ(ctx_.frames()[0]->length(), 20);
  EXPECT_EQ(ctx_.frames()[1]->length(), 30);
  EXPECT_EQ(ctx_.frames()[2]->length(), 40);
  EXPECT_EQ(handler_.size(), 0);
}

} // namespace apache::thrift::fast_thrift::frame::read::handler
