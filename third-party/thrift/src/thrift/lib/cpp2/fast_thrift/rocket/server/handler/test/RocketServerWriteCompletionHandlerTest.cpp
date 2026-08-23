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

#include <thrift/lib/cpp2/fast_thrift/rocket/server/handler/RocketServerWriteCompletionHandler.h>

#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>
#include <thrift/lib/cpp2/fast_thrift/transport/WriteCompletion.h>

#include <gtest/gtest.h>

#include <cstdint>
#include <utility>
#include <vector>

namespace apache::thrift::fast_thrift::rocket::server::handler {
namespace {

namespace cp = apache::thrift::fast_thrift::channel_pipeline;
namespace transport = apache::thrift::fast_thrift::transport;

// Minimal Context — records what the handler fired upstream, tagged with the
// event id so the test can prove it re-fires under a different id than it
// subscribes to.
class CapturingContext {
 public:
  void fireEvent(RocketServerEventId ev, cp::TypeErasedBox box) noexcept {
    ids_.push_back(ev);
    events_.push_back(std::move(box).take<RocketWriteCompleteEvent>());
  }

  const std::vector<RocketServerEventId>& ids() const noexcept { return ids_; }
  const std::vector<RocketWriteCompleteEvent>& events() const noexcept {
    return events_;
  }

 private:
  std::vector<RocketServerEventId> ids_;
  std::vector<RocketWriteCompleteEvent> events_;
};

cp::TypeErasedBox frameWriteComplete(
    uint32_t streamId, transport::WriteCompletionStatus status) noexcept {
  return cp::TypeErasedBox(
      FrameWriteCompleteEvent{.streamId = streamId, .status = status});
}

} // namespace

TEST(
    RocketServerWriteCompletionHandlerTest, FrameCompletionBecomesRocketEvent) {
  RocketServerWriteCompletionHandler handler;
  CapturingContext ctx;

  handler.onEvent(
      ctx,
      RocketServerEventId::FrameWriteComplete,
      frameWriteComplete(7, transport::WriteCompletionStatus::Success));

  ASSERT_EQ(ctx.events().size(), 1u);
  EXPECT_EQ(ctx.events()[0].streamId, 7u);
  EXPECT_EQ(ctx.events()[0].status, transport::WriteCompletionStatus::Success);
  // Re-fired under a different id than it subscribes to, so the handler's own
  // output is never routed back into it.
  EXPECT_EQ(ctx.ids()[0], RocketServerEventId::RocketWriteComplete);
}

TEST(RocketServerWriteCompletionHandlerTest, ErrorStatusIsForwarded) {
  RocketServerWriteCompletionHandler handler;
  CapturingContext ctx;

  handler.onEvent(
      ctx,
      RocketServerEventId::FrameWriteComplete,
      frameWriteComplete(9, transport::WriteCompletionStatus::Error));

  ASSERT_EQ(ctx.events().size(), 1u);
  EXPECT_EQ(ctx.events()[0].streamId, 9u);
  EXPECT_EQ(ctx.events()[0].status, transport::WriteCompletionStatus::Error);
}

TEST(RocketServerWriteCompletionHandlerTest, EachFrameFiresItsOwnEvent) {
  RocketServerWriteCompletionHandler handler;
  CapturingContext ctx;

  // A batch that carried several frames arrives as several FrameWriteCompletes
  // (FragmentCompletionTracker already split it); the handler must not collapse
  // them.
  handler.onEvent(
      ctx,
      RocketServerEventId::FrameWriteComplete,
      frameWriteComplete(1, transport::WriteCompletionStatus::Success));
  handler.onEvent(
      ctx,
      RocketServerEventId::FrameWriteComplete,
      frameWriteComplete(3, transport::WriteCompletionStatus::Success));

  ASSERT_EQ(ctx.events().size(), 2u);
  EXPECT_EQ(ctx.events()[0].streamId, 1u);
  EXPECT_EQ(ctx.events()[1].streamId, 3u);
}

} // namespace apache::thrift::fast_thrift::rocket::server::handler
