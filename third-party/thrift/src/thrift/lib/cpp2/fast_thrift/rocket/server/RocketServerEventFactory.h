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
#include <utility>

#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/Event.h>
#include <thrift/lib/cpp2/fast_thrift/transport/WriteCompletion.h>

namespace apache::thrift::fast_thrift::rocket::server {

/**
 * Per-pipeline event factory for the rocket-server pipeline.
 *
 * `make(status, bytes)` satisfies the WriteCompleteEventFactory concept used
 * by TransportHandlerT — produces a TransportWriteComplete event per writev.
 *
 * `makeBatchWriteComplete(status, frameCount, bytes)` is used by
 * WriteCompletionTrackerT to fire the enriched per-batch event upstream after
 * popping its frame-count FIFO.
 *
 * `makeFrameWriteComplete(status, streamId)` is used by
 * FragmentCompletionTrackerT to fan that batch event back out into one event
 * per original frame.
 */
struct RocketServerEventFactory {
  using EventId = RocketServerEventId;
  using TransportWriteCompleteEventType = TransportWriteCompleteEvent;

  // Batch-level event consumed by the fragmentation handler's tracker: the
  // event it subscribes to, plus the message type carried.
  using BatchWriteCompleteEventType = BatchWriteCompleteEvent;
  static constexpr EventId kBatchWriteCompleteEvent =
      EventId::BatchWriteComplete;

  static std::pair<
      EventId,
      apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox>
  make(
      apache::thrift::fast_thrift::transport::WriteCompletionStatus status,
      size_t bytes) noexcept {
    return {
        EventId::TransportWriteComplete,
        apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox(
            TransportWriteCompleteEvent{
                .status = status,
                .bytes = bytes,
            })};
  }

  static std::pair<
      EventId,
      apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox>
  makeBatchWriteComplete(
      apache::thrift::fast_thrift::transport::WriteCompletionStatus status,
      size_t frameCount,
      size_t bytes) noexcept {
    return {
        EventId::BatchWriteComplete,
        apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox(
            BatchWriteCompleteEvent{
                .status = status,
                .frameCount = frameCount,
                .bytes = bytes,
            })};
  }

  static std::pair<
      EventId,
      apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox>
  makeFrameWriteComplete(
      apache::thrift::fast_thrift::transport::WriteCompletionStatus status,
      uint32_t streamId) noexcept {
    return {
        EventId::FrameWriteComplete,
        apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox(
            FrameWriteCompleteEvent{
                .streamId = streamId,
                .status = status,
            })};
  }
};

static_assert(
    apache::thrift::fast_thrift::transport::WriteCompleteEventFactory<
        RocketServerEventFactory>,
    "RocketServerEventFactory must satisfy WriteCompleteEventFactory concept");

} // namespace apache::thrift::fast_thrift::rocket::server
