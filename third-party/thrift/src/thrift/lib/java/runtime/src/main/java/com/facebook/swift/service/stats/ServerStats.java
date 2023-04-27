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
import io.airlift.stats.DecayCounter;
import io.airlift.stats.Distribution;
import io.airlift.stats.ExponentialDecay;
import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ThreadPoolExecutor;
import java.util.concurrent.atomic.AtomicLong;

/**
 * Implemetation of StatsSource to capture Thrift Server Counters. It tracks following counters:
 * received_requests, sent_replies, active_requests, queued_requests, accepted_connections,
 * dropped_connections and rejected_connections. More details for these counters:
 * https://our.internmc.facebook.com/intern/wiki/Thrift/FB303_Counters/ .To fetch connection related
 * counters and queued_requests, we need to pass instance of ThriftServer to this object.
 */
public class ServerStats {

  // Counter properties
  private final AtomicLong receivedRequests;
  /**
   * DecayCounters decays exponentially using a formula to calculate counts in a rolling time window
   * https://github.com/airlift/airlift/blob/master/stats/src/main/java/io/airlift/stats/DecayCounter.java
   * DecayCounters are used for 1 minute and 1 hour count of various counters.
   */
  private final DecayCounter receivedRequestsOneMin;

  private final DecayCounter receivedRequestsOneHour;

  private final AtomicLong sentReplies;
  private final DecayCounter sentRepliesOneMin;
  private final DecayCounter sentRepliesOneHour;

  private final AtomicLong activeRequests;
  private final DecayCounter activeRequestsOneMin;
  private final DecayCounter activeRequestsOneHour;

  private final Distribution processTime;
  private final Distribution processTimeOneMin;
  private final Distribution processTimeOneHour;

  private final Distribution readTime;
  private final Distribution readTimeOneMin;
  private final Distribution readTimeOneHour;

  private final Distribution writeTime;
  private final Distribution writeTimeOneMin;
  private final Distribution writeTimeOneHour;

  private final AtomicLong outOfDirectMemroyErrors;

  private final ConcurrentHashMap<String, AtomicLong> methodCounters = new ConcurrentHashMap<>();
  private final ConcurrentHashMap<String, DecayCounter> methodDecayCounters =
      new ConcurrentHashMap<>();
  private final ConcurrentHashMap<String, Distribution> methodDurations = new ConcurrentHashMap<>();

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
  private static final String METHOD_PROCESS_TIME = ".time_process_us.avg";

  // Common Key Prefix/Suffixes
  private static final String THRIFT = "thrift.";
  private static final String P99 = ".p99";
  private static final String P95 = ".p95";
  private static final String AVG = ".avg";
  private static final String ONE_MINUTE = ".60";
  private static final String ONE_HOUR = ".3600";

  private static final Map<String, String> ATTRIBUTE_MAP = new HashMap<>();

  public ServerStats() {
    // Initializing all counters
    this.receivedRequests = new AtomicLong(0);
    this.receivedRequestsOneMin = new DecayCounter(ExponentialDecay.oneMinute());
    this.receivedRequestsOneHour = new DecayCounter(ExponentialDecay.seconds(3600));

    this.sentReplies = new AtomicLong(0);
    this.sentRepliesOneMin = new DecayCounter(ExponentialDecay.oneMinute());
    this.sentRepliesOneHour = new DecayCounter(ExponentialDecay.seconds(3600));

    this.activeRequests = new AtomicLong(0);
    this.activeRequestsOneMin = new DecayCounter(ExponentialDecay.oneMinute());
    this.activeRequestsOneHour = new DecayCounter(ExponentialDecay.seconds(3600));

    this.processTime = new Distribution();
    this.processTimeOneMin = new Distribution(ExponentialDecay.oneMinute());
    this.processTimeOneHour = new Distribution(ExponentialDecay.seconds(3600));

    this.readTime = new Distribution();
    this.readTimeOneMin = new Distribution(ExponentialDecay.oneMinute());
    this.readTimeOneHour = new Distribution(ExponentialDecay.seconds(3600));

    this.writeTime = new Distribution();
    this.writeTimeOneMin = new Distribution(ExponentialDecay.oneMinute());
    this.writeTimeOneHour = new Distribution(ExponentialDecay.seconds(3600));

    this.outOfDirectMemroyErrors = new AtomicLong(0);
  }

  public void setNiftyMetrics(NiftyMetrics niftyMetrics) {
    this.niftyMetrics = niftyMetrics;
  }

  public void setThreadPoolExecutor(ThreadPoolExecutor threadPoolExecutor) {
    this.threadPoolExecutor = threadPoolExecutor;
  }

