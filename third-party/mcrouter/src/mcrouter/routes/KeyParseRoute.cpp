/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "KeyParseRoute.h"

#include <folly/Format.h>
#include <folly/fibers/WhenN.h>

namespace facebook {
namespace memcache {
namespace mcrouter {

KeyParseRouteSettings parseKeyParseRouteSettings(const folly::dynamic& json) {
  KeyParseRouteSettings settings;
  checkLogic(
      json.count("num_routing_parts"),
      "KeyParseRoute: missing num_routing_parts parameter");
  auto numRoutingParts = parseInt(
      *json.get_ptr("num_routing_parts"),
      "num_routing_parts",
      1,
      std::numeric_limits<int64_t>::max());
  settings.numRoutingParts = numRoutingParts;

  auto* delimiter = json.get_ptr("delimiter");
  checkLogic(
      delimiter && delimiter->isString() && delimiter->getString().size() == 1,
      "KeyParseRoute: missing single char delimiter");
  settings.delimiter = delimiter->getString()[0];
  return settings;
}

} // namespace mcrouter
} // namespace memcache
} // namespace facebook
