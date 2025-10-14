/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <cstdint>
#include <limits>

// QUIC style flow controller using offsets

class FlowController {
 public:
  FlowController(uint64_t initialMax = 0) : maxOffset_(initialMax) {
  }

  bool reserve(uint64_t length) {
    if (length == 0) {
      return true;
    }

    if (currentOffset_ >= std::numeric_limits<uint64_t>::max() - length) {
      return false;
    }

    uint64_t newOffset = currentOffset_ + length;
    if (newOffset > maxOffset_) {
      return false;
    }
    currentOffset_ = newOffset;
    return true;
  }

  bool grant(uint64_t offset) {
    if (offset <= maxOffset_) {
      return false;
    }
    maxOffset_ = offset;
    return true;
  }

  [[nodiscard]] bool isBlocked() const {
    return currentOffset_ >= maxOffset_;
  }

  uint64_t getCurrentOffset() const {
    return currentOffset_;
  }
  uint64_t getMaxOffset() const {
    return maxOffset_;
  }

  uint64_t getAvailable() const {
    return maxOffset_ - currentOffset_;
  }

 private:
  uint64_t currentOffset_{0};
  uint64_t maxOffset_;
};
