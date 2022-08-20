/*
 * Copyright (c) Facebook, Inc. and its affiliates.
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

#include <thrift/lib/cpp/test/loadgen/RNG.h>

#include <boost/random/lognormal_distribution.hpp>
#include <boost/random/uniform_int.hpp>
#include <boost/random/uniform_real.hpp>
#include <boost/random/variate_generator.hpp>

#include <folly/ThreadLocal.h>

namespace apache {
namespace thrift {
namespace loadgen {

const bool RNG::has_fixed_range;

namespace {

// We use seedRNG just to choose seeds for new thread-local RNGs
boost::mt19937 seedRNG;

struct RNGImpl {
  RNGImpl() : rng(), rngWrapper(&rng) {
    // Pick a value from seedRNG to seed this new rng
    rng.seed(seedRNG());
  }

  /// The actual RNG
  boost::mt19937 rng;

  /**
   * An RNG wrapper object.
   *
   * We store a thread local RNG rather than always creating RNG objects onthe
   * fly so that RNG::getRNG() can return a reference rather than a new object.
   * Many of the boost random APIs require a reference, so this allows callers
   * to pass the result of RNG::getRNG() directly to boost, rather than having
   * to create a local RNG object on their stack.
   */
  RNG rngWrapper;
};

folly::ThreadLocal<RNGImpl> threadLocalRNG;

} // unnamed namespace

RNG& RNG::getRNG() {
  return threadLocalRNG.get()->rngWrapper;
}

void RNG::setGlobalSeed(result_type s) {
  seedRNG.seed(s);
}

uint32_t RNG::getU32() {
  boost::uniform_int<uint32_t> distribution;
  return distribution(getRNG());
}

uint32_t RNG::getU32(uint32_t max) {
  boost::uniform_int<uint32_t> distribution(0, max);
  return distribution(getRNG());
}

uint32_t RNG::getU32(uint32_t min, uint32_t max) {
  boost::uniform_int<uint32_t> distribution(min, max);
  return distribution(getRNG());
}

double RNG::getReal() {
  boost::uniform_real<double> distribution;
  return distribution(getRNG());
}

double RNG::getReal(double min, double max) {
  boost::uniform_real<double> distribution(min, max);
  return distribution(getRNG());
}

double RNG::getLogNormal(double mean, double sigma) {
  // If the sigma is negative, default to half of the mean.
  // This produces fairly nicely shaped distributions
  if (sigma < 0) {
    sigma = mean / 2;
  }
  boost::lognormal_distribution<double> dist(mean, sigma);
  boost::variate_generator<RNG, boost::lognormal_distribution<double>> gen(
      getRNG(), dist);
  return gen();
}

} // namespace loadgen
} // namespace thrift
} // namespace apache
