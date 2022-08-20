/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <limits>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include "mcrouter/lib/carbon/Util.h"
#include "mcrouter/lib/carbon/test/Util.h"

TEST(SerializedFormat, integers) {
  // Serialization for signed integers should output unsigned integers in the
  // following zigzag pattern:
  //  0 -> 0,  -1 -> 1,  1 -> 2,  -2 -> 3,  2 -> 4, etc.
  using VectorPair = std::vector<std::pair<int16_t, int16_t>>;

  auto& matchingRanges = carbon::test::util::satisfiedSubranges<int16_t>;

  EXPECT_EQ(
      (VectorPair{
          {std::numeric_limits<int16_t>::min(),
           std::numeric_limits<int16_t>::max()}}),
      matchingRanges([](int16_t i) {
        const auto zigzagged = carbon::util::zigzag(i);
        if (i >= 0) {
          return 2 * static_cast<uint16_t>(i) == zigzagged;
        } else {
          return 2 * static_cast<uint16_t>(-1 * i) - 1 == zigzagged;
        }
      }));
}
