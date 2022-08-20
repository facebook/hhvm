/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "McrouterFiberContext.h"

namespace facebook {
namespace memcache {
namespace mcrouter {

const RequestClass RequestClass::kFailover{0x1};
const RequestClass RequestClass::kShadow{0x2};

const char* RequestClass::toString() const {
  if (is(RequestClass::kFailover) && is(RequestClass::kShadow)) {
    return "failover|shadow";
  } else if (is(RequestClass::kFailover)) {
    return "failover";
  } else if (is(RequestClass::kShadow)) {
    return "shadow";
  }
  return "normal";
}
} // namespace mcrouter
} // namespace memcache
} // namespace facebook
