/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <algorithm>
#include <cstdio>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include <folly/Conv.h>

#include "mcrouter/lib/Ch3HashFunc.h"
#include "mcrouter/lib/WeightedCh3HashFunc.h"
#include "mcrouter/lib/fbi/hash.h"

using namespace facebook::memcache;

/* Just test the same behaviour as Ch3 for weights = 1.0 */
TEST(WeightedCh3HashFunc, basic) {
  WeightedCh3HashFunc func_100(std::vector<double>(100, 1.0));
  WeightedCh3HashFunc func_1({1.0});
  WeightedCh3HashFunc func_max(
      std::vector<double>(furc_maximum_pool_size(), 1.0));
  WeightedCh3HashFunc func_99999(std::vector<double>(99999, 1.0));

  EXPECT_TRUE(97 == func_100("sample"));
  EXPECT_TRUE(0 == func_1("sample"));

  EXPECT_TRUE(72 == func_100(""));
  EXPECT_TRUE(0 == func_1(""));

  EXPECT_TRUE(6173600 == func_max(""));
  EXPECT_TRUE(5167780 == func_max("sample"));

  std::string test_max_key;

  //-128 .. 127
  for (int i = 0; i < 256; ++i) {
    test_max_key.push_back(i - 128);
  }
  EXPECT_TRUE(31015 == func_99999(test_max_key));

  // 127 .. -128
  std::reverse(test_max_key.begin(), test_max_key.end());
  EXPECT_TRUE(67101 == func_99999(test_max_key));
}

/* Zero weight -> will give up and return a valid index */
TEST(WeightedCh3HashFunc, zeroWeight) {
  WeightedCh3HashFunc func_1({0.0});
  WeightedCh3HashFunc func_100(std::vector<double>(100, 0.0));

  EXPECT_TRUE(0 == func_1("sample"));
  EXPECT_TRUE(0 == func_1(""));

  EXPECT_TRUE(59 == func_100("sample"));
  EXPECT_TRUE(45 == func_100(""));
}

/* Compare ch3 and wch3 */
TEST(WeightedCh3HashFunc, reducedWeight) {
  Ch3HashFunc ch3_func_3(3);
  WeightedCh3HashFunc wch3_func_3({1.0, 1.0, 0.7});

  std::vector<size_t> ch3_counts(3, 0);
  std::vector<size_t> wch3_counts(3, 0);

  for (size_t i = 0; i < 1000; ++i) {
    auto key = folly::to<std::string>(i);
    auto ch3_i = ch3_func_3(key);
    auto wch3_i = wch3_func_3(key);
    if (ch3_i != 2) {
      /* hosts with weight 1.0 still get all of their traffic unchanged */
      EXPECT_TRUE(ch3_i == wch3_i);
    }

    ++ch3_counts[ch3_func_3(key)];
    ++wch3_counts[wch3_func_3(key)];
  }

  /* Note reduced weight for the 3rd box */
  EXPECT_TRUE(std::vector<size_t>({307, 342, 351}) == ch3_counts);
  EXPECT_TRUE(std::vector<size_t>({341, 371, 288}) == wch3_counts);
}

/* Compare ch3 and wch3 */
TEST(WeightedCh3HashFunc, randomWeights) {
  Ch3HashFunc ch3_func_10(10);
  WeightedCh3HashFunc wch3_func_10(
      {0.429, 0.541, 0.117, 0.998, 0.283, 0.065, 0.109, 0.042, 0.676, 0.943});

  std::vector<size_t> ch3_counts(10, 0);
  std::vector<size_t> wch3_counts(10, 0);

  for (size_t i = 0; i < 10000; ++i) {
    auto key = folly::to<std::string>(i);

    ++ch3_counts[ch3_func_10(key)];
    ++wch3_counts[wch3_func_10(key)];
  }

  EXPECT_TRUE(
      std::vector<size_t>(
          {995, 955, 1046, 968, 1032, 972, 1016, 1038, 1010, 968}) ==
      ch3_counts);
  EXPECT_TRUE(
      std::vector<size_t>(
          {1016, 1252, 288, 2354, 661, 195, 247, 122, 1668, 2197}) ==
      wch3_counts);
}

