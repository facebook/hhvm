/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "mcrouter/lib/carbon/ReplyCommon.h"

namespace carbon {

const std::shared_ptr<const facebook::memcache::AccessPoint>&
ReplyCommon::destination() const noexcept {
  return destination_;
}

void ReplyCommon::setDestination(
    std::shared_ptr<const facebook::memcache::AccessPoint> ap) noexcept {
  destination_ = std::move(ap);
}

void ReplyCommon::setRegion(const std::string& region) {
  region_.emplace(region);
}

const std::optional<std::string>& ReplyCommon::getRegion() const {
  return region_;
}

ReplyCommonThrift::ReplyCommonThrift(carbon::Result result__)
    : result_(result__) {}

carbon::Result ReplyCommonThrift::result() const {
  return result_;
}

carbon::Result& ReplyCommonThrift::result() {
  return result_;
}

} // namespace carbon
