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
#include <cstddef>

namespace apache::thrift::fast_thrift::frame::write {

/**
 * Configuration for IntervalBatchingHandler.
 *
 * Controls interval-based write batching behavior, mirroring WriteBatcher's
 * batching semantics from thrift async.
 *
 * Usage:
 *   IntervalBatchingHandlerConfig config{
 *       .batchingInterval = std::chrono::milliseconds(1),
 *       .batchingSize = 64,
 *       .batchingByteSize = 64 * 1024,
 *   };
 */
struct IntervalBatchingHandlerConfig {
  /**
   * Time interval before flushing pending writes.
   *
   * When non-zero, a HHWheelTimer schedules a flush after this interval.
   * When zero, a LoopCallback flushes at the end of the current event loop
   * iteration.
   */
  std::chrono::milliseconds batchingInterval{std::chrono::milliseconds::zero()};

  /**
   * Maximum pending frame count before forcing an early flush.
   *
   * When the count of buffered frames reaches this threshold, the handler
   * cancels the timer and schedules an immediate flush via LoopCallback.
   */
  size_t batchingSize{32};

  /**
   * Maximum pending byte size before forcing an early flush.
   *
   * When the total buffered bytes reaches this threshold, the handler
   * cancels the timer and schedules an immediate flush via LoopCallback.
   * Set to 0 to disable byte-based early flush.
   */
  size_t batchingByteSize{1024 * 1024};
};

} // namespace apache::thrift::fast_thrift::frame::write