// With unit weights, WeightedCh3HashFunc must return the same value as
// furc_hash for all inputs. This is the fundamental contract.
TEST(WeightedCh3HashFunc, unitWeightsMatchFurcHash) {
  static const uint32_t kPoolSizes[] = {
      2, 3, 5, 10, 100, 1000, 10000, 100000, 1000000};
  char key[32];
  for (uint32_t n : kPoolSizes) {
    std::vector<double> w(n, 1.0);
    WeightedCh3HashFunc func(w);
    for (uint32_t i = 0; i < 10000; i++) {
      int len = snprintf(key, sizeof(key), "eq_%u", i);
      size_t wch3 = func(folly::StringPiece(key, len));
      uint32_t furc = furc_hash(key, len, n);
      ASSERT_EQ(wch3, furc) << "key=\"" << key << "\", n=" << n;
    }
  }
}

// Per-pool polynomial checksums with unit weights (100K keys each).
// Matches the furc_hash checksums, providing an independent verification
// path through the WeightedCh3HashFunc → furc_hash call chain.
TEST(WeightedCh3HashFunc, bulkChecksumUnitWeights) {
  // clang-format off
  static const uint32_t kPoolSizes[] = {
      2, 3, 7, 10, 16, 100, 1000, 10000, 100000};
  static const uint32_t kExpected[] = {
      1346413954u, //  n=2
       914505363u, //  n=3
      4252552508u, //  n=7
      2894016351u, //  n=10
      2995729350u, //  n=16
      2289223413u, //  n=100
      2251851331u, //  n=1000
      1328492545u, //  n=10000
      1053928120u, //  n=100000
  };
  // clang-format on
  char key[32];
  for (size_t p = 0; p < sizeof(kPoolSizes) / sizeof(kPoolSizes[0]); p++) {
    uint32_t n = kPoolSizes[p];
    std::vector<double> w(n, 1.0);
    WeightedCh3HashFunc func(w);
    uint32_t acc = 0;
    for (uint32_t i = 0; i < 100000; i++) {
      int len = snprintf(key, sizeof(key), "k%u", i);
      acc = acc * 31u + (uint32_t)func(folly::StringPiece(key, len));
    }
    EXPECT_EQ(acc, kExpected[p]) << "Checksum mismatch for n=" << n;
  }
}

// Per-pool polynomial checksums with weight=0.5 (10K keys each).
// Exercises the retry/rehash path where SpookyHash probability
// check fails ~50% of the time.
TEST(WeightedCh3HashFunc, bulkChecksumHalfWeights) {
  // clang-format off
  static const uint32_t kPoolSizes[] = {2, 10, 100, 1000, 10000};
  static const uint32_t kExpected[] = {
      2395690262u, //  n=2
       547444706u, //  n=10
      2951198297u, //  n=100
      1336857622u, //  n=1000
      3378951161u, //  n=10000
  };
  // clang-format on
  char key[32];
  for (size_t p = 0; p < sizeof(kPoolSizes) / sizeof(kPoolSizes[0]); p++) {
    uint32_t n = kPoolSizes[p];
    std::vector<double> w(n, 0.5);
    WeightedCh3HashFunc func(w);
    uint32_t acc = 0;
    for (uint32_t i = 0; i < 10000; i++) {
      int len = snprintf(key, sizeof(key), "k%u", i);
      acc = acc * 31u + (uint32_t)func(folly::StringPiece(key, len));
    }
    EXPECT_EQ(acc, kExpected[p]) << "Checksum mismatch for n=" << n;
  }
}

