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
  explicit TLResponseCodeStats(const std::string& name, uint8_t verbosity);

  void addStatus(int status);

  BaseStats::TLTimeseries statusNone;
  BaseStats::TLTimeseries statusOther;
  BaseStats::TLTimeseries status1xx;
  BaseStats::TLTimeseries status2xx;
  BaseStats::TLTimeseries status3xx;
  BaseStats::TLTimeseries status4xx;
  BaseStats::TLTimeseries status5xx;

  // TODO: all the counters below are marked for deprecation.

  std::optional<BaseStats::TLTimeseries> status39x;

  std::optional<BaseStats::TLTimeseriesMinuteAndAllTime> status200;
  std::optional<BaseStats::TLTimeseriesMinuteAndAllTime> status206;
  std::optional<BaseStats::TLTimeseriesMinuteAndAllTime> status301;
  std::optional<BaseStats::TLTimeseriesMinuteAndAllTime> status302;
  std::optional<BaseStats::TLTimeseriesMinuteAndAllTime> status303;
  std::optional<BaseStats::TLTimeseriesMinuteAndAllTime> status304;
  std::optional<BaseStats::TLTimeseriesMinuteAndAllTime> status307;
  std::optional<BaseStats::TLTimeseriesMinuteAndAllTime> status395;
  std::optional<BaseStats::TLTimeseriesMinuteAndAllTime> status396;
  std::optional<BaseStats::TLTimeseriesMinuteAndAllTime> status397;
  std::optional<BaseStats::TLTimeseriesMinuteAndAllTime> status398;
  std::optional<BaseStats::TLTimeseriesMinuteAndAllTime> status399;
  std::optional<BaseStats::TLTimeseriesMinuteAndAllTime> status400;
  std::optional<BaseStats::TLTimeseriesMinuteAndAllTime> status401;
  std::optional<BaseStats::TLTimeseriesMinuteAndAllTime> status403;
  std::optional<BaseStats::TLTimeseriesMinuteAndAllTime> status404;
  std::optional<BaseStats::TLTimeseriesMinuteAndAllTime> status408;
  std::optional<BaseStats::TLTimeseriesMinuteAndAllTime> status429;
  std::optional<BaseStats::TLTimeseriesMinuteAndAllTime> status500;
  std::optional<BaseStats::TLTimeseriesMinuteAndAllTime> status501;
  std::optional<BaseStats::TLTimeseriesMinuteAndAllTime> status502;
  std::optional<BaseStats::TLTimeseriesMinuteAndAllTime> status503;
  std::optional<BaseStats::TLTimeseriesMinuteAndAllTime> status504;
};

} // namespace proxygen
