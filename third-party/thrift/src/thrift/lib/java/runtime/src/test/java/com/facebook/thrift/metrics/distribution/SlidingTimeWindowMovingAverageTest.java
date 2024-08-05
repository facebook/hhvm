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

import static org.assertj.core.api.Assertions.assertThat;

import com.facebook.thrift.metrics.rate.SlidingTimeWindowMovingAverages;
import java.util.concurrent.TimeUnit;
import org.junit.Before;
import org.junit.Test;

public class SlidingTimeWindowMovingAverageTest {
  private SlidingTimeWindowMovingAverages rate;
  private final TestClock testClock = new TestClock();

  @Before
  public void setUp() {
    rate = new SlidingTimeWindowMovingAverages(testClock);
  }

  private void advanceSeconds(long seconds) {
    testClock.incrementSec(seconds);
    rate.update(0);
  }

  private void advanceMinutes(long minutes) {
    advanceSeconds(TimeUnit.MINUTES.toSeconds(minutes));
  }

  @Test
  public void testNormalQuantilesOverOneCycle() {
    for (int i = 1; i <= 60; i++) {
      advanceSeconds(1);
      rate.update(1);
    }

    // One Minute, one per second
    long oneMinRate = rate.oneMinuteRate();
    assertThat(oneMinRate).isEqualTo(60);
  }

  @Test
  public void testOneMinuteDataDecaysToZeroOver1Minute() {
    for (int i = 1; i <= 60; i++) {
      advanceSeconds(1);
      rate.update(1);
    }

    assertThat(rate.oneMinuteRate()).isEqualTo(60);

    for (int i = 0, expected = 55; i < 60; i += 5, expected -= 5) {
      advanceSeconds(5);
      assertThat(rate.oneMinuteRate()).isEqualTo(expected);
    }
  }

  @Test
  public void testTenMinuteDataDecaysToZeroOver10Minutes() {
    for (int i = 1; i <= 600; i++) {
      advanceSeconds(1);
      rate.update(1);
    }

    assertThat(rate.tenMinuteRate()).isEqualTo(600);

    for (int i = 0, expected = 540; i < 600; i += 60, expected -= 60) {
      advanceSeconds(60);
      assertThat(rate.tenMinuteRate()).isEqualTo(expected);
    }
  }

  @Test
  public void testOneHourDataDecaysToZeroOver10Minutes() {
    for (int i = 1; i <= 60; i++) {
      advanceMinutes(1);
      rate.update(1);
    }

    assertThat(rate.oneHourRate()).isEqualTo(60);

    for (int i = 0, expected = 59; i < 3_600; i += 60, expected -= 1) {
      advanceSeconds(60);
      assertThat(rate.oneHourRate()).isEqualTo(expected);
    }
  }
}
