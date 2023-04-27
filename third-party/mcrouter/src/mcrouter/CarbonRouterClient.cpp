/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "mcrouter/CarbonRouterClient.h"
#include "mcrouter/mcrouter_sr_deps.h"

namespace facebook::memcache::mcrouter::detail {

bool srHostWithShardFuncCarbonRouterClient(
    const HostWithShard& hostWithShard,
    const RequestClass& requestClass,
    uint64_t& hash,
    uint64_t& hint) {
  auto& host = hostWithShard.first;
  if (!requestClass.is(RequestClass::kShadow) && host &&
      host->location().getTWTaskID()) {
    hash = *host->location().getTWTaskID();
    hint = RoutingHintEncoder::encodeRoutingHint(hostWithShard);
    return true;
  }
  return false;
}

} // namespace facebook::memcache::mcrouter::detail
