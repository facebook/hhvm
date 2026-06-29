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

#include <thrift/lib/cpp2/fast_thrift/frame/handler/FrameWriteCompletionHandler.h>

#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>
#include <thrift/lib/cpp2/fast_thrift/frame/FrameType.h>
#include <thrift/lib/cpp2/fast_thrift/frame/read/FrameParser.h>
#include <thrift/lib/cpp2/fast_thrift/frame/read/ParsedFrame.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/ComposedFrame.h>
#include <thrift/lib/cpp2/fast_thrift/transport/WriteCompletion.h>

#include <gtest/gtest.h>
#include <folly/ExceptionWrapper.h>
#include <folly/io/IOBuf.h>

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <utility>
#include <vector>

namespace apache::thrift::fast_thrift::frame::handler {
namespace {

namespace cp = apache::thrift::fast_thrift::channel_pipeline;
using transport::WriteCompletionStatus;

// Minimal event factory: supplies the handler's event enum and message types
// without pulling in any concrete protocol (rocket) enum.
struct FakeEventFactory {
  enum class EventId : uint32_t { BatchWriteComplete, FrameWriteComplete };

  struct BatchEvent {
    WriteCompletionStatus status;
    size_t frameCount;
  };
  struct FrameEvent {
    uint32_t streamId;
    WriteCompletionStatus status;
  };

  using BatchWriteCompleteEventType = BatchEvent;
  // Read only via the handler's kSubscribedEvents, which the unit test never
  // accesses — mark to silence -Wunused-const-variable in this anon namespace.
  [[maybe_unused]] static constexpr EventId kBatchWriteCompleteEvent =
      EventId::BatchWriteComplete;

  static std::pair<EventId, cp::TypeErasedBox> makeFrameWriteComplete(
      WriteCompletionStatus status, uint32_t streamId) noexcept {
    return {
        EventId::FrameWriteComplete,
        cp::TypeErasedBox(FrameEvent{.streamId = streamId, .status = status})};
  }
};

using Handler = FrameWriteCompletionHandlerT<FakeEventFactory>;

// Minimal Context. fireWrite retains the message (mirroring the downstream
// codec taking ownership); fireEvent captures the FrameWriteComplete events the
// handler fans out; fireRead captures inbound frames passed through.
class MockContext {
 public:
  cp::Result fireWrite(cp::TypeErasedBox&& msg) noexcept {
    writes_.push_back(std::move(msg));
    return nextWriteResult_;
  }

  cp::Result fireRead(cp::TypeErasedBox&& msg) noexcept {
    reads_.push_back(std::move(msg));
    return cp::Result::Success;
  }

  void fireException(folly::exception_wrapper&& /*e*/) noexcept {
    ++exceptionCount_;
  }

  void fireEvent(FakeEventFactory::EventId ev, cp::TypeErasedBox box) noexcept {
    if (ev == FakeEventFactory::EventId::FrameWriteComplete) {
      frameFired_.push_back(box.get<FakeEventFactory::FrameEvent>());
    }
  }

  void setNextWriteResult(cp::Result r) noexcept { nextWriteResult_ = r; }
  const std::vector<FakeEventFactory::FrameEvent>& frameFired() const noexcept {
    return frameFired_;
  }
  int exceptionCount() const noexcept { return exceptionCount_; }

