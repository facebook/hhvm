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
import io.micrometer.core.instrument.Measurement;
import io.micrometer.core.instrument.Statistic;
import io.micrometer.core.instrument.distribution.DistributionStatisticConfig;
import java.util.Arrays;

public class ThriftSummary extends ThriftAbstractSummary {

  @Deprecated
  public ThriftSummary(
      Id id, Clock clock, DistributionStatisticConfig distributionStatisticConfig, double scale) {
    this(id, clock, distributionStatisticConfig, scale, false);
  }

  public ThriftSummary(
      Id id,
      Clock clock,
      DistributionStatisticConfig distributionStatisticConfig,
      double scale,
      boolean supportsAggregablePercentiles) {
    super(id, clock, distributionStatisticConfig, scale, supportsAggregablePercentiles);
  }

  @Override
  public long count() {
    double value = histogram.getCount();
    return Double.isInfinite(value) ? 0L : (long) Math.round(value);
  }

  @Override
  public double totalAmount() {
    double value = histogram.getTotal();
    return Double.isInfinite(value) ? 0L : (long) Math.round(value);
  }

  @Override
  public double max() {
    double value = histogram.getTotal();
    return Double.isInfinite(value) ? 0L : (long) Math.round(value);
  }

  @Override
  public Iterable<Measurement> measure() {
    return Arrays.asList(
        new Measurement(() -> (double) count(), Statistic.COUNT),
        new Measurement(this::totalAmount, Statistic.TOTAL),
        new Measurement(this::max, Statistic.MAX));
  }
}
