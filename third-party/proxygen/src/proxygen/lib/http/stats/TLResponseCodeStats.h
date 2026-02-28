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

struct TLResponseCodeStats {
  explicit TLResponseCodeStats(const std::string& name, uint8_t verbosity);

  void addStatus(int status);

  StatsWrapper::TLTimeseries statusNone;
  StatsWrapper::TLTimeseries statusOther;
  StatsWrapper::TLTimeseries status1xx;
  StatsWrapper::TLTimeseries status2xx;
  StatsWrapper::TLTimeseries status3xx;
  StatsWrapper::TLTimeseries status4xx;
  StatsWrapper::TLTimeseries status5xx;

  // TODO: all the counters below are marked for deprecation.

  std::optional<StatsWrapper::TLTimeseries> status39x;

  std::optional<StatsWrapper::TLTimeseriesMinuteAndAllTime> status200;
  std::optional<StatsWrapper::TLTimeseriesMinuteAndAllTime> status206;
  std::optional<StatsWrapper::TLTimeseriesMinuteAndAllTime> status301;
  std::optional<StatsWrapper::TLTimeseriesMinuteAndAllTime> status302;
  std::optional<StatsWrapper::TLTimeseriesMinuteAndAllTime> status303;
  std::optional<StatsWrapper::TLTimeseriesMinuteAndAllTime> status304;
  std::optional<StatsWrapper::TLTimeseriesMinuteAndAllTime> status307;
  std::optional<StatsWrapper::TLTimeseriesMinuteAndAllTime> status395;
  std::optional<StatsWrapper::TLTimeseriesMinuteAndAllTime> status396;
  std::optional<StatsWrapper::TLTimeseriesMinuteAndAllTime> status397;
  std::optional<StatsWrapper::TLTimeseriesMinuteAndAllTime> status398;
  std::optional<StatsWrapper::TLTimeseriesMinuteAndAllTime> status399;
  std::optional<StatsWrapper::TLTimeseriesMinuteAndAllTime> status400;
  std::optional<StatsWrapper::TLTimeseriesMinuteAndAllTime> status401;
  std::optional<StatsWrapper::TLTimeseriesMinuteAndAllTime> status403;
  std::optional<StatsWrapper::TLTimeseriesMinuteAndAllTime> status404;
  std::optional<StatsWrapper::TLTimeseriesMinuteAndAllTime> status408;
  std::optional<StatsWrapper::TLTimeseriesMinuteAndAllTime> status429;
  std::optional<StatsWrapper::TLTimeseriesMinuteAndAllTime> status500;
  std::optional<StatsWrapper::TLTimeseriesMinuteAndAllTime> status501;
  std::optional<StatsWrapper::TLTimeseriesMinuteAndAllTime> status502;
  std::optional<StatsWrapper::TLTimeseriesMinuteAndAllTime> status503;
  std::optional<StatsWrapper::TLTimeseriesMinuteAndAllTime> status504;
};

} // namespace proxygen
