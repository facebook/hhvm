/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <proxygen/lib/stats/PeriodicStats.h>
#include <proxygen/lib/stats/ResourceData.h>

namespace proxygen {

/**
 * ResourceStats:
 *
 * A class designed to abstract away the internals of retrieving
 * various resource utilization metrics, built using PeriodicStats.
 * See PeriodicStats class documentation for specifics.
 */
class ResourceStats : public PeriodicStats<ResourceData> {
 public:
  /**
   * Note: as CPU pct utilization requires intervals and at init time there
   * is only a single data point, pct utilization is initially seeded from
   * proc loadavg.
   */
  explicit ResourceStats(std::unique_ptr<Resources> resources);
  ~ResourceStats() override;

 protected:
  /**
   * Override getNewData so that we can return an instance of ResourceData.
   */
  ResourceData* getNewData() const override;

  /**
   * Abstraction that enables callers to provide their own implementations
   * of the entity that actually queries various metrics.
   */
  std::unique_ptr<Resources> resources_;
};

} // namespace proxygen
