/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <sys/time.h>

#include <cmath>

#include <gtest/gtest.h>

#include "mcrouter/lib/fbi/WeightedFurcHash.h"
#include "mcrouter/lib/fbi/hash.h"

#define MAX_KEY_LENGTH 249

#define NUM_SAMPLES 1000000 /* one million */
#define NUM_POOLS 42

#define NUM_SERVERS 64
#define NUM_LOOKUPS 1000000

#define HASH_STOP "|#|"
#define EXTREME_KEY_INTERVAL                  \
  100 /* every EXTREME_KEY_INTERVAL, just use \
       * a uniform random generator to        \
       * determine the key length. */

namespace {
double drand() {
  double u = rand();
  return u / (RAND_MAX + 1.0);
}

double drand_in_range(double low, double high) {
  assert(low < high);
  double range = high - low;
  return (low + range * drand());
}

int rand_in_range(int low, int high) {
  assert(low < high);
  int range = high - low;
  return (low + rand() % range);
}

double dnormal_variate(double mu, double sigma) {
  while (true) {
    double u1 = drand();
    double u2 = 1.0 - drand();
    double z = 1.7155277699214135 * (u1 - 0.5) / u2;
    double zz = z * z / 4.0;
    if (zz <= -log(u2)) {
      return mu + z * sigma;
    }
  }
}

double dlognormal_variate(double mu, double sigma) {
  double u = exp(dnormal_variate(mu, sigma));
  return u;
}

uint32_t lognormal_variate(
    double mu,
    double sigma,
    uint32_t min_clip,
    uint32_t max_clip) {
  double u = dlognormal_variate(mu, sigma);
  uint32_t rv;

  assert(min_clip <= max_clip);

  if (max_clip != 0 && u >= (double)max_clip) {
    rv = max_clip;
  } else if (u <= min_clip) {
    rv = min_clip;
  } else {
    rv = (uint32_t)u;
  }
  return rv;
}

char* make_random_key(char* buffer, size_t maxLength) {
  size_t klen, i;

  if (rand() % EXTREME_KEY_INTERVAL == 0) {
    klen = (rand() % maxLength) + 1;
  } else {
    klen = lognormal_variate(2.9, 0.5, 1, maxLength);
  }

  if (!buffer) {
    buffer = (char*)malloc(klen + 1);
    assert(buffer);
  }

  for (i = 0; i < klen; i++) {
    char c;
    while (!isprint((c = rand() % 256)) || c == ' ') {
      ;
    }

    buffer[i] = c;
  }
  buffer[klen] = '\0';

  if (strncmp(buffer, HASH_STOP, sizeof(HASH_STOP) - 1) == 0) {
    buffer[0] = 'x';
  }

  return buffer;
}
} // namespace

/**
 * This verifies that
 *   1) the load is evenly balanced across servers.
 *   2) the act of adding a server to a pool will never result in a server
 *      handling keyspace that it previously handled but no longer does.
 *      If this occurs, then stale data may be returned.
 */
