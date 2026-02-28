/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "mcrouter/routes/RendezvousRouteHelpers.h"

#include "mcrouter/lib/fbi/cpp/util.h"

namespace facebook {
namespace memcache {
namespace mcrouter {

std::vector<folly::StringPiece> getTags(
    const folly::dynamic& json,
    size_t numRoutes,
    const std::string& nameForErrorMessage) {
  std::vector<folly::StringPiece> endpoints;

  auto jtags = json.get_ptr("tags");
  checkLogic(
      jtags, "{}: tags needed for Rendezvous hash route", nameForErrorMessage);
  checkLogic(jtags->isArray(), "{}: tags is not an array", nameForErrorMessage);
  checkLogic(
      jtags->size() == numRoutes,
      "{}: number of tags {} doesn't match number of route handles {}",
      nameForErrorMessage,
      jtags->size(),
      numRoutes);

  endpoints.reserve(jtags->size());
  for (const auto& jtag : *jtags) {
    checkLogic(jtag.isString(), "{}: tag is not a string", nameForErrorMessage);
    endpoints.push_back(jtag.stringPiece());
  }

  return endpoints;
}

} // namespace mcrouter
} // namespace memcache
} // namespace facebook
