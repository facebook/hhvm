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

#pragma once

#include <cstddef>
#include <deque>
#include <utility>

#include <folly/Portability.h>

#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Backpressure.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/common/Messages.h>

namespace apache::thrift::fast_thrift::thrift {

/**
 * WriteBufferBackpressureHandler — duplex pipeline handler that buffers
 * outbound responses when the downstream pipeline returns
 * `Result::Backpressure`, then drains them in FIFO order on
 * `onWriteReady()`.
 *
 *   ThriftServerTransportAdapter (head)
 *     → [thrift handlers] → WriteBufferBackpressureHandler → tail adapter
 *
 * Without this handler, `Backpressure` propagated all the way to codegen
 * where it was silently discarded — responses would keep slamming into a
 * full pipeline. This handler is the per-connection buffering point that
 * absorbs the signal:
 *
 *   - `onWrite`: if the buffer flag is set, enqueue and surface `Success`
 *     (the response is logically accepted — it will go out on
 *     `onWriteReady`). Otherwise fire downstream; if downstream returns
 *     `Backpressure`, arm the flag for subsequent writes.
 *   - `onWriteReady`: clear the flag and drain the FIFO via
 *     `ctx.fireWrite`. If downstream re-asserts `Backpressure` mid-drain,
 *     re-arm the flag and leave the remainder for the next cycle.
 *   - `onPipelineInactive`: clear the deque and reset the flag — once the
 *     wire is gone, buffered responses can't go out anyway.
 *
 * The handler does no in-flight bookkeeping. It pairs cleanly with
 * `GracefulDrainHandler` (which tracks in-flight independently): on
 * graceful drain, `ctx.pipeline()->deactivate()` triggers
 * `onPipelineInactive` here and the buffer is dropped.
 */
template <typename Context>
class WriteBufferBackpressureHandler {
 public:
  WriteBufferBackpressureHandler() = default;

  // Detected by makeHandlerNode — registers this handler in the
  // pipeline's writeReadyList so onWriteReady() actually fires.
  channel_pipeline::WriteReadyHook writeReadyHook_;

  // HandlerLifecycle
  void handlerAdded(Context& /*ctx*/) noexcept {}
  void handlerRemoved(Context& /*ctx*/) noexcept {}
  void onPipelineActive(Context& /*ctx*/) noexcept {}
  void onReadReady(Context& /*ctx*/) noexcept {}

  // Inbound: forward, but signal Backpressure upstream as long as
  // outbound is saturated. The transport pauses reads to avoid
  // accumulating in-flight requests whose responses can't be drained.
  // Error from downstream is terminal and not overridden.
  channel_pipeline::Result onRead(
      Context& ctx, channel_pipeline::TypeErasedBox&& msg) noexcept {
    auto result = ctx.fireRead(std::move(msg));
    if (FOLLY_UNLIKELY(
            backpressured_ && result != channel_pipeline::Result::Error)) {
      return channel_pipeline::Result::Backpressure;
    }
    return result;
  }

  void onException(Context& ctx, folly::exception_wrapper&& e) noexcept {
    ctx.fireException(std::move(e));
  }

  // Outbound: gate writes on the backpressure flag. When set, queue and
  // surface Success. Otherwise fire downstream and arm the flag if
  // downstream signals Backpressure.
  channel_pipeline::Result onWrite(
      Context& ctx, channel_pipeline::TypeErasedBox&& msg) noexcept {
    if (FOLLY_UNLIKELY(backpressured_)) {
      pendingResponses_.push_back(
          msg.template take<ThriftServerResponseMessage>());
      return channel_pipeline::Result::Success;
    }
    auto result = ctx.fireWrite(std::move(msg));
    if (FOLLY_UNLIKELY(result == channel_pipeline::Result::Backpressure)) {
      backpressured_ = true;
      ctx.awaitWriteReady();
    }
    return result;
  }

  // Downstream has room again. Drain FIFO; on re-asserted Backpressure,
  // halt and leave the flag set (it was already true coming in). Only
  // clear the flag on a full drain.
  void onWriteReady(Context& ctx) noexcept {
    while (!pendingResponses_.empty()) {
      auto response = std::move(pendingResponses_.front());
      pendingResponses_.pop_front();
      auto result =
          ctx.fireWrite(channel_pipeline::erase_and_box(std::move(response)));
      if (result == channel_pipeline::Result::Backpressure) {
        return;
      }
    }
    backpressured_ = false;
    ctx.cancelAwaitWriteReady();
    // Reads were paused upstream via onRead returning Backpressure while
    // the buffer was filling. Now that it has fully drained, resume them.
    ctx.pipeline()->onReadReady();
  }

  // Pipeline tear-down: the wire is gone, so buffered responses can't go
  // out. Drop them.
  void onPipelineInactive(Context& ctx) noexcept {
    pendingResponses_.clear();
    backpressured_ = false;
    ctx.cancelAwaitWriteReady();
  }

  // === Test accessors ===
  bool isBackpressured() const noexcept { return backpressured_; }
  size_t pendingResponseCount() const noexcept {
    return pendingResponses_.size();
  }

 private:
  std::deque<ThriftServerResponseMessage> pendingResponses_;
  bool backpressured_{false};
};

} // namespace apache::thrift::fast_thrift::thrift
