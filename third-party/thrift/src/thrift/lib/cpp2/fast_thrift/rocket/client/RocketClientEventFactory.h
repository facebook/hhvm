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
#include <thrift/lib/cpp2/fast_thrift/rocket/client/Event.h>
#include <thrift/lib/cpp2/fast_thrift/transport/WriteCompletion.h>

namespace apache::thrift::fast_thrift::rocket::client {

/**
 * Per-pipeline event factory for the rocket-client pipeline.
 *
 * `make(status, bytes)` satisfies the WriteCompleteEventFactory concept used
 * by TransportHandlerT — produces a TransportWriteComplete event per writev.
 *
 * `makeRocketWriteComplete(status, frameCount, bytes)` is used by
 * WriteCompletionTrackerT to fire the enriched per-rocket-batch event
 * upstream after popping its frame-count FIFO.
 */
struct RocketClientEventFactory {
  using EventId = RocketClientEventId;
  using TransportWriteCompleteEventType = TransportWriteCompleteEvent;

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
  makeRocketWriteComplete(
      apache::thrift::fast_thrift::transport::WriteCompletionStatus status,
      size_t frameCount,
      size_t bytes) noexcept {
    return {
        EventId::RocketWriteComplete,
        apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox(
            RocketWriteCompleteEvent{
                .status = status,
                .frameCount = frameCount,
                .bytes = bytes,
            })};
  }
};

static_assert(
    apache::thrift::fast_thrift::transport::WriteCompleteEventFactory<
        RocketClientEventFactory>,
    "RocketClientEventFactory must satisfy WriteCompleteEventFactory concept");

} // namespace apache::thrift::fast_thrift::rocket::client
