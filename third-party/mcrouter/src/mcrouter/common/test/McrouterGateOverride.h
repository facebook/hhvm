/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <string>
#include <vector>

#include <folly/container/F14Map.h>

#include "configerator/distribution/api/ScopedConfigeratorFake.h"
#include "configerator/structs/luna/gen-cpp2/sampling_config_types.h"

namespace facebook {
namespace memcache {
namespace mcrouter {

/*
 * Sets the config backing MCROUTER_GATE_CHECK to the input config.
 */
void setMcrouterGates(
    configerator::ScopedConfigeratorFake& configFake,
    folly::F14FastMap<std::string, luna::sampling::SamplingExpr> config);

/*
 * Enables/disables the input features for MCROUTER_GATE_CHECK and leaves the
 * rest of the features the same as the production config.
 */
void overrideMcrouterGates(
    configerator::ScopedConfigeratorFake& configFake,
    const std::vector<std::string>& enableFeatures,
    const std::vector<std::string>& disableFeatures);

} // namespace mcrouter
} // namespace memcache
} // namespace facebook
