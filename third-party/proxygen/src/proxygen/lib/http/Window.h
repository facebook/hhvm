/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <cstdint>

namespace proxygen {

/**
 * A class that implements SPDY & HTTP/2 window management. This class
 * should be used for both connection and stream level flow control.
 */
class Window {
 public:
  /**
   * Constructs a new Window.
   * @param capacity The initial capacity of this Window. The initial size will
   *                 also be equal to this. This parameter must not be > 2^31 -1
   */
  explicit Window(uint32_t capacity);

  /**
   * Returns the number of bytes available to be consumed in this
   * window. This could become a negative number if the initial window is
   * set to a smaller number.
   */
  int32_t getSize() const;

  /**
   * Returns the number of bytes available to be consumed in this
   * window. If that number went negative somehow, this function clamps
   * the return value to zero.
   */
  uint32_t getNonNegativeSize() const;

  /**
   * Returns the size of the initial window. That is, the total number of
   * bytes allowed to be outstanding on this window.
   */
  uint32_t getCapacity() const;

  /**
   * Returns the number of bytes reserved in this window. If multiple
   * calls to free() caused this number to go negative, this function
   * returns 0.
   */
  uint32_t getOutstanding() const;

  /**
   * @param amount The amount to reduce the window size by. Increases
   *               bytes outstanding by amount.
   * @param strict If true, reserve() will return false if there is
   *               no space remaining in the window. If false,
   *               reserve() will return true until the integer
   *               overflows.
   */
  bool reserve(uint32_t amount, bool strict = true);

  /**
   * Increment the window size by amount. Decrease bytes outstanding by
   * amount. The window size could become greater than the capacity here.
   */
  bool free(uint32_t amount);

  /**
   * Sets a new initial window size. This will also apply the delta
   * between the current window size and the new window size. Returns false
   * iff applying the new initial window fails.
   */
  bool setCapacity(uint32_t capacity);

 private:
  int32_t outstanding_{0};
  int32_t capacity_{0}; // always positive
};

} // namespace proxygen
