/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "mcrouter/routes/RoutingUtils.h"

#include <folly/json/dynamic.h>
#include "mcrouter/lib/fbi/cpp/util.h"
#include "mcrouter/options.h"

namespace facebook {
namespace memcache {
namespace mcrouter {

static constexpr uint32_t kMaxTotalFanout = 32 * 1024;

uint32_t getAdditionalFanout(
    const folly::dynamic& json,
    const McrouterOptions& opts) {
  auto jAdditionalFanout = json.get_ptr("additional_fanout");
  if (!jAdditionalFanout) {
    return 0;
  }

  checkLogic(jAdditionalFanout->isInt(), "additional_fanout is not an integer");
  uint32_t additionalFanout = jAdditionalFanout->getInt();
  if (!opts.thread_affinity) {
    checkLogic(
        static_cast<uint64_t>(additionalFanout + 1) *
                static_cast<uint64_t>(opts.num_proxies) <=
            kMaxTotalFanout,
        "(additional_fanout={} + 1) * num_proxies={} must be <= {}",
        additionalFanout,
        opts.num_proxies,
        kMaxTotalFanout);
  } else {
    checkLogic(
        static_cast<uint64_t>(additionalFanout + 1) <= kMaxTotalFanout,
        "(additional_fanout={} + 1) must be <= {}",
        additionalFanout,
        kMaxTotalFanout);
  }

  return additionalFanout;
}

} // namespace mcrouter
} // namespace memcache
} // namespace facebook