TEST(ch3, verify_correctness) {
  uint32_t i, j;
  uint32_t maximum_pool_size = furc_maximum_pool_size();
  char key[MAX_KEY_LENGTH + 1];
  std::vector<uint64_t> pools[NUM_POOLS];
  uint32_t sizes[NUM_POOLS];
  size_t num_pools;
  auto weights = std::make_unique<std::array<double, 1U << 23U>>();
  weights->fill(1.0);

  srand(time(nullptr));

  for (num_pools = 0; /* see end of loop */; ++num_pools) {
    if (num_pools == 0) {
      sizes[num_pools] = 1;
    } else if (num_pools == NUM_POOLS - 1) {
      sizes[num_pools] = maximum_pool_size;
    } else if (num_pools % 2 == 1) { // grow pool size geometrically
      sizes[num_pools] = sizes[num_pools - 1] * drand_in_range(1.5, 2.5);
    } else { // grow pool size arithmetically
      sizes[num_pools] = sizes[num_pools - 1] + rand_in_range(1, 11);
    }

    /* Make sure we don't exceed the maximum pool size. */
    if (sizes[num_pools] > maximum_pool_size) {
      sizes[num_pools] = maximum_pool_size;
    }

    pools[num_pools] = std::vector<uint64_t>(sizes[num_pools]);

    if (sizes[num_pools] == maximum_pool_size) {
      break;
    }
  }

  for (i = 0; i < NUM_SAMPLES; ++i) {
    size_t previous_num = -1;
    int len;

    make_random_key(key, MAX_KEY_LENGTH);
    len = strlen(key);

    // hash the same key in each pool, in increasing pool size order
    for (j = 0; j < num_pools; ++j) {
      size_t num = furc_hash(key, len, sizes[j]);
      EXPECT_LT(num, sizes[j]);

      // Verify that the weighted furc yields identical result with weights at 1
      assert(sizes[j] <= weights->size());
      folly::Range<const double*> weightRange(
          weights->cbegin(), weights->cbegin() + sizes[j]);
      size_t weighted = facebook::mcrouter::weightedFurcHash(
          folly::StringPiece(key, len), weightRange);
      EXPECT_EQ(num, weighted);

      ++pools[j][num];

      // make sure that this key either hashes the same server,
      // or hashes to a new server
      if (previous_num != num && j > 0) {
        EXPECT_GE(num, sizes[j - 1]);
      }

      previous_num = num;
    }
  }

  for (i = 0; i < num_pools; ++i) {
    /* Verify that load is evenly distributed. This isn't easy to do
       generally without significantly increasing the runtime by choosing
       a huge NUM_SAMPLES, so just check pools up to 1000 in size. */

    uint32_t pool_size = sizes[i];
    if (pool_size > 1000) {
      break;
    }
    double expected_mean = ((double)NUM_SAMPLES) / pool_size;

    double max_diff = 0;
    double sum = 0;
    for (j = 0; j < pool_size; j++) {
      double diff = std::abs(pools[i][j] - expected_mean);
      if (diff > max_diff) {
        max_diff = diff;
      }
      sum += pools[i][j];
    }
    double mean = sum / pool_size;
    // expect the sample mean to be within 5% of expected mean
    EXPECT_NEAR(mean, expected_mean, expected_mean * 0.05);

    // expect the maximum deviation from mean to be within 15%
    EXPECT_NEAR(max_diff, 0, mean * 0.15);

    sum = 0;
    for (j = 0; j < pool_size; j++) {
      double diff = pools[i][j] - mean;
      sum += diff * diff;
    }
    double stddev = sqrt(sum / pool_size);
    // expect the standard deviation to be < 5%
    EXPECT_NEAR(stddev, 0, mean * 0.05);
  }
}

