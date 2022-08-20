/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <sodium/randombytes.h>
#include <array>

namespace fizz {

template <size_t Size>
struct RandomGenerator {
  /**
   * Returns an array of random data.
   */
  std::array<uint8_t, Size> generateRandom() {
    std::array<uint8_t, Size> random;
    randombytes_buf(random.data(), Size);
    return random;
  }
};

template <typename T, typename = std::enable_if_t<std::is_integral<T>::value>>
struct RandomNumGenerator {
  /**
   * Fills a POD type T with random data.
   */
  T generateRandom() {
    T random;
    randombytes_buf(&random, sizeof(random));
    return random;
  }
};
} // namespace fizz
