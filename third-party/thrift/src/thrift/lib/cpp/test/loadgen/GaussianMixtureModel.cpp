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

#include <thrift/lib/cpp/test/loadgen/GaussianMixtureModel.h>

#include <cmath>
#include <vector>
#include <boost/random/normal_distribution.hpp>
#include <boost/random/variate_generator.hpp>
#include <glog/logging.h>
#include <thrift/lib/cpp/test/loadgen/RNG.h>

namespace apache::thrift::loadgen {

GaussianMixtureModel::GaussianMixtureModel(const folly::dynamic& cfgDict) {
  auto gmmCfgDict = cfgDict["gmm"];
  logTransformed = cfgDict["log_transformed"].getBool();
  CHECK(gmmCfgDict.isObject());
  nComponents = gmmCfgDict["n_components"].getInt();
  auto mus_2 = gmmCfgDict["mus_2"];
  auto sigmas_2 = gmmCfgDict["sigmas_2"];
  auto weights = gmmCfgDict["weights"];
  double weightCumsum = 0;
  CHECK_EQ(nComponents, mus_2.size());
  CHECK_EQ(nComponents, sigmas_2.size());
  CHECK_EQ(nComponents, weights.size());
  for (uint32_t idx = 0; idx < nComponents; ++idx) {
    weightCumsum += weights[idx].getDouble();
    weightsCumsum.push_back(weightCumsum);
    double mean = mus_2[idx].getDouble();
    double sigma = sigmas_2[idx].getDouble();
    this->mus.push_back(mean);
    this->sigmas.push_back(sigma);
  }
  threadLocalGaussianGenerators =
      folly::ThreadLocal<TGaussianGenerators>([this] {
        TGaussianGenerators ggP = TGaussianGenerators();
        for (uint32_t idx = 0; idx < this->nComponents; ++idx) {
          boost::normal_distribution<double> dist(
              this->mus[idx], this->sigmas[idx]);
          ggP.emplace_back(RNG::getRNG(), dist);
        }
        return ggP;
      });
  // check the sum of weights is close to 1.0
  CHECK_LE(std::abs(weightCumsum - 1.0), 1e-7);
}

double GaussianMixtureModel::getSample() {
  // assuming RNG::getReal() by default return [0, 1)
  double componentChoice = RNG::getReal();
  for (uint32_t idx = 0; idx < nComponents; ++idx) {
    if (weightsCumsum[idx] >= componentChoice) {
      double gmmSample = (*threadLocalGaussianGenerators.get())[idx]();
      if (logTransformed) {
        return exp(gmmSample);
      } else {
        return gmmSample;
      }
    }
  }
  LOG(FATAL)
      << "The weights vector or the uniform random real are probably wrong.";
  return 0;
}

} // namespace apache::thrift::loadgen
