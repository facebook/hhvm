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

#include <chrono>
#include <cstdint>
#include <string_view>

#include <thrift/lib/cpp2/logging/ThriftEvent.h>

namespace apache::thrift {

/**
 * Pure virtual interface for real-time Thrift stream/sink counters.
 *
 * Implementations emit counters (e.g. fb303 timeseries, gauges) on each
 * lifecycle event. Methods are called inline from ThriftStreamLog /
 * ThriftSinkLog on every event, so implementations must be lightweight.
 *
 * All methods default to noop so that partial implementations are possible.
 */
class IThriftServerCounters {
 public:
  virtual ~IThriftServerCounters() = default;

  // ---------------------------------------------------------------------------
  // Stream counters
  // ---------------------------------------------------------------------------

  virtual void onStreamSubscribe(std::string_view /*methodName*/) {}

  virtual void onStreamNext(std::string_view /*methodName*/) {}

  virtual void onStreamCredit(
      std::string_view /*methodName*/,
      uint32_t /*credits*/,
      uint32_t /*creditsAfter*/) {}

  virtual void onStreamPause(
      std::string_view /*methodName*/, detail::StreamPauseReason /*reason*/) {}

  virtual void onStreamResume(
      std::string_view /*methodName*/,
      detail::StreamPauseReason /*reason*/,
      std::chrono::milliseconds /*pauseDuration*/) {}

  virtual void onStreamComplete(
      std::string_view /*methodName*/,
      detail::StreamEndReason /*reason*/,
      uint32_t /*totalPauseEvents*/,
      std::chrono::milliseconds /*totalPauseDuration*/) {}

  virtual void onStreamChunkGenerationInterval(
      std::string_view /*methodName*/, std::chrono::milliseconds /*interval*/) {
  }

  virtual void onStreamChunkSendDelay(
      std::string_view /*methodName*/, std::chrono::milliseconds /*delay*/) {}

  // ---------------------------------------------------------------------------
  // Sink counters
  // ---------------------------------------------------------------------------

  virtual void onSinkSubscribe(std::string_view /*methodName*/) {}

  virtual void onSinkNext(std::string_view /*methodName*/) {}

  virtual void onSinkConsumed(std::string_view /*methodName*/) {}

  virtual void onSinkCancel(std::string_view /*methodName*/) {}

  virtual void onSinkCredit(
      std::string_view /*methodName*/, uint32_t /*credits*/) {}

  virtual void onSinkComplete(
      std::string_view /*methodName*/, detail::SinkEndReason /*reason*/) {}

  virtual void onSinkReceiveInterval(
      std::string_view /*methodName*/, std::chrono::milliseconds /*interval*/) {
  }

  virtual void onSinkConsumeLatency(
      std::string_view /*methodName*/, std::chrono::milliseconds /*latency*/) {}

  virtual void onSinkApproxPausedDuration(
      std::string_view /*methodName*/, std::chrono::milliseconds /*duration*/) {
  }

  // ---------------------------------------------------------------------------
  // BiDi counters
  // ---------------------------------------------------------------------------

  virtual void onBiDiSubscribe(std::string_view /*methodName*/) {}

  virtual void onBiDiSinkNext(std::string_view /*methodName*/) {}

  virtual void onBiDiSinkCredit(
      std::string_view /*methodName*/, uint32_t /*credits*/) {}

  virtual void onBiDiStreamNext(std::string_view /*methodName*/) {}

  virtual void onBiDiSinkFirstChunkLatency(
      std::string_view /*methodName*/, std::chrono::milliseconds /*latency*/) {}

  virtual void onBiDiStreamFirstChunkLatency(
      std::string_view /*methodName*/, std::chrono::milliseconds /*latency*/) {}

  virtual void onBiDiStreamPause(
      std::string_view /*methodName*/, detail::StreamPauseReason /*reason*/) {}

  virtual void onBiDiStreamResume(
      std::string_view /*methodName*/,
      detail::StreamPauseReason /*reason*/,
      std::chrono::milliseconds /*pauseDuration*/) {}

  virtual void onBiDiStreamGenerationInterval(
      std::string_view /*methodName*/, std::chrono::milliseconds /*interval*/) {
  }

  virtual void onBiDiComplete(
      std::string_view /*methodName*/,
      detail::BiDiEndReason /*reason*/,
      uint32_t /*sinkTotalChunks*/,
      uint32_t /*streamTotalChunks*/,
      std::chrono::milliseconds /*duration*/) {}
};

} // namespace apache::thrift
