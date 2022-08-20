/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <proxygen/lib/stats/BaseStats.h>

namespace proxygen {

struct TLResponseCodeStats {
  explicit TLResponseCodeStats(const std::string& name);

  void addStatus(int status);

  BaseStats::TLTimeseries statusNone;
  BaseStats::TLTimeseries statusOther;
  BaseStats::TLTimeseries status1xx;
  BaseStats::TLTimeseries status2xx;
  BaseStats::TLTimeseries status3xx;
  BaseStats::TLTimeseries status4xx;
  BaseStats::TLTimeseries status5xx;

  // TODO: all the counters below are marked for deprecation.

  BaseStats::TLTimeseries status39x;

  BaseStats::TLTimeseriesMinuteAndAllTime status200;
  BaseStats::TLTimeseriesMinuteAndAllTime status206;
  BaseStats::TLTimeseriesMinuteAndAllTime status301;
  BaseStats::TLTimeseriesMinuteAndAllTime status302;
  BaseStats::TLTimeseriesMinuteAndAllTime status303;
  BaseStats::TLTimeseriesMinuteAndAllTime status304;
  BaseStats::TLTimeseriesMinuteAndAllTime status307;
  BaseStats::TLTimeseriesMinuteAndAllTime status395;
  BaseStats::TLTimeseriesMinuteAndAllTime status396;
  BaseStats::TLTimeseriesMinuteAndAllTime status397;
  BaseStats::TLTimeseriesMinuteAndAllTime status398;
  BaseStats::TLTimeseriesMinuteAndAllTime status399;
  BaseStats::TLTimeseriesMinuteAndAllTime status400;
  BaseStats::TLTimeseriesMinuteAndAllTime status401;
  BaseStats::TLTimeseriesMinuteAndAllTime status403;
  BaseStats::TLTimeseriesMinuteAndAllTime status404;
  BaseStats::TLTimeseriesMinuteAndAllTime status408;
  BaseStats::TLTimeseriesMinuteAndAllTime status429;
  BaseStats::TLTimeseriesMinuteAndAllTime status500;
  BaseStats::TLTimeseriesMinuteAndAllTime status501;
  BaseStats::TLTimeseriesMinuteAndAllTime status502;
  BaseStats::TLTimeseriesMinuteAndAllTime status503;
  BaseStats::TLTimeseriesMinuteAndAllTime status504;
};

} // namespace proxygen
