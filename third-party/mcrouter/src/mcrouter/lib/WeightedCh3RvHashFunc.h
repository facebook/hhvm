/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "mcrouter/lib/HashFunctionType.h"
#include "mcrouter/lib/Observable.h"
#include "mcrouter/lib/RuntimeVarsData.h"

namespace facebook {
namespace memcache {

/**
 * A weighted CH3 hash function using runtime var to update weights.
 *
 */
class WeightedCh3RvHashFunc {
 public:
  using ObservableRuntimeVars =
      mcrouter::Observable<std::shared_ptr<const mcrouter::RuntimeVarsData>>;

  /**
   * @param json  Json object of the following format:
   *              {
   *                "weights_rv": "weights_rv_name"
   *              }
   * @param n     Number of servers in the config.
   */
  explicit WeightedCh3RvHashFunc(
      const folly::dynamic& json,
      size_t n,
      std::shared_ptr<ObservableRuntimeVars> rvObservable);

  WeightedCh3RvHashFunc(const WeightedCh3RvHashFunc& other) = default;
  WeightedCh3RvHashFunc& operator=(const WeightedCh3RvHashFunc& other) =
      default;
  WeightedCh3RvHashFunc(WeightedCh3RvHashFunc&& other) = default;
  WeightedCh3RvHashFunc& operator=(WeightedCh3RvHashFunc&& other) = default;
  ~WeightedCh3RvHashFunc();

  size_t operator()(folly::StringPiece key) const {
    return hash(key, weights_->weights);
  }

  static const char* type() {
    return "WeightedCh3Rv";
  }

  static HashFunctionType typeId() {
    return HashFunctionType::WeightedCh3Rv;
  }

  static size_t hash(
      folly::StringPiece key,
      folly::Range<const double*> weights,
      size_t retryCount = kNumTries);

  const std::vector<double>& weights() const {
    return weights_->weights;
  }

 private:
  static constexpr size_t kNumTries = 32;

  struct Weights {
    ObservableRuntimeVars::CallbackHandle rvObservableHandle;
    std::vector<double> weights;
    std::string weightsRv;

    Weights() = default;
    Weights(const Weights& other) = delete;
    Weights(Weights&& other) = delete;
    Weights& operator=(const Weights& other) = delete;
    Weights& operator=(Weights&& other) = delete;
    ~Weights();

    void updateRuntimeVarsData(
        std::shared_ptr<const mcrouter::RuntimeVarsData> oldVars,
        std::shared_ptr<const mcrouter::RuntimeVarsData> newVars);
  };

  std::shared_ptr<Weights> weights_;
};

} // namespace memcache
} // namespace facebook
