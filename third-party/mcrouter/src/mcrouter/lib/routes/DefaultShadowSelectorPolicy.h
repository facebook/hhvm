/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <memory>

#include <folly/Function.h>

#include "mcrouter/lib/McResUtil.h"
#include "mcrouter/lib/network/gen/MemcacheMessages.h"

namespace facebook {
namespace memcache {

/**
 * Default shadow selector policy: send exactly the same request to shadow
 * as the original; send out shadow requests right away.
 */
class DefaultShadowSelectorPolicy {
 public:
  DefaultShadowSelectorPolicy() = default;

  template <class Reply>
  folly::Function<void(const Reply&)> makePostShadowReplyFn() const {
    return {};
  }
};

} // namespace memcache
} // namespace facebook
