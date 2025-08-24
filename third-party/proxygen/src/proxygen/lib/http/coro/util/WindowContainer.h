/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/logging/xlog.h>
#include <proxygen/lib/http/Window.h>

namespace proxygen::coro {

class WindowContainer {
 public:
  explicit WindowContainer(uint32_t capacity) : recvWindow_(capacity) {
  }

  bool reserve(size_t length, uint16_t padding, bool strict = true) {
    if (length + padding > std::numeric_limits<uint32_t>::max() ||
        !recvWindow_.reserve(uint32_t(length) + padding, strict)) {
      return false;
    }
    recvWindow_.free(padding);
    recvToAck_ += padding;
    return true;
  }

  size_t processed(size_t amount) {
    XCHECK_LE(amount, std::numeric_limits<uint32_t>::max());
    XCHECK(recvWindow_.free(uint32_t(amount)));
    recvToAck_ += amount;
    size_t ret = 0;
    if (recvToAck_ >= kMinThreshold ||
        recvToAck_ >= recvWindow_.getCapacity() / kUpdateThreshold) {
      ret = recvToAck_;
      recvToAck_ = 0;
    }
    return ret;
  }

  uint32_t setCapacity(uint32_t capacity) {
    if (capacity < recvWindow_.getCapacity()) {
      XLOG(ERR) << "Can't shrink window capacity " << capacity << " < "
                << recvWindow_.getCapacity();
      return 0;
    } else {
      uint32_t delta = capacity - recvWindow_.getCapacity();
      XCHECK(recvWindow_.setCapacity(capacity)) << "setCapacity overflow";
      return delta;
    }
  }

  int32_t getSize() const {
    return recvWindow_.getSize();
  }

  [[nodiscard]] const Window& getWindow() const {
    return recvWindow_;
  }

 private:
  const static uint8_t kUpdateThreshold = 2;
  constexpr static uint32_t kMinThreshold = 128 * 1024;

  Window recvWindow_;
  size_t recvToAck_{0};
};

} // namespace proxygen::coro
