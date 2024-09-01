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

import com.facebook.thrift.metrics.common.Clock;
import com.facebook.thrift.metrics.common.NanoClock;
import java.time.Duration;
import java.time.Instant;
import java.util.ArrayList;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicLong;
import java.util.concurrent.atomic.LongAdder;

public class SlidingTimeWindowMovingAverages implements Rate {
  private static final long TIME_WINDOW_DURATION_MINUTES = 60L;
  private static final long TICK_INTERVAL = TimeUnit.SECONDS.toNanos(1L);
  private static final Duration TIME_WINDOW_DURATION =
      Duration.ofMinutes(TIME_WINDOW_DURATION_MINUTES);
  private static final int NUMBER_OF_BUCKETS =
      (int) (TIME_WINDOW_DURATION.toNanos() / TICK_INTERVAL);

  private final AtomicLong lastTick;
  private final Clock clock;
  private ArrayList<LongAdder> buckets;
  private int oldestBucketIndex;
  private int currentBucketIndex;
  private final Instant bucketBaseTime;
  Instant oldestBucketTime;

  public SlidingTimeWindowMovingAverages() {
    this(new NanoClock());
  }

  public SlidingTimeWindowMovingAverages(Clock clock) {
    this.clock = clock;
    long startTime = clock.tickNanos();
    this.lastTick = new AtomicLong(startTime);
    this.buckets = new ArrayList<>(NUMBER_OF_BUCKETS);

    for (int i = 0; i < NUMBER_OF_BUCKETS; ++i) {
      this.buckets.add(new LongAdder());
    }

    this.bucketBaseTime = Instant.ofEpochSecond(0L, startTime);
    this.oldestBucketTime = this.bucketBaseTime;
    this.oldestBucketIndex = 0;
    this.currentBucketIndex = 0;
  }

  public void update(long n) {
    tickIfNecessary();
    (buckets.get(this.currentBucketIndex)).add(n);
  }

  public void tickIfNecessary() {
    long oldTick = this.lastTick.get();
    long newTick = this.clock.tickNanos();
    long age = newTick - oldTick;
    if (age >= TICK_INTERVAL) {
      long newLastTick = newTick - age % TICK_INTERVAL;
      if (this.lastTick.compareAndSet(oldTick, newLastTick)) {
        Instant currentInstant = Instant.ofEpochSecond(0L, newLastTick);
        this.currentBucketIndex = this.normalizeIndex(this.calculateIndexOfTick(currentInstant));
        this.cleanOldBuckets(currentInstant);
      }
    }
  }

  @Override
  public long oneMinuteRate() {
    tickIfNecessary();
    return this.getMinuteRate(1);
  }

  @Override
  public long tenMinuteRate() {
    tickIfNecessary();
    return this.getMinuteRate(10);
  }

  @Override
  public long oneHourRate() {
    tickIfNecessary();
    return this.getMinuteRate(60);
  }

  private long getMinuteRate(int minutes) {
    Instant now = Instant.ofEpochSecond(0L, this.lastTick.get());
    return this.sumBuckets(now, (int) (TimeUnit.MINUTES.toNanos(minutes) / TICK_INTERVAL));
  }

  int calculateIndexOfTick(Instant tickTime) {
    return (int) (Duration.between(this.bucketBaseTime, tickTime).toNanos() / TICK_INTERVAL);
  }

  int normalizeIndex(int index) {
    int mod = index % NUMBER_OF_BUCKETS;
    return mod >= 0 ? mod : mod + NUMBER_OF_BUCKETS;
  }

  private void cleanOldBuckets(Instant currentTick) {
    Instant oldestStillNeededTime =
        currentTick.minus(TIME_WINDOW_DURATION).plusNanos(TICK_INTERVAL);
    Instant youngestNotInWindow = this.oldestBucketTime.plus(TIME_WINDOW_DURATION);
    int newOldestIndex;
    if (oldestStillNeededTime.isAfter(youngestNotInWindow)) {
      newOldestIndex = this.oldestBucketIndex;
      this.oldestBucketTime = currentTick;
    } else {
      if (!oldestStillNeededTime.isAfter(this.oldestBucketTime)) {
        return;
      }

      newOldestIndex = this.normalizeIndex(this.calculateIndexOfTick(oldestStillNeededTime));
      this.oldestBucketTime = oldestStillNeededTime;
    }

    this.cleanBucketRange(this.oldestBucketIndex, newOldestIndex);
    this.oldestBucketIndex = newOldestIndex;
  }

  private void cleanBucketRange(int fromIndex, int toIndex) {
    int i;
    if (fromIndex < toIndex) {
      for (i = fromIndex; i < toIndex; ++i) {
        buckets.get(i).reset();
      }
    } else {
      for (i = fromIndex; i < NUMBER_OF_BUCKETS; ++i) {
        buckets.get(i).reset();
      }

      for (i = 0; i < toIndex; ++i) {
        buckets.get(i).reset();
      }
    }
  }

  private long sumBuckets(Instant toTime, int numberOfBuckets) {
    // increment toIndex to include the current bucket into the sum
    int toIndex = normalizeIndex(calculateIndexOfTick(toTime) + 1);
    int fromIndex = normalizeIndex(toIndex - numberOfBuckets);
    LongAdder adder = new LongAdder();

    if (fromIndex < toIndex) {
      buckets.stream()
          .skip(fromIndex)
          .limit(toIndex - fromIndex)
          .mapToLong(LongAdder::longValue)
          .forEach(adder::add);
    } else {
      buckets.stream().limit(toIndex).mapToLong(LongAdder::longValue).forEach(adder::add);
      buckets.stream().skip(fromIndex).mapToLong(LongAdder::longValue).forEach(adder::add);
    }

    return adder.longValue();
  }
}
