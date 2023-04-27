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

import io.micrometer.core.instrument.Clock;
import io.micrometer.core.instrument.distribution.DistributionStatisticConfig;
import java.util.concurrent.atomic.AtomicIntegerFieldUpdater;
import java.util.concurrent.atomic.AtomicLong;

/**
 * A Decay Counter class managed by ThriftMeterRegistry metrics, if you are working with a decay of
 * T seconds and buffer length B it reports increments made on the last T - E seconds where E is
 * positive and at most T/B.
 *
 * <p>This code was based on Micrometer implementation to calculate the maximum reported value in
 * some meters, then adapted and bug fixed to work as a decay counter
 */
public class TimeWindowCount {
  private static final AtomicIntegerFieldUpdater<TimeWindowCount> rotatingUpdater =
      AtomicIntegerFieldUpdater.newUpdater(TimeWindowCount.class, "rotating");

  private final Clock clock;
  private final long durationBetweenRotatesMillis;
  private AtomicLong[] ringBuffer;
  private int currentBucket;
  private volatile long lastRotateTimestampMillis;

  @SuppressWarnings({"unused", "FieldCanBeLocal"})
  private volatile int rotating; // 0 - not rotating, 1 - rotating

  @SuppressWarnings("ConstantConditions")
  public TimeWindowCount(Clock clock, DistributionStatisticConfig config) {
    this(clock, config.getExpiry().toMillis(), config.getBufferLength());
  }

  /**
   * Initialize the decay counter
   *
   * @param clock The clock to measure the time
   * @param rotateFrequencyMillis The decay time for the counter
   * @param bufferLength How much buckets we will use, the bigger it is the precise it becomes, and
   *     also the resouces it uses
   */
  public TimeWindowCount(Clock clock, long rotateFrequencyMillis, int bufferLength) {
    this.clock = clock;
    this.durationBetweenRotatesMillis = rotateFrequencyMillis / bufferLength;
    this.lastRotateTimestampMillis = clock.wallTime();
    this.currentBucket = 0;

    this.ringBuffer = new AtomicLong[bufferLength];
    for (int i = 0; i < bufferLength; i++) {
      this.ringBuffer[i] = new AtomicLong();
    }
  }

  /**
   * Increment the Decay Counter
   *
   * @param value How much to increment the counter
   */
  public void add(long value) {
    rotate();
    for (AtomicLong current : ringBuffer) {
      updateValue(current, value);
    }
  }

  /**
   * Consultate the value of the decay counter at this time
   *
   * @return the value of the decay counter at this time
   */
  public double poll() {
    rotate();
    synchronized (this) {
      return ringBuffer[currentBucket].get();
    }
  }

  /**
   * Function to update the values of the buckets caused by an increment on the counter
   *
   * @param current The current bucket to be updated/incremented
   * @param value How much the bucket will increment
   */
  private void updateValue(AtomicLong current, long value) {
    current.getAndAdd(value);
  }

  /**
   * Moves to another bucket if necessary when is requested and clears the ones that need to be
   * "decayed".
   */
  private void rotate() {
    long timeSinceLastRotateMillis = clock.wallTime() - lastRotateTimestampMillis;
    if (timeSinceLastRotateMillis < durationBetweenRotatesMillis) {
      // Need to wait more for next rotation.
      return;
    }

    if (!rotatingUpdater.compareAndSet(this, 0, 1)) {
      // Being rotated by other thread already.
      return;
    }

    try {
      int iterations = 0;
      synchronized (this) {
        do {
          ringBuffer[currentBucket].set(0);
          if (++currentBucket >= ringBuffer.length) {
            currentBucket = 0;
          }
          timeSinceLastRotateMillis -= durationBetweenRotatesMillis;
          lastRotateTimestampMillis += durationBetweenRotatesMillis;
        } while (timeSinceLastRotateMillis >= durationBetweenRotatesMillis
            && ++iterations < ringBuffer.length);
      }
    } finally {
      rotating = 0;
    }
  }
}
