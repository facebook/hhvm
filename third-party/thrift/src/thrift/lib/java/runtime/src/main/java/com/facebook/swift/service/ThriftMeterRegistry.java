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

package com.facebook.swift.service;

import com.facebook.swift.service.stats.ThriftDecayCounter;
import com.facebook.swift.service.stats.ThriftSummary;
import com.facebook.swift.service.stats.ThriftTimer;
import io.micrometer.core.instrument.Clock;
import io.micrometer.core.instrument.Counter;
import io.micrometer.core.instrument.DistributionSummary;
import io.micrometer.core.instrument.FunctionCounter;
import io.micrometer.core.instrument.FunctionTimer;
import io.micrometer.core.instrument.Gauge;
import io.micrometer.core.instrument.Measurement;
import io.micrometer.core.instrument.Meter;
import io.micrometer.core.instrument.MeterRegistry;
import io.micrometer.core.instrument.Timer;
import io.micrometer.core.instrument.cumulative.CumulativeCounter;
import io.micrometer.core.instrument.cumulative.CumulativeFunctionCounter;
import io.micrometer.core.instrument.cumulative.CumulativeFunctionTimer;
import io.micrometer.core.instrument.distribution.DistributionStatisticConfig;
import io.micrometer.core.instrument.distribution.ValueAtPercentile;
import io.micrometer.core.instrument.distribution.pause.PauseDetector;
import io.micrometer.core.instrument.internal.DefaultGauge;
import io.micrometer.core.instrument.internal.DefaultMeter;
import java.time.Duration;
import java.util.Collections;
import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.TimeUnit;
import java.util.function.Supplier;
import java.util.function.ToDoubleFunction;
import java.util.function.ToLongFunction;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public class ThriftMeterRegistry extends MeterRegistry {
  private static final Logger LOGGER = LoggerFactory.getLogger(ThriftMeterRegistry.class);

  private ThriftMeterRegistry() {
    super(Clock.SYSTEM);
  }

  private static final ThriftMeterRegistry INSTANCE = new ThriftMeterRegistry();

  public static ThriftMeterRegistry getInstance() {
    return INSTANCE;
  }

  private static final double[] PERCENTILES = {0.50, 0.90, 0.95, 0.99};

  @Override
  protected <T> Gauge newGauge(Meter.Id id, T obj, ToDoubleFunction<T> valueFunction) {
    return new DefaultGauge<>(id, obj, valueFunction);
  }

  @Override
  protected Counter newCounter(Meter.Id id) {
    String decay = id.getTag("decay");
    if (decay == null) {
      return new CumulativeCounter(id);
    }
    if (decay.equals("3600") || decay.equals("60")) {
      return new ThriftDecayCounter(id, Integer.parseInt(decay));
    }
    throw new IllegalArgumentException("Illegal usage of decay tag");
  }

  @Override
  protected Timer newTimer(
      Meter.Id id,
      DistributionStatisticConfig distributionStatisticConfig,
      PauseDetector pauseDetector) {
    return new ThriftTimer(
        id, Clock.SYSTEM, distributionStatisticConfig, pauseDetector, TimeUnit.MICROSECONDS);
  }

  @Override
  protected DistributionSummary newDistributionSummary(
      Meter.Id id, DistributionStatisticConfig distributionStatisticConfig, double scale) {
    return new ThriftSummary(id, Clock.SYSTEM, distributionStatisticConfig, scale, false);
  }

  @Override
  protected Meter newMeter(Meter.Id id, Meter.Type type, Iterable<Measurement> measurements) {
    return new DefaultMeter(id, type, measurements);
  }

  @Override
  protected <T> FunctionTimer newFunctionTimer(
      Meter.Id id,
      T obj,
      ToLongFunction<T> countFunction,
      ToDoubleFunction<T> totalTimeFunction,
      TimeUnit totalTimeFunctionUnit) {
    return new CumulativeFunctionTimer<>(
        id, obj, countFunction, totalTimeFunction, totalTimeFunctionUnit, totalTimeFunctionUnit);
  }

  @Override
  protected <T> FunctionCounter newFunctionCounter(
      Meter.Id id, T obj, ToDoubleFunction<T> countFunction) {
    return new CumulativeFunctionCounter<>(id, obj, countFunction);
  }

  @Override
  protected TimeUnit getBaseTimeUnit() {
    return TimeUnit.MICROSECONDS;
  }

  @Override
  protected DistributionStatisticConfig defaultHistogramConfig() {
    return DistributionStatisticConfig.builder()
        .percentilesHistogram(true)
        .percentiles(PERCENTILES)
        .minimumExpectedValue(0.1)
        .maximumExpectedValue(Double.POSITIVE_INFINITY)
        .percentilePrecision(2)
        .expiry(null)
        .bufferLength(1)
        .build();
  }

  /**
   * Build a new set of common Distribution Summaries if they did not exist
   *
   * @param name The name of the Distribution Summaries
   */
  public void buildSummarySet(String name) {
    this.buildDecaySummary(name, Duration.ofMinutes(1));
    this.buildDecaySummary(name, Duration.ofHours(1));
    this.buildSummary(name);
  }

  /**
   * Records a measure of an event in a new or existing set of common Distribution Summaries
   *
   * @param name The name of the Distribution Summaries to record the measure of that kind of event
   * @param amount The measure of the event to be recorded
   */
  public void recordSummarySet(String name, long amount) {
    this.recordDecaySummary(name, Duration.ofMinutes(1), amount);
    this.recordDecaySummary(name, Duration.ofHours(1), amount);
    this.recordSummary(name, amount);
  }

  /**
   * Build a new DistributionSummary with decay if it did not exist
   *
   * @param name The name of the DistributionSummary
   * @param expiry The decay time of the DistributionSummary
   */
  public void buildDecaySummary(String name, Duration expiry) {
    try {
      DistributionSummary.builder(name)
          .tag("decay", Long.toString(expiry.getSeconds()))
          .distributionStatisticExpiry(expiry)
          .register(this);
    } catch (Throwable t) {
      LOGGER.error("error building decay summary", t);
    }
  }

  /**
   * Records a measure of an event in a new or existing DistributionSummary with decay
   *
   * @param name The name of the DistributionSummary to record the measures of that kind of event
   * @param expiry The decay time of the DistributionSummary
   * @param amount The measure of the event to be recorded
   */
  public void recordDecaySummary(String name, Duration expiry, double amount) {
    try {
      DistributionSummary.builder(name)
          .tag("decay", Long.toString(expiry.getSeconds()))
          .distributionStatisticExpiry(expiry)
          .register(this)
          .record(amount);
    } catch (Throwable t) {
      LOGGER.error("error recording decay summary", t);
    }
  }

  /**
   * Build a new default DistributionSummary if it did not exist
   *
   * @param name The name of the DistributionSummary
   */
  public void buildSummary(String name) {
    try {
      this.summary(name);
    } catch (Throwable t) {
      LOGGER.error("error building common summary", t);
    }
  }

  /**
   * Records a measure of an event in a new or existing default DistributionSummary
   *
   * @param name The name of the DistributionSummary to record the measures of that kind of event
   * @param amount The measure of the event to be recorded
   */
  public void recordSummary(String name, double amount) {
    try {
      this.summary(name).record(amount);
    } catch (Throwable t) {
      LOGGER.error("error recording common summary", t);
    }
  }

  /**
   * Build a new set of common timers if they did not exist
   *
   * @param name The name of the Timers
   */
  public void buildTimerSet(String name) {
    this.buildDecayTimer(name, Duration.ofMinutes(1));
    this.buildDecayTimer(name, Duration.ofHours(1));
    this.buildTimer(name);
  }

  /**
   * Records a time of an event in a new or existing set of common timers
   *
   * @param name The name of the Timer to record the time of that kind of event
   * @param time The time of the event to be recorded
   */
  public void recordTimerSet(String name, long time) {
    this.recordDecayTimer(name, Duration.ofMinutes(1), time);
    this.recordDecayTimer(name, Duration.ofHours(1), time);
    this.recordTimer(name, time);
  }

  /**
   * Build a new Timer with decay if it did not exist
   *
   * @param name The name of the Timer
   * @param expiry The decay time of the Timer
   */
  public void buildDecayTimer(String name, Duration expiry) {
    try {
      Timer.builder(name)
          .tags("decay", Long.toString(expiry.getSeconds()))
          .distributionStatisticExpiry(expiry)
          .register(this);
    } catch (Throwable t) {
      LOGGER.error("error building decay timer", t);
    }
  }

  /**
   * Records a time of an event in a new or existing Timer with decay
   *
   * @param name The name of the Timer to record the time of that kind of event
   * @param expiry The decay time of the Timer
   * @param time The time of the event to be recorded
   */
  public void recordDecayTimer(String name, Duration expiry, long time) {
    try {
      Timer.builder(name)
          .tags("decay", Long.toString(expiry.getSeconds()))
          .distributionStatisticExpiry(expiry)
          .register(this)
          .record(time, TimeUnit.MICROSECONDS);
    } catch (Throwable t) {
      LOGGER.error("error recording decay timer", t);
    }
  }

  /**
   * Build a new default Timer if it did not exist
   *
   * @param name The name of the Timer
   */
  public void buildTimer(String name) {
    try {
      Timer.builder(name).register(this);
    } catch (Throwable t) {
      LOGGER.error("error buliding common timer", t);
    }
  }

  /**
   * Records a time of an event in a new or existing default Timer
   *
   * @param name The name of the Timer to record the time of that kind of event
   * @param time The time of the event to be recorded
   */
  public void recordTimer(String name, long time) {
    try {
      Timer.builder(name).register(this).record(time, TimeUnit.MICROSECONDS);
    } catch (Throwable t) {
      LOGGER.error("error recording common timer", t);
    }
  }

  /**
   * Build a new set of common counters if they did not exist yet
   *
   * @param name The name of the counters to increment
   */
  public void buildCounterSet(String name) {
    this.buildDecayCounter(name, Duration.ofMinutes(1));
    this.buildDecayCounter(name, Duration.ofHours(1));
    this.buildCounter(name);
  }

  /**
   * Increments the value of a new or existing set of common counters
   *
   * @param name The name of the counters to increment
   * @param amount How much to increase the counters
   */
  public void incrementCounterSet(String name, long amount) {
    this.incrementDecayCounter(name, Duration.ofMinutes(1), amount);
    this.incrementDecayCounter(name, Duration.ofHours(1), amount);
    this.incrementCounter(name, amount);
  }

  /**
   * Build a new decay Counter if it did not exist yet
   *
   * @param name The name of the Counter
   * @param expiry The decay time of the Counter
   */
  public void buildDecayCounter(String name, Duration expiry) {
    try {
      Counter.builder(name).tags("decay", Long.toString(expiry.getSeconds())).register(this);
    } catch (Throwable t) {
      LOGGER.error("error building decay counter", t);
    }
  }

  /**
   * Increments the value of a new or existing decay Counter
   *
   * @param name The name of the Counter
   * @param expiry The decay time of the Counter
   * @param amount How much to increase the Counter
   */
  public void incrementDecayCounter(String name, Duration expiry, long amount) {
    try {
      Counter.builder(name)
          .tags("decay", Long.toString(expiry.getSeconds()))
          .register(this)
          .increment(amount);
    } catch (Throwable t) {
      LOGGER.error("error incrementing decay counter", t);
    }
  }

  /**
   * Builds a new default Counter if it did not exist yet
   *
   * @param name The name of the Counter
   */
  public void buildCounter(String name) {
    try {
      this.counter(name);
    } catch (Throwable t) {
      LOGGER.error("error building common counter", t);
    }
  }

  /**
   * Increments the value of a new or existing default Counter
   *
   * @param name The name of the Counter
   * @param amount How much to increase the Counter
   */
  public void incrementCounter(String name, long amount) {
    try {
      this.counter(name).increment(amount);
    } catch (Throwable t) {
      LOGGER.error("error increment common counter", t);
    }
  }

  /**
   * Returns the value of new or existing default Counter
   *
   * @param name The name of the counter
   * @return the value for that given counter name
   */
  public long getCounterValue(String name) {
    return (long) this.counter(name).count();
  }

  /**
   * Returns the value of new or existing decay Counter
   *
   * @param name The name of the counter
   * @param expiry The decay time of the counter
   * @return the value for that given counter name
   */
  public long getDecayCounterValue(String name, Duration expiry) {
    return (long) this.counter(name, "decay", Long.toString(expiry.getSeconds())).count();
  }

  /**
   * Creates a new Gauge, if it did not exist yet, that reports the value specified by a function or
   * number
   *
   * @param name The name of the Gauge
   * @param Supplier<Number> The function that generates the value to be reported by the Gauge
   */
  public <T extends Number> void buildGauge(String name, Supplier<Number> f) {
    try {
      Gauge.builder(name, f).strongReference(true).register(this);
    } catch (Throwable t) {
      LOGGER.error("error registering gauge", t);
    }
  }

  /**
   * Function that returns all the meter values
   *
   * @return A map with all the meters
   */
  public Map<String, Long> getCounters() {
    return this.getCounters(null, false);
  }

  /**
   * Function that returns all the meter values that starts with the given prefix
   *
   * @param prefixFilter the prefix to determine wheather to include or not a meter
   * @return A map with the specified meters
   */
  public Map<String, Long> getCounters(String prefixFilter) {
    return getCounters(prefixFilter, false);
  }

  /**
   * Functions that returns the meter values
   *
   * @param prefixFilter A filter to only include meters with the give prefix
   * @param exclude If true instread of including the meters with the given prefix it exclude them
   * @return A map with the specified meters
   */
  public Map<String, Long> getCounters(String prefixFilter, boolean exclude) {
    Map<String, Long> counters = new HashMap<>();
    this.forEachMeter(
        m -> {
          String name = m.getId().getName();
          if (filterName(name, prefixFilter, exclude)) {
            String decay = m.getId().getTag("decay") == null ? "" : "." + m.getId().getTag("decay");
            m.use(
                gauge -> {
                  counters.put(name, (long) Math.round(gauge.value()));
                },
                counter -> {
                  counters.put(name + decay, (long) Math.round(counter.count()));
                },
                timer -> {
                  counters.put(
                      name + ".avg" + decay, (long) Math.round(timer.mean(TimeUnit.MICROSECONDS)));

                  for (ValueAtPercentile p : timer.takeSnapshot().percentileValues()) {
                    counters.put(
                        name
                            + ".p"
                            + Long.toString((long) Math.round(100 * p.percentile()))
                            + decay,
                        (long) Math.round(p.value(TimeUnit.MICROSECONDS)));
                  }
                },
                summary -> {
                  counters.put(name + ".avg" + decay, (long) Math.round(summary.mean()));
                  for (ValueAtPercentile p : summary.takeSnapshot().percentileValues()) {
                    counters.put(
                        name
                            + ".p"
                            + Long.toString((long) Math.round(100 * p.percentile()))
                            + decay,
                        (long) Math.round(p.value()));
                  }
                },
                longTimer -> {},
                timeGauge -> {},
                functionCounter -> {},
                functionTimer -> {},
                meter -> {});
          }
        });

    return counters;
  }

  /**
   * Given the name of a meter and the given conditions if it should be included in the map
   * generated by {@link }
   *
   * @param name The name of the meter
   * @param prefixFilter A filter to determine if the meter has to be included
   * @param exclude If true instread of include the meter with the given prefix it exclude it
   * @return A Boolean with the answer of wheather to include or not the given meter
   */
  private boolean filterName(String name, String prefixFilter, boolean exclude) {
    if (prefixFilter == null) {
      return !exclude;
    }
    if (name.startsWith(prefixFilter)) {
      return !exclude;
    }
    return exclude;
  }

  public Map<String, String> getAttributes() {
    return Collections.emptyMap();
  }
}
