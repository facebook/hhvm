/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.facebook.swift.service.stats;

import io.micrometer.core.instrument.Clock;
import io.micrometer.core.instrument.distribution.DistributionStatisticConfig;
import io.micrometer.core.instrument.distribution.pause.PauseDetector;
import io.micrometer.core.instrument.util.TimeUtils;
import java.util.concurrent.TimeUnit;

public class ThriftTimer extends ThriftAbstractTimer {

  public ThriftTimer(
      Id id,
      Clock clock,
      DistributionStatisticConfig distributionStatisticConfig,
      PauseDetector pauseDetector,
      TimeUnit baseTimeUnit) {
    this(id, clock, distributionStatisticConfig, pauseDetector, baseTimeUnit, false);
  }

  public ThriftTimer(
      Id id,
      Clock clock,
      DistributionStatisticConfig distributionStatisticConfig,
      PauseDetector pauseDetector,
      TimeUnit baseTimeUnit,
      boolean supportsAggregablePercentiles) {
    super(
        id,
        clock,
        distributionStatisticConfig,
        pauseDetector,
        baseTimeUnit,
        supportsAggregablePercentiles);
  }

  @Override
  public long count() {
    double value = histogram.getCount();
    return Double.isInfinite(value) ? 0L : (long) Math.round(value);
  }

  @Override
  public double totalTime(TimeUnit unit) {
    double value = TimeUtils.nanosToUnit(histogram.getTotal(), unit);
    return Double.isInfinite(value) ? 0L : value;
  }

  @Override
  public double max(TimeUnit unit) {
    double value = TimeUtils.nanosToUnit(histogram.getMax(), unit);
    return Double.isInfinite(value) ? 0L : value;
  }
}
