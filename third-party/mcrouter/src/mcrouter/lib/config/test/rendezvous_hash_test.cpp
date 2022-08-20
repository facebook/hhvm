/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <cmath>
#include <map>
#include <random>
#include <vector>

#include <gtest/gtest.h>

#include <folly/Conv.h>

#include "mcrouter/lib/config/RendezvousHash.h"

using namespace facebook::memcache;

TEST(RendezvousHash, Consistency) {
  constexpr size_t kNumNodes = 10;
  std::vector<std::pair<std::string, double>> nodes;
  for (size_t i = 0; i < kNumNodes; ++i) {
    nodes.emplace_back(folly::to<std::string>("key", i), 1);
  }
  RendezvousHash hashes(nodes.begin(), nodes.end());

  constexpr size_t kNumKeys = 10000;
  size_t mapping[kNumKeys];
  for (size_t i = 0; i < kNumKeys; ++i) {
    mapping[i] = hashes.get(i);
  }

  for (size_t i = 0; i < kNumKeys; ++i) {
    ASSERT_EQ(mapping[i], hashes.get(i));
  }
}

TEST(RendezvousHash, ConsistencyShuffle) {
  constexpr size_t kNumNodes = 10;
  std::vector<std::pair<std::string, double>> nodes;
  for (size_t i = 0; i < kNumNodes; ++i) {
    nodes.emplace_back(folly::to<std::string>("key", i), i);
  }
  RendezvousHash hashes(nodes.begin(), nodes.end());

  constexpr size_t kNumKeys = 10000;
  size_t mapping[kNumKeys];
  for (size_t i = 0; i < kNumKeys; ++i) {
    mapping[i] = hashes.get(i);
  }

  std::mt19937 rng;
  std::shuffle(nodes.begin(), nodes.end(), rng);
  hashes = RendezvousHash(nodes.begin(), nodes.end());

  // traffic shouldn't flow after reshuffle
  for (size_t i = 0; i < kNumKeys; ++i) {
    auto id = hashes.get(i);
    // weight is the same as id before reshuffle
    auto idBeforeShuffle = nodes[id].second;
    ASSERT_EQ(mapping[i], idBeforeShuffle);
  }
}

TEST(RendezvousHash, ConsistencyWithNewNode) {
  constexpr size_t kNumNodes = 10;
  std::vector<std::pair<std::string, double>> nodes;
  for (size_t i = 0; i < kNumNodes; ++i) {
    nodes.emplace_back(folly::to<std::string>("key", i), 1);
  }
  RendezvousHash hashes(nodes.begin(), nodes.end());

  constexpr size_t kNumKeys = 10000;
  size_t mapping[kNumKeys];
  for (uint64_t i = 0; i < kNumKeys; ++i) {
    mapping[i] = hashes.get(i);
  }
  // Adding a new node and rebuild the hash
  nodes.emplace_back(folly::to<std::string>("key", kNumNodes), 1);
  hashes = RendezvousHash(nodes.begin(), nodes.end());

  // traffic should only flow to the new node
  for (size_t i = 0; i < kNumKeys; ++i) {
    size_t id = hashes.get(i);
    ASSERT_TRUE(id == mapping[i] || id == kNumNodes) << id << " " << mapping[i];
  }
}

TEST(RendezvousHash, ConsistencyWithIncreasedWeight) {
  constexpr size_t kNumNodes = 10;
  std::vector<std::pair<std::string, double>> nodes;
  for (size_t i = 0; i < kNumNodes; ++i) {
    nodes.emplace_back(folly::to<std::string>("key", i), i);
  }
  RendezvousHash hashes(nodes.begin(), nodes.end());

  constexpr size_t kNumKeys = 10000;
  size_t mapping[kNumKeys];
  for (size_t i = 0; i < kNumKeys; ++i) {
    mapping[i] = hashes.get(i);
  }

  // Increase the weight by 2
  for (auto& node : nodes) {
    node.second *= 2;
  }
  hashes = RendezvousHash(nodes.begin(), nodes.end());

  // traffic shouldn't flow at all
  for (size_t i = 0; i < kNumKeys; ++i) {
    ASSERT_EQ(mapping[i], hashes.get(i));
  }
}

TEST(RendezvousHash, ConsistentFlowToIncreasedWeightNode) {
  constexpr size_t kNumNodes = 10;
  std::vector<std::pair<std::string, double>> nodes;
  for (size_t i = 0; i < kNumNodes; ++i) {
    nodes.emplace_back(folly::to<std::string>("key", i), i);
  }
  RendezvousHash hashes(nodes.begin(), nodes.end());

  constexpr size_t kNumKeys = 10000;
  size_t mapping[kNumKeys];
  for (size_t i = 0; i < 10000; ++i) {
    mapping[i] = hashes.get(i);
  }

  // Increase the weight for a single node
  nodes[0].second = 10;
  hashes = RendezvousHash(nodes.begin(), nodes.end());

  // traffic should only flow to the first node
  for (size_t i = 0; i < kNumKeys; ++i) {
    size_t id = hashes.get(i);
    ASSERT_TRUE(id == mapping[i] || id == 0);
  }
}

