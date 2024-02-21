/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <vector>

#include <gtest/gtest.h>

#include <folly/json/json.h>

#include "mcrouter/lib/WeightedChHashFuncBase.h"

using namespace facebook::memcache;

/// Ensure parseWeights throws errors appropriately
TEST(WeightedChHashFuncBase, parseWeightsTest) {
  const size_t n = 2;
  // Json is an object
  {
    folly::dynamic settings = folly::dynamic::array;
    ASSERT_THROW(
        WeightedChHashFuncBase::parseWeights(settings, n), std::logic_error);
  }
  // Weights key exists
  {
    folly::dynamic settings = folly::dynamic::object;
    ASSERT_THROW(
        WeightedChHashFuncBase::parseWeights(settings, n), std::logic_error);
  }
  // Weights is an array
  {
    folly::dynamic settings = folly::dynamic::object;
    settings["weights"] = false;
    ASSERT_THROW(
        WeightedChHashFuncBase::parseWeights(settings, n), std::logic_error);
  }
  // Weights is in range [0, 1.0]
  {
    folly::dynamic settings = folly::dynamic::object;
    {
      settings["weights"] = folly::dynamic::array(0.5, 2);
      ASSERT_THROW(
          WeightedChHashFuncBase::parseWeights(settings, n), std::logic_error);
    }
    {
      settings["weights"] = folly::dynamic::array(0.5, -1);
      ASSERT_THROW(
          WeightedChHashFuncBase::parseWeights(settings, n), std::logic_error);
    }
    {
      settings["weights"] = folly::dynamic::array(0.5, 0.75);
      ASSERT_NO_THROW(WeightedChHashFuncBase::parseWeights(settings, n));
    }
    {
      settings["weights"] = folly::dynamic::array(0, 1.0);
      ASSERT_NO_THROW(WeightedChHashFuncBase::parseWeights(settings, n));
    }
  }
  // Number of weights is smaller than the number of servers */
  {
    const size_t biggerN = 10;
    folly::dynamic settings = folly::dynamic::object;
    settings["weights"] = folly::dynamic::array(0.9, 0.75);
    const std::vector<double> weights =
        WeightedChHashFuncBase::parseWeights(settings, biggerN);
    ASSERT_EQ(weights.size(), biggerN);
    ASSERT_EQ(weights[0], 0.9);
    ASSERT_EQ(weights[1], 0.75);
    for (uint i = 2; i < weights.size(); ++i) {
      ASSERT_EQ(weights[i], 0.5);
    }
  }
}
