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

#include <atomic>
#include <chrono>
#include <string_view>

#include <thrift/lib/cpp2/logging/IThriftRequestLogging.h>
#include <thrift/lib/cpp2/logging/IThriftServerCounters.h>
#include <thrift/lib/cpp2/logging/ThriftEvent.h>

namespace apache::thrift {

/**
 * Per-bidi-session lifecycle log, analogous to ThriftStreamLog / ThriftSinkLog.
 *
 * Accumulates state and dispatches to IThriftServerCounters on each event.
 *
 * Threading contract: log(BiDiSinkNextEvent) is called from the IO thread
 * (via ServerBiDiSinkBridge::onSinkNext). log(BiDiSinkCreditEvent) is called
 * from the executor thread (via ServerBiDiSinkBridge::getInput). Stream
 * events are called from the executor thread. finish() is called from the IO
 * thread (via ServerCallbackStapler destructor). Cross-thread fields
 * (sink counts, stream counts, subscribeTime) use atomics.
 *
 * Held as shared_ptr since it is shared across stream bridge, sink bridge,
 * and stapler.
 */
class ThriftBiDiLog {
 public:
  ThriftBiDiLog(
      std::string_view methodName,
      IThriftServerCounters* counters,
      IThriftRequestLogging* logging);

  ~ThriftBiDiLog();

  ThriftBiDiLog(const ThriftBiDiLog&) = delete;
  ThriftBiDiLog& operator=(const ThriftBiDiLog&) = delete;
  ThriftBiDiLog(ThriftBiDiLog&&) = delete;
  ThriftBiDiLog& operator=(ThriftBiDiLog&&) = delete;

  void log(const detail::BiDiSubscribeEvent& event);
  void log(const detail::BiDiSinkNextEvent& event);
  void log(const detail::BiDiSinkCreditEvent& event);
  void log(const detail::BiDiStreamNextEvent& event);
  void log(const detail::BiDiStreamCreditEvent& event);
  void log(const detail::BiDiStreamPauseEvent& event);
  void log(const detail::BiDiFinallyEvent& event);

 private:
  void finish(detail::BiDiEndReason reason);

  std::string_view methodName_;
  IThriftServerCounters* counters_;
  // TODO: Add IThriftRequestLogging bidi completion callback for parity with
  // ThriftStreamLog/ThriftSinkLog.

  std::atomic<bool> finished_{false};
  std::atomic<bool> wasSubscribed_{false};

  // Written on the IO thread (subscribe), read on the executor thread (stream
  // first-chunk latency) and IO thread (sink first-chunk latency, finish).
  // Atomic because the destructor (which calls finish()) can run on any thread
  // — whichever drops the last shared_ptr.
  std::atomic<std::chrono::steady_clock::time_point> subscribeTime_{};

  // Sink (client -> server) tracking — atomic because called from IO thread
  std::atomic<bool> isSinkFirstChunk_{true};
  std::atomic<uint32_t> sinkTotalChunks_{0};

  // Stream (server -> client) tracking — these two are only accessed from the
  // executor thread (inside the stream generation coroutine), so no atomic.
  bool isStreamFirstChunk_{true};
  std::chrono::steady_clock::time_point lastChunkGeneratedTime_;
  // Written on executor, read in finish() from IO thread — must be atomic.
  std::atomic<uint32_t> streamTotalChunks_{0};
};

} // namespace apache::thrift
