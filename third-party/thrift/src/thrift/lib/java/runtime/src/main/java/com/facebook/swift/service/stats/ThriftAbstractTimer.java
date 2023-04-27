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
import io.micrometer.core.instrument.Timer;
import io.micrometer.core.instrument.distribution.DistributionStatisticConfig;
import io.micrometer.core.instrument.distribution.HistogramSnapshot;
import io.micrometer.core.instrument.distribution.ValueAtPercentile;
import io.micrometer.core.instrument.distribution.pause.ClockDriftPauseDetector;
import io.micrometer.core.instrument.distribution.pause.PauseDetector;
import io.micrometer.core.instrument.util.MeterEquivalence;
import io.micrometer.core.lang.Nullable;
import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.concurrent.Callable;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.TimeUnit;
import java.util.function.Supplier;
import org.LatencyUtils.IntervalEstimator;
import org.LatencyUtils.SimplePauseDetector;
import org.LatencyUtils.TimeCappedMovingAverageIntervalEstimator;

public abstract class ThriftAbstractTimer extends AbstractMeter implements Timer {
  private static Map<PauseDetector, org.LatencyUtils.PauseDetector> pauseDetectorCache =
      new ConcurrentHashMap<>();

  protected final Clock clock;
  protected final Distribution histogram;
  protected final DistributionStatisticConfig distributionStatisticConfig;
  private final TimeUnit baseTimeUnit;

  // Only used when pause detection is enabled
  @Nullable private IntervalEstimator intervalEstimator = null;

  @Nullable private org.LatencyUtils.PauseDetector pauseDetector;

  /**
   * Creates a new timer.
   *
   * @param id The timer's name and tags.
   * @param clock The clock used to measure latency.
   * @param distributionStatisticConfig Configuration determining which distribution statistics are
   *     sent.
   * @param pauseDetector Compensation for coordinated omission.
   * @param baseTimeUnit The time scale of this timer.
   * @deprecated Timer implementations should now declare at construction time whether they support
   *     aggregable percentiles or not. By declaring it up front, Micrometer can memory optimize the
   *     histogram structure used to store distribution statistics.
   */
  @Deprecated
  protected ThriftAbstractTimer(
      Id id,
      Clock clock,
      DistributionStatisticConfig distributionStatisticConfig,
      PauseDetector pauseDetector,
      TimeUnit baseTimeUnit) {
    this(id, clock, distributionStatisticConfig, pauseDetector, baseTimeUnit, false);
  }

  /**
   * Creates a new timer.
   *
   * @param id The timer's name and tags.
   * @param clock The clock used to measure latency.
   * @param distributionStatisticConfig Configuration determining which distribution statistics are
   *     sent.
   * @param pauseDetector Compensation for coordinated omission.
   * @param baseTimeUnit The time scale of this timer.
   * @param supportsAggregablePercentiles Indicates whether the registry supports percentile
   *     approximations from histograms.
   */
  protected ThriftAbstractTimer(
      Id id,
      Clock clock,
      DistributionStatisticConfig distributionStatisticConfig,
      PauseDetector pauseDetector,
      TimeUnit baseTimeUnit,
      boolean supportsAggregablePercentiles) {
    super(id);
    this.clock = clock;
    this.baseTimeUnit = baseTimeUnit;

    initPauseDetector(pauseDetector);

    histogram =
        Objects.isNull(distributionStatisticConfig.getExpiry())
            ? new Distribution()
            : new Distribution(
                ExponentialDecay.seconds(
                    (int) distributionStatisticConfig.getExpiry().getSeconds()));
    this.distributionStatisticConfig = distributionStatisticConfig;
  }

  private void initPauseDetector(PauseDetector pauseDetectorType) {
    pauseDetector =
        pauseDetectorCache.computeIfAbsent(
            pauseDetectorType,
            detector -> {
              if (detector instanceof ClockDriftPauseDetector) {
                ClockDriftPauseDetector clockDriftPauseDetector =
                    (ClockDriftPauseDetector) detector;
                return new SimplePauseDetector(
                    clockDriftPauseDetector.getSleepInterval().toNanos(),
                    clockDriftPauseDetector.getPauseThreshold().toNanos(),
                    1,
                    false);
              }
              return null;
            });

    if (pauseDetector instanceof SimplePauseDetector) {
      this.intervalEstimator =
          new TimeCappedMovingAverageIntervalEstimator(128, 10000000000L, pauseDetector);

      pauseDetector.addListener(
          (pauseLength, pauseEndTime) -> {
            //            System.out.println("Pause of length " + (pauseLength / 1e6) + "ms, end
            // time " + pauseEndTime);
            if (intervalEstimator != null) {
              long estimatedInterval = intervalEstimator.getEstimatedInterval(pauseEndTime);
              long observedLatencyMinbar = pauseLength - estimatedInterval;
              if (observedLatencyMinbar >= estimatedInterval) {
                recordValueWithExpectedInterval(observedLatencyMinbar, estimatedInterval);
              }
            }
          });
    }
  }

  private void recordValueWithExpectedInterval(
      long nanoValue, long expectedIntervalBetweenValueSamples) {
    record(nanoValue, TimeUnit.NANOSECONDS);
    if (expectedIntervalBetweenValueSamples <= 0) return;
    for (long missingValue = nanoValue - expectedIntervalBetweenValueSamples;
        missingValue >= expectedIntervalBetweenValueSamples;
        missingValue -= expectedIntervalBetweenValueSamples) {
      record(missingValue, TimeUnit.NANOSECONDS);
    }
  }

  @Override
  public <T> T recordCallable(Callable<T> f) throws Exception {
    final long s = clock.monotonicTime();
    try {
      return f.call();
    } finally {
      final long e = clock.monotonicTime();
      record(e - s, TimeUnit.NANOSECONDS);
    }
  }

  @Override
  public <T> T record(Supplier<T> f) {
    final long s = clock.monotonicTime();
    try {
      return f.get();
    } finally {
      final long e = clock.monotonicTime();
      record(e - s, TimeUnit.NANOSECONDS);
    }
  }

  @Override
  public void record(Runnable f) {
    final long s = clock.monotonicTime();
    try {
      f.run();
    } finally {
      final long e = clock.monotonicTime();
      record(e - s, TimeUnit.NANOSECONDS);
    }
  }

  @Override
  public final void record(long amount, TimeUnit unit) {
    if (amount >= 0) {
      histogram.add(TimeUnit.NANOSECONDS.convert(amount, unit));

      if (intervalEstimator != null) {
        intervalEstimator.recordInterval(clock.monotonicTime());
      }
    }
  }

  @Override
  public HistogramSnapshot takeSnapshot() {
    final ValueAtPercentile[] values;

    synchronized (this) {
      values = takeValueSnapshot();
    }

    return new HistogramSnapshot(
        count(), totalTime(TimeUnit.NANOSECONDS), max(TimeUnit.NANOSECONDS), values, null, null);
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

  @Override
  public TimeUnit baseTimeUnit() {
    return baseTimeUnit;
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

  @Override
  public void close() {
    if (pauseDetector != null) {
      pauseDetector.shutdown();
    }
  }
}
