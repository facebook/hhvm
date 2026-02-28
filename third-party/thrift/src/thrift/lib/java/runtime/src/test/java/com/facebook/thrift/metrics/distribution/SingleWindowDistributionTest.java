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

package com.facebook.thrift.metrics.distribution;

import static com.facebook.thrift.metrics.distribution.Quantile.AVG;
import static com.facebook.thrift.metrics.distribution.Quantile.MAX;
import static com.facebook.thrift.metrics.distribution.Quantile.MIN;
import static com.facebook.thrift.metrics.distribution.Quantile.P50;
import static com.facebook.thrift.metrics.distribution.Quantile.P75;
import static com.facebook.thrift.metrics.distribution.Quantile.P90;
import static com.facebook.thrift.metrics.distribution.Quantile.P95;
import static com.facebook.thrift.metrics.distribution.Quantile.P99;
import static com.facebook.thrift.metrics.distribution.Quantile.SUM;
import static java.util.Arrays.asList;
import static org.assertj.core.api.Assertions.assertThat;

import java.util.Map;
import java.util.concurrent.TimeUnit;
import org.junit.Before;
import org.junit.Test;

public class SingleWindowDistributionTest {
  private SingleWindowDistribution dist;
  private final TestClock testClock = new TestClock();

  @Before
  public void setUp() {
    dist =
        new SingleWindowDistribution(
            new NoopScheduledExecutorService(),
            testClock,
            asList(P50, P75, P90, P95, P99, AVG, MIN, MAX, SUM));
  }

  private void advanceSeconds(long seconds) {
    for (long i = 0; i < seconds; i += 10) {
      testClock.incrementSec(10);
      dist.performIntervalSampleImpl();
    }
  }

  private void advanceMinutes(long minutes) {
    advanceSeconds(TimeUnit.MINUTES.toSeconds(minutes));
  }

  @Test
  public void testNormalQuantilesOverOneCycle() {
    for (int i = 1; i <= 100; i++) {
      dist.add(i);
    }

    advanceSeconds(10);

    // One Minute
    Map<Quantile, Long> quantiles = dist.getOneMinuteQuantiles();
    assertThat(quantiles.get(P99)).isEqualTo(99L);
    assertThat(quantiles.get(P95)).isEqualTo(95L);
    assertThat(quantiles.get(P90)).isEqualTo(90L);
    assertThat(quantiles.get(P75)).isEqualTo(75L);
    assertThat(quantiles.get(P50)).isEqualTo(50L);

    assertThat(quantiles.get(MIN)).isEqualTo(1);
    assertThat(quantiles.get(MAX)).isEqualTo(100);
    assertThat(quantiles.get(AVG)).isEqualTo(50);

    assertThat(quantiles.get(SUM)).isEqualTo(100);
  }

  @Test
  public void testSamplesPhaseOutOneMinute() {
    for (int i = 1; i <= 50; i++) {
      dist.add(i);
    }

    advanceSeconds(10);

    for (int i = 51; i <= 100; i++) {
      dist.add(i);
    }

    advanceSeconds(50);

    Map<Quantile, Long> quantiles = dist.getOneMinuteQuantiles();
    assertThat(quantiles.get(P99)).isEqualTo(99L);
    assertThat(quantiles.get(P95)).isEqualTo(95L);
    assertThat(quantiles.get(P90)).isEqualTo(90L);
    assertThat(quantiles.get(P75)).isEqualTo(75L);
    assertThat(quantiles.get(P50)).isEqualTo(50L);

    assertThat(quantiles.get(MIN)).isEqualTo(1);
    assertThat(quantiles.get(MAX)).isEqualTo(100);
    assertThat(quantiles.get(AVG)).isEqualTo(50);

    assertThat(quantiles.get(SUM)).isEqualTo(100);

    // Expire data from first window
    advanceSeconds(10);

    quantiles = dist.getOneMinuteQuantiles();
    assertThat(quantiles.get(P99)).isEqualTo(100);
    assertThat(quantiles.get(P95)).isEqualTo(98);
    assertThat(quantiles.get(P90)).isEqualTo(95);
    assertThat(quantiles.get(P75)).isEqualTo(88);
    assertThat(quantiles.get(P50)).isEqualTo(75);

    assertThat(quantiles.get(MIN)).isEqualTo(51);
    assertThat(quantiles.get(MAX)).isEqualTo(100);
    assertThat(quantiles.get(AVG)).isEqualTo(75);

    assertThat(quantiles.get(SUM)).isEqualTo(50);
  }

  @Test
  public void testSamplesPhaseOutAllData() {
    for (int i = 1; i <= 50; i++) {
      dist.add(i);
    }

    advanceSeconds(10);

    for (int i = 51; i <= 100; i++) {
      dist.add(i);
    }

    advanceSeconds(10);

    // One Minute
    Map<Quantile, Long> quantiles = dist.getOneMinuteQuantiles();
    assertThat(quantiles.get(P99)).isEqualTo(99L);
    assertThat(quantiles.get(P95)).isEqualTo(95L);
    assertThat(quantiles.get(P90)).isEqualTo(90L);
    assertThat(quantiles.get(P75)).isEqualTo(75L);
    assertThat(quantiles.get(P50)).isEqualTo(50L);

    assertThat(quantiles.get(MIN)).isEqualTo(1);
    assertThat(quantiles.get(MAX)).isEqualTo(100);
    assertThat(quantiles.get(AVG)).isEqualTo(50);

    assertThat(quantiles.get(SUM)).isEqualTo(100);

    // Expire data from first window
    advanceSeconds(60);

    // One Minute
    quantiles = dist.getOneMinuteQuantiles();
    assertThat(quantiles.get(P99)).isEqualTo(0);
    assertThat(quantiles.get(P95)).isEqualTo(0);
    assertThat(quantiles.get(P90)).isEqualTo(0);
    assertThat(quantiles.get(P75)).isEqualTo(0);
    assertThat(quantiles.get(P50)).isEqualTo(0);

    assertThat(quantiles.get(MIN)).isEqualTo(0);
    assertThat(quantiles.get(MAX)).isEqualTo(0);
    assertThat(quantiles.get(AVG)).isEqualTo(0);

    assertThat(quantiles.get(SUM)).isEqualTo(0);
  }
}
