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

#include <thrift/lib/cpp2/logging/ThriftBiDiLog.h>

#include <utility>

namespace apache::thrift {

ThriftBiDiLog::ThriftBiDiLog(
    std::string_view methodName,
    IThriftServerCounters* counters,
    IThriftRequestLogging* /* logging */)
    : methodName_(methodName), counters_(counters) {}

ThriftBiDiLog::~ThriftBiDiLog() {
  if (!finished_.load(std::memory_order_acquire)) {
    finish(detail::BiDiEndReason::ERROR);
  }
}

void ThriftBiDiLog::log(const detail::BiDiSubscribeEvent& /*event*/) {
  auto now = std::chrono::steady_clock::now();
  subscribeTime_.store(now, std::memory_order_relaxed);
  wasSubscribed_.store(true, std::memory_order_release);
  if (counters_) {
    counters_->onBiDiSubscribe(methodName_);
  }
}

void ThriftBiDiLog::log(const detail::BiDiSinkNextEvent& /*event*/) {
  if (counters_) {
    counters_->onBiDiSinkNext(methodName_);
  }

  if (isSinkFirstChunk_.exchange(false, std::memory_order_relaxed)) {
    auto now = std::chrono::steady_clock::now();
    auto latencyMs = std::chrono::duration_cast<std::chrono::milliseconds>(
        now - subscribeTime_.load(std::memory_order_relaxed));
    if (counters_) {
      counters_->onBiDiSinkFirstChunkLatency(methodName_, latencyMs);
    }
  }

  sinkTotalChunks_.fetch_add(1, std::memory_order_relaxed);
}

void ThriftBiDiLog::log(const detail::BiDiSinkCreditEvent& event) {
  if (counters_) {
    counters_->onBiDiSinkCredit(methodName_, event.credits);
  }
}

void ThriftBiDiLog::log(const detail::BiDiStreamNextEvent& /*event*/) {
  auto now = std::chrono::steady_clock::now();

  if (counters_) {
    counters_->onBiDiStreamNext(methodName_);
  }

  if (std::exchange(isStreamFirstChunk_, false)) {
    auto latencyMs = std::chrono::duration_cast<std::chrono::milliseconds>(
        now - subscribeTime_.load(std::memory_order_relaxed));
    if (counters_) {
      counters_->onBiDiStreamFirstChunkLatency(methodName_, latencyMs);
    }
  } else {
    auto intervalMs = std::chrono::duration_cast<std::chrono::milliseconds>(
        now - lastChunkGeneratedTime_);
    if (counters_) {
      counters_->onBiDiStreamGenerationInterval(methodName_, intervalMs);
    }
  }

  streamTotalChunks_.fetch_add(1, std::memory_order_relaxed);
  lastChunkGeneratedTime_ = now;
}

void ThriftBiDiLog::log(const detail::BiDiStreamCreditEvent& /*event*/) {
  // No metric to collect for stream credit currently
}

void ThriftBiDiLog::log(const detail::BiDiStreamPauseEvent& /*event*/) {
  // No metric to collect for stream pause currently
}

void ThriftBiDiLog::log(const detail::BiDiFinallyEvent& event) {
  finish(event.reason);
}

void ThriftBiDiLog::finish(detail::BiDiEndReason reason) {
  if (finished_.exchange(true, std::memory_order_release)) {
    return;
  }

  if (!wasSubscribed_.load(std::memory_order_acquire)) {
    return;
  }

  auto durationMs = std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::steady_clock::now() -
      subscribeTime_.load(std::memory_order_relaxed));

  if (counters_) {
    counters_->onBiDiComplete(
        methodName_,
        reason,
        sinkTotalChunks_.load(std::memory_order_relaxed),
        streamTotalChunks_.load(std::memory_order_relaxed),
        durationMs);
  }
}

} // namespace apache::thrift