// clang-format off
static const struct {
  const char* key;
  size_t len;
  uint32_t m;
  uint32_t expected;
} kGoldenValues[] = {
    {"key0", 4, 1, 0},
    {"key0", 4, 2, 0},
    {"key0", 4, 3, 0},
    {"key0", 4, 5, 4},
    {"key0", 4, 7, 5},
    {"key0", 4, 10, 7},
    {"key0", 4, 16, 14},
    {"key0", 4, 50, 22},
    {"key0", 4, 100, 22},
    {"key0", 4, 500, 413},
    {"key0", 4, 1000, 950},
    {"key0", 4, 5000, 3375},
    {"key0", 4, 10000, 3375},
    {"key0", 4, 100000, 59045},
    {"key0", 4, 1000000, 500854},
    {"key0", 4, 8388608, 4701525},
    {"key0", 4, 16777216, 10218333},
    {"key0", 4, 1073741824, 856543823},
    {"key0", 4, 4294967295u, 2635611118u},
    {"key1", 4, 1, 0},
    {"key1", 4, 2, 0},
    {"key1", 4, 3, 2},
    {"key1", 4, 5, 2},
    {"key1", 4, 7, 2},
    {"key1", 4, 10, 2},
    {"key1", 4, 16, 2},
    {"key1", 4, 50, 2},
    {"key1", 4, 100, 2},
    {"key1", 4, 500, 2},
    {"key1", 4, 1000, 808},
    {"key1", 4, 5000, 808},
    {"key1", 4, 10000, 808},
    {"key1", 4, 100000, 24368},
    {"key1", 4, 1000000, 768680},
    {"key1", 4, 8388608, 7205857},
    {"key1", 4, 16777216, 7205857},
    {"key1", 4, 1073741824, 144851818},
    {"key1", 4, 4294967295u, 1919030847u},
    {"key2", 4, 1, 0},
    {"key2", 4, 2, 0},
    {"key2", 4, 3, 0},
    {"key2", 4, 5, 4},
    {"key2", 4, 7, 4},
    {"key2", 4, 10, 4},
    {"key2", 4, 16, 14},
    {"key2", 4, 50, 40},
    {"key2", 4, 100, 40},
    {"key2", 4, 500, 279},
    {"key2", 4, 1000, 794},
    {"key2", 4, 5000, 3741},
    {"key2", 4, 10000, 3741},
    {"key2", 4, 100000, 99468},
    {"key2", 4, 1000000, 99468},
    {"key2", 4, 8388608, 8033017},
    {"key2", 4, 16777216, 8033017},
    {"key2", 4, 1073741824, 720979435},
    {"key2", 4, 4294967295u, 720979435u},
    {"key3", 4, 1, 0},
    {"key3", 4, 2, 1},
    {"key3", 4, 3, 1},
    {"key3", 4, 5, 1},
    {"key3", 4, 7, 5},
    {"key3", 4, 10, 5},
    {"key3", 4, 16, 5},
    {"key3", 4, 50, 5},
    {"key3", 4, 100, 73},
    {"key3", 4, 500, 214},
    {"key3", 4, 1000, 214},
    {"key3", 4, 5000, 1118},
    {"key3", 4, 10000, 1118},
    {"key3", 4, 100000, 98344},
    {"key3", 4, 1000000, 567453},
    {"key4", 4, 1, 0},
    {"key4", 4, 2, 0},
    {"key4", 4, 3, 2},
    {"key4", 4, 5, 2},
    {"key4", 4, 7, 2},
    {"key4", 4, 10, 8},
    {"key4", 4, 16, 10},
    {"key4", 4, 50, 47},
    {"key4", 4, 100, 47},
    {"key4", 4, 500, 101},
    {"key4", 4, 1000, 878},
    {"key4", 4, 5000, 878},
    {"key4", 4, 10000, 878},
    {"key4", 4, 100000, 19896},
    {"key4", 4, 1000000, 187582},
    {"someKey_ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789", 44, 1, 0},
    {"someKey_ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789", 44, 2, 0},
    {"someKey_ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789", 44, 3, 0},
    {"someKey_ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789", 44, 5, 4},
    {"someKey_ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789", 44, 7, 4},
    {"someKey_ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789", 44, 10, 8},
    {"someKey_ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789", 44, 16, 8},
    {"someKey_ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789", 44, 50, 18},
    {"someKey_ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789", 44, 100, 18},
    {"someKey_ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789", 44, 500, 18},
    {"someKey_ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789", 44, 1000, 964},
    {"someKey_ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789", 44, 5000, 1334},
    {"someKey_ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789", 44, 10000, 5716},
    {"someKey_ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789", 44, 100000, 54971},
    {"someKey_ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789", 44, 1000000, 580644},
    {"short", 5, 1, 0},
    {"short", 5, 2, 1},
    {"short", 5, 3, 2},
    {"short", 5, 5, 4},
    {"short", 5, 7, 6},
    {"short", 5, 10, 6},
    {"short", 5, 16, 12},
    {"short", 5, 50, 12},
    {"short", 5, 100, 12},
    {"short", 5, 500, 172},
    {"short", 5, 1000, 569},
    {"short", 5, 5000, 4565},
    {"short", 5, 10000, 4565},
    {"short", 5, 100000, 94116},
    {"short", 5, 1000000, 826962},
    {"short", 5, 8388608, 6473107},
    {"short", 5, 16777216, 10975421},
    {"short", 5, 1073741824, 304085075},
    {"short", 5, 4294967295u, 3727818566u},
    {"a", 1, 1, 0},
    {"a", 1, 2, 1},
    {"a", 1, 3, 1},
    {"a", 1, 5, 4},
    {"a", 1, 7, 4},
    {"a", 1, 10, 4},
    {"a", 1, 16, 13},
    {"a", 1, 50, 34},
    {"a", 1, 100, 34},
    {"a", 1, 500, 34},
    {"a", 1, 1000, 900},
    {"a", 1, 5000, 1565},
    {"a", 1, 10000, 1565},
    {"a", 1, 100000, 55594},
    {"a", 1, 1000000, 570739},
    {"a", 1, 8388608, 7695180},
    {"a", 1, 16777216, 11809035},
    {"a", 1, 1073741824, 207565192},
    {"a", 1, 4294967295u, 2134617590u},
    {"ab", 2, 1, 0},
    {"ab", 2, 2, 1},
    {"ab", 2, 3, 2},
    {"ab", 2, 5, 2},
    {"ab", 2, 7, 2},
    {"ab", 2, 10, 2},
    {"ab", 2, 16, 2},
    {"ab", 2, 50, 35},
    {"ab", 2, 100, 35},
    {"ab", 2, 500, 429},
    {"ab", 2, 1000, 429},
    {"ab", 2, 5000, 4150},
    {"ab", 2, 10000, 9404},
    {"ab", 2, 100000, 13199},
    {"ab", 2, 1000000, 103282},
    {"abc", 3, 1, 0},
    {"abc", 3, 2, 0},
    {"abc", 3, 3, 0},
    {"abc", 3, 5, 0},
    {"abc", 3, 7, 5},
    {"abc", 3, 10, 5},
    {"abc", 3, 16, 15},
    {"abc", 3, 50, 47},
    {"abc", 3, 100, 69},
    {"abc", 3, 500, 371},
    {"abc", 3, 1000, 985},
    {"abc", 3, 5000, 3019},
    {"abc", 3, 10000, 3019},
    {"abc", 3, 100000, 44962},
    {"abc", 3, 1000000, 391441},
    {"mcrouter:pool:regional:0001", 27, 1, 0},
    {"mcrouter:pool:regional:0001", 27, 2, 0},
    {"mcrouter:pool:regional:0001", 27, 3, 2},
    {"mcrouter:pool:regional:0001", 27, 5, 3},
    {"mcrouter:pool:regional:0001", 27, 7, 3},
    {"mcrouter:pool:regional:0001", 27, 10, 3},
    {"mcrouter:pool:regional:0001", 27, 16, 13},
    {"mcrouter:pool:regional:0001", 27, 50, 27},
    {"mcrouter:pool:regional:0001", 27, 100, 52},
    {"mcrouter:pool:regional:0001", 27, 500, 493},
    {"mcrouter:pool:regional:0001", 27, 1000, 829},
    {"mcrouter:pool:regional:0001", 27, 5000, 4143},
    {"mcrouter:pool:regional:0001", 27, 10000, 7488},
    {"mcrouter:pool:regional:0001", 27, 100000, 36359},
    {"mcrouter:pool:regional:0001", 27, 1000000, 669957},
    {"mcrouter:pool:regional:0001", 27, 8388608, 6119431},
    {"mcrouter:pool:regional:0001", 27, 16777216, 13341220},
    {"mcrouter:pool:regional:0001", 27, 1073741824, 656478206},
    {"mcrouter:pool:regional:0001", 27, 4294967295u, 3984053628u},
    {"mcrouter:pool:regional:0002", 27, 1, 0},
    {"mcrouter:pool:regional:0002", 27, 2, 1},
    {"mcrouter:pool:regional:0002", 27, 3, 2},
    {"mcrouter:pool:regional:0002", 27, 5, 4},
    {"mcrouter:pool:regional:0002", 27, 7, 5},
    {"mcrouter:pool:regional:0002", 27, 10, 5},
    {"mcrouter:pool:regional:0002", 27, 16, 13},
    {"mcrouter:pool:regional:0002", 27, 50, 13},
    {"mcrouter:pool:regional:0002", 27, 100, 13},
    {"mcrouter:pool:regional:0002", 27, 500, 405},
    {"mcrouter:pool:regional:0002", 27, 1000, 788},
    {"mcrouter:pool:regional:0002", 27, 5000, 788},
    {"mcrouter:pool:regional:0002", 27, 10000, 6708},
    {"mcrouter:pool:regional:0002", 27, 100000, 65904},
    {"mcrouter:pool:regional:0002", 27, 1000000, 589635},
    {"user:12345:profile", 18, 1, 0},
    {"user:12345:profile", 18, 2, 1},
    {"user:12345:profile", 18, 3, 1},
    {"user:12345:profile", 18, 5, 4},
    {"user:12345:profile", 18, 7, 4},
    {"user:12345:profile", 18, 10, 4},
    {"user:12345:profile", 18, 16, 10},
    {"user:12345:profile", 18, 50, 29},
    {"user:12345:profile", 18, 100, 77},
    {"user:12345:profile", 18, 500, 77},
    {"user:12345:profile", 18, 1000, 712},
    {"user:12345:profile", 18, 5000, 4344},
    {"user:12345:profile", 18, 10000, 5119},
    {"user:12345:profile", 18, 100000, 96279},
    {"user:12345:profile", 18, 1000000, 368844},
    {"user:67890:timeline", 19, 1, 0},
    {"user:67890:timeline", 19, 2, 0},
    {"user:67890:timeline", 19, 3, 0},
    {"user:67890:timeline", 19, 5, 0},
    {"user:67890:timeline", 19, 7, 5},
    {"user:67890:timeline", 19, 10, 7},
    {"user:67890:timeline", 19, 16, 15},
    {"user:67890:timeline", 19, 50, 15},
    {"user:67890:timeline", 19, 100, 73},
    {"user:67890:timeline", 19, 500, 104},
    {"user:67890:timeline", 19, 1000, 748},
    {"user:67890:timeline", 19, 5000, 3756},
    {"user:67890:timeline", 19, 10000, 6138},
    {"user:67890:timeline", 19, 100000, 86163},
    {"user:67890:timeline", 19, 1000000, 86163},
    {"cache:item:999999", 17, 1, 0},
    {"cache:item:999999", 17, 2, 1},
    {"cache:item:999999", 17, 3, 2},
    {"cache:item:999999", 17, 5, 2},
    {"cache:item:999999", 17, 7, 2},
    {"cache:item:999999", 17, 10, 2},
    {"cache:item:999999", 17, 16, 2},
    {"cache:item:999999", 17, 50, 17},
    {"cache:item:999999", 17, 100, 17},
    {"cache:item:999999", 17, 500, 184},
    {"cache:item:999999", 17, 1000, 673},
    {"cache:item:999999", 17, 5000, 3278},
    {"cache:item:999999", 17, 10000, 5532},
    {"cache:item:999999", 17, 100000, 72068},
    {"cache:item:999999", 17, 1000000, 180793},
    {"", 0, 1, 0},
    {"", 0, 2, 1},
    {"", 0, 3, 1},
    {"", 0, 5, 1},
    {"", 0, 7, 6},
    {"", 0, 10, 8},
    {"", 0, 16, 12},
    {"", 0, 50, 12},
    {"", 0, 100, 72},
    {"", 0, 500, 72},
    {"", 0, 1000, 72},
    {"", 0, 5000, 3031},
    {"", 0, 10000, 3031},
    {"", 0, 100000, 86003},
    {"", 0, 1000000, 407687},
};
// clang-format on

