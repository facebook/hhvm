/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "WeightedCh3RvHashFunc.h"

#include <folly/json/dynamic.h>

#include "mcrouter/lib/WeightedCh3HashFunc.h"
#include "mcrouter/lib/fbi/cpp/util.h"

namespace facebook {
namespace memcache {

WeightedCh3RvHashFunc::WeightedCh3RvHashFunc(
    const folly::dynamic& json,
    size_t n,
    std::shared_ptr<WeightedCh3RvHashFunc::ObservableRuntimeVars>
        rvObservable) {
  checkLogic(json.isObject(), "WeightedCh3RvHashFunc: not an object");
  checkLogic(
      rvObservable != nullptr,
      "WeightedCh3RvHashFunc: runtime vars observable is not available");

  auto jWeightsRv = json.get_ptr("weights_rv");
  checkLogic(
      jWeightsRv != nullptr, "WeightedCh3RvHashFunc: weights_rv is missing");
  checkLogic(
      jWeightsRv->isString(),
      "WeightedCh3RvHashFunc: weights_rv is not a string");

  weights_ = std::make_shared<Weights>();

  weights_->weightsRv = jWeightsRv->getString();
  weights_->weights.resize(n, 0.5);
  weights_->rvObservableHandle = rvObservable->subscribeAndCall(
      [&weights = *weights_](
          std::shared_ptr<const mcrouter::RuntimeVarsData> oldVars,
          std::shared_ptr<const mcrouter::RuntimeVarsData> newVars) {
        weights.updateRuntimeVarsData(std::move(oldVars), std::move(newVars));
      });
  return;
}

WeightedCh3RvHashFunc::~WeightedCh3RvHashFunc() {
  weights_.reset();
}

size_t WeightedCh3RvHashFunc::hash(
    folly::StringPiece key,
    folly::Range<const double*> weights,
    size_t retryCount) {
  return WeightedCh3HashFunc::hash(key, weights, retryCount);
}

WeightedCh3RvHashFunc::Weights::~Weights() {
  rvObservableHandle.reset();
}

void WeightedCh3RvHashFunc::Weights::updateRuntimeVarsData(
    std::shared_ptr<const mcrouter::RuntimeVarsData> /* oldVars */,
    std::shared_ptr<const mcrouter::RuntimeVarsData> newVars) {
  if (newVars != nullptr) {
    auto d = newVars->getVariableByName("hash_route_weights");
    if (d != nullptr) {
      folly::json::serialization_opts opts;
      opts.sort_keys = true;
      opts.pretty_formatting = true;
    }
  }
  if (!newVars || weightsRv.empty()) {
    return;
  }

  auto weightsRvs = newVars->getVariableByName("hash_route_weights");
  checkLogic(
      weightsRvs != nullptr && weightsRvs.isObject(),
      "WeightedCh3RvHashFunc: runtime var hash_route_weights is not an object");

  auto jWeights = weightsRvs.get_ptr(weightsRv);
  checkLogic(
      jWeights != nullptr,
      "WeightedCh3RvHashFunc: runtime var {} doesn't exist",
      weightsRv);
  checkLogic(
      jWeights->isArray(),
      "WeightedCh3RvHashFunc: runtime var {} is not an array",
      weightsRv);

  LOG_IF(ERROR, jWeights->size() < weights.size())
      << "WeightedCh3RvHashFunc: CONFIG IS BROKEN!!! number of weights ("
      << jWeights->size()
      << ") from runtime var is smaller than number of weights in hash func ("
      << weights.size() << "). Missing weights are set to 0.5";

  LOG_IF(ERROR, jWeights->size() > weights.size())
      << "WeightedCh3RvHashFunc: CONFIG IS BROKEN!!! number of weights ("
      << jWeights->size()
      << ") from runtime var is larger than number of weights in hash func ("
      << weights.size() << "). Additional weights are ignored";

  auto numWeights = std::min(weights.size(), jWeights->size());
  for (size_t i = 0; i < numWeights; ++i) {
    checkLogic(
        (*jWeights)[i].isNumber(),
        "WeightedCh3RvHashFunc: runtime var weight is not number");

    const auto newWeight = (*jWeights)[i].asDouble();
    checkLogic(
        0 <= newWeight && newWeight <= 1.0,
        "WeightedCh3RvHashFunc: runtime var weight must be in range [0, 1.0]");

    weights[i] = newWeight;
  }
  for (size_t i = numWeights; i < weights.size(); ++i) {
    weights[i] = 0.5;
  }
}

} // namespace memcache
} // namespace facebook
