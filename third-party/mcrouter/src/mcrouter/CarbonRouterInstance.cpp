/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "CarbonRouterInstance.h"

#include <folly/io/async/EventBase.h>
#include <folly/json.h>

#include "mcrouter/lib/AuxiliaryCPUThreadPool.h"
#include "mcrouter/lib/fbi/cpp/LogFailure.h"

namespace facebook {
namespace memcache {
namespace mcrouter {

namespace detail {

bool isValidRouterName(folly::StringPiece name) {
  if (name.empty()) {
    return false;
  }

  for (auto c : name) {
    if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
          (c >= '0' && c <= '9') || (c == '_') || (c == '-'))) {
      return false;
    }
  }

  return true;
}

} // namespace detail

void freeAllRouters() {
  if (auto manager = detail::McrouterManager::getSingletonInstance()) {
    manager->freeAllMcrouters();
  }
}
} // namespace mcrouter
} // namespace memcache
} // namespace facebook