TEST(ch3, golden_values) {
  for (size_t i = 0; i < sizeof(kGoldenValues) / sizeof(kGoldenValues[0]);
       i++) {
    const auto& e = kGoldenValues[i];
    uint32_t got = furc_hash(e.key, e.len, e.m);
    EXPECT_EQ(got, e.expected)
        << "furc_hash(\"" << e.key << "\", " << e.len << ", " << e.m << ")";
  }
}

TEST(ch3, null_key_returns_zero) {
  EXPECT_EQ(furc_hash(nullptr, 0, 1), 0u);
  EXPECT_EQ(furc_hash(nullptr, 0, 10), 0u);
  EXPECT_EQ(furc_hash(nullptr, 0, 1000), 0u);
}

TEST(ch3, m_zero_returns_zero) {
  EXPECT_EQ(furc_hash("key", 3, 0), 0u);
}

TEST(ch3, m_beyond_max_pool_size) {
  uint32_t max_size = furc_maximum_pool_size();
  const char* key = "test_key";
  size_t len = strlen(key);

  uint32_t m = max_size + 1;
  EXPECT_LT(furc_hash(key, len, m), m);

  m = max_size * 2;
  EXPECT_LT(furc_hash(key, len, m), m);
}

TEST(ch3, power_of_two_boundaries) {
  // clang-format off
  static const struct { uint32_t m; uint32_t expected; } kPo2Values[] = {
      {4, 0},     {8, 7},     {32, 22},    {64, 22},
      {128, 115}, {256, 115}, {512, 413},  {1024, 950},
      {2048, 1735}, {4096, 3375}, {8192, 3375},
  };
  // clang-format on
  for (const auto& e : kPo2Values) {
    uint32_t got = furc_hash("key0", 4, e.m);
    EXPECT_EQ(got, e.expected) << "furc_hash(\"key0\", 4, " << e.m << ")";
  }
}

