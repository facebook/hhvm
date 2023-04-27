/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <chrono>

namespace proxygen { namespace compress {
enum class SchemeType { QPACK, QMIN, HPACK };

// Metadata about encoded blocks.  In a real stack, these might be
// conveyed via HTTP frame (HEADERS or PUSH_PROMISE) flags.
struct FrameFlags {
  FrameFlags(bool ooo = false, bool depends = false)
      : allowOOO(ooo), QPACKPrefixHasDepends(depends) {
  }

  bool allowOOO{false};
  bool QPACKPrefixHasDepends{false};
};

struct SimParams {
  SchemeType type;
  int64_t seed;
  std::chrono::milliseconds rtt;
  double lossProbability;
  double delayProbability;
  std::chrono::milliseconds maxDelay;
  uint16_t minOOOThresh;
  bool blend;
  bool samePacketCompression;
  uint32_t tableSize;
  uint32_t maxBlocking;
};

struct SimStats {
  uint64_t allowedOOO{0};
  uint64_t packetLosses{0};
  uint64_t maxQueueBufferBytes{0};
  std::chrono::milliseconds holDelay{0};
  uint64_t uncompressed{0};
  uint64_t compressed{0};
  uint64_t packets{0};
};
}} // namespace proxygen::compress