TEST(RendezvousHash, ConsistentFlowToDecreasedWeightNodes) {
  constexpr size_t kNumNodes = 18;
  std::vector<std::pair<std::string, double>> nodes;
  for (size_t i = 0; i < kNumNodes; ++i) {
    nodes.emplace_back(folly::to<std::string>("key", i), 100);
  }
  RendezvousHash hashes(nodes.begin(), nodes.end());

  constexpr size_t kNumKeys = 10000;
  size_t mapping[kNumKeys];
  for (uint64_t i = 0; i < kNumKeys; ++i) {
    mapping[i] = hashes.get(i);
  }

  // decrease the weights for 5 nodes
  for (size_t i = 0; i < 5; ++i) {
    nodes[i].second = 50;
  }
  hashes = RendezvousHash(nodes.begin(), nodes.end());

  for (size_t i = 0; i < kNumKeys; ++i) {
    // traffic shouldn't flow to nodes with decreased weights
    size_t id = hashes.get(i);
    ASSERT_TRUE(id == mapping[i] || id >= 5);
  }
}

TEST(RendezvousHash, ConsistentFlowToDecreasedWeightNode) {
  constexpr size_t kNumNodes = 10;
  std::vector<std::pair<std::string, double>> nodes;
  for (size_t i = 0; i < kNumNodes; ++i) {
    nodes.emplace_back(folly::to<std::string>("key", i), i);
  }
  RendezvousHash hashes(nodes.begin(), nodes.end());

  constexpr size_t kNumKeys = 10000;
  size_t mapping[kNumKeys];
  for (size_t i = 0; i < kNumKeys; ++i) {
    mapping[i] = hashes.get(i);
  }

  // zero the weight of the last node
  nodes.back().second = 0;
  hashes = RendezvousHash(nodes.begin(), nodes.end());
  for (size_t i = 0; i < kNumKeys; ++i) {
    // traffic should only flow from the zero weight node to others
    size_t id = hashes.get(i);
    if (mapping[i] == kNumNodes - 1) {
      ASSERT_TRUE(mapping[i] != id);
    } else {
      ASSERT_TRUE(mapping[i] == id);
    }
  }
}

TEST(RendezvousHash, ConsistencyWithDecreasedWeight) {
  constexpr size_t kNumNodes = 10;
  std::vector<std::pair<std::string, uint64_t>> nodes;
  for (size_t i = 0; i < kNumNodes; ++i) {
    nodes.emplace_back(folly::to<std::string>("key", i), i * 2);
  }
  RendezvousHash hashes(nodes.begin(), nodes.end());

  constexpr size_t kNumKeys = 10000;
  size_t mapping[kNumKeys];
  for (size_t i = 0; i < kNumKeys; ++i) {
    mapping[i] = hashes.get(i);
  }

  // Decrease the weight by 2
  for (auto& it : nodes) {
    it.second /= 2;
  }
  hashes = RendezvousHash(nodes.begin(), nodes.end());

  // traffic shouldn't flow at all
  for (size_t i = 0; i < kNumKeys; ++i) {
    ASSERT_EQ(mapping[i], hashes.get(i));
  }
}

TEST(ConsistentHashRing, DistributionAccuracy) {
  constexpr size_t kNumRequests = 20000;
  folly::StringPiece keys[] = {
      "first_proxy",
      "second_proxy",
      "third_proxy",
      "fourth_proxy",
      "a",
      "b",
      "c",
      "d",
      "e",
      "f"};

  std::vector<double> weights[] = {
      {248.0, 342.0, 2.0, 384.0},
      {10.0, 10.0, 10.0, 10.0},
      {25.0, 25.0, 10.0, 10.0},
      {100.0, 10.0, 10.0, 1.0},
      {100.0, 5.0, 5.0, 5.0},
      {922337203685.0, 12395828300.0, 50192385101.0, 59293845010.0},
      {0.5, 0.1, 0.01, 1.0},
      {0.9, 1.1, 10, 0.01},
      {0.01, 0.001, 0.0001, 0.0},
      {0.5, 1.0},
      {0.1},
      {100, 1, 0, 0, 0, 0, 0, 0, 1, 1},
      {0.5, 1.0, 0.5, 1.0, 0.5, 1.0, 0.5, 1.0},
      {1e-7, 1e-8, 1e-9},
      {1e-8, 1.0},
  };

  for (auto& weight : weights) {
    std::vector<std::pair<folly::StringPiece, double>> nodes;
    for (size_t i = 0; i < weight.size(); ++i) {
      nodes.emplace_back(keys[i], weight[i]);
    }
    RendezvousHash hashes(nodes.begin(), nodes.end());

    std::vector<size_t> distribution(weight.size());
    for (size_t i = 0; i < kNumRequests; ++i) {
      distribution[hashes.get(i)]++;
    }

    double totalWeight = 0;

    for (auto& w : weight) {
      totalWeight += w;
    }

    double maxError = 0.0;
    for (size_t i = 0; i < weight.size(); ++i) {
      double expected = 100.0 * weight[i] / totalWeight;
      double actual = 100.0 * distribution[i] / kNumRequests;

      maxError = std::max(maxError, std::fabs(expected - actual));
    }
    // make sure the error rate is less than 1.0%
    EXPECT_LE(maxError, 1.0);
  }
}