TEST(ch3, bulk_checksum) {
  // Per-pool polynomial checksums (acc = acc * 31 + hash) over 100K keys.
  // Unlike XOR, polynomial hashing is order-sensitive and uses the full
  // 32-bit space even for small pool sizes (XOR of m=2 outputs is ~0).
  // Each pool gets an independent check — no cross-pool cancellation.
  static const uint32_t kPoolSizes[] = {
      2,
      3,
      5,
      7,
      10,
      16,
      50,
      100,
      500,
      1000,
      5000,
      10000,
      100000,
      1000000,
      8388608,
      16777216,
      100000000,
      1073741824,
      4294967295u};
  // clang-format off
  static const uint32_t kExpectedChecksums[] = {
      1346413954u, // m=2
       914505363u, // m=3
      1517365794u, // m=5
      4252552508u, // m=7
      2894016351u, // m=10
      2995729350u, // m=16
      1717182892u, // m=50
      2289223413u, // m=100
      2400597860u, // m=500
      2251851331u, // m=1000
      2711796627u, // m=5000
      1328492545u, // m=10000
      1053928120u, // m=100000
       209068971u, // m=1000000
      2328588562u, // m=8388608
      1406750828u, // m=16777216
        55292389u, // m=100000000
      1798155495u, // m=1073741824
       504907336u, // m=4294967295
  };
  // clang-format on
  char key[32];

  for (size_t p = 0; p < sizeof(kPoolSizes) / sizeof(kPoolSizes[0]); p++) {
    uint32_t acc = 0;
    for (uint32_t i = 0; i < 100000; i++) {
      int len = snprintf(key, sizeof(key), "k%u", i);
      acc = acc * 31u + furc_hash(key, len, kPoolSizes[p]);
    }
    EXPECT_EQ(acc, kExpectedChecksums[p])
        << "Checksum mismatch for pool size " << kPoolSizes[p];
  }
}

