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

#include <thrift/lib/cpp2/fast_thrift/frame/FrameType.h>

#include <folly/io/IOBuf.h>

#include <cstddef>
#include <cstdint>

namespace apache::thrift::fast_thrift::frame::read {

/**
 * FragmentState - Holds pending fragment data for a single stream.
 *
 * This struct is used by FrameDefragmentationHandler to accumulate
 * fragments for a stream with hasFollows() == true. When the final
 * fragment arrives (hasFollows() == false), the accumulated payload
 * is used to construct a complete ParsedFrame.
 *
 * Design rationale:
 * - Zero-copy buffer chaining via IOBuf::prependChain() - no memcpy
 * - Original frame type/flags preserved for reassembly
 * - Stream ID stored for debugging and validation
 * - Accumulated bytes tracked for potential size limits (future)
 */
struct FragmentState {
  // Original frame type from first fragment (REQUEST_*, not PAYLOAD)
  FrameType originalType{FrameType::RESERVED};

  // Original flags from first fragment (includes metadata bit, etc.)
  uint16_t originalFlags{0};

  // Stream ID (for debugging/validation)
  uint32_t streamId{0};

  // Metadata size from first fragment (only first fragment has metadata)
  uint16_t metadataSize{0};

  // Accumulated payload chain (metadata + data from all fragments)
  // Zero-copy: buffers are chained, not copied
  std::unique_ptr<folly::IOBuf> payload;

  // Total accumulated bytes (for metrics/limits)
  size_t accumulatedBytes{0};

  /**
   * Append a buffer to an existing chain (zero-copy - just pointer
   * manipulation). Precondition: payload must not be null (first fragment sets
   * payload directly).
   */
  void appendToChain(std::unique_ptr<folly::IOBuf> buf, size_t len) {
    payload->prependChain(std::move(buf));
    accumulatedBytes += len;
  }
};

} // namespace apache::thrift::fast_thrift::frame::read
