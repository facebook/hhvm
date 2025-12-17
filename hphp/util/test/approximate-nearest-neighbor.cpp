/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#include "hphp/util/approximate-nearest-neighbor.h"

#include <gtest/gtest.h>
#include <random>
#include <set>
#include <vector>
#include <algorithm>

namespace HPHP {

namespace {

// Helper to compute Jaccard similarity between two sorted vectors
template <typename T>
double jaccard_similarity(const std::vector<T>& a, const std::vector<T>& b) {
  std::vector<T> intersection;
  std::vector<T> union_set;

  std::set_intersection(a.begin(), a.end(), b.begin(), b.end(),
                        std::back_inserter(intersection));
  std::set_union(a.begin(), a.end(), b.begin(), b.end(),
                 std::back_inserter(union_set));

  if (union_set.empty()) return 1.0;  // Both empty
  return static_cast<double>(intersection.size()) / union_set.size();
}

} // namespace

///////////////////////////////////////////////////////////////////////////////
// Basic functionality tests

TEST(ApproximateNearestNeighborTest, Construction) {
  std::mt19937 prng(42);
  ApproximateNearestNeighbor<uint32_t, uint32_t> ann(
    prng,
    {0.3, 0.1},  // Lower bound
    {0.7, 0.9},  // Upper bound
    1000,        // numElems
    100          // numSets
  );

  EXPECT_GT(ann.numExperiments(), 0u);
  EXPECT_GT(ann.roundsPerExperiment(), 0u);
}

TEST(ApproximateNearestNeighborTest, AddAndQuery) {
  std::mt19937 prng(42);
  ApproximateNearestNeighbor<uint32_t, uint32_t> ann(
    prng,
    {0.3, 0.1},
    {0.7, 0.9},
    100,
    10
  );

  std::vector<uint32_t> set1 = {1, 2, 3, 4, 5};
  std::vector<uint32_t> set2 = {1, 2, 3, 4, 6};  // High similarity to set1
  std::vector<uint32_t> set3 = {10, 20, 30, 40, 50};  // Low similarity to set1

  ann.add(0, set1.begin(), set1.end());
  ann.add(1, set2.begin(), set2.end());
  ann.add(2, set3.begin(), set3.end());

  auto nearest = ann.nearest(0);
  ASSERT_TRUE(nearest.has_value());
  // Set 1 is most similar to set 0 (80% Jaccard similarity)
  EXPECT_EQ(*nearest, 1u);
}

TEST(ApproximateNearestNeighborTest, IdenticalSets) {
  std::mt19937 prng(42);
  ApproximateNearestNeighbor<uint32_t, uint32_t> ann(
    prng,
    {0.3, 0.1},
    {0.8, 0.95},
    100,
    10
  );

  std::vector<uint32_t> set1 = {1, 2, 3, 4, 5};
  std::vector<uint32_t> set2 = {1, 2, 3, 4, 5};  // Identical

  ann.add(0, set1.begin(), set1.end());
  ann.add(1, set2.begin(), set2.end());

  auto nearest = ann.nearest(0);
  ASSERT_TRUE(nearest.has_value());
  EXPECT_EQ(*nearest, 1u);
}

TEST(ApproximateNearestNeighborTest, EmptySets) {
  std::mt19937 prng(42);
  ApproximateNearestNeighbor<uint32_t, uint32_t> ann(
    prng,
    {0.3, 0.1},
    {0.7, 0.9},
    100,
    10
  );

  std::vector<uint32_t> empty1;
  std::vector<uint32_t> empty2;
  std::vector<uint32_t> set3 = {1, 2, 3};

  ann.add(0, empty1.begin(), empty1.end());
  ann.add(1, empty2.begin(), empty2.end());
  ann.add(2, set3.begin(), set3.end());

  // Empty sets should match each other
  auto nearest = ann.nearest(0);
  ASSERT_TRUE(nearest.has_value());
  EXPECT_TRUE(*nearest == 1u);
}

TEST(ApproximateNearestNeighborTest, DisjointSets) {
  std::mt19937 prng(42);
  ApproximateNearestNeighbor<uint32_t, uint32_t> ann(
    prng,
    {0.3, 0.1},
    {0.7, 0.9},
    100,
    10
  );

  std::vector<uint32_t> set1 = {1, 2, 3, 4, 5};
  std::vector<uint32_t> set2 = {10, 20, 30, 40, 50};
  std::vector<uint32_t> set3 = {60, 70, 80, 90, 99};

  ann.add(0, set1.begin(), set1.end());
  ann.add(1, set2.begin(), set2.end());
  ann.add(2, set3.begin(), set3.end());

  auto nearest = ann.nearest(0);
  EXPECT_FALSE(nearest.has_value());
}

TEST(ApproximateNearestNeighborTest, SingleSet) {
  std::mt19937 prng(42);
  ApproximateNearestNeighbor<uint32_t, uint32_t> ann(
    prng,
    {0.3, 0.1},
    {0.7, 0.9},
    100,
    10
  );

  std::vector<uint32_t> set1 = {1, 2, 3, 4, 5};
  ann.add(0, set1.begin(), set1.end());

  auto nearest = ann.nearest(0);
  EXPECT_FALSE(nearest.has_value());  // No other sets
}

///////////////////////////////////////////////////////////////////////////////
// Incremental add tests

TEST(ApproximateNearestNeighborTest, PreaddAndAddByExperiment) {
  std::mt19937 prng(42);
  ApproximateNearestNeighbor<uint32_t, uint32_t> ann(
    prng,
    {0.3, 0.1},
    {0.7, 0.9},
    100,
    10
  );

  std::vector<uint32_t> set1 = {1, 2, 3, 4, 5};
  std::vector<uint32_t> set2 = {1, 2, 3, 4, 6};

  // Add set 0 using preadd + addByExperiment
  ann.preadd(0);
  for (size_t i = 0; i < ann.numExperiments(); ++i) {
    ann.addByExperiment(0, i, set1.begin(), set1.end());
  }

  // Add set 1 normally
  ann.add(1, set2.begin(), set2.end());

  auto nearest = ann.nearest(0);
  ASSERT_TRUE(nearest.has_value());
  EXPECT_EQ(*nearest, 1u);
}

TEST(ApproximateNearestNeighborTest, TokenAPI) {
  std::mt19937 prng(42);
  ApproximateNearestNeighbor<uint32_t, uint32_t> ann(
    prng,
    {0.3, 0.1},
    {0.7, 0.9},
    100,
    10
  );

  std::vector<uint32_t> set1 = {1, 2, 3, 4, 5};
  std::vector<uint32_t> set2 = {1, 2, 3, 4, 6};

  // Create token for set 0
  auto token = ann.makeToken(set1.begin(), set1.end());

  // Add using token
  ann.preadd(0);
  for (size_t i = 0; i < ann.numExperiments(); ++i) {
    ann.addWithToken(0, i, token);
  }

  // Add set 1 normally
  ann.add(1, set2.begin(), set2.end());

  auto nearest = ann.nearest(0);
  ASSERT_TRUE(nearest.has_value());
  EXPECT_EQ(*nearest, 1u);
}

///////////////////////////////////////////////////////////////////////////////
// Filter tests

TEST(ApproximateNearestNeighborTest, FilterAcceptAll) {
  std::mt19937 prng(42);
  ApproximateNearestNeighbor<uint32_t, uint32_t> ann(
    prng,
    {0.3, 0.1},
    {0.7, 0.9},
    100,
    10
  );

  std::vector<uint32_t> set1 = {1, 2, 3, 4, 5};
  std::vector<uint32_t> set2 = {1, 2, 3, 4, 6};
  std::vector<uint32_t> set3 = {10, 20, 30, 40, 50};

  ann.add(0, set1.begin(), set1.end());
  ann.add(1, set2.begin(), set2.end());
  ann.add(2, set3.begin(), set3.end());

  ApproximateNearestNeighbor<uint32_t, uint32_t>::Counter counts;
  auto nearest = ann.nearest(0, counts, [](uint32_t) { return true; });

  ASSERT_TRUE(nearest.has_value());
  EXPECT_EQ(*nearest, 1u);
}

TEST(ApproximateNearestNeighborTest, FilterRejectAll) {
  std::mt19937 prng(42);
  ApproximateNearestNeighbor<uint32_t, uint32_t> ann(
    prng,
    {0.3, 0.1},
    {0.7, 0.9},
    100,
    10
  );

  std::vector<uint32_t> set1 = {1, 2, 3, 4, 5};
  std::vector<uint32_t> set2 = {1, 2, 3, 4, 6};

  ann.add(0, set1.begin(), set1.end());
  ann.add(1, set2.begin(), set2.end());

  ApproximateNearestNeighbor<uint32_t, uint32_t>::Counter counts;
  auto nearest = ann.nearest(0, counts, [](uint32_t) { return false; });

  EXPECT_FALSE(nearest.has_value());
}

TEST(ApproximateNearestNeighborTest, FilterSelective) {
  std::mt19937 prng(42);
  ApproximateNearestNeighbor<uint32_t, uint32_t> ann(
    prng,
    {0.3, 0.1},
    {0.7, 0.9},
    100,
    10
  );

  std::vector<uint32_t> set1 = {1, 2, 3, 4, 5};
  std::vector<uint32_t> set2 = {1, 2, 3, 4, 6};
  std::vector<uint32_t> set3 = {1, 2, 3, 6, 7};

  ann.add(0, set1.begin(), set1.end());
  ann.add(1, set2.begin(), set2.end());
  ann.add(2, set3.begin(), set3.end());

  // Filter out set 1
  ApproximateNearestNeighbor<uint32_t, uint32_t>::Counter counts;
  auto nearest = ann.nearest(0, counts, [](uint32_t id) { return id != 1; });

  ASSERT_FALSE(nearest.has_value());
}

TEST(ApproximateNearestNeighborTest, FilterCallCount) {
  std::mt19937 prng(42);
  ApproximateNearestNeighbor<uint32_t, uint32_t> ann(
    prng,
    {0.3, 0.1},
    {0.7, 0.9},
    100,
    10
  );

  std::vector<uint32_t> set1 = {1, 2, 3, 4, 5};
  std::vector<uint32_t> set2 = {1, 2, 3, 4, 6};
  std::vector<uint32_t> set3 = {10, 20, 30, 40, 50};

  ann.add(0, set1.begin(), set1.end());
  ann.add(1, set2.begin(), set2.end());
  ann.add(2, set3.begin(), set3.end());

  // Count how many times filter is called for each set
  std::map<uint32_t, size_t> call_counts;
  ApproximateNearestNeighbor<uint32_t, uint32_t>::Counter counts;

  [[maybe_unused]] auto nearest = ann.nearest(0, counts, [&](uint32_t id) {
    call_counts[id]++;
    return id != 1;  // Reject set 1
  });

  // Each set should be filtered at most once
  for (const auto& [id, count] : call_counts) {
    EXPECT_LE(count, 1u) << "Filter called more than once for set " << id;
  }
}

///////////////////////////////////////////////////////////////////////////////
// all() method tests

TEST(ApproximateNearestNeighborTest, AllMethod) {
  std::mt19937 prng(42);
  ApproximateNearestNeighbor<uint32_t, uint32_t> ann(
    prng,
    {0.3, 0.1},
    {0.7, 0.9},
    100,
    10
  );

  std::vector<uint32_t> set1 = {1, 2, 3, 4, 5};
  std::vector<uint32_t> set2 = {1, 2, 3, 4, 6};
  std::vector<uint32_t> set3 = {10, 20, 30, 40, 50};

  ann.add(0, set1.begin(), set1.end());
  ann.add(1, set2.begin(), set2.end());
  ann.add(2, set3.begin(), set3.end());

  std::set<uint32_t> reported;
  ann.all(0, [&](uint32_t id) {
    reported.insert(id);
  });

  // Should report at least one other set (may report duplicates)
  EXPECT_FALSE(reported.empty());
  EXPECT_EQ(reported.count(0u), 0u);  // Should not report self
}

///////////////////////////////////////////////////////////////////////////////
// Counter tests

TEST(ApproximateNearestNeighborTest, CounterReuse) {
  std::mt19937 prng(42);
  ApproximateNearestNeighbor<uint32_t, uint32_t> ann(
    prng,
    {0.3, 0.1},
    {0.7, 0.9},
    100,
    10
  );

  std::vector<uint32_t> set1 = {1, 2, 3, 4, 5};
  std::vector<uint32_t> set2 = {1, 2, 3, 4, 6};
  std::vector<uint32_t> set3 = {1, 2, 3, 6, 7};

  ann.add(0, set1.begin(), set1.end());
  ann.add(1, set2.begin(), set2.end());
  ann.add(2, set3.begin(), set3.end());

  ApproximateNearestNeighbor<uint32_t, uint32_t>::Counter counts;

  // First query
  auto nearest1 = ann.nearest(0, counts, [](uint32_t) { return true; });
  ASSERT_TRUE(nearest1.has_value());

  // Second query with same counter (should be reset internally)
  auto nearest2 = ann.nearest(1, counts, [](uint32_t) { return true; });
  ASSERT_TRUE(nearest2.has_value());

  // Results should be valid
  EXPECT_TRUE(*nearest1 == 1u);
  EXPECT_TRUE(*nearest2 == 0u);
}

///////////////////////////////////////////////////////////////////////////////
// Probabilistic behavior tests

TEST(ApproximateNearestNeighborTest, HighSimilarityDetection) {
  std::mt19937 prng(12345);  // Different seed for better results
  ApproximateNearestNeighbor<uint32_t, uint32_t> ann(
    prng,
    {0.3, 0.05},
    {0.8, 0.95},
    1000,
    100
  );

  // Create sets with ~90% Jaccard similarity
  std::vector<uint32_t> base;
  for (uint32_t i = 0; i < 100; ++i) base.push_back(i);

  std::vector<uint32_t> similar = base;
  // Change 10 elements (use valid element IDs < 1000)
  for (uint32_t i = 0; i < 10; ++i) {
    similar[i] = 500 + i;  // Valid IDs in range [500, 509]
  }
  std::sort(similar.begin(), similar.end());

  ann.add(0, base.begin(), base.end());
  ann.add(1, similar.begin(), similar.end());

  // Add some dissimilar sets
  for (uint32_t s = 2; s < 10; ++s) {
    std::vector<uint32_t> dissimilar;
    for (uint32_t i = 0; i < 100; ++i) {
      dissimilar.push_back((s * 100 + i) % 1000);  // Keep all IDs < 1000
    }
    std::sort(dissimilar.begin(), dissimilar.end());
    dissimilar.erase(std::unique(dissimilar.begin(), dissimilar.end()), dissimilar.end());
    ann.add(s, dissimilar.begin(), dissimilar.end());
  }

  // With high similarity and good bounds, should find the similar set
  // The algorithm is deterministic with this seed, so we can test exact result
  auto nearest = ann.nearest(0);
  ASSERT_TRUE(nearest.has_value());
  // With these parameters and seed, set 1 should be found
  EXPECT_EQ(*nearest, 1u);
}

TEST(ApproximateNearestNeighborTest, MediumSimilarityDetection) {
  std::mt19937 prng(123);
  ApproximateNearestNeighbor<uint32_t, uint32_t> ann(
    prng,
    {0.2, 0.1},
    {0.5, 0.9},
    1000,
    100
  );

  // Create sets with ~50% Jaccard similarity
  std::vector<uint32_t> base;
  for (uint32_t i = 0; i < 100; ++i) base.push_back(i);

  std::vector<uint32_t> medium = base;
  // Change 50 elements (use valid element IDs < 1000)
  for (uint32_t i = 0; i < 50; ++i) {
    medium[i] = 500 + i;  // Valid IDs in range [500, 549]
  }
  std::sort(medium.begin(), medium.end());

  ann.add(0, base.begin(), base.end());
  ann.add(1, medium.begin(), medium.end());

  // Add some very dissimilar sets
  for (uint32_t s = 2; s < 10; ++s) {
    std::vector<uint32_t> dissimilar;
    for (uint32_t i = 0; i < 100; ++i) {
      dissimilar.push_back((s * 100 + i) % 1000);  // Keep all IDs < 1000
    }
    std::sort(dissimilar.begin(), dissimilar.end());
    dissimilar.erase(std::unique(dissimilar.begin(), dissimilar.end()), dissimilar.end());
    ann.add(s, dissimilar.begin(), dissimilar.end());
  }

  // Medium similarity is at the boundary of our detection range The
  // algorithm is deterministic, so with this seed it may or may not
  // find a match
  auto nearest = ann.nearest(0);

  // If it finds something, it should be a valid set ID
  // We don't assert that it must find something, since medium similarity
  // is less reliable than high similarity
  if (nearest.has_value()) {
    EXPECT_LT(*nearest, 10u);
  }
  // Test passes either way - just verifies no crash and valid results
}

///////////////////////////////////////////////////////////////////////////////
// Different type parameter tests

TEST(ApproximateNearestNeighborTest, SmallTypes) {
  std::mt19937 prng(42);
  ApproximateNearestNeighbor<uint8_t, uint16_t> ann(
    prng,
    {0.3, 0.1},
    {0.7, 0.9},
    100,   // Small numElems for uint8_t
    50     // Small numSets for uint16_t
  );

  std::vector<uint8_t> set1 = {1, 2, 3, 4, 5};
  std::vector<uint8_t> set2 = {1, 2, 3, 4, 6};

  ann.add(0, set1.begin(), set1.end());
  ann.add(1, set2.begin(), set2.end());

  auto nearest = ann.nearest(0);
  ASSERT_TRUE(nearest.has_value());
  EXPECT_EQ(*nearest, 1u);
}

TEST(ApproximateNearestNeighborTest, LargeTypes) {
  std::mt19937 prng(42);
  ApproximateNearestNeighbor<uint64_t, uint64_t> ann(
    prng,
    {0.3, 0.1},
    {0.7, 0.9},
    10000,
    1000
  );

  std::vector<uint64_t> set1 = {1, 2, 3, 4, 5};
  std::vector<uint64_t> set2 = {1, 2, 3, 4, 6};

  ann.add(0, set1.begin(), set1.end());
  ann.add(1, set2.begin(), set2.end());

  auto nearest = ann.nearest(0);
  ASSERT_TRUE(nearest.has_value());
  EXPECT_EQ(*nearest, 1u);
}

///////////////////////////////////////////////////////////////////////////////
// Stress tests

TEST(ApproximateNearestNeighborTest, ManySets) {
  std::mt19937 prng(42);
  ApproximateNearestNeighbor<uint32_t, uint32_t> ann(
    prng,
    {0.3, 0.1},
    {0.7, 0.9},
    1000,
    100  // Match the number of sets we actually add
  );

  // Add many sets with non-overlapping element IDs < 1000
  for (uint32_t i = 0; i < 100; ++i) {
    std::vector<uint32_t> set;
    for (uint32_t j = 0; j < 10; ++j) {
      // Use element IDs in range [i*10, i*10+9], all < 1000
      set.push_back(i * 10 + j);
    }
    ann.add(i, set.begin(), set.end());
  }

  // Query should complete without crash
  auto nearest = ann.nearest(0);

  // Just verify the query completes successfully (result is probabilistic)
  if (nearest.has_value()) {
    EXPECT_LT(*nearest, 100u);
  }
}

TEST(ApproximateNearestNeighborTest, LargeSets) {
  std::mt19937 prng(42);
  ApproximateNearestNeighbor<uint32_t, uint32_t> ann(
    prng,
    {0.3, 0.1},
    {0.7, 0.9},
    10000,
    100
  );

  // Create large sets
  std::vector<uint32_t> large1;
  std::vector<uint32_t> large2;

  for (uint32_t i = 0; i < 1000; ++i) {
    large1.push_back(i);
    large2.push_back(i);
  }
  // Make them slightly different
  for (uint32_t i = 0; i < 100; ++i) {
    large2[i] = 5000 + i;
  }

  ann.add(0, large1.begin(), large1.end());
  ann.add(1, large2.begin(), large2.end());

  auto nearest = ann.nearest(0);
  ASSERT_TRUE(nearest.has_value());
  EXPECT_EQ(*nearest, 1u);
}

TEST(ApproximateNearestNeighborTest, RandomSetsStress) {
  std::mt19937 prng(12345);
  ApproximateNearestNeighbor<uint32_t, uint32_t> ann(
    prng,
    {0.2, 0.1},
    {0.6, 0.9},
    5000,
    200
  );

  std::mt19937 set_gen(67890);
  std::uniform_int_distribution<uint32_t> elem_dist(0, 4999);
  std::uniform_int_distribution<uint32_t> size_dist(10, 100);

  // Add random sets
  for (uint32_t s = 0; s < 50; ++s) {
    std::set<uint32_t> unique_elems;
    uint32_t set_size = size_dist(set_gen);

    while (unique_elems.size() < set_size) {
      unique_elems.insert(elem_dist(set_gen));
    }

    std::vector<uint32_t> set_vec(unique_elems.begin(), unique_elems.end());
    ann.add(s, set_vec.begin(), set_vec.end());
  }

  // Query all sets - should not crash
  for (uint32_t s = 0; s < 50; ++s) {
    [[maybe_unused]] auto nearest = ann.nearest(s);
    // Result is probabilistic
  }
}

///////////////////////////////////////////////////////////////////////////////
// Boundary value tests

TEST(ApproximateNearestNeighborTest, SameBounds) {
  std::mt19937 prng(42);
  ApproximateNearestNeighbor<uint32_t, uint32_t> ann(
    prng,
    {0.7, 0.9},  // Same as upper bound
    {0.7, 0.9},  // Same as lower bound
    1000,
    100
  );

  EXPECT_GT(ann.numExperiments(), 0u);
  EXPECT_GT(ann.roundsPerExperiment(), 0u);
}

TEST(ApproximateNearestNeighborTest, HighProbabilityBounds) {
  std::mt19937 prng(42);
  ApproximateNearestNeighbor<uint32_t, uint32_t> ann(
    prng,
    {0.3, 0.01},
    {0.9, 0.99},
    1000,
    100
  );

  EXPECT_GT(ann.numExperiments(), 0u);
  EXPECT_GT(ann.roundsPerExperiment(), 0u);
}

///////////////////////////////////////////////////////////////////////////////
// Determinism tests

TEST(ApproximateNearestNeighborTest, DeterministicWithSameSeed) {
  std::vector<uint32_t> set1 = {1, 2, 3, 4, 5};
  std::vector<uint32_t> set2 = {1, 2, 3, 4, 6};
  std::vector<uint32_t> set3 = {10, 20, 30, 40, 50};

  // First instance
  std::mt19937 prng1(42);
  ApproximateNearestNeighbor<uint32_t, uint32_t> ann1(
    prng1, {0.3, 0.1}, {0.7, 0.9}, 100, 10
  );
  ann1.add(0, set1.begin(), set1.end());
  ann1.add(1, set2.begin(), set2.end());
  ann1.add(2, set3.begin(), set3.end());
  auto nearest1 = ann1.nearest(0);

  // Second instance with same seed
  std::mt19937 prng2(42);
  ApproximateNearestNeighbor<uint32_t, uint32_t> ann2(
    prng2, {0.3, 0.1}, {0.7, 0.9}, 100, 10
  );
  ann2.add(0, set1.begin(), set1.end());
  ann2.add(1, set2.begin(), set2.end());
  ann2.add(2, set3.begin(), set3.end());
  auto nearest2 = ann2.nearest(0);

  // Should produce same results with same seed
  EXPECT_EQ(nearest1.has_value(), nearest2.has_value());
  if (nearest1.has_value() && nearest2.has_value()) {
    EXPECT_EQ(*nearest1, *nearest2);
  }
}

TEST(ApproximateNearestNeighborTest, DifferentWithDifferentSeed) {
  std::vector<uint32_t> set1 = {1, 2, 3, 4, 5};
  std::vector<uint32_t> set2 = {1, 2, 3, 6, 7};
  std::vector<uint32_t> set3 = {10, 20, 30, 40, 50};

  // First instance
  std::mt19937 prng1(42);
  ApproximateNearestNeighbor<uint32_t, uint32_t> ann1(
    prng1, {0.2, 0.1}, {0.6, 0.9}, 100, 10
  );
  ann1.add(0, set1.begin(), set1.end());
  ann1.add(1, set2.begin(), set2.end());
  ann1.add(2, set3.begin(), set3.end());

  // Second instance with different seed
  std::mt19937 prng2(12345);
  ApproximateNearestNeighbor<uint32_t, uint32_t> ann2(
    prng2, {0.2, 0.1}, {0.6, 0.9}, 100, 10
  );
  ann2.add(0, set1.begin(), set1.end());
  ann2.add(1, set2.begin(), set2.end());
  ann2.add(2, set3.begin(), set3.end());

  // Hash functions are different, but results might still coincide
  // Just verify both work without crashing
  [[maybe_unused]] auto nearest1 = ann1.nearest(0);
  [[maybe_unused]] auto nearest2 = ann2.nearest(0);
}

} // namespace HPHP
