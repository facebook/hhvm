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

#include <memory>
#include <folly/ExceptionWrapper.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/HandlerTag.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>
#include <thrift/lib/cpp2/fast_thrift/common/Stats.h>

namespace apache::thrift::fast_thrift {

HANDLER_TAG(rocket_metrics_handler);

// DuplexHandler that observes rocket-layer messages and bumps counters.
// Completely pass-through — does not inspect or modify message contents.
//
// Hot-path cost per direction: one incrementValue() (~0.1ns) + pipeline
// forward.
//
// Template parameter Dir controls active-request gauge semantics:
//   Server: onRead increments active (incoming request), onWrite decrements
//   Client: onWrite increments active (outgoing request), onRead decrements
//
// Pipeline placement:
//   Server: ... → RocketServerStreamStateHandler → [RocketMetricsHandler] →
//   RocketServerAppAdapter Client: ... → RocketClientErrorFrameHandler →
//   [RocketMetricsHandler] → ...
template <Direction Dir, typename Stats>
class RocketMetricsHandler {
  static_assert(FastThriftStatsConcept<Stats>);

 public:
  explicit RocketMetricsHandler(std::shared_ptr<Stats> stats)
      : stats_(std::move(stats)) {}

  template <typename Context>
  void handlerAdded(Context& /*ctx*/) noexcept {}

  template <typename Context>
  void handlerRemoved(Context& /*ctx*/) noexcept {}

  template <typename Context>
  void onPipelineActive(Context& /*ctx*/) noexcept {}

  template <typename Context>
  void onReadReady(Context& /*ctx*/) noexcept {}

  template <typename Context>
  [[nodiscard]] channel_pipeline::Result onRead(
      Context& ctx, channel_pipeline::TypeErasedBox&& msg) noexcept {
    stats_->rocketInbound.incrementValue(1);
    if constexpr (Dir == Direction::Server) {
      stats_->rocketActive.incrementValue(1);
    } else {
      stats_->rocketActive.incrementValue(-1);
    }
    return ctx.fireRead(std::move(msg));
  }

  template <typename Context>
  void onException(Context& ctx, folly::exception_wrapper&& e) noexcept {
    stats_->rocketErrors.incrementValue(1);
    ctx.fireException(std::move(e));
  }

  template <typename Context>
  [[nodiscard]] channel_pipeline::Result onWrite(
      Context& ctx, channel_pipeline::TypeErasedBox&& msg) noexcept {
    stats_->rocketOutbound.incrementValue(1);
    if constexpr (Dir == Direction::Server) {
      stats_->rocketActive.incrementValue(-1);
    } else {
      stats_->rocketActive.incrementValue(1);
    }
    return ctx.fireWrite(std::move(msg));
  }

  template <typename Context>
  void onWriteReady(Context& /*ctx*/) noexcept {}

  template <typename Context>
  void onPipelineInactive(Context& /*ctx*/) noexcept {}

 private:
  std::shared_ptr<Stats> stats_;
};

} // namespace apache::thrift::fast_thrift