 private:
  cp::Result nextWriteResult_{cp::Result::Success};
  std::vector<cp::TypeErasedBox> writes_;
  std::vector<cp::TypeErasedBox> reads_;
  std::vector<FakeEventFactory::FrameEvent> frameFired_;
  int exceptionCount_{0};
};

// Boxes an outbound REQUEST_RESPONSE frame with an already-assigned streamId
// (the stream-state layer assigns it upstream before this handler).
cp::TypeErasedBox makeOutboundFrame(uint32_t streamId) {
  return cp::erase_and_box(
      ComposedFrame{
          .frameType = FrameType::REQUEST_RESPONSE,
          .streamId = streamId,
      });
}

// Serializes a minimal wire frame header so parseFrame yields a real
// ParsedFrame (streamId + type + flags), as the wire codec would deliver.
std::unique_ptr<folly::IOBuf> buildFrame(
    FrameType type, uint32_t streamId, uint16_t flags = 0) {
  const auto& desc = getDescriptor(type);
  size_t headerSize = desc.headerSize > 0 ? desc.headerSize : kBaseHeaderSize;

  auto buf = folly::IOBuf::create(headerSize);
  auto* data = buf->writableData();
  std::memset(data, 0, headerSize);

  data[0] = static_cast<uint8_t>((streamId >> 24) & 0xFF);
  data[1] = static_cast<uint8_t>((streamId >> 16) & 0xFF);
  data[2] = static_cast<uint8_t>((streamId >> 8) & 0xFF);
  data[3] = static_cast<uint8_t>(streamId & 0xFF);

  uint16_t typeAndFlags =
      (static_cast<uint16_t>(type) << detail::kFlagsBits) | flags;
  data[4] = static_cast<uint8_t>((typeAndFlags >> 8) & 0xFF);
  data[5] = static_cast<uint8_t>(typeAndFlags & 0xFF);

  buf->append(headerSize);
  return buf;
}

cp::TypeErasedBox makeInboundFrame(
    FrameType type, uint32_t streamId, uint16_t flags = 0) {
  return cp::erase_and_box(read::parseFrame(buildFrame(type, streamId, flags)));
}

cp::TypeErasedBox batchComplete(
    WriteCompletionStatus status, size_t frameCount) {
  return cp::TypeErasedBox(
      FakeEventFactory::BatchEvent{.status = status, .frameCount = frameCount});
}

} // namespace

// One BatchWriteComplete fans out into one FrameWriteComplete per outbound
// frame, in write order, carrying each frame's streamId.
TEST(FrameWriteCompletionHandlerTest, BatchFansOutPerFrameInOrder) {
  Handler handler;
  MockContext ctx;

  (void)handler.onWrite(ctx, makeOutboundFrame(1));
  (void)handler.onWrite(ctx, makeOutboundFrame(3));
  EXPECT_EQ(handler.pendingCount(), 2u);

  handler.onEvent(
      ctx,
      FakeEventFactory::EventId::BatchWriteComplete,
      batchComplete(WriteCompletionStatus::Success, 2));

  ASSERT_EQ(ctx.frameFired().size(), 2u);
  EXPECT_EQ(ctx.frameFired()[0].streamId, 1u);
  EXPECT_EQ(ctx.frameFired()[1].streamId, 3u);
  EXPECT_EQ(ctx.frameFired()[0].status, WriteCompletionStatus::Success);
  EXPECT_EQ(handler.pendingCount(), 0u);
}

// A write whose downstream result is Error records no FIFO entry (no phantom),
// so later completions stay aligned.
TEST(FrameWriteCompletionHandlerTest, SynchronousWriteErrorCreatesNoEntry) {
  Handler handler;
  MockContext ctx;
  ctx.setNextWriteResult(cp::Result::Error);

  (void)handler.onWrite(ctx, makeOutboundFrame(1));

  EXPECT_EQ(handler.pendingCount(), 0u);
  EXPECT_TRUE(ctx.frameFired().empty());
}

// A terminal inbound response fires the frame's completion early (Success — the
// response proves the write reached the wire) and tombstones the entry, so the
// later batch completion skips it (exactly-once).
TEST(
    FrameWriteCompletionHandlerTest,
    EarlyTerminalResponseFiresOnceAndTombstones) {
  Handler handler;
  MockContext ctx;

  (void)handler.onWrite(ctx, makeOutboundFrame(1));

  (void)handler.onRead(
      ctx, makeInboundFrame(FrameType::PAYLOAD, 1, detail::kCompleteBit));

  ASSERT_EQ(ctx.frameFired().size(), 1u);
  EXPECT_EQ(ctx.frameFired()[0].streamId, 1u);
  EXPECT_EQ(ctx.frameFired()[0].status, WriteCompletionStatus::Success);

  // The batch completion for that frame now arrives — the tombstoned entry is
  // popped but not re-fired.
  handler.onEvent(
      ctx,
      FakeEventFactory::EventId::BatchWriteComplete,
      batchComplete(WriteCompletionStatus::Success, 1));

  EXPECT_EQ(ctx.frameFired().size(), 1u);
  EXPECT_EQ(handler.pendingCount(), 0u);
}

// A non-terminal inbound frame does not complete its write; the entry stays
// live for its batch completion.
TEST(FrameWriteCompletionHandlerTest, NonTerminalResponseLeavesEntryLive) {
  Handler handler;
  MockContext ctx;

  (void)handler.onWrite(ctx, makeOutboundFrame(1));

  // PAYLOAD without the complete flag is non-terminal.
  (void)handler.onRead(ctx, makeInboundFrame(FrameType::PAYLOAD, 1, 0));

  EXPECT_TRUE(ctx.frameFired().empty());
  EXPECT_EQ(handler.pendingCount(), 1u);
}

// On graceful close, every still-live write fires FrameWriteComplete{Error};
// entries already completed early are skipped.
TEST(
    FrameWriteCompletionHandlerTest,
    PipelineInactiveDrainsLiveWritesSkippingTombstoned) {
  Handler handler;
  MockContext ctx;

  (void)handler.onWrite(ctx, makeOutboundFrame(1));
  (void)handler.onWrite(ctx, makeOutboundFrame(3));

  // Complete streamId 1 early; it must not be re-fired on drain.
  (void)handler.onRead(
      ctx, makeInboundFrame(FrameType::PAYLOAD, 1, detail::kCompleteBit));
  ASSERT_EQ(ctx.frameFired().size(), 1u);

  handler.onPipelineInactive(ctx);

  ASSERT_EQ(ctx.frameFired().size(), 2u);
  EXPECT_EQ(ctx.frameFired()[1].streamId, 3u);
  EXPECT_EQ(ctx.frameFired()[1].status, WriteCompletionStatus::Error);
  EXPECT_EQ(handler.pendingCount(), 0u);
}

// An inbound exception is forwarded downstream unchanged.
TEST(FrameWriteCompletionHandlerTest, ExceptionIsForwarded) {
  Handler handler;
  MockContext ctx;

  handler.onException(
      ctx, folly::make_exception_wrapper<std::runtime_error>("boom"));

  EXPECT_EQ(ctx.exceptionCount(), 1);
}

} // namespace apache::thrift::fast_thrift::frame::handler
