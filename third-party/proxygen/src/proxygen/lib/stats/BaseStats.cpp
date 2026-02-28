/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "proxygen/lib/stats/BaseStats.h"

DEFINE_bool(basestats_all_time_timeseries,
            false,
            "If true, BaseStats will use all-time aggregations");

/*static*/
bool proxygen::BaseStats::isAllTimeTimeseriesEnabled() {
  return FLAGS_basestats_all_time_timeseries;
}