  public void requestReceived(long readTimeDuration, String methodName) {
    receivedRequests.incrementAndGet();
    receivedRequestsOneMin.add(1);
    receivedRequestsOneHour.add(1);

    activeRequests.incrementAndGet();
    activeRequestsOneMin.add(1);
    activeRequestsOneHour.add(1);

    readTime.add(readTimeDuration);
    readTimeOneMin.add(readTimeDuration);
    readTimeOneHour.add(readTimeDuration);

    incrementCounterValues(methodName + METHOD_NUM_CALLS);
  }

  public void publishWriteTime(long writeTimeDuration) {
    writeTime.add(writeTimeDuration);
    writeTimeOneMin.add(writeTimeDuration);
    writeTimeOneHour.add(writeTimeDuration);
  }

  public void replySent(long processTimeDuration, String methodName) {
    sentReplies.incrementAndGet();
    sentRepliesOneMin.add(1);
    sentRepliesOneHour.add(1);

    activeRequests.decrementAndGet();
    activeRequestsOneMin.add(-1);
    activeRequestsOneHour.add(-1);

    processTime.add(processTimeDuration);
    processTimeOneMin.add(processTimeDuration);
    processTimeOneHour.add(processTimeDuration);

    incrementCounterValues(methodName + METHOD_NUM_PROCESSED);
    addHistogramValue(methodName + METHOD_PROCESS_TIME, processTimeDuration);
  }

  public void error(String methodName) {
    incrementCounterValues(methodName + METHOD_NUM_EXCEPTIONS);
  }

