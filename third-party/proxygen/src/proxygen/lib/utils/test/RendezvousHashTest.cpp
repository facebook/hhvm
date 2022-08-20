/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <folly/Conv.h>
#include <folly/container/Foreach.h>
#include <folly/portability/GTest.h>
#include <map>
#include <vector>

#include <proxygen/lib/utils/RendezvousHash.h>

using namespace proxygen;

TEST(RendezvousHash, Consistency) {
  RendezvousHash hashes;
  std::vector<std::pair<std::string, uint64_t>> nodes;
  for (int i = 0; i < 10; ++i) {
    nodes.emplace_back(folly::to<std::string>("key", i), 1);
  }
  hashes.build(nodes);

  for (size_t rank = 0; rank < nodes.size() + 2; rank++) {
    std::map<uint64_t, size_t> mapping;
    for (int i = 0; i < 10000; ++i) {
      mapping[i] = hashes.get(i, rank);
    }

    for (auto&& [key, expected] : mapping) {
      EXPECT_EQ(expected, hashes.get(key, rank));
    }
  }
}

TEST(RendezvousHash, ConsistencyWithNewNode) {
  RendezvousHash hashes;
  int numNodes = 10;
  std::vector<std::pair<std::string, uint64_t>> nodes;
  for (int i = 0; i < numNodes; ++i) {
    nodes.emplace_back(folly::to<std::string>("key", i), 1);
  }
  hashes.build(nodes);
  std::map<uint64_t, size_t> mapping;
  for (uint64_t i = 0; i < 10000; ++i) {
    mapping[i] = hashes.get(i);
  }
  hashes = RendezvousHash();
  // Adding a new node and rebuild the hash
  nodes.emplace_back(folly::to<std::string>("key", numNodes), 1);
  hashes.build(nodes);
  // traffic should only flow to the new node
  for (auto&& [key, expected] : mapping) {
    size_t id = hashes.get(key);
    EXPECT_TRUE(expected == id || numNodes == int(id));
  }
}

TEST(RendezvousHash, ConsistencyWithIncreasedWeight) {
  RendezvousHash hashes;
  int numNodes = 10;
  std::vector<std::pair<std::string, uint64_t>> nodes;
  for (int i = 0; i < numNodes; ++i) {
    nodes.emplace_back(folly::to<std::string>("key", i), i);
  }
  hashes.build(nodes);

  std::map<uint64_t, size_t> mapping;
  for (uint64_t i = 0; i < 10000; ++i) {
    mapping[i] = hashes.get(i);
  }

  // Increase the weight by 2
  nodes.clear();
  hashes = RendezvousHash();
  for (int i = 0; i < numNodes; ++i) {
    nodes.emplace_back(folly::to<std::string>("key", i), i * 2);
  }
  hashes.build(nodes);

  // traffic shouldn't flow at all
  for (auto&& [key, expected] : mapping) {
    EXPECT_EQ(expected, hashes.get(key));
  }
}

TEST(RendezvousHash, ConsistentFlowToIncreasedWeightNode) {
  RendezvousHash hashes;
  int numNodes = 10;
  std::vector<std::pair<std::string, uint64_t>> nodes;
  for (int i = 0; i < numNodes; ++i) {
    nodes.emplace_back(folly::to<std::string>("key", i), i);
  }
  hashes.build(nodes);

  std::map<uint64_t, size_t> mapping;
  for (uint64_t i = 0; i < 10000; ++i) {
    mapping[i] = hashes.get(i);
  }

  nodes.clear();
  // Increase the weight for a single node
  hashes = RendezvousHash();

  nodes.emplace_back(folly::to<std::string>("key", 0), 10);

  for (int i = 1; i < numNodes; ++i) {
    nodes.emplace_back(folly::to<std::string>("key", i), i);
  }
  hashes.build(nodes);
  // traffic should only flow to the first node
  for (auto&& [key, expected] : mapping) {
    size_t id = hashes.get(key);
    EXPECT_TRUE(expected == id || 0 == int(id));
  }
}

TEST(RendezvousHash, ConsistentFlowToDecreasedWeightNodes) {
  RendezvousHash hashes;
  int numNodes = 18;
  std::vector<std::pair<std::string, uint64_t>> nodes;
  for (int i = 0; i < numNodes; ++i) {
    nodes.emplace_back(folly::to<std::string>("key", i), 100);
  }
  hashes.build(nodes);
  std::map<uint64_t, size_t> mapping;
  for (uint64_t i = 0; i < 10000; ++i) {
    mapping[i] = hashes.get(i);
  }

  nodes.clear();
  hashes = RendezvousHash();

  // decrease the weights for 5 nodes
  for (int i = 0; i < 5; ++i) {
    nodes.emplace_back(folly::to<std::string>("key", i), 50);
  }

  // keep the weights for the rest unchanged
  for (int i = 5; i < numNodes; ++i) {
    nodes.emplace_back(folly::to<std::string>("key", i), 100);
  }

  hashes.build(nodes);
  for (auto&& [key, expected] : mapping) {
    // traffic should only flow to nodes with decreased nodes
    size_t id = hashes.get(key);
    EXPECT_TRUE(expected == id || id >= 5);
  }
}