TEST(ch3, result_in_range_all_m) {
  static const uint32_t kPoolSizes[] = {
      2,          3,           4,          5,       7,        8,
      10,         15,          16,         17,      31,       32,
      33,         63,          64,         65,      100,      127,
      128,        129,         255,        256,     257,      1000,
      10000,      100000,      1000000,    8388608, 16777216, 100000000,
      1073741824, 2147483648u, 4294967295u};
  char key[64];
  for (uint32_t pool : kPoolSizes) {
    for (uint32_t i = 0; i < 10000; i++) {
      int len = snprintf(key, sizeof(key), "range_%u", i);
      uint32_t result = furc_hash(key, len, pool);
      ASSERT_LT(result, pool)
          << "furc_hash(\"" << key << "\", " << len << ", " << pool << ")";
    }
  }
}

TEST(ch3, m_two_binary_output) {
  char key[32];
  uint32_t counts[2] = {0, 0};
  for (uint32_t i = 0; i < 100000; i++) {
    int len = snprintf(key, sizeof(key), "bin_%u", i);
    uint32_t result = furc_hash(key, len, 2);
    ASSERT_LT(result, 2u);
    counts[result]++;
  }
  EXPECT_NEAR(counts[0], 50000, 1000);
  EXPECT_NEAR(counts[1], 50000, 1000);
}

