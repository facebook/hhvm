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

import com.facebook.nifty.core.NiftyMetrics;
import com.facebook.thrift.metrics.distribution.MultiWindowDistribution;
import com.facebook.thrift.metrics.distribution.Quantile;
import com.facebook.thrift.metrics.rate.ExpMovingAverageRate;
import com.facebook.thrift.metrics.rate.SlidingTimeWindowMovingCounter;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ThreadPoolExecutor;
import java.util.concurrent.atomic.LongAdder;

/**
 * Implementation of StatsSource to capture Thrift Server Counters. It tracks following counters:
 * received_requests, sent_replies, active_requests, queued_requests, accepted_connections,
 * dropped_connections and rejected_connections. More details for these counters:
 * https://our.internmc.facebook.com/intern/wiki/Thrift/FB303_Counters/ .To fetch connection related
 * counters and queued_requests, we need to pass instance of ThriftServer to this object.
 */
public class ServerStats {
  private final ConcurrentHashMap<String, ExpMovingAverageRate> movingAverages =
      new ConcurrentHashMap<>();
  private final ConcurrentHashMap<String, SlidingTimeWindowMovingCounter> counters =
      new ConcurrentHashMap<>();
  private final ConcurrentHashMap<String, MultiWindowDistribution> distributions =
      new ConcurrentHashMap<>();
  private final ConcurrentHashMap<String, LongAdder> allTimeCounters = new ConcurrentHashMap<>();

  // Thrift Server properties
  private NiftyMetrics niftyMetrics;
  private ThreadPoolExecutor threadPoolExecutor;

  // Request related Counter Keys
  private static final String RECEIVED_REQUESTS_KEY = "thrift.received_requests.count";
  private static final String SENT_REPLIES_KEY = "thrift.sent_replies.count";
  private static final String OUT_OF_DIRECT_MEMORY_EXCEPTIONS_KEY =
      "thrift.out_of_direct_memory_num_exceptions.count";
  private static final String ACTIVE_REQUESTS_KEY = "thrift.active_requests.avg";
  private static final String QUEUED_REQUESTS_KEY = "thrift.queued_requests.avg.60";
  // Connection related Counter Keys
  private static final String CHANNEL_COUNT_KEY = "thrift.channel.count";
  private static final String BYTES_READ_KEY = "thrift.bytes_read.count";
  private static final String BYTES_WRITTEN_KEY = "thrift.bytes_written.count";
  private static final String ACCEPTED_CONNS_KEY = "thrift.accepted_connections.count";
  private static final String DROPPED_CONNS_KEY = "thrift.dropped_conns.count";
  private static final String REJECTED_CONNS_KEY = "thrift.rejected_conns.count";
  // Latency related counter keys
  private static final String PROCESS_TIME_KEY = "thrift.process_time";
  private static final String READ_TIME_KEY = "thrift.read_time";
  private static final String WRITE_TIME_KEY = "thrift.write_time";
  // Per method counter keys
  private static final String METHOD_NUM_CALLS = ".num_calls.sum";
  private static final String METHOD_NUM_PROCESSED = ".num_processed.sum";
  private static final String METHOD_NUM_EXCEPTIONS = ".num_exceptions.sum";
  private static final String METHOD_PROCESS_TIME = ".time_process_us";

  // Common Key Prefix/Suffixes
  private static final String THRIFT = "thrift.";

  private static final String ONE_MINUTE = ".60";
  private static final String ONE_HOUR = ".3600";

  private static final Map<String, String> ATTRIBUTE_MAP = new HashMap<>();

  public ServerStats() {}

  public void markDirectOomError() {
    incrementCounter(OUT_OF_DIRECT_MEMORY_EXCEPTIONS_KEY, 1);
  }

  public void setNiftyMetrics(NiftyMetrics niftyMetrics) {
    this.niftyMetrics = niftyMetrics;
  }

  public void setThreadPoolExecutor(ThreadPoolExecutor threadPoolExecutor) {
    this.threadPoolExecutor = threadPoolExecutor;
  }

  public void requestReceived(long readTimeDuration, String methodName) {
    incrementCounter(RECEIVED_REQUESTS_KEY, 1);

    incrementAverages(ACTIVE_REQUESTS_KEY, 1);

    updateDistribution(READ_TIME_KEY, readTimeDuration);

    incrementCounter(THRIFT + methodName + METHOD_NUM_CALLS, 1);
  }

  public void publishWriteTime(long writeTimeDuration) {
    updateDistribution(WRITE_TIME_KEY, writeTimeDuration);
  }

  public void replySent(long processTimeDuration, String methodName) {
    incrementCounter(SENT_REPLIES_KEY, 1);
    incrementAverages(ACTIVE_REQUESTS_KEY, 1);
    incrementCounter(THRIFT + methodName + METHOD_NUM_PROCESSED, 1);

    updateDistribution(PROCESS_TIME_KEY, processTimeDuration);
    updateDistribution(THRIFT + methodName + METHOD_PROCESS_TIME, processTimeDuration);
  }

