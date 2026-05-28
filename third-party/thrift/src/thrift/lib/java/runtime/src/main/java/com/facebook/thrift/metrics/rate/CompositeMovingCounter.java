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
import java.util.Arrays;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicLong;
import java.util.concurrent.atomic.AtomicLongArray;
import java.util.concurrent.atomic.LongAdder;

/**
 * Composite sliding-window moving counter that exposes 1-minute, 10-minute, and 1-hour rates from a
 * single hot path. Writers hit one {@link LongAdder} reservoir; one packed {@link AtomicLong} state
 * machine elects rotations; each rotation fans the closed base-tick delta into three internal rings
 * at progressively coarser resolutions.
 *
 * <h2>Layout</h2>
 *
 * Base tick is fixed at 1 second. Three rings of 60 buckets each:
 *
 * <ul>
 *   <li>{@code oneMinuteRing}: 60 × 1 s buckets — covers the last ~60 s.
 *   <li>{@code tenMinuteRing}: 60 × 10 s buckets — covers the last ~600 s.
 *   <li>{@code oneHourRing}: 60 × 60 s buckets — covers the last ~3600 s.
 * </ul>
 *
 * <p>Coarser rings are <i>derived</i> from the base rotation, not driven by their own state
 * machines. There is exactly one {@code state} CAS per rotation, regardless of how many rings are
 * being maintained.
 *
 * <h2>State machine</h2>
 *
 * State is packed into one {@code AtomicLong} as {@code (tick << 2) | mode}, with the mode being
 * one of {@link #STABLE}, {@link #ROTATING_FAST}, or {@link #ROTATING_SLOW}. Bucket slots within
 * each ring are stored in two parallel {@link AtomicLongArray}s: one for the absolute ring-tick
 * identity, one for the per-ring-tick delta. The {@code bucketTicks} stamp is required for
 * correctness, not merely defensive — rotations do not zero-fill intermediate slots, so
 * out-of-window historical data persists in each ring until naturally overwritten.
 *
 * <h2>Window-edge approximation</h2>
 *
 * Each ring's nominal window is {@code numBuckets * ticksPerBucket} base ticks, but the actual
 * coverage at the leading edge varies by up to {@code ticksPerBucket} depending on where in the
 * current ring-tick the read happens. For 10 m, that's ±10 s; for 1 hr, ±60 s. Acceptable for rate
 * metrics; document expectations with downstream consumers if they need finer precision than the
 * coarser ring's bucket size.
 *
 * <h2>Sample-time semantics</h2>
 *
 * A sample's tick attribution is determined by when {@code reservoir.add(n)} actually executes (the
 * LongAdder write becoming globally visible to a subsequent {@code sum()}), not by when {@link
 * #add(long)} was called. A writer that wins a rotation re-validates state before depositing, so
 * its sample lands in the truly-current tick — not in the just-published one.
 *
 * <h2>Boundary races</h2>
 *
 * Two distinct boundary behaviors, depending on the rotation mode in flight:
 *
 * <ul>
 *   <li><b>{@link #ROTATING_FAST} (gap == 1):</b> a writer observing this state with {@code nowTick
 *       <= t + 1} adds to the reservoir immediately and accepts a one-tick skew on the 1-minute
 *       ring. The coarser rings absorb the same delta atomically when the rotation publishes.
 *   <li><b>{@link #ROTATING_SLOW} (gap > 1):</b> writers spin (via {@link SpinWait#spin()}) until
 *       the rotation publishes the new state, then deposit. No one-tick skew for slow-path writers.
 *       "Spin" is a tight retry loop, not a blocking wait.
 * </ul>
 */
public final class CompositeMovingCounter implements Rate {

  static final int STABLE = 0;
  static final int ROTATING_FAST = 1;
  static final int ROTATING_SLOW = 2;

  static final long SENTINEL_TICK = Long.MIN_VALUE;

  // Fixed layout for this class. Base tick = 1 second; three rings as described in the class doc.
  private static final long BASE_TICK_NANOS = TimeUnit.SECONDS.toNanos(1L);
  private static final int RING_BUCKETS = 60;
  private static final long ONE_MINUTE_TICKS_PER_BUCKET = 1L;
  private static final long TEN_MINUTE_TICKS_PER_BUCKET = 10L;
  private static final long ONE_HOUR_TICKS_PER_BUCKET = 60L;

  private final Clock clock;
  private final long startNanos;

  private final LongAdder reservoir = new LongAdder();
  private final AtomicLong state;
  private final AtomicLong snapshotSum = new AtomicLong(0L);

  private final Ring oneMinuteRing;
  private final Ring tenMinuteRing;
  private final Ring oneHourRing;

  public CompositeMovingCounter() {
    this(new NanoClock());
  }

