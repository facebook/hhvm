/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/Window.h>

#include <glog/logging.h>
#include <limits>

namespace proxygen {

Window::Window(uint32_t capacity) {
  CHECK(setCapacity(capacity));
}

int32_t Window::getSize() const {
  return capacity_ - outstanding_;
}

uint32_t Window::getNonNegativeSize() const {
  auto size = getSize();
  return size > 0 ? size : 0;
}

uint32_t Window::getCapacity() const {
  // This conversion is safe since we always ensure capacity_ > 0
  return static_cast<uint32_t>(capacity_);
}

uint32_t Window::getOutstanding() const {
  return outstanding_ < 0 ? 0 : outstanding_;
}

bool Window::reserve(const uint32_t amount, bool strict) {
  if (amount > std::numeric_limits<int32_t>::max()) {
    VLOG(3) << "Cannot shrink window by more than 2^31 - 1. "
            << "Attempted decrement of " << amount;
    return false;
  }
  const int32_t limit =
      std::numeric_limits<int32_t>::max() - static_cast<int32_t>(amount);
  if (outstanding_ > 0 && limit < outstanding_) {
    VLOG(3) << "Overflow detected. Window change failed.";
    return false;
  }
  const int32_t newOutstanding = outstanding_ + amount;
  if (strict && newOutstanding > capacity_) {
    VLOG(3) << "Outstanding bytes (" << newOutstanding << ") exceeded "
            << "window capacity (" << capacity_ << ")";
    return false;
  }
  outstanding_ = newOutstanding;
  return true;
}

bool Window::free(const uint32_t amount) {
  if (amount > std::numeric_limits<int32_t>::max()) {
    VLOG(3) << "Cannot expand window by more than 2^31 - 1. "
            << "Attempted increment of " << amount;
    return false;
  }
  const int32_t limit =
      std::numeric_limits<int32_t>::min() + static_cast<int32_t>(amount);
  if (outstanding_ < 0 && limit > outstanding_) {
    VLOG(3) << "Underflow detected. Window change failed.";
    return false;
  }
  const int32_t newOutstanding = outstanding_ - amount;
  if (newOutstanding < capacity_ - std::numeric_limits<int32_t>::max()) {
    VLOG(3) << "Window exceeded 2^31 - 1. Window change failed.";
    return false;
  }
  outstanding_ = newOutstanding;
  return true;
}

bool Window::setCapacity(const uint32_t capacity) {
  if (capacity > std::numeric_limits<int32_t>::max()) {
    VLOG(3) << "Cannot set initial window > 2^31 -1.";
    return false;
  }

  const int32_t diff = static_cast<int32_t>(capacity) - capacity_;
  if (diff > 0) {
    const int32_t size = getSize();
    if (size > 0 && diff > (std::numeric_limits<int32_t>::max() - size)) {
      VLOG(3) << "Increasing the capacity overflowed the window";
      return false;
    }
  }

  capacity_ = static_cast<int32_t>(capacity);
  return true;
}

} // namespace proxygen
