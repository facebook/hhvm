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

package com.facebook.thrift.util.resources;

import com.facebook.thrift.metrics.Ewma;
import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.atomic.LongAdder;
import org.HdrHistogram.Histogram;
import org.HdrHistogram.Recorder;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

final class ForkJoinPoolSchedulerStats {
  private static final Logger LOGGER = LoggerFactory.getLogger(ForkJoinPoolSchedulerStats.class);
  private final String name;
  private final Ewma pending;
  private final Ewma active;
  private final LongAdder disposed;
  private final LongAdder completed;

  private final Recorder executionTime;

  ForkJoinPoolSchedulerStats(String name) {
    this.name = name;
    this.pending = Ewma.oneMinuteDecay();
    this.active = Ewma.oneMinuteDecay();
    this.disposed = new LongAdder();
    this.completed = new LongAdder();
    this.executionTime = new Recorder(3, false);
  }

  public long incrementPendingAndStartRecordingTime() {
    pending.insert(1);
    return System.nanoTime();
  }

  public void incrementActiveTasks() {
    active.insert(1);
  }

  public void incrementDisposedTasksAndRecordTime(long ignore) {
    disposed.add(1);
  }

  public void incrementCompletedTasksAndRecordTime(long startTime) {
    completed.add(1);
    long time = System.nanoTime() - startTime;
    executionTime.recordValue(time);
  }

  public Map<String, Long> getStats() {
    Map<String, Long> stats = new HashMap<>();

    double result = pending.value();
    stats.put(
        "thrift." + name + ".pending_tasks.avg.60",
        Double.isInfinite(result) ? 0L : Math.round(result));

    result = active.value();
    stats.put(
        "thrift." + name + ".active_tasks.avg.60",
        Double.isInfinite(result) ? 0L : Math.round(result));

    stats.put("thrift." + name + ".complete_tasks.sum", completed.longValue());

    final Histogram histogram = executionTime.getIntervalHistogram();
    stats.put("thrift." + name + ".execution_time.avg", histogram.getValueAtPercentile(50.0));
    stats.put("thrift." + name + ".execution_time.p90", histogram.getValueAtPercentile(90));

    return stats;
  }
}
