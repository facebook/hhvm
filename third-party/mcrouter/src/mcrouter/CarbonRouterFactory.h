/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <unordered_map>

#include <folly/Range.h>

#include "mcrouter/CarbonRouterInstance.h"

namespace facebook {
namespace memcache {
namespace mcrouter {

/**
 * Creates a carbon router from a flavor file.
 *
 * @param persistenceId   Persistence ID of your carbon router instance.
 *                        If an instance with the given persistenceId already
 *                        exists, returns a pointer to it.
 *                        Otherwise, spins up a new CarbonRouterInstance.
 * @param flavorUri       URI of the flavor containing the router options.
 *                        The flavor URI has to be prefixed by it's provider
 *                        (e.g. if it's a file, it should be: "file:<PATH>").
 * @param optionOverrides Optional params containing any option that should
 *                        override the option provided by the flavor.
 *
 * @return                A pointer to CarbonRouterInstance.
 *                        May return nullptr if config is invalid or if
 *                        CarbonRouterManager singleton is unavailable.
 */
template <class RouterInfo>
CarbonRouterInstance<RouterInfo>* createRouterFromFlavor(
    folly::StringPiece persistenceId,
    folly::StringPiece flavorUri,
    std::unordered_map<std::string, std::string> optionOverrides =
        std::unordered_map<std::string, std::string>());

/**
 * Creates a carbon router from a flavor file and returns a shared_ptr to it.
 *
 * @param flavorUri       URI of the flavor containing the router options.
 *                        The flavor URI has to be prefixed by it's provider
 *                        (e.g. if it's a file, it should be: "file:<PATH>").
 * @param optionOverrides Optional params containing any option that should
 *                        override the option provided by the flavor.
 *
 * @return                A shared_ptr to CarbonRouterInstance.
 *                        May return nullptr if config is invalid or if
 *                        CarbonRouterManager singleton is unavailable.
 */
template <class RouterInfo>
std::shared_ptr<CarbonRouterInstance<RouterInfo>> createRouterFromFlavor(
    folly::StringPiece flavorUri,
    std::unordered_map<std::string, std::string> optionOverrides =
        std::unordered_map<std::string, std::string>());

} // namespace mcrouter
} // namespace memcache
} // namespace facebook

#include "CarbonRouterFactory-inl.h"
