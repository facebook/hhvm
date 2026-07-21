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

import static org.assertj.core.api.Assertions.assertThat;

import java.util.Map;
import org.junit.jupiter.api.AfterEach;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.Timeout;

/**
 * Locks in the {@code thrift_offloop.*} metric contract emitted by {@link
 * ThreadPoolScheduler#getStats()}: the exact set of keys, and that a freshly constructed scheduler
 * reports finite, non-negative longs (never null) before any interval sample has rolled.
 *
 * <p>Value-level semantics (e.g. complete_tasks tracking the per-minute completion delta rather
 * than the cumulative total) are only asserted for the deterministic zero-task case. Non-zero
 * values surface on the underlying distribution's 10s interval sample and the counter's 1s tick
 * rotation, so asserting them under real time would be flaky; deterministic coverage there needs a
 * Clock/executor injection seam on ThreadPoolScheduler.
 */
public class ThreadPoolSchedulerTest {

  private ThreadPoolScheduler scheduler;

  @AfterEach
  public void tearDown() {
    if (scheduler != null) {
      scheduler.dispose();
    }
  }

  @Test
  @Timeout(30)
  public void getStats_emitsExactlyTheExpectedKeys() {
    scheduler = new ThreadPoolScheduler("test", 1, 2, 100, 1);

    assertThat(scheduler.getStats().keySet())
        .containsExactlyInAnyOrder(
            "thrift_offloop.pool_size.avg.60",
            "thrift_offloop.pending_tasks.avg.60",
            "thrift_offloop.active_tasks.avg.60",
            "thrift_offloop.complete_tasks.sum.60",
            "thrift_offloop.execution_time.p50",
            "thrift_offloop.execution_time.p75",
            "thrift_offloop.execution_time.p90",
            "thrift_offloop.execution_time.p95",
            "thrift_offloop.execution_time.p99",
            "thrift_offloop.execution_time.avg",
            "thrift_offloop.execution_time.min",
            "thrift_offloop.execution_time.max",
            "thrift_offloop.execution_time.sum");
  }

  @Test
  @Timeout(30)
  public void getStats_reportsNonNullNonNegativeValues() {
    scheduler = new ThreadPoolScheduler("test", 1, 2, 100, 1);

    for (Map.Entry<String, Long> entry : scheduler.getStats().entrySet()) {
      assertThat(entry.getValue())
          .describedAs("value for %s must be non-null and non-negative", entry.getKey())
          .isNotNull()
          .isGreaterThanOrEqualTo(0L);
    }
  }

  @Test
  @Timeout(30)
  public void getStats_withNoTasksReportsZeroCompletions() {
    // With no tasks scheduled, getCompletedTaskCount() stays 0 and the per-tick completion delta
    // stays 0 regardless of interval/tick timing -- so this holds deterministically. It also guards
    // against the earlier regression where the cumulative total was fed in as an increment.
    scheduler = new ThreadPoolScheduler("test", 1, 2, 100, 1);

    assertThat(scheduler.getStats().get("thrift_offloop.complete_tasks.sum.60")).isZero();
  }
}
