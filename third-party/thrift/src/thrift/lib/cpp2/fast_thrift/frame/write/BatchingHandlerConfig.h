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

namespace apache::thrift::fast_thrift::frame::write {

/**
 * Configuration for BatchingFrameHandler.
 *
 * Controls batching behavior for write coalescing.
 *
 * Usage with designated initializers:
 *   BatchingHandlerConfig config{
 *       .maxPendingBytes = 32 * 1024,  // 32KB for low-latency
 *       .maxPendingFrames = 16,
 *   };
 *
 * Or use defaults:
 *   BatchingHandlerConfig config{};
 */
struct BatchingHandlerConfig {
  /**
   * Maximum pending bytes before forcing a flush.
   *
   * If the total bytes queued exceeds this threshold, the handler
   * forces an immediate flush rather than waiting for the EventBase
   * LoopCallback.
   *
   * Default: 1MB - balances syscall reduction with throughput.
   *
   * Tuning:
   *   - Low-latency requirements: Decrease to 16KB-32KB
   *   - High-throughput bulk transfers: Increase to 128KB+
   */
  size_t maxPendingBytes{1024 * 1024}; // 1MB

  /**
   * Maximum pending frames before forcing a flush.
   *
   * Backstop limit for the many-small-frames case where byte count
   * is low but frame count is high. Prevents unbounded queue growth.
   *
   * Default: 32 frames.
   *
   * Tuning:
   *   - Low-latency: Decrease to 8-16
   *   - High-throughput: Increase to 64+
   */
  size_t maxPendingFrames{32};
};

} // namespace apache::thrift::fast_thrift::frame::write
