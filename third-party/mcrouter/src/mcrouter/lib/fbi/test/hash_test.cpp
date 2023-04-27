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
    while (!isprint((c = rand() % 256)) || c == ' ')
      ;

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

    if (sizes[num_pools] == maximum_pool_size)
      break;
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
    if (pool_size > 1000)
      break;
    double expected_mean = ((double)NUM_SAMPLES) / pool_size;

    double max_diff = 0;
    double sum = 0;
    for (j = 0; j < pool_size; j++) {
      double diff = std::abs(pools[i][j] - expected_mean);
      if (diff > max_diff)
        max_diff = diff;
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

static uint32_t __attribute__((__noinline__))
inconsistent_hashing_lookup(uint32_t hash_value, uint32_t pool_size) {
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
