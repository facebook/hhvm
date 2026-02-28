/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <proxygen/lib/stats/StatsWrapper.h>

namespace proxygen {

struct ResponseCodeStatsMinute {
  explicit ResponseCodeStatsMinute(const std::string& name);

  void addStatus(int status);

  StatsWrapper::TLTimeseriesMinute statusOther;
  StatsWrapper::TLTimeseriesMinute status1xx;
  StatsWrapper::TLTimeseriesMinute status2xx;
  StatsWrapper::TLTimeseriesMinute status3xx;
  StatsWrapper::TLTimeseriesMinute status4xx;
  StatsWrapper::TLTimeseriesMinute status5xx;
};

} // namespace proxygen
