/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <limits>

#include <folly/portability/GTest.h>
#include <proxygen/lib/sampling/Sampling.h>

using proxygen::Sampling;
using std::string;

TEST(SamplingTests, Basic) {
  Sampling rate(1.0);
  EXPECT_EQ(rate.isLucky(), true);
  EXPECT_EQ(rate.getWeight(), 1);

  rate = Sampling(0.0);
  EXPECT_EQ(rate.isLucky(), false);
  EXPECT_EQ(rate.getWeight(), 0);

  rate = Sampling(0.5);
  EXPECT_EQ(rate.getWeight(), 2);

  rate = Sampling(0.00001);
  EXPECT_EQ(rate.getWeight(), 100000);

  rate = Sampling(0.000001);
  EXPECT_EQ(rate.getWeight(), 1000000);

  // this is beyond the error tolerance
  rate = Sampling(0.000000000001);
  EXPECT_EQ(rate.getWeight(), 0);
}

TEST(SamplingTests, UpdateRate) {
  Sampling sampling(0.5);
  EXPECT_EQ(sampling.getWeight(), 2);
  sampling.updateRate(0.05);
  EXPECT_EQ(sampling.getWeight(), 20);
}

TEST(SamplingTests, Key) {
  Sampling sampling(1.0);
  EXPECT_EQ(sampling.getIntRate(), std::numeric_limits<uint32_t>::max());
  sampling.updateRate(0);
  EXPECT_EQ(sampling.getIntRate(), 0);
  sampling.updateRate(0.5);
  EXPECT_EQ(sampling.getIntRate(), std::numeric_limits<uint32_t>::max() / 2);
}

TEST(SamplingTests, HashKey) {
  Sampling sampling(1.0);
  string key = "test";
  EXPECT_TRUE(sampling.isLucky(key));
  sampling.updateRate(0.0);
  EXPECT_FALSE(sampling.isLucky(key));
  sampling.updateRate(0.5);
  EXPECT_FALSE(sampling.isLucky(key));
  // repeat
  EXPECT_FALSE(sampling.isLucky(key));
}

TEST(SamplingTests, runSampled) {
  {
    int counter = 0;

    Sampling sampling(1.0);
    for (int i = 0; i < 3; i++) {
      sampling.runSampled([&] { ++counter; });
    }
    EXPECT_EQ(counter, 3);
  }
  {
    int counter = 0;
    Sampling sampling(0);
    for (int i = 0; i < 3; i++) {
      sampling.runSampled([&] { ++counter; });
    }
    EXPECT_EQ(counter, 0);
  }
  {
    int counter = 0;
    Sampling sampling(0.5);
    for (int i = 0; i < 100; i++) {
      sampling.runSampled([&] { ++counter; });
    }
    EXPECT_GT(counter, 0);
    EXPECT_LT(counter, 100);
  }
}
