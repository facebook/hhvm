/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "ThreadUtil.h"

#include <folly/Conv.h>
#include <folly/system/ThreadName.h>

#include "mcrouter/options.h"

namespace facebook {
namespace memcache {
namespace mcrouter {

void mcrouterSetThisThreadName(
    const McrouterOptions& opts,
    folly::StringPiece prefix,
    folly::Optional<size_t> threadIdx) {
  auto name = folly::to<std::string>(prefix, "-", opts.router_name);
  if (threadIdx.has_value()) {
    name.append(folly::to<std::string>("-", threadIdx.value()));
  }
  if (!folly::setThreadName(name)) {
    LOG(WARNING) << "Unable to set thread name to " << name;
  }
}
} // namespace mcrouter
} // namespace memcache
} // namespace facebook
