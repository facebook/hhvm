/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/stats/ResourceStats.h>

namespace proxygen {

ResourceStats::ResourceStats(std::unique_ptr<Resources> resources)
    : PeriodicStats<ResourceData>(
          new ResourceData(resources->getCurrentData())),
      resources_(std::move(resources)) {
}

ResourceStats::~ResourceStats() {
  // Stop refreshing on destruction so the function scheduler thread can
  // not access destructed class members.
  stopRefresh();
}

ResourceData* ResourceStats::getNewData() const {
  return new ResourceData(resources_->getCurrentData());
}

} // namespace proxygen