TEST(RendezvousHash, ConsistentFlowToDecreasedWeightNode) {
  RendezvousHash hashes;
  int numNodes = 10;
  std::vector<std::pair<std::string, uint64_t>> nodes;
  for (int i = 0; i < numNodes; ++i) {
    nodes.emplace_back(folly::to<std::string>("key", i), i);
  }
  hashes.build(nodes);
  std::map<uint64_t, size_t> mapping;
  for (uint64_t i = 0; i < 10000; ++i) {
    mapping[i] = hashes.get(i);
  }

  // Increase the weight for a single node
  nodes.clear();
  hashes = RendezvousHash();

  for (int i = 0; i < numNodes - 1; ++i) {
    nodes.emplace_back(folly::to<std::string>("key", i), i);
  }

  // zero the weight of the last node
  nodes.emplace_back(folly::to<std::string>("key", numNodes - 1), 0);
  hashes.build(nodes);
  for (auto&& [key, expected] : mapping) {
    // traffic should only flow from the zero weight cluster to others
    size_t id = hashes.get(key);
    if (expected == (uint64_t)numNodes - 1) {
      EXPECT_TRUE(expected != id);
    } else {
      EXPECT_TRUE(expected == id);
    }
  }
}

TEST(RendezvousHash, ConsistencyWithDecreasedWeight) {
  RendezvousHash hashes;
  int numNodes = 10;
  std::vector<std::pair<std::string, uint64_t>> nodes;
  for (int i = 0; i < numNodes; ++i) {
    nodes.emplace_back(folly::to<std::string>("key", i), i * 2);
  }
  hashes.build(nodes);
  std::map<uint64_t, size_t> mapping;
  for (uint64_t i = 0; i < 10000; ++i) {
    mapping[i] = hashes.get(i);
  }

  // Decrease the weight by 2
  nodes.clear();
  hashes = RendezvousHash();

  for (int i = 0; i < numNodes; ++i) {
    nodes.emplace_back(folly::to<std::string>("key", i), i);
  }
  hashes.build(nodes);

  // traffic shouldn't flow at all
  for (auto&& [key, expected] : mapping) {
    EXPECT_EQ(expected, hashes.get(key));
  }
}

TEST(ConsistentHashRing, DistributionAccuracy) {
  std::vector<std::string> keys = {
      "ash_proxy", "prn_proxy", "snc_proxy", "frc_proxy"};

  std::vector<std::vector<uint64_t>> weights = {
      {248, 342, 2, 384},
      {10, 10, 10, 10},
      {25, 25, 10, 10},
      {100, 10, 10, 1},
      {100, 5, 5, 5},
      {922337203685, 12395828300, 50192385101, 59293845010}};

  for (auto& weight : weights) {
    RendezvousHash hashes;
    std::vector<std::pair<std::string, uint64_t>> nodes;
    FOR_EACH_RANGE(i, 0, keys.size()) {
      nodes.emplace_back(keys[i], weight[i]);
    }
    hashes.build(nodes);

    std::vector<uint64_t> distribution(keys.size());

    for (uint64_t i = 0; i < 21000; ++i) {
      distribution[hashes.get(i)]++;
    }

    uint64_t totalWeight = 0;

    for (auto& w : weight) {
      totalWeight += w;
    }

    double maxError = 0.0;
    for (size_t i = 0; i < keys.size(); ++i) {
      double expected = 100.0 * weight[i] / totalWeight;
      double actual = 100.0 * distribution[i] / 21000;

      maxError = std::max(maxError, fabs(expected - actual));
    }
    // make sure the error rate is less than 1.0%
    EXPECT_LE(maxError, 1.0);
  }
}

TEST(RendezvousHash, selectNUnweighted) {
  RendezvousHash hashes;
  std::vector<std::pair<std::string, uint64_t>> nodes;
  int size = 100;
  for (int i = 0; i < size; ++i) {
    nodes.emplace_back(folly::to<std::string>("key", i), 1);
  }
  hashes.build(nodes);
  auto seed = 91484253;

  // rank > size
  auto select = hashes.selectNUnweighted(seed, size + 10);
  EXPECT_EQ(select.size(), size);
  for (int i = 0; i < size; i++) {
    EXPECT_EQ(select[i], i);
  }

  // check valid index in selection
  int rank = size / 4;
  select = hashes.selectNUnweighted(seed, rank);
  EXPECT_EQ(select.size(), rank);
  std::set<size_t> uniqueIndex;
  for (auto index : select) {
    EXPECT_EQ(uniqueIndex.count(index), 0);
    uniqueIndex.insert(index);
    EXPECT_LT(index, size);
  }

  // change seed, check different selection
  for (int i = 1; i < 100; i++) {
    select = hashes.selectNUnweighted(seed + i, rank);
    int different = 0;
    for (auto index : select) {
      if (uniqueIndex.count(index) == 0) {
        different++;
      }
    }
    EXPECT_GT(different, 0);
  }
}