// Per-pool polynomial checksums with weight=0.0 (10K keys each).
// All retries fail; returns the last furc_hash index after salting.
TEST(WeightedCh3HashFunc, bulkChecksumZeroWeights) {
  // clang-format off
  static const uint32_t kPoolSizes[] = {2, 10, 100, 1000};
  static const uint32_t kExpected[] = {
      3431444300u, //  n=2
      1290067504u, //  n=10
      2604325093u, //  n=100
       337394182u, //  n=1000
  };
  // clang-format on
  char key[32];
  for (size_t p = 0; p < sizeof(kPoolSizes) / sizeof(kPoolSizes[0]); p++) {
    uint32_t n = kPoolSizes[p];
    std::vector<double> w(n, 0.0);
    WeightedCh3HashFunc func(w);
    uint32_t acc = 0;
    for (uint32_t i = 0; i < 10000; i++) {
      int len = snprintf(key, sizeof(key), "k%u", i);
      acc = acc * 31u + (uint32_t)func(folly::StringPiece(key, len));
    }
    EXPECT_EQ(acc, kExpected[p]) << "Checksum mismatch for n=" << n;
  }
}

// Mixed weights: verify distribution is proportional to weights,
// and that zero-weight servers get no traffic.
TEST(WeightedCh3HashFunc, mixedWeightsDistribution) {
  double mixed[] = {1.0, 0.5, 0.0, 0.75, 1.0};
  WeightedCh3HashFunc func(std::vector<double>(mixed, mixed + 5));
  std::vector<size_t> counts(5, 0);
  char key[32];

  uint32_t acc = 0;
  for (uint32_t i = 0; i < 10000; i++) {
    int len = snprintf(key, sizeof(key), "d%u", i);
    size_t h = func(folly::StringPiece(key, len));
    ASSERT_LT(h, 5u);
    counts[h]++;
    acc = acc * 31u + (uint32_t)h;
  }

  // Zero-weight server must get zero traffic
  EXPECT_EQ(counts[2], 0u);
  // Unit-weight servers should get the most traffic
  EXPECT_GT(counts[0], counts[1]);
  EXPECT_GT(counts[4], counts[1]);
  // Exact checksum
  EXPECT_EQ(acc, 2309057571u);
}

// Result must always be in range [0, n) for all weight configurations.
TEST(WeightedCh3HashFunc, resultInRange) {
  static const uint32_t kPoolSizes[] = {1, 2, 3, 5, 10, 100, 1000, 10000};
  static const double kWeights[] = {0.0, 0.25, 0.5, 0.75, 1.0};
  char key[64];

  for (uint32_t n : kPoolSizes) {
    for (double w : kWeights) {
      WeightedCh3HashFunc func(std::vector<double>(n, w));
      for (uint32_t i = 0; i < 1000; i++) {
        int len = snprintf(key, sizeof(key), "rng_%u", i);
        size_t h = func(folly::StringPiece(key, len));
        ASSERT_LT(h, n) << "key=\"" << key << "\", n=" << n << ", weight=" << w;
      }
    }
  }
}

// Determinism: same inputs must always produce same output.
TEST(WeightedCh3HashFunc, determinism) {
  WeightedCh3HashFunc func_unit(std::vector<double>(100, 1.0));
  WeightedCh3HashFunc func_half(std::vector<double>(100, 0.5));
  WeightedCh3HashFunc func_zero(std::vector<double>(100, 0.0));

  static const char* const kKeys[] = {"det_test", "another", "", "x"};
  for (const char* key : kKeys) {
    auto sp = folly::StringPiece(key, strlen(key));
    size_t u = func_unit(sp);
    size_t h = func_half(sp);
    size_t z = func_zero(sp);
    for (int rep = 0; rep < 10; rep++) {
      EXPECT_EQ(func_unit(sp), u);
      EXPECT_EQ(func_half(sp), h);
      EXPECT_EQ(func_zero(sp), z);
    }
  }
}

