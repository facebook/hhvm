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

#define __STDC_FORMAT_MACROS

#include <thrift/lib/cpp/test/loadgen/WeightedLoadConfig.h>

#include <thrift/lib/cpp/Thrift.h>
#include <thrift/lib/cpp/test/loadgen/OpEnabledState.h>
#include <thrift/lib/cpp/test/loadgen/RNG.h>

using namespace std;

namespace apache {
namespace thrift {
namespace loadgen {

WeightedLoadConfig::WeightedLoadConfig(uint32_t numOps) : weightsSum_(0) {
  weights_.resize(numOps, 0);
  names_.resize(numOps, "");
}

void WeightedLoadConfig::setOpInfo(
    uint32_t opType, const string& name, uint32_t weight) {
  if (opType >= weights_.size()) {
    throw TLibraryException("setOpInfo() called with invalid op type");
  }

  uint32_t oldWeight = weights_[opType];
  weights_[opType] = weight;
  names_[opType] = name;

  weightsSum_ += (weight - oldWeight);
}

uint32_t WeightedLoadConfig::getOpWeight(uint32_t opType) {
  if (opType < weights_.size()) {
    return weights_[opType];
  }
  return 0;
}

uint32_t WeightedLoadConfig::getNumOpTypes() const {
  return weights_.size();
}

uint32_t WeightedLoadConfig::pickOpType() {
  if (weightsSum_ == 0) {
    T_ERROR("at least one operation weight must be non-zero");
    abort();
  }

  // Pick a random value from [0, weightsSum_),
  // and see which weight it falls into.
  uint32_t value = RNG::getRNG().getU32(0, weightsSum_ - 1);
  for (uint32_t n = 0; n < weights_.size(); ++n) {
    if (value < weights_[n]) {
      return n;
    }
    value -= weights_[n];
  }

  // This should never happen.
  T_ERROR(
      "random number %" PRIu32 " exceeded max %" PRIu32,
      weightsSum_ + value,
      weightsSum_ - 1);
  abort();
}

string WeightedLoadConfig::getOpName(uint32_t opType) {
  if (opType < names_.size() && !names_[opType].empty()) {
    return names_[opType];
  }

  // Undefined op type.  Default to our parent class' behavior.
  return LoadConfig::getOpName(opType);
}

void WeightedLoadConfig::configureEnabledState(OpEnabledState* state) const {
  for (uint32_t op = 0; op < weights_.size(); ++op) {
    bool enabled = (weights_[op] > 0);
    state->setEnabled(op, enabled);
  }
}

} // namespace loadgen
} // namespace thrift
} // namespace apache