  public CompositeMovingCounter(Clock clock) {
    if (clock == null) {
      throw new NullPointerException("clock");
    }
    this.clock = clock;
    this.startNanos = clock.tickNanos();
    this.state = new AtomicLong(encode(0L, STABLE));
    this.oneMinuteRing = new Ring(RING_BUCKETS, ONE_MINUTE_TICKS_PER_BUCKET);
    this.tenMinuteRing = new Ring(RING_BUCKETS, TEN_MINUTE_TICKS_PER_BUCKET);
    this.oneHourRing = new Ring(RING_BUCKETS, ONE_HOUR_TICKS_PER_BUCKET);
  }

  @Override
  public void add(long n) {
    if (n < 0L) {
      throw new IllegalArgumentException("Negative increments not supported, got " + n);
    }

    while (true) {
      long s = state.get();
      long t = stateTick(s);
      int m = stateMode(s);
      long nowTick = currentTick();

      if (m == ROTATING_SLOW) {
        SpinWait.spin();
        continue;
      }
      if (m == ROTATING_FAST) {
        if (nowTick <= t + 1L) {
          // In-window add: ±1-tick skew is acceptable for the fast path on the 1-minute ring.
          // Coarser rings (10 m, 1 hr) get the same delta on the next rotation.
          reservoir.add(n);
          return;
        }
        // Our clock has raced ahead while another writer holds ROTATING_FAST(t). Spin until they
        // publish STABLE(t+1); on the next loop iteration our (now larger) gap routes us through
        // the slow path or the hot path as appropriate.
        SpinWait.spin();
        continue;
      }

      long gap = nowTick - t;
      if (gap <= 0L) {
        // Hot path: same tick, single LongAdder.add().
        reservoir.add(n);
        return;
      }
      // Cross-tick: CAS into the matching rotation mode and run it if we win, then fall through to
      // re-validate state + currentTick on the next loop iteration. Re-validation is required: a
      // winner preempted between publishing STABLE(...) and reservoir.add(n) would otherwise
      // attribute its sample to a now-stale active tick.
      if (gap == 1L) {
        if (state.compareAndSet(s, encode(t, ROTATING_FAST))) {
          rotate(t, t + 1L);
        }
      } else if (state.compareAndSet(s, encode(t, ROTATING_SLOW))) {
        rotate(t, nowTick);
      }
    }
  }

  @Override
  public long oneMinuteRate() {
    return windowSum(oneMinuteRing);
  }

  @Override
  public long tenMinuteRate() {
    return windowSum(tenMinuteRing);
  }

  @Override
  public long oneHourRate() {
    return windowSum(oneHourRing);
  }

  /**
   * Closes {@code fromTick} into all three rings and publishes {@code toTick} as the new active
   * tick. The single rotation fans out: each ring absorbs the same per-base-tick delta. For the 1 m
   * ring (ticksPerBucket=1) this is always a fresh slot write; for the 10 m and 1 hr rings,
   * adjacent base ticks within the same coarse window accumulate into the same slot via {@link
   * Ring#add}'s addAndGet path.
   *
   * <p>Write order discipline is critical: rings get the data first, then snapshotSum advances,
   * then state publishes STABLE last. Readers waiting for STABLE are guaranteed to observe both the
   * new ring data and the new snapshotSum.
   */
  private void rotate(long fromTick, long toTick) {
    long sum = reservoir.sum();
    long delta = sum - snapshotSum.get();

    oneMinuteRing.add(fromTick, delta);
    tenMinuteRing.add(fromTick, delta);
    oneHourRing.add(fromTick, delta);

    snapshotSum.set(sum);
    state.set(encode(toTick, STABLE)); // publish state LAST
  }

  /**
   * Sum over the requested ring (closed buckets covered by the ring's window plus the in-flight
   * base tick from the reservoir).
   *
   * <p><b>No double-counting between {@code ring.sum} and {@code inFlight}:</b> for the 1 m ring
   * the current ring-tick's bucket always misses the bucketTicks check (the in-flight base tick has
   * not been rotated into a slot yet) and {@code inFlight} covers it. For the 10 m and 1 hr rings,
   * the current ring-tick's bucket holds <i>closed</i> base ticks at the start of the current
   * coarse window (those that already rotated into it); {@code inFlight} covers the current
   * 1-second base tick. Disjoint by construction, since per-base-tick rotations only write to a
   * ring's bucket on close.
   */
  private long windowSum(Ring ring) {
    while (true) {
      long s1 = state.get();
      long activeTick = stateTick(s1);
      int m1 = stateMode(s1);

      if (m1 != STABLE) {
        SpinWait.spin();
        continue;
      }

      long nowTick = currentTick();
      if (nowTick > activeTick) {
        long gap = nowTick - activeTick;
        int newMode = (gap == 1L) ? ROTATING_FAST : ROTATING_SLOW;
        if (state.compareAndSet(s1, encode(activeTick, newMode))) {
          rotate(activeTick, (gap == 1L) ? activeTick + 1L : nowTick);
        }
        continue;
      }

      long snap = snapshotSum.get();
      long inFlight = reservoir.sum() - snap;
      long total = ring.sum(activeTick) + inFlight;

      // Load-bearing: bracket the (snap, sum, ring.sum) reads with state.get() at start AND end.
      // If a rotation snuck in between, snap is pre-rotation while sum/ring may be post-rotation,
      // so the computed total can mix two epochs. State values are monotonic (tick only goes up),
      // so observing the same state twice means no rotation happened in between. Do NOT treat
      // this check as defensive code that can be removed.
      if (state.get() == s1) {
        return total;
      }
    }
  }