// Reducing a weight should only redirect traffic FROM that server,
// not between other servers. Servers with weight=1.0 must keep
// all their original traffic.
TEST(WeightedCh3HashFunc, weightReductionConsistency) {
  static const size_t kN = 100;
  WeightedCh3HashFunc func_full(std::vector<double>(kN, 1.0));

  // Reduce server 42's weight to 0.3
  std::vector<double> reduced(kN, 1.0);
  reduced[42] = 0.3;
  WeightedCh3HashFunc func_reduced(reduced);

  char key[32];
  size_t redirected = 0;
  for (uint32_t i = 0; i < 10000; i++) {
    int len = snprintf(key, sizeof(key), "cons_%u", i);
    auto sp = folly::StringPiece(key, len);
    size_t full = func_full(sp);
    size_t red = func_reduced(sp);

    if (full != 42) {
      // Keys NOT originally on server 42 must stay on the same server
      EXPECT_EQ(full, red) << "key=\"" << key << "\" moved from " << full
                           << " to " << red << " when only server 42 changed";
    } else if (red != 42) {
      redirected++;
    }
  }
  // Some traffic from server 42 should have been redirected (~70%)
  EXPECT_GT(redirected, 0u);
}

// n=1 with any weight: only valid index is 0.
TEST(WeightedCh3HashFunc, singleServer) {
  for (double w : {0.0, 0.5, 1.0}) {
    WeightedCh3HashFunc func({w});
    char key[32];
    for (uint32_t i = 0; i < 100; i++) {
      int len = snprintf(key, sizeof(key), "s%u", i);
      EXPECT_EQ(func(folly::StringPiece(key, len)), 0u);
    }
  }
}

// Bulk checksums with long keys (128 bytes) that exercise
// multi-chunk SpookyHash processing. Guards against hash value
// changes from internal optimizations (e.g., ShortSpookyHashState).
TEST(WeightedCh3HashFunc, longKeyBulkChecksumHalfWeights) {
  std::string baseKey(128, 'A');
  for (size_t i = 0; i < 128; i++) {
    baseKey[i] = 'A' + (i % 26);
  }

  // clang-format off
  static const uint32_t kPoolSizes[] = {2, 10, 100, 1000};
  static const uint32_t kExpected[] = {
      3784301429u, //  n=2
       707485837u, //  n=10
      4273108085u, //  n=100
       954106693u, //  n=1000
  };
  // clang-format on
  char suffix[16];
  for (size_t p = 0; p < sizeof(kPoolSizes) / sizeof(kPoolSizes[0]); p++) {
    uint32_t n = kPoolSizes[p];
    std::vector<double> w(n, 0.5);
    WeightedCh3HashFunc func(w);
    uint32_t acc = 0;
    for (uint32_t i = 0; i < 10000; i++) {
      int suffixLen = snprintf(suffix, sizeof(suffix), "%u", i);
      std::string key = baseKey;
      key.append(suffix, suffixLen);
      acc = acc * 31u + (uint32_t)func(folly::StringPiece(key));
    }
    EXPECT_EQ(acc, kExpected[p]) << "Checksum mismatch for n=" << n;
  }
}

TEST(WeightedCh3HashFunc, longKeyBulkChecksumZeroWeights) {
  std::string baseKey(128, 'A');
  for (size_t i = 0; i < 128; i++) {
    baseKey[i] = 'A' + (i % 26);
  }

  // clang-format off
  static const uint32_t kPoolSizes[] = {2, 10, 100};
  static const uint32_t kExpected[] = {
      2171781178u, //  n=2
      2937312667u, //  n=10
       532711027u, //  n=100
  };
  // clang-format on
  char suffix[16];
  for (size_t p = 0; p < sizeof(kPoolSizes) / sizeof(kPoolSizes[0]); p++) {
    uint32_t n = kPoolSizes[p];
    std::vector<double> w(n, 0.0);
    WeightedCh3HashFunc func(w);
    uint32_t acc = 0;
    for (uint32_t i = 0; i < 10000; i++) {
      int suffixLen = snprintf(suffix, sizeof(suffix), "%u", i);
      std::string key = baseKey;
      key.append(suffix, suffixLen);
      acc = acc * 31u + (uint32_t)func(folly::StringPiece(key));
    }
    EXPECT_EQ(acc, kExpected[p]) << "Checksum mismatch for n=" << n;
  }
}
