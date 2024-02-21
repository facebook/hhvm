/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "mcrouter/lib/WeightedChHashFuncBase.h"

#include <glog/logging.h>

#include <folly/json/dynamic.h>
#include "mcrouter/lib/fbi/cpp/util.h"

namespace facebook {
namespace memcache {

std::vector<double> WeightedChHashFuncBase::parseWeights(
    const folly::dynamic& json,
    size_t n) {
  std::vector<double> weights;
  weights.reserve(n);

  checkLogic(
      json.isObject() && json.count("weights"),
      "WeightedChHashFunc: not an object or no weights");
  checkLogic(
      json["weights"].isArray(), "WeightedChHashFunc: weights is not array");
  const auto& jWeights = json["weights"];
  LOG_IF(ERROR, jWeights.size() < n)
      << "WeightedChHashFunc: CONFIG IS BROKEN!!! number of weights ("
      << jWeights.size() << ") is smaller than number of servers (" << n
      << "). Missing weights are set to 0.5";
  size_t i = 0;
  for (const auto& weight : jWeights) {
    if (i == n) {
      break;
    }
    checkLogic(weight.isNumber(), "WeightedChHashFunc: weight is not number");
    const auto weightNum = weight.asDouble();
    checkLogic(
        0 <= weightNum && weightNum <= 1.0,
        "WeightedChHashFunc: weight must be in range [0, 1.0]");
    weights.push_back(weightNum);
    ++i;
  }
  weights.resize(n, 0.5);
  return weights;
}

} // namespace memcache
} // namespace facebook
