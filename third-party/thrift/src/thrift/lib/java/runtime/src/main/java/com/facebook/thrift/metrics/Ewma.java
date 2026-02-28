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

package com.facebook.thrift.metrics;

import java.util.concurrent.TimeUnit;
import java.util.concurrent.locks.ReentrantLock;

/**
 * Estimated weighted average that uses a {@link ReentrantLock} instead of synchronized. A
 * ReetranctLock will work with virtual threads (Project Loom)
 */
public class Ewma {

  private static final Clock DEFAULT_CLOCK = new Clock();

  private final long decay;

  private long lastUpdateTimestamp;

  private final Clock clock;

  private final ReentrantLock lock;

  private volatile double average;

  public static Ewma oneHourDecay() {
    return new Ewma(1, TimeUnit.HOURS, 0.0d, DEFAULT_CLOCK);
  }

  public static Ewma oneMinuteDecay() {
    return new Ewma(1, TimeUnit.MINUTES, 0.0d, DEFAULT_CLOCK);
  }

  public static Ewma oneSecondDecay() {
    return new Ewma(1, TimeUnit.SECONDS, 0.0d, DEFAULT_CLOCK);
  }

  public Ewma(long halfLife, TimeUnit unit, double initialValue, Clock clock) {
    this.lock = new ReentrantLock();
    this.decay = clock.unit().convert((long) (halfLife / Math.log(2)), unit);
    this.average = initialValue;
    this.clock = clock;
    this.lastUpdateTimestamp = 0;
  }

  public void insert(double value) {
    lock.lock();
    try {
      final long now = clock.now();
      final double elapsed = Math.max(0, now - lastUpdateTimestamp);

      lastUpdateTimestamp = now;

      double weight = Math.exp(-elapsed / decay);
      average = weight * average + (1.0 - weight) * value;
    } finally {
      lock.unlock();
    }
  }

  public double value() {
    return average;
  }

  @Override
  public String toString() {
    return "Ewma(average=" + average + ")";
  }
}
