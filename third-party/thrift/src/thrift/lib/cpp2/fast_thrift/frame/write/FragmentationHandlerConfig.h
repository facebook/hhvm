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
 * Configuration for FrameFragmentationHandler.
 *
 * Controls fragmentation behavior for HOL blocking mitigation.
 * See: FrameFragmentationHandler.md for design documentation.
 *
 * Usage with designated initializers:
 *   FragmentationHandlerConfig config{
 *       .maxFragmentSize = 32 * 1024,  // 32KB for low-latency
 *   };
 *
 * Or use defaults:
 *   FragmentationHandlerConfig config{};
 */
struct FragmentationHandlerConfig {
  /**
   * Maximum fragment size in bytes.
   *
   * Payloads larger than this are fragmented into chunks of this size.
   * Also used as the bypass threshold: frames <= this size go directly
   * to the immediate queue without fragmentation.
   *
   * Default: 16KB - reflects average payload size at Meta, providing good
   * interleaving granularity for HOL blocking mitigation.
   *
   * Tuning:
   *   - High-latency networks: Increase to 64KB+ to reduce header overhead
   *   - Low-latency requirements: Keep at 16KB or decrease for finer
   * interleaving
   */
  size_t maxFragmentSize{16 * 1024};

  /**
   * Maximum pending bytes before forcing a flush.
   *
   * If the total bytes queued across all streams exceeds this threshold,
   * the handler forces an immediate flush rather than waiting for the
   * EventBase LoopCallback.
   *
   * Default: 512KB - provides memory pressure limit while allowing batching.
   *
   * Tuning:
   *   - Memory-constrained: Decrease to 256KB
   *   - High-throughput bulk transfers: Increase to 1MB+
   */
  size_t maxPendingBytes{512 * 1024};

  /**
   * Maximum pending frames before forcing a flush.
   *
   * Backstop limit for the many-small-frames case where byte count
   * is low but frame count is high. Prevents unbounded queue growth.
   *
   * Default: 128 frames.
   */
  size_t maxPendingFrames{128};

  /**
   * Minimum payload size worth fragmenting.
   *
   * Payloads smaller than this are never fragmented, even if they
   * exceed maxFragmentSize (edge case). Avoids fragmentation overhead
   * on tiny payloads.
   *
   * Default: 1KB.
   */
  size_t minSizeToFragment{1024};
};

} // namespace apache::thrift::fast_thrift::frame::write