  private long currentTick() {
    long delta = clock.tickNanos() - startNanos;
    if (delta < 0L) {
      return 0L;
    }
    return delta / BASE_TICK_NANOS;
  }

  static long stateTick(long s) {
    return s >>> 2;
  }

  static int stateMode(long s) {
    return (int) (s & 0x3L);
  }

  static long encode(long tick, int mode) {
    return (tick << 2) | (long) (mode & 0x3);
  }

  // Visible for tests.
  Ring oneMinuteRingRef() {
    return oneMinuteRing;
  }

  Ring tenMinuteRingRef() {
    return tenMinuteRing;
  }

  Ring oneHourRingRef() {
    return oneHourRing;
  }

  long stateValue() {
    return state.get();
  }

  /**
   * Storage-only ring used by {@link CompositeMovingCounter}. Each slot stores the absolute
   * ring-tick stamp and the accumulated delta for that ring-tick. Rings have no state machine of
   * their own; the composite's single state CAS coordinates everything.
   */
  static final class Ring {
    private final int numBuckets;
    private final long ticksPerBucket;
    private final AtomicLongArray bucketTicks;
    private final AtomicLongArray bucketDeltas;

    Ring(int numBuckets, long ticksPerBucket) {
      this.numBuckets = numBuckets;
      this.ticksPerBucket = ticksPerBucket;

      long[] initialTicks = new long[numBuckets];
      Arrays.fill(initialTicks, SENTINEL_TICK);
      this.bucketTicks = new AtomicLongArray(initialTicks);
      this.bucketDeltas = new AtomicLongArray(numBuckets);
    }

    /**
     * Absorb {@code delta} for the ring-tick that {@code baseTick} maps into. If the slot already
     * holds the same ring-tick (we're still inside the coarse window), accumulate; otherwise the
     * slot held a stale ring-tick (or SENTINEL) and we overwrite both delta and tick stamp.
     *
     * <p><b>Safe to read-then-write</b> because {@link CompositeMovingCounter#rotate} is the sole
     * caller and runs under state-CAS election. Rings are never written concurrently within a
     * single ring; the apparent TOCTOU between {@code bucketTicks.get} and the subsequent write is
     * illusory in this protocol.
     *
     * <p>For the overwrite branch, write order is delta first, then tick (the tick stamp publishes
     * the slot). Concurrent readers observing the new tick are guaranteed to observe the new delta
     * via {@link AtomicLongArray}'s volatile semantics.
     */
    void add(long baseTick, long delta) {
      long ringTick = baseTick / ticksPerBucket;
      int slot = slotFor(ringTick);

      if (bucketTicks.get(slot) == ringTick) {
        bucketDeltas.addAndGet(slot, delta);
        return;
      }
      bucketDeltas.set(slot, delta); // delta first
      bucketTicks.set(slot, ringTick); // then tick (publishes the slot)
    }

    /**
     * Sum the most recent {@code numBuckets} ring-ticks ending at {@code activeBaseTick}'s
     * ring-tick (inclusive). Slots whose tick stamp doesn't match the expected ring-tick are
     * skipped — that's how stale (out-of-window or pre-wrap) data is excluded.
     */
    long sum(long activeBaseTick) {
      long currentRingTick = activeBaseTick / ticksPerBucket;
      long total = 0L;

      for (int k = 0; k < numBuckets; k++) {
        long expected = currentRingTick - k;
        if (expected < 0L) {
          break;
        }
        int slot = slotFor(expected);
        long bt = bucketTicks.get(slot); // read tick FIRST (acquire)
        if (bt == expected) {
          total += bucketDeltas.get(slot); // then delta — guaranteed visible
        }
      }
      return total;
    }

    private int slotFor(long ringTick) {
      return (int) (ringTick % (long) numBuckets);
    }

    // Visible for tests.
    long bucketTickAt(int slot) {
      return bucketTicks.get(slot);
    }

    long bucketDeltaAt(int slot) {
      return bucketDeltas.get(slot);
    }

    int numBuckets() {
      return numBuckets;
    }

    long ticksPerBucket() {
      return ticksPerBucket;
    }
  }
}