  public void error(String methodName) {
    incrementCounter(THRIFT + methodName + METHOD_NUM_EXCEPTIONS, 1);
  }

  public Map<String, Long> getCounters() {
    Map<String, Long> resultCounters = new HashMap<>(64);

    allTimeCounters.forEach((key, counter) -> resultCounters.put(key, counter.sum()));

    counters.forEach(
        (key, counter) -> {
          resultCounters.put(key + ONE_MINUTE, counter.oneMinuteRate());
          resultCounters.put(key + ONE_HOUR, counter.oneHourRate());
        });

    movingAverages.forEach(
        (key, counter) -> {
          resultCounters.put(key + ONE_MINUTE, counter.oneMinuteRate());
          resultCounters.put(key + ONE_HOUR, counter.oneHourRate());
        });

    distributions.forEach(
        (key, dist) -> {
          addQuantilesToCounters(key, ONE_MINUTE, resultCounters, dist.getOneMinuteQuantiles());
          addQuantilesToCounters(key, ONE_HOUR, resultCounters, dist.getOneHourQuantiles());
          addQuantilesToCounters(key, "", resultCounters, dist.getAllTimeQuantiles());
        });

    if (threadPoolExecutor != null && threadPoolExecutor.getQueue() != null) {
      resultCounters.put(QUEUED_REQUESTS_KEY, (long) threadPoolExecutor.getQueue().size());
    }
    if (niftyMetrics != null) {
      resultCounters.put(CHANNEL_COUNT_KEY, (long) niftyMetrics.getChannelCount());

      resultCounters.put(BYTES_READ_KEY, niftyMetrics.getBytesRead());
      resultCounters.put(BYTES_WRITTEN_KEY, niftyMetrics.getBytesWritten());

      resultCounters.put(ACCEPTED_CONNS_KEY, niftyMetrics.getAcceptedConnections());
      resultCounters.put(
          ACCEPTED_CONNS_KEY + ONE_MINUTE, niftyMetrics.getAcceptedConnectionsOneMin());
      resultCounters.put(
          ACCEPTED_CONNS_KEY + ONE_HOUR, niftyMetrics.getAcceptedConnectionsOneHour());

      resultCounters.put(DROPPED_CONNS_KEY, niftyMetrics.getDroppedConnections());
      resultCounters.put(
          DROPPED_CONNS_KEY + ONE_MINUTE, niftyMetrics.getDroppedConnectionsOneMin());
      resultCounters.put(DROPPED_CONNS_KEY + ONE_HOUR, niftyMetrics.getDroppedConnectionsOneHour());

      resultCounters.put(REJECTED_CONNS_KEY, niftyMetrics.getRejectedConnections());
      resultCounters.put(
          REJECTED_CONNS_KEY + ONE_MINUTE, niftyMetrics.getRejectedConnectionsOneMin());
      resultCounters.put(
          REJECTED_CONNS_KEY + ONE_HOUR, niftyMetrics.getRejectedConnectionsOneHour());
    }

    return resultCounters;
  }

  private void addQuantilesToCounters(
      String baseKey, String windowKey, Map<String, Long> counters, Map<Quantile, Long> quantiles) {
    for (Map.Entry<Quantile, Long> entry : quantiles.entrySet()) {
      counters.put(baseKey + "." + entry.getKey().getKey() + windowKey, entry.getValue());
    }
  }

  public Map<String, String> getAttributes() {
    return ATTRIBUTE_MAP;
  }

  private void incrementAverages(String key, int value) {
    // Optimistic get-first to avoid ConcurrentHashMap bucket-level locking in computeIfAbsent
    ExpMovingAverageRate rate = movingAverages.get(key);
    if (rate == null) {
      rate = movingAverages.computeIfAbsent(key, k -> new ExpMovingAverageRate());
    }
    rate.add(value);
  }

  private void incrementCounter(String key, int value) {
    // Optimistic get-first to avoid ConcurrentHashMap bucket-level locking in computeIfAbsent
    LongAdder adder = allTimeCounters.get(key);
    if (adder == null) {
      adder = allTimeCounters.computeIfAbsent(key, k -> new LongAdder());
    }
    adder.add(value);

    SlidingTimeWindowMovingCounter counter = counters.get(key);
    if (counter == null) {
      counter = counters.computeIfAbsent(key, k -> new SlidingTimeWindowMovingCounter());
    }
    counter.add(value);
  }

  private void updateDistribution(String key, long value) {
    // Optimistic get-first to avoid ConcurrentHashMap bucket-level locking in computeIfAbsent
    MultiWindowDistribution dist = distributions.get(key);
    if (dist == null) {
      dist =
          distributions.computeIfAbsent(
              key,
              k ->
                  new MultiWindowDistribution(
                      Arrays.asList(Quantile.AVG, Quantile.P95, Quantile.P99)));
    }
    dist.add(value);
  }
}
