/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <cstddef>
#include <limits>

// QUIC style flow controller using offsets

class FlowController {
 public:
  FlowController(size_t initialMax = 0) : maxOffset_(initialMax) {
  }

  bool reserve(size_t length) {
    if (length == 0) {
      return true;
    }

    if (currentOffset_ >= std::numeric_limits<size_t>::max() - length) {
      return false;
    }

    size_t newOffset = currentOffset_ + length;
    if (newOffset > maxOffset_) {
      return false;
    }
    currentOffset_ = newOffset;
    return true;
  }

  bool grant(size_t offset) {
    if (offset <= maxOffset_) {
      return false;
    }
    maxOffset_ = offset;
    return true;
  }

  [[nodiscard]] bool isBlocked() const {
    return currentOffset_ >= maxOffset_;
  }

  size_t getCurrentOffset() const {
    return currentOffset_;
  }
  size_t getMaxOffset() const {
    return maxOffset_;
  }

  size_t getAvailable() const {
    return maxOffset_ - currentOffset_;
  }

 private:
  size_t currentOffset_{0};
  size_t maxOffset_;
};
