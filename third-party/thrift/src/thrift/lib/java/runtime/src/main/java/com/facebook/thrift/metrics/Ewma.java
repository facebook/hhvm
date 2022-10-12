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
import java.util.concurrent.atomic.DoubleAdder;
import java.util.concurrent.atomic.LongAdder;
import org.HdrHistogram.WriterReaderPhaser;

/**
 * Estimated Weighted Moving Average that uses {@link WriterReaderPhaser} from HdrHistogram to make
 * writes lock-free.
 */
public class Ewma {
  private static class Counter {
    private final DoubleAdder value = new DoubleAdder();
    private final LongAdder count = new LongAdder();

    private void insert(double d) {
      value.add(d);
      count.increment();
    }

    private double avgAndReset() {
      double v = value.sumThenReset();
      double c = Math.max(1, count.sumThenReset());
      return v / c;
    }
  }

  private Counter active;
  private Counter inactive;

  private long stamp;

  final long tau;

  double ewma;

  private final WriterReaderPhaser recordingPhaser;

  public static Ewma oneMinuteIntervalEWMA() {
    return new Ewma(1, TimeUnit.MINUTES);
  }

  public Ewma(long halflife, TimeUnit unit) {
    this.active = new Counter();
    this.inactive = new Counter();
    this.stamp = System.nanoTime();
    double v = halflife / Math.log(2);
    this.tau = TimeUnit.NANOSECONDS.convert((long) v, unit);
    this.recordingPhaser = new WriterReaderPhaser();
  }

  public void insert(double d) {
    long criticalValueAtEnter = recordingPhaser.writerCriticalSectionEnter();
    try {
      active.insert(d);
    } finally {
      recordingPhaser.writerCriticalSectionExit(criticalValueAtEnter);
    }
  }

  public double value() {
    try {
      recordingPhaser.readerLock();

      final Counter temp = inactive;
      inactive = active;
      active = temp;

      recordingPhaser.flipPhase();

      final double unrecorded = inactive.avgAndReset();
      final long now = System.nanoTime();
      final double elapsed = Math.max(0, now - stamp);

      stamp = now;

      double w = Math.exp(-elapsed / tau);
      ewma = w * ewma + (1.0 - w) * unrecorded;
      return ewma;
    } finally {
      recordingPhaser.readerUnlock();
    }
  }
}
