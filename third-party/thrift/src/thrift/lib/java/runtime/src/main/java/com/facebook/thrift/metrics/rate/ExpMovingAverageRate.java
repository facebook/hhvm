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

package com.facebook.thrift.metrics.rate;

import static java.lang.Math.exp;
import static java.util.concurrent.TimeUnit.MINUTES;
import static java.util.concurrent.TimeUnit.SECONDS;

import com.facebook.thrift.metrics.common.Clock;
import com.facebook.thrift.metrics.common.NanoClock;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicLong;
import java.util.concurrent.atomic.LongAdder;

public class ExpMovingAverageRate implements Rate {
  private static final long TICK_INTERVAL_SEC = 5L;
  private static final long TICK_INTERVAL_NANO = SECONDS.toNanos(TICK_INTERVAL_SEC);
  private static final Clock NANO_CLOCK = new NanoClock();

  private final EWMA m1;
  private final EWMA m10;
  private final EWMA m60;
  private final AtomicLong lastTick;
  private final Clock clock;

  public ExpMovingAverageRate(Clock clock) {
    this.m1 = EWMA.oneMinuteEWMA();
    this.m10 = EWMA.tenMinuteEWMA();
    this.m60 = EWMA.sixtyMinuteEWMA();
    this.clock = clock;

    long now = clock.tickNanos();
    this.lastTick = new AtomicLong(now);
  }

  public ExpMovingAverageRate() {
    this(NANO_CLOCK);
  }

  public void update(long count) {
    if (lastTick == null) {
      return;
    }

    tickIfNeeded();

    m1.update(count);
    m10.update(count);
    m60.update(count);
  }

  private void tickIfNeeded() {
    long oldTick = lastTick.get();
    long newTick = clock.tickNanos();
    long diff = newTick - oldTick;

    if (diff > TICK_INTERVAL_NANO) {
      long newIntervalStartTick = newTick - diff % TICK_INTERVAL_NANO;

      if (lastTick.compareAndSet(oldTick, newIntervalStartTick)) {
        long elapsedTicks = diff / TICK_INTERVAL_NANO;

        for (long i = 0; i < elapsedTicks; ++i) {
          if (m1 != null) {
            m1.tick();
          }

          if (m10 != null) {
            m10.tick();
          }

          if (m60 != null) {
            m60.tick();
          }
        }
      }
    }
  }

  @Override
  public long oneMinuteRate() {
    tickIfNeeded();
    return m1.getRate();
  }

  @Override
  public long tenMinuteRate() {
    tickIfNeeded();
    return m10.getRate();
  }

  @Override
  public long oneHourRate() {
    tickIfNeeded();
    return m60.getRate();
  }

  private static class EWMA {
    private static final double SECONDS_PER_MINUTE = 60.0;
    private static final int ONE_MINUTE = 1;
    private static final int TEN_MINUTES = 10;
    private static final int SIXTY_MINUTES = 60;
    private static final double M1_ALPHA =
        1 - exp(-TICK_INTERVAL_SEC / SECONDS_PER_MINUTE / ONE_MINUTE);
    private static final double M10_ALPHA =
        1 - exp(-TICK_INTERVAL_SEC / SECONDS_PER_MINUTE / TEN_MINUTES);
    private static final double M60_ALPHA =
        1 - exp(-TICK_INTERVAL_SEC / SECONDS_PER_MINUTE / SIXTY_MINUTES);

    private volatile boolean initialized = false;
    // Rate is count per nanos
    private volatile double rate = 0.0;

    private final LongAdder uncounted = new LongAdder();
    private final double alpha, interval;

    /**
     * Creates a new EWMA which is equivalent to one minute load average and which expects to be
     * ticked every 5 seconds.
     *
     * @return a one-minute EWMA
     */
    public static EWMA oneMinuteEWMA() {
      return new EWMA(M1_ALPHA, TICK_INTERVAL_SEC, TimeUnit.SECONDS);
    }

    /**
     * Creates a new EWMA which is equivalent to ten minute load average and which expects to be
     * ticked every 5 seconds.
     *
     * @return a ten-minute EWMA
     */
    public static EWMA tenMinuteEWMA() {
      return new EWMA(M10_ALPHA, TICK_INTERVAL_SEC, TimeUnit.SECONDS);
    }

    /**
     * Creates a new EWMA which is equivalent to one hour load average and which expects to be
     * ticked every 5 seconds.
     *
     * @return a one-hour EWMA
     */
    public static EWMA sixtyMinuteEWMA() {
      return new EWMA(M60_ALPHA, TICK_INTERVAL_SEC, TimeUnit.SECONDS);
    }

    /**
     * Create a new EWMA with a specific smoothing constant.
     *
     * @param alpha the smoothing constant
     * @param interval the expected tick interval
     * @param intervalUnit the time unit of the tick interval
     */
    public EWMA(double alpha, long interval, TimeUnit intervalUnit) {
      this.interval = intervalUnit.toNanos(interval);
      this.alpha = alpha;
    }

    /**
     * Update the moving average with a new value.
     *
     * @param n the new value
     */
    public void update(long n) {
      uncounted.add(n);
    }

    /** Mark the passage of time and decay the current rate accordingly. */
    public void tick() {
      final long count = uncounted.sumThenReset();
      final double instantRate = count / interval;
      if (initialized) {
        final double oldRate = this.rate;
        rate = oldRate + (alpha * (instantRate - oldRate));
      } else {
        rate = instantRate;
        initialized = true;
      }
    }

    /**
     * rate is count per nanoseconds, we aim to return a one-minute rate, so we must multiply by
     * nanos in 1 minute
     *
     * @return the rate per minute
     */
    public long getRate() {
      return (long) (rate * (double) MINUTES.toNanos(1));
    }
  }
}