  public Map<String, Long> getCounters() {
    Map<String, Long> counters = new HashMap<>();
    counters.put(RECEIVED_REQUESTS_KEY, receivedRequests.get());
    counters.put(RECEIVED_REQUESTS_KEY + ONE_MINUTE, Math.round(receivedRequestsOneMin.getCount()));
    counters.put(RECEIVED_REQUESTS_KEY + ONE_HOUR, Math.round(receivedRequestsOneHour.getCount()));

    counters.put(SENT_REPLIES_KEY, sentReplies.get());
    counters.put(SENT_REPLIES_KEY + ONE_MINUTE, Math.round(sentRepliesOneMin.getCount()));
    counters.put(SENT_REPLIES_KEY + ONE_HOUR, Math.round(sentRepliesOneHour.getCount()));

    counters.put(ACTIVE_REQUESTS_KEY, activeRequests.get());
    counters.put(ACTIVE_REQUESTS_KEY + ONE_MINUTE, Math.round(activeRequestsOneMin.getCount()));
    counters.put(ACTIVE_REQUESTS_KEY + ONE_HOUR, Math.round(activeRequestsOneHour.getCount()));

    counters.put(READ_TIME_KEY + AVG, Math.round(readTime.getAvg()));
    counters.put(READ_TIME_KEY + AVG + ONE_MINUTE, Math.round(readTimeOneMin.getAvg()));
    counters.put(READ_TIME_KEY + AVG + ONE_HOUR, Math.round(readTimeOneHour.getAvg()));

    counters.put(READ_TIME_KEY + P95, Math.round(readTime.getP95()));
    counters.put(READ_TIME_KEY + P95 + ONE_MINUTE, Math.round(readTimeOneMin.getP95()));
    counters.put(READ_TIME_KEY + P95 + ONE_HOUR, Math.round(readTimeOneHour.getP95()));

    counters.put(READ_TIME_KEY + P99, Math.round(readTime.getP99()));
    counters.put(READ_TIME_KEY + P99 + ONE_MINUTE, Math.round(readTimeOneMin.getP99()));
    counters.put(READ_TIME_KEY + P99 + ONE_HOUR, Math.round(readTimeOneHour.getP99()));

    counters.put(PROCESS_TIME_KEY + AVG, Math.round(processTime.getAvg()));
    counters.put(PROCESS_TIME_KEY + AVG + ONE_MINUTE, Math.round(processTimeOneMin.getAvg()));
    counters.put(PROCESS_TIME_KEY + AVG + ONE_HOUR, Math.round(processTimeOneHour.getAvg()));

    counters.put(PROCESS_TIME_KEY + P95, Math.round(processTime.getP95()));
    counters.put(PROCESS_TIME_KEY + P95 + ONE_MINUTE, Math.round(processTimeOneMin.getP95()));
    counters.put(PROCESS_TIME_KEY + P95 + ONE_HOUR, Math.round(processTimeOneHour.getP95()));

    counters.put(PROCESS_TIME_KEY + P99, Math.round(processTime.getP99()));
    counters.put(PROCESS_TIME_KEY + P99 + ONE_MINUTE, Math.round(processTimeOneMin.getP99()));
    counters.put(PROCESS_TIME_KEY + P99 + ONE_HOUR, Math.round(processTimeOneHour.getP99()));

    counters.put(WRITE_TIME_KEY + AVG, Math.round(writeTime.getAvg()));
    counters.put(WRITE_TIME_KEY + AVG + ONE_MINUTE, Math.round(writeTimeOneMin.getAvg()));
    counters.put(WRITE_TIME_KEY + AVG + ONE_HOUR, Math.round(writeTimeOneHour.getAvg()));

    counters.put(WRITE_TIME_KEY + P95, Math.round(writeTime.getP95()));
    counters.put(WRITE_TIME_KEY + P95 + ONE_MINUTE, Math.round(writeTimeOneMin.getP95()));
    counters.put(WRITE_TIME_KEY + P95 + ONE_HOUR, Math.round(writeTimeOneHour.getP95()));

    counters.put(OUT_OF_DIRECT_MEMORY_EXCEPTIONS_KEY, outOfDirectMemroyErrors.get());

    counters.put(WRITE_TIME_KEY + P99, Math.round(writeTime.getP99()));
    counters.put(WRITE_TIME_KEY + P99 + ONE_MINUTE, Math.round(writeTimeOneMin.getP99()));
    counters.put(WRITE_TIME_KEY + P99 + ONE_HOUR, Math.round(writeTimeOneHour.getP99()));

    if (threadPoolExecutor != null && threadPoolExecutor.getQueue() != null) {
      counters.put(QUEUED_REQUESTS_KEY, (long) threadPoolExecutor.getQueue().size());
    }
    if (niftyMetrics != null) {
      counters.put(ACCEPTED_CONNS_KEY, niftyMetrics.getAcceptedConnections());
      counters.put(ACCEPTED_CONNS_KEY + ONE_MINUTE, niftyMetrics.getAcceptedConnectionsOneMin());
      counters.put(ACCEPTED_CONNS_KEY + ONE_HOUR, niftyMetrics.getAcceptedConnectionsOneHour());

      counters.put(DROPPED_CONNS_KEY, niftyMetrics.getDroppedConnections());
      counters.put(DROPPED_CONNS_KEY + ONE_MINUTE, niftyMetrics.getDroppedConnectionsOneMin());
      counters.put(DROPPED_CONNS_KEY + ONE_HOUR, niftyMetrics.getDroppedConnectionsOneHour());

      counters.put(REJECTED_CONNS_KEY, niftyMetrics.getRejectedConnections());
      counters.put(REJECTED_CONNS_KEY + ONE_MINUTE, niftyMetrics.getRejectedConnectionsOneMin());
      counters.put(REJECTED_CONNS_KEY + ONE_HOUR, niftyMetrics.getRejectedConnectionsOneHour());
    }

    methodCounters.forEach(
        (key, value) -> {
          counters.put(THRIFT + key, value.get());
        });
    methodDecayCounters.forEach(
        (key, value) -> {
          counters.put(THRIFT + key, Math.round(value.getCount()));
        });
    methodDurations.forEach(
        (key, value) -> {
          counters.put(THRIFT + key, Math.round(value.getAvg()));
        });
    return counters;
  }

  public Map<String, String> getAttributes() {
    return ATTRIBUTE_MAP;
  }

  public void markDirectOomError() {
    this.outOfDirectMemroyErrors.incrementAndGet();
  }

  private void incrementCounterValues(String key) {
    AtomicLong counter = methodCounters.computeIfAbsent(key, k -> new AtomicLong(0));
    DecayCounter counterOneMin =
        methodDecayCounters.computeIfAbsent(
            key + ONE_MINUTE, k -> new DecayCounter(ExponentialDecay.oneMinute()));
    DecayCounter counterOneHour =
        methodDecayCounters.computeIfAbsent(
            key + ONE_HOUR, k -> new DecayCounter(ExponentialDecay.oneMinute()));

    counter.incrementAndGet();
    counterOneMin.add(1);
    counterOneHour.add(1);
  }

  private void addHistogramValue(String key, long value) {
    Distribution duration = methodDurations.computeIfAbsent(key, k -> new Distribution());
    Distribution durationOneMin =
        methodDurations.computeIfAbsent(
            key + ONE_MINUTE, k -> new Distribution(ExponentialDecay.oneMinute()));
    Distribution durationOneHour =
        methodDurations.computeIfAbsent(
            key + ONE_HOUR, k -> new Distribution(ExponentialDecay.seconds(3600)));

    duration.add(value);
    durationOneMin.add(value);
    durationOneHour.add(value);
  }
}
