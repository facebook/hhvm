/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <vector>

#include <folly/executors/IOThreadPoolExecutor.h>
#include <folly/io/async/EventBase.h>

namespace facebook {
namespace memcache {
namespace mcrouter {

/**
 * Convenience method to convert the keepalives returned by getAllEventBases()
 * into raw pointers, to accommodate for AsyncMcServer::Options interface.
 * TODO: Change Options to accept KeepAlives instead?
 */
inline std::vector<folly::EventBase*> extractEvbs(
    folly::IOThreadPoolExecutorBase& ex) {
  auto evbKeepAlives = ex.getAllEventBases();
  std::vector<folly::EventBase*> ret;
  ret.reserve(evbKeepAlives.size());
  for (const auto& ka : evbKeepAlives) {
    ret.push_back(ka.get());
  }
  return ret;
}

} // namespace mcrouter
} // namespace memcache
} // namespace facebook
