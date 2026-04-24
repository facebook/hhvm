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
#include <thrift/lib/cpp2/fast_thrift/frame/test/MockContext.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/handler/FrameLengthEncoderHandler.h>

namespace apache::thrift::fast_thrift::frame::write::handler {

using apache::thrift::fast_thrift::channel_pipeline::BytesPtr;
using apache::thrift::fast_thrift::channel_pipeline::erase_and_box;
using apache::thrift::fast_thrift::channel_pipeline::Result;
using apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox;
using test::MockContext;

namespace {

size_t readFrameLength(const uint8_t* buf) {
  return (static_cast<size_t>(buf[0]) << 16) |
      (static_cast<size_t>(buf[1]) << 8) | static_cast<size_t>(buf[2]);
}

} // namespace

class FrameLengthEncoderHandlerTest : public ::testing::Test {
 protected:
  static BytesPtr buildPayload(size_t size) {
    auto buf = folly::IOBuf::create(size);
    std::memset(buf->writableData(), 'x', size);
    buf->append(size);
    return buf;
  }

  Result callOnWrite(BytesPtr buf) {
    return handler_.onWrite(ctx_, erase_and_box(std::move(buf)));
  }

  MockContext ctx_;
  FrameLengthEncoderHandler handler_;
};

TEST_F(FrameLengthEncoderHandlerTest, SingleFrame) {
  auto result = callOnWrite(buildPayload(20));

  EXPECT_EQ(result, Result::Success);
  ASSERT_EQ(ctx_.writtenFrames().size(), 1);

  const auto& output = ctx_.writtenFrames()[0];
  auto totalLength = output->computeChainDataLength();
  EXPECT_EQ(totalLength, kMetadataLengthSize + 20);

  auto coalesced = output->coalesce();
  auto encodedLength = readFrameLength(coalesced.data());
  EXPECT_EQ(encodedLength, 20);
}

TEST_F(FrameLengthEncoderHandlerTest, EmptyFrame) {
  auto result = callOnWrite(buildPayload(0));

  EXPECT_EQ(result, Result::Success);
  ASSERT_EQ(ctx_.writtenFrames().size(), 1);

  const auto& output = ctx_.writtenFrames()[0];
  auto totalLength = output->computeChainDataLength();
  EXPECT_EQ(totalLength, kMetadataLengthSize);

  auto coalesced = output->coalesce();
  auto encodedLength = readFrameLength(coalesced.data());
  EXPECT_EQ(encodedLength, 0);
}

TEST_F(FrameLengthEncoderHandlerTest, LargeFrame) {
  constexpr size_t payloadSize = 65536;
  auto result = callOnWrite(buildPayload(payloadSize));

  EXPECT_EQ(result, Result::Success);
  ASSERT_EQ(ctx_.writtenFrames().size(), 1);

  const auto& output = ctx_.writtenFrames()[0];
  auto totalLength = output->computeChainDataLength();
  EXPECT_EQ(totalLength, kMetadataLengthSize + payloadSize);

  auto coalesced = output->coalesce();
  auto encodedLength = readFrameLength(coalesced.data());
  EXPECT_EQ(encodedLength, payloadSize);
}

TEST_F(FrameLengthEncoderHandlerTest, MaximumFrameLength) {
  constexpr size_t maxPayloadSize = (1 << 24) - 1;
  auto payload = folly::IOBuf::create(maxPayloadSize);
  payload->append(maxPayloadSize);

  auto result = callOnWrite(std::move(payload));

  EXPECT_EQ(result, Result::Success);
  ASSERT_EQ(ctx_.writtenFrames().size(), 1);

  const auto& output = ctx_.writtenFrames()[0];
  auto totalLength = output->computeChainDataLength();
  EXPECT_EQ(totalLength, kMetadataLengthSize + maxPayloadSize);

  auto coalesced = output->coalesce();
  auto encodedLength = readFrameLength(coalesced.data());
  EXPECT_EQ(encodedLength, maxPayloadSize);
}

TEST_F(FrameLengthEncoderHandlerTest, ChainedIOBuf) {
  auto chunk1 = folly::IOBuf::create(10);
  std::memset(chunk1->writableData(), 'a', 10);
  chunk1->append(10);

  auto chunk2 = folly::IOBuf::create(10);
  std::memset(chunk2->writableData(), 'b', 10);
  chunk2->append(10);

  chunk1->appendToChain(std::move(chunk2));
  EXPECT_EQ(chunk1->computeChainDataLength(), 20);

  auto result = callOnWrite(std::move(chunk1));

  EXPECT_EQ(result, Result::Success);
  ASSERT_EQ(ctx_.writtenFrames().size(), 1);

  const auto& output = ctx_.writtenFrames()[0];
  auto totalLength = output->computeChainDataLength();
  EXPECT_EQ(totalLength, kMetadataLengthSize + 20);

  auto coalesced = output->coalesce();
  auto encodedLength = readFrameLength(coalesced.data());
  EXPECT_EQ(encodedLength, 20);
}

TEST_F(FrameLengthEncoderHandlerTest, MultipleWrites) {
  for (size_t i = 0; i < 3; ++i) {
    auto result = callOnWrite(buildPayload(20));
    EXPECT_EQ(result, Result::Success);
    EXPECT_EQ(ctx_.writtenFrames().size(), i + 1);
  }

  for (const auto& output : ctx_.writtenFrames()) {
    auto coalesced = output->coalesce();
    auto encodedLength = readFrameLength(coalesced.data());
    EXPECT_EQ(encodedLength, 20);
  }
}

TEST_F(FrameLengthEncoderHandlerTest, VariablePayloadSizes) {
  std::vector<size_t> sizes = {1, 100, 1000, 10000};

  for (size_t size : sizes) {
    ctx_.reset();
    auto result = callOnWrite(buildPayload(size));

    EXPECT_EQ(result, Result::Success);
    ASSERT_EQ(ctx_.writtenFrames().size(), 1);

    const auto& output = ctx_.writtenFrames()[0];
    auto totalLength = output->computeChainDataLength();
    EXPECT_EQ(totalLength, kMetadataLengthSize + size);

    auto coalesced = output->coalesce();
    auto encodedLength = readFrameLength(coalesced.data());
    EXPECT_EQ(encodedLength, size);
  }
}

TEST_F(FrameLengthEncoderHandlerTest, ErrorPropagation) {
  ctx_.setReturnError(true);

  auto result = callOnWrite(buildPayload(20));

  EXPECT_EQ(result, Result::Error);
}

TEST_F(FrameLengthEncoderHandlerTest, BackpressurePropagation) {
  ctx_.setReturnBackpressure(true);

  auto result = callOnWrite(buildPayload(20));

  EXPECT_EQ(result, Result::Backpressure);
  EXPECT_EQ(ctx_.writtenFrames().size(), 1);
}

TEST_F(FrameLengthEncoderHandlerTest, PayloadDataPreserved) {
  auto payload = folly::IOBuf::create(5);
  uint8_t* data = payload->writableData();
  data[0] = 0x01;
  data[1] = 0x02;
  data[2] = 0x03;
  data[3] = 0x04;
  data[4] = 0x05;
  payload->append(5);

  auto result = callOnWrite(std::move(payload));

  EXPECT_EQ(result, Result::Success);
  ASSERT_EQ(ctx_.writtenFrames().size(), 1);

  const auto& output = ctx_.writtenFrames()[0];
  auto coalesced = output->coalesce();

  EXPECT_EQ(coalesced.size(), kMetadataLengthSize + 5);
  EXPECT_EQ(coalesced.data()[kMetadataLengthSize + 0], 0x01);
  EXPECT_EQ(coalesced.data()[kMetadataLengthSize + 1], 0x02);
  EXPECT_EQ(coalesced.data()[kMetadataLengthSize + 2], 0x03);
  EXPECT_EQ(coalesced.data()[kMetadataLengthSize + 3], 0x04);
  EXPECT_EQ(coalesced.data()[kMetadataLengthSize + 4], 0x05);
}

TEST_F(FrameLengthEncoderHandlerTest, HandlerLifecycle) {
  handler_.handlerAdded(ctx_);
  handler_.onPipelineActive(ctx_);
  handler_.onWriteReady(ctx_);

  auto result = callOnWrite(buildPayload(20));
  EXPECT_EQ(result, Result::Success);
  EXPECT_EQ(ctx_.writtenFrames().size(), 1);

  handler_.onPipelineInactive(ctx_);
  handler_.handlerRemoved(ctx_);
}

TEST_F(FrameLengthEncoderHandlerTest, HeadroomOptimization) {
  constexpr size_t payloadSize = 20;
  auto buf = folly::IOBuf::create(kMetadataLengthSize + payloadSize);
  buf->advance(kMetadataLengthSize);
  std::memset(buf->writableData(), 'y', payloadSize);
  buf->append(payloadSize);

  ASSERT_GE(buf->headroom(), kMetadataLengthSize);
  ASSERT_FALSE(buf->isSharedOne());

  auto result = callOnWrite(std::move(buf));

  EXPECT_EQ(result, Result::Success);
  ASSERT_EQ(ctx_.writtenFrames().size(), 1);

  const auto& output = ctx_.writtenFrames()[0];
  auto totalLength = output->computeChainDataLength();
  EXPECT_EQ(totalLength, kMetadataLengthSize + payloadSize);

  auto coalesced = output->coalesce();
  auto encodedLength = readFrameLength(coalesced.data());
  EXPECT_EQ(encodedLength, payloadSize);

  for (size_t i = 0; i < payloadSize; ++i) {
    EXPECT_EQ(coalesced.data()[kMetadataLengthSize + i], 'y');
  }
}

TEST_F(FrameLengthEncoderHandlerTest, NoHeadroomFallback) {
  constexpr size_t payloadSize = 20;
  auto buf = folly::IOBuf::create(payloadSize);
  std::memset(buf->writableData(), 'z', payloadSize);
  buf->append(payloadSize);

  ASSERT_LT(buf->headroom(), kMetadataLengthSize);

  auto result = callOnWrite(std::move(buf));

  EXPECT_EQ(result, Result::Success);
  ASSERT_EQ(ctx_.writtenFrames().size(), 1);

  const auto& output = ctx_.writtenFrames()[0];
  auto totalLength = output->computeChainDataLength();
  EXPECT_EQ(totalLength, kMetadataLengthSize + payloadSize);

  auto coalesced = output->coalesce();
  auto encodedLength = readFrameLength(coalesced.data());
  EXPECT_EQ(encodedLength, payloadSize);

  for (size_t i = 0; i < payloadSize; ++i) {
    EXPECT_EQ(coalesced.data()[kMetadataLengthSize + i], 'z');
  }
}

} // namespace apache::thrift::fast_thrift::frame::write::handler
