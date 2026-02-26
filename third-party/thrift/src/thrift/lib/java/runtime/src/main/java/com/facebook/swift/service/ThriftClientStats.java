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

import static java.util.Arrays.asList;

import com.facebook.thrift.metrics.distribution.MultiWindowDistribution;
import com.facebook.thrift.metrics.distribution.Quantile;
import com.facebook.thrift.metrics.rate.SlidingTimeWindowMovingCounter;
import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.atomic.LongAdder;

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

  private final ConcurrentHashMap<String, SlidingTimeWindowMovingCounter> counters =
      new ConcurrentHashMap<>();
  private final ConcurrentHashMap<String, MultiWindowDistribution> distributions =
      new ConcurrentHashMap<>();
  private final ConcurrentHashMap<String, LongAdder> allTimeCounters = new ConcurrentHashMap<>();

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
          resultCounters.put(THRIFT_CLIENT + key + ONE_MINUTE, value.oneMinuteRate());
          resultCounters.put(THRIFT_CLIENT + key + ONE_HOUR, value.oneHourRate());
        });

    allTimeCounters.forEach(
        (key, counter) -> resultCounters.put(THRIFT_CLIENT + key, counter.sum()));

    distributions.forEach(
        (key, dist) -> {
          addQuantilesToCounters(
              THRIFT_CLIENT + key, ONE_MINUTE, resultCounters, dist.getOneMinuteQuantiles());
          addQuantilesToCounters(
              THRIFT_CLIENT + key, ONE_HOUR, resultCounters, dist.getOneHourQuantiles());
          addQuantilesToCounters(
              THRIFT_CLIENT + key, "", resultCounters, dist.getAllTimeQuantiles());
        });
    return resultCounters;
  }

  private void addQuantilesToCounters(
      String baseKey, String windowKey, Map<String, Long> counters, Map<Quantile, Long> quantiles) {
    for (Map.Entry<Quantile, Long> entry : quantiles.entrySet()) {
      counters.put(baseKey + "." + entry.getKey().getKey() + windowKey, entry.getValue());
    }
  }

  private void incrementCounterValues(String key) {
    // Optimistic get-first to avoid ConcurrentHashMap bucket-level locking in computeIfAbsent
    LongAdder adder = allTimeCounters.get(key);
    if (adder == null) {
      adder = allTimeCounters.computeIfAbsent(key, k -> new LongAdder());
    }
    adder.increment();

    SlidingTimeWindowMovingCounter counter = counters.get(key);
    if (counter == null) {
      counter = counters.computeIfAbsent(key, k -> new SlidingTimeWindowMovingCounter());
    }
    counter.add(1);
  }

  private void addHistogramValue(String key, long value) {
    // Optimistic get-first to avoid ConcurrentHashMap bucket-level locking in computeIfAbsent
    MultiWindowDistribution dist = distributions.get(key);
    if (dist == null) {
      dist =
          distributions.computeIfAbsent(
              key,
              k ->
                  new MultiWindowDistribution(
                      asList(Quantile.P99, Quantile.P90, Quantile.AVG, Quantile.SUM)));
    }
    dist.add(value);
  }
}