TEST(ch3, determinism) {
  static const char* const kKeys[] = {
      "determinism_test",
      "another_key",
      "",
      "x",
      "mcrouter:pool:regional:0042"};
  static const uint32_t kPoolSizes[] = {
      2, 7, 100, 1000, 10000, 8388608, 4294967295u};
  for (const char* key : kKeys) {
    size_t len = strlen(key);
    for (uint32_t pool : kPoolSizes) {
      uint32_t first = furc_hash(key, len, pool);
      for (int rep = 0; rep < 10; rep++) {
        EXPECT_EQ(furc_hash(key, len, pool), first)
            << "Non-deterministic for key=\"" << key << "\", m=" << pool;
      }
    }
  }
}

TEST(ch3, zero_length_key) {
  const char* key = "ignored_content";
  for (uint32_t m : {2u, 10u, 1000u, 1000000u}) {
    uint32_t result = furc_hash(key, 0, m);
    EXPECT_LT(result, m);
  }
  // When len=0, key content is irrelevant — only the seed matters
  EXPECT_EQ(furc_hash("aaa", 0, 1000), furc_hash("bbb", 0, 1000));
}

TEST(ch3, maximum_pool_size_value) {
  EXPECT_EQ(furc_maximum_pool_size(), 8388608u);
}

TEST(ch3, binary_key_with_embedded_nulls) {
  const char key1[] = "abc\0def";
  const char key2[] = "abc\0xyz";
  const char key3[] = "abc";
  uint32_t m = 1000000;

  uint32_t h1 = furc_hash(key1, 7, m);
  uint32_t h2 = furc_hash(key2, 7, m);
  uint32_t h3 = furc_hash(key3, 3, m);

  EXPECT_LT(h1, m);
  EXPECT_LT(h2, m);
  EXPECT_LT(h3, m);
  // Hash uses all len bytes, not just up to the first null
  EXPECT_NE(h1, h2);
  EXPECT_NE(h1, h3);
}

TEST(ch3, consistency_at_power_of_two_boundaries) {
  char key[64];
  static const uint32_t kBoundaries[] = {
      4, 8, 16, 32, 64, 128, 256, 512, 1024, 4096, 8192};
  for (uint32_t boundary : kBoundaries) {
    for (uint32_t i = 0; i < 10000; i++) {
      int len = snprintf(key, sizeof(key), "cons_%u", i);
      uint32_t at = furc_hash(key, len, boundary);
      uint32_t at_plus = furc_hash(key, len, boundary + 1);
      // Key either stays on same partition or moves to the new one
      if (at != at_plus) {
        EXPECT_EQ(at_plus, boundary)
            << "key=\"" << key << "\", m=" << boundary << ": moved from " << at
            << " to " << at_plus;
      }
    }
  }
}

static uint32_t __attribute__((__noinline__)) inconsistent_hashing_lookup(
    uint32_t hash_value,
    uint32_t pool_size) {
  asm(""); /* Enforce noinline. */
  return hash_value % pool_size;
}

