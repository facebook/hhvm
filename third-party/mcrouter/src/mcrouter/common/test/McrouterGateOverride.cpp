/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "mcrouter/common/test/McrouterGateOverride.h"

#include "common/config/ConfigeratorConfig.h"
#include "luna/common/sampling/tests/TestUtils.h"
#include "luna/common/sampling/utils/SamplingConfigHelpers.h"

namespace facebook {
namespace memcache {
namespace mcrouter {

namespace {
const std::string kMcrouterConfigPath = "mcrouter/gating/gates";
}

void setMcrouterGates(
    configerator::ScopedConfigeratorFake& configFake,
    folly::F14FastMap<std::string, luna::sampling::SamplingExpr> config) {
  luna::sampling::test::overrideMultiSampler(
      configFake, kMcrouterConfigPath, std::move(config));
}

void overrideMcrouterGates(
    configerator::ScopedConfigeratorFake& configFake,
    const std::vector<std::string>& enableFeatures,
    const std::vector<std::string>& disableFeatures) {
  using namespace facebook::luna::sampling;
  folly::F14FastMap<std::string, SamplingExpr> config;

  // Try to load existing production config, if available
  try {
    config::ConfigeratorConfig<MultiSamplingExpr> prodConfig(
        kMcrouterConfigPath);
    auto prodConfigPtr = prodConfig.get();
    if (prodConfigPtr) {
      for (auto& useCase : prodConfigPtr->useCases().value()) {
        config.try_emplace(useCase.first, useCase.second);
      }
    }
  } catch (...) {
    // If prod config is not available, start with empty config
  }

  // Override specified features
  for (const auto& feature : enableFeatures) {
    config[feature] = alwaysSample();
  }
  for (const auto& feature : disableFeatures) {
    config[feature] = neverSample();
  }

  setMcrouterGates(configFake, std::move(config));
}

} // namespace mcrouter
} // namespace memcache
} // namespace facebook
