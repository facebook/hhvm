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

import io.airlift.stats.Distribution;
import io.airlift.stats.ExponentialDecay;
import io.micrometer.core.instrument.AbstractMeter;
import io.micrometer.core.instrument.Clock;
import io.micrometer.core.instrument.DistributionSummary;
import io.micrometer.core.instrument.distribution.DistributionStatisticConfig;
import io.micrometer.core.instrument.distribution.HistogramSnapshot;
import io.micrometer.core.instrument.distribution.ValueAtPercentile;
import io.micrometer.core.instrument.util.MeterEquivalence;
import io.micrometer.core.lang.Nullable;
import java.util.ArrayList;
import java.util.List;
import java.util.Objects;

public abstract class ThriftAbstractSummary extends AbstractMeter implements DistributionSummary {
  protected final Distribution histogram;
  protected final DistributionStatisticConfig distributionStatisticConfig;
  private final double scale;

  protected ThriftAbstractSummary(
      Id id,
      Clock clock,
      DistributionStatisticConfig distributionStatisticConfig,
      double scale,
      boolean supportsAggregablePercentiles) {
    super(id);
    this.scale = scale;

    histogram =
        Objects.isNull(distributionStatisticConfig.getExpiry())
            ? new Distribution()
            : new Distribution(
                ExponentialDecay.seconds(
                    (int) distributionStatisticConfig.getExpiry().getSeconds()));
    this.distributionStatisticConfig = distributionStatisticConfig;
  }

  @Override
  public final void record(double amount) {
    if (amount >= 0) {
      double scaledAmount = this.scale * amount;
      histogram.add((long) Math.round(scaledAmount));
    }
  }

  @Override
  public HistogramSnapshot takeSnapshot() {
    final ValueAtPercentile[] values;

    synchronized (this) {
      values = takeValueSnapshot();
    }

    return new HistogramSnapshot(count(), totalAmount(), max(), values, null, null);
  }

  private ValueAtPercentile[] takeValueSnapshot() {
    if (distributionStatisticConfig.getPercentiles() == null
        || distributionStatisticConfig.getPercentiles().length == 0) {
      return null;
    }

    ArrayList<Double> percentiles = new ArrayList<>();
    for (int i = 0; i < distributionStatisticConfig.getPercentiles().length; i++) {
      percentiles.add(distributionStatisticConfig.getPercentiles()[i]);
    }
    List<Double> percentilesValues = histogram.getPercentiles(percentiles);

    final ValueAtPercentile[] values = new ValueAtPercentile[percentiles.size()];
    for (int i = 0; i < percentiles.size(); i++) {
      values[i] =
          new ValueAtPercentile(
              percentiles.get(i),
              Double.isInfinite(percentilesValues.get(i)) ? 0L : percentilesValues.get(i));
    }
    return values;
  }

  @SuppressWarnings("EqualsWhichDoesntCheckParameterClass")
  @Override
  public boolean equals(@Nullable Object o) {
    return MeterEquivalence.equals(this, o);
  }

  @Override
  public int hashCode() {
    return MeterEquivalence.hashCode(this);
  }
}