TEST(ch3, timing) {
  unsigned i;
  struct timeval lstart, lend;
  uint64_t start, end;
  std::vector<char> keys((MAX_KEY_LENGTH + 1) * NUM_LOOKUPS);
  char* keys_itr;

  printf("Servers:\t\t%d\n", NUM_SERVERS);
  printf("Lookups:\t\t%d\n", NUM_LOOKUPS);

  printf("Generating lookup keys...");
  fflush(stdout);

  srand(time(nullptr));
  for (i = 0, keys_itr = keys.data(); i < NUM_LOOKUPS;
       ++i, keys_itr += MAX_KEY_LENGTH + 1) {
    make_random_key(keys_itr, MAX_KEY_LENGTH);
  }
  printf(" done\n");

  printf("Starting INconsistent hashing timing tests...");
  fflush(stdout);

  gettimeofday(&lstart, nullptr);
  for (i = 0, keys_itr = keys.data(); i < NUM_LOOKUPS;
       ++i, keys_itr += MAX_KEY_LENGTH + 1) {
    uint32_t hash_code = crc32_hash(keys_itr, strlen(keys_itr));
    uint32_t server_num = inconsistent_hashing_lookup(hash_code, NUM_SERVERS);

    (void)server_num; /* to avoid compiler warning */
  }
  gettimeofday(&lend, nullptr);
  printf(" done\n");

  start = ((uint64_t)lstart.tv_sec) * 1000000 + lstart.tv_usec;
  end = ((uint64_t)lend.tv_sec) * 1000000 + lend.tv_usec;
  printf(
      "Lookup:\t\t\t%zdus total\t%0.3fus/query\n",
      (end - start),
      ((float)(end - start)) / NUM_LOOKUPS);

  printf("Starting consistent hashing timing tests...");
  fflush(stdout);

  gettimeofday(&lstart, nullptr);
  for (i = 0, keys_itr = keys.data(); i < NUM_LOOKUPS;
       ++i, keys_itr += MAX_KEY_LENGTH + 1) {
    auto res = furc_hash(keys_itr, strlen(keys_itr), NUM_SERVERS);
    EXPECT_LT(res, NUM_SERVERS);
  }
  gettimeofday(&lend, nullptr);
  printf(" done\n");

  start = ((uint64_t)lstart.tv_sec) * 1000000 + lstart.tv_usec;
  end = ((uint64_t)lend.tv_sec) * 1000000 + lend.tv_usec;
  printf(
      "Lookup:\t\t\t%zdus total\t%0.3fus/query\n",
      (end - start),
      ((float)(end - start)) / NUM_LOOKUPS);
}

TEST(ch3, weighted_furc_hash_all_one) {
  char key[MAX_KEY_LENGTH + 1];
  int len;
  srand(12345);
  std::array<double, 1000> weights;
  weights.fill(1.0);

  for (uint32_t size = 1; size <= 1000; ++size) {
    make_random_key(key, MAX_KEY_LENGTH);
    len = strlen(key);
    size_t classic = furc_hash(key, len, size);
    EXPECT_LT(classic, size);
    folly::Range<const double*> weightRange(
        weights.cbegin(), weights.cbegin() + size);
    size_t weighted = facebook::mcrouter::weightedFurcHash(
        folly::StringPiece(key, len), weightRange);
    EXPECT_EQ(classic, weighted);
  }
}

TEST(ch3, weighted_furc_hash_all_75pct) {
  char key[MAX_KEY_LENGTH + 1];
  int len;
  srand(1234567);
  auto weights = std::make_unique<std::array<double, 10000>>();
  weights->fill(0.75);

  size_t sameCount = 0;
  for (uint32_t size = 5000; size <= 10000; ++size) {
    make_random_key(key, MAX_KEY_LENGTH);
    len = strlen(key);
    size_t classic = furc_hash(key, len, size);
    EXPECT_LT(classic, size);
    folly::Range<const double*> weightRange(
        weights->cbegin(), weights->cbegin() + size);
    size_t weighted = facebook::mcrouter::weightedFurcHash(
        folly::StringPiece(key, len), weightRange);
    EXPECT_LT(weighted, size);
    if (classic == weighted) {
      sameCount++;
    }
  }
  // Empirically for the seed, it's 3723, which is roughly 75% of 5000, as
  // expected.
  EXPECT_EQ(3723, sameCount);
}
