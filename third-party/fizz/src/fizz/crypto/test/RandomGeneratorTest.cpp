/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <folly/portability/GTest.h>

#include <fizz/crypto/RandomGenerator.h>

#include <fizz/crypto/test/TestUtil.h>

namespace fizz {
namespace test {

TEST(RandomGeneratorTest, TestRandom) {
  useMockRandom();

  auto random = RandomGenerator<32>().generateRandom();
  std::array<uint8_t, 32> expected;
  expected.fill(0x44);
  EXPECT_EQ(random, expected);
}

TEST(RandomGeneratorTest, TestRandomUInt32) {
  useMockRandom();

  auto random = RandomNumGenerator<uint32_t>().generateRandom();
  EXPECT_EQ(random, 0x44444444);
}
} // namespace test
} // namespace fizz
