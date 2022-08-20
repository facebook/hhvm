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

import io.airlift.stats.DecayCounter;
import io.airlift.stats.Distribution;
import io.airlift.stats.ExponentialDecay;
import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.atomic.AtomicLong;

public class ThriftClientStats {

  // Counter Keys
  private static final String READS_KEY = ".num_reads.sum";
  private static final String WRITES_KEY = ".num_writes.sum";
  private static final String CALLS_KEY = ".num_calls.sum";
  private static final String CANCEL_KEY = ".num_cancels.sum";
  private static final String EXCEPTIONS_KEY = ".num_exceptions.sum";
  private static final String READ_TIME = ".time_read_us";
  private static final String WRITE_TIME = ".time_write_us";
  private static final String PROCESS_TIME = ".time_process_us";

  // Common Key Prefixes/Suffixes
  private static final String THRIFT_CLIENT = "thrift_client.";
  private static final String ONE_MINUTE = ".60";
  private static final String ONE_HOUR = ".3600";

  private static final String INTERACTION = "interaction.";

  private static final String CREATED = ".created";

  private static final String DISPOSED = ".diposed";
  private final Map<String, AtomicLong> counters = new ConcurrentHashMap<>();
  private final Map<String, DecayCounter> decayCounters = new ConcurrentHashMap<>();
  private final Map<String, Distribution> distributions = new ConcurrentHashMap<>();
  private final Map<String, Distribution> oneMinuteDistributions = new ConcurrentHashMap<>();
  private final Map<String, Distribution> oneHourDistributions = new ConcurrentHashMap<>();

  public ThriftClientStats() {}

  public void interactionCreated(String interactionName) {
    incrementCounterValues(INTERACTION + interactionName + CREATED);
  }

  public void interactionDisposed(String interactionName) {
    incrementCounterValues(INTERACTION + interactionName + DISPOSED);
  }

  public void publishWrite(String methodName, long writeDuration) {
    incrementCounterValues(methodName + WRITES_KEY);
    addHistogramValue(methodName + WRITE_TIME, writeDuration);
  }

  public void publishRead(String methodName, long readDuration) {
    incrementCounterValues(methodName + READS_KEY);
    addHistogramValue(methodName + READ_TIME, readDuration);
  }

  public void call(String methodName) {
    incrementCounterValues(methodName + CALLS_KEY);
  }

  public void complete(String methodName, long processDuration) {
    addHistogramValue(methodName + PROCESS_TIME, processDuration);
  }

  public void cancel(String methodName) {
    incrementCounterValues(methodName + CANCEL_KEY);
  }

  public void error(String methodName) {
    incrementCounterValues(methodName + EXCEPTIONS_KEY);
  }

  public Map<String, Long> getCounters() {
    Map<String, Long> resultCounters = new HashMap<>();
    counters.forEach(
        (key, value) -> {
          resultCounters.put(THRIFT_CLIENT + key, value.get());
        });

    decayCounters.forEach(
        (key, value) -> {
          resultCounters.put(THRIFT_CLIENT + key, Math.round(value.getCount()));
        });

    distributions.forEach((key, value) -> addCountersToResults("", key, value, resultCounters));

    oneMinuteDistributions.forEach(
        (key, value) -> addCountersToResults(".60", key, value, resultCounters));

    oneHourDistributions.forEach(
        (key, value) -> addCountersToResults(".3600", key, value, resultCounters));
    return resultCounters;
  }

  private static void addCountersToResults(
      String postFix, String key, Distribution value, Map<String, Long> resultCounters) {
    double result = value.getAvg();
    double sum = value.getCount();

    resultCounters.put(
        THRIFT_CLIENT + key + ".sum" + postFix, Double.isInfinite(sum) ? 0L : Math.round(sum));

    resultCounters.put(
        THRIFT_CLIENT + key + ".avg" + postFix,
        Double.isInfinite(result) ? 0L : Math.round(result));

    result = value.getP90();
    resultCounters.put(
        THRIFT_CLIENT + key + ".p90" + postFix,
        Double.isInfinite(result) ? 0L : Math.round(result));

    result = value.getP99();
    resultCounters.put(
        THRIFT_CLIENT + key + ".p99" + postFix,
        Double.isInfinite(result) ? 0L : Math.round(result));
  }

  private void incrementCounterValues(String key) {
    AtomicLong counter = counters.computeIfAbsent(key, k -> new AtomicLong(0));
    DecayCounter counterOneMin =
        decayCounters.computeIfAbsent(
            key + ONE_MINUTE, k -> new DecayCounter(ExponentialDecay.oneMinute()));
    DecayCounter counterOneHour =
        decayCounters.computeIfAbsent(
            key + ONE_HOUR, k -> new DecayCounter(ExponentialDecay.seconds(3600)));

    counter.incrementAndGet();
    counterOneMin.add(1);
    counterOneHour.add(1);
  }

  private void addHistogramValue(String key, long value) {
    Distribution distribution = distributions.computeIfAbsent(key, k -> new Distribution());

    Distribution distributionOneMin =
        oneMinuteDistributions.computeIfAbsent(
            key, k -> new Distribution(ExponentialDecay.oneMinute()));

    Distribution distributionOneHour =
        oneHourDistributions.computeIfAbsent(
            key, k -> new Distribution(ExponentialDecay.seconds(3600)));

    // Catching exception from airlift:stats:0.196. A stable fix is yet to be relased
    // https://github.com/prestosql/presto/issues/3965
    try {
      distribution.add(value);
      distributionOneMin.add(value);
      distributionOneHour.add(value);
    } catch (Throwable t) {
      // Suppress exception/error while adding value to distribution
    }
  }
}
