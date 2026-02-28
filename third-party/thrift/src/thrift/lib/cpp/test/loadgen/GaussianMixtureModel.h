/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <vector>
#include <boost/random/normal_distribution.hpp>
#include <boost/random/variate_generator.hpp>
#include <folly/ThreadLocal.h>
#include <folly/dynamic.h>
#include <thrift/lib/cpp/test/loadgen/RNG.h>

namespace apache::thrift::loadgen {

/**
 * A Gaussian Mixture Model (GMM) that relies on the loadgen::RNG and
 * boost::normal_distribution
 *
 * A GMM config looks like:
 * ```json
 * {
 *   "id": "cost_model_name",
 *   "metrics": {
 *     "cpu_time": {
 *       "log_transformed": true,
 *       "min": 0.0,
 *       "max": 10.0,
 *       "mean": 5.0,
 *       "strip_left": 0,
 *       "strip_right": 0.01,
 *       "gmm": {
 *         "n_components": 3,
 *         "mus": [
 *           5.052434022867472,
 *           5.797919361306436,
 *           4.560631557715017
 *         ],
 *         "sigmas": [
 *           0.22243745992799488,
 *           0.2534454625260933,
 *           0.22913981810890852
 *         ],
 *         "weights": [
 *           0.37574910896323904,
 *           0.2817285538897495,
 *           0.34252233714701397
 *         ]
 *       }
 *     },
 *     "off_cpu_time": {
 *       ... // same as the above "cpu_time"
 *     }
 *   }
 * }
 * ```
 *
 * Config semantics:
 *   - "id": the identifier of the config.
 *   - "cpu_time": the distribution of time spent on CPU (i.e. emulated by
 *   `burn` in the loadgen)
 *   - "off_cpu_time": the distribution of time spent off-CPU (i.e. emulated by
 *   `sleep` in the loadgen).
 *     - "log_transformed": whether the random variable (cpu/off_cpu time) has
 *     been log-transformed before fitting to GMM. If true, the random variable
 *     is actually modeled as a mixture of log-normal distribution.
 *     - "min", "max", "mean": some high-level statistics of the raw data used
 *     to fit the GMM.
 *     - "strip_left": the percentile on the left tail of the distribution that
 *     got discard in the GMM. This parameter documents how outliers were
 *     removed from the original dataset.
 *     - "strip_right": the percentile on the right tail of the distribution
 *     that got discarded in the GMM. E.g., 0.01 means only model up to p99.
 *     - "gmm": the parameters of the fitted GMM
 *       - "n_components": the number of independent normal distribution
 *       - "mus": the $\mu$ in each normal distribution $N(\mu, \sigma^2)$
 *       - "sigmas": the $\sigma$ in each $N(\mu, \sigma^2)$
 *       - "weights": how different normal distributions are mixed together. Sum
 *       should be 1.0
 *
 * How samples are generated from a GaussianMixtureModel:
 *   1. Uniformly sample one normal distribution based on the "weights" of all
 *   components.
 *   2. Take a sample X from the chosen normal distribution
 *   3. If the random variable was log-transformed, return exp(X) instead of X.
 */
class GaussianMixtureModel {
 public:
  explicit GaussianMixtureModel(const folly::dynamic& cfgDict);
  double getSample();

 private:
  bool logTransformed;
  uint32_t nComponents;
  std::vector<double> mus; // location
  std::vector<double> sigmas; // scale
  // accumulated sum of weights, last element should be 1.0
  std::vector<double> weightsCumsum;
  // the normal distribution generators, one for each gaussian component
  // the vector of generators is ThreadLocal because:
  //   1. the boost normal distribution sampling relies on
  //   apache::thrift::loadgen::RNG
  //   2. the underlying RNG is ThreadLocal
  using TGaussianGenerators = std::vector<
      boost::variate_generator<RNG, boost::normal_distribution<double>>>;
  folly::ThreadLocal<TGaussianGenerators> threadLocalGaussianGenerators;
};

} // namespace apache::thrift::loadgen
