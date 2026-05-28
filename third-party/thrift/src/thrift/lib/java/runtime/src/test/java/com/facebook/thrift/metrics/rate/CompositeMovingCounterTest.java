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

import static org.assertj.core.api.Assertions.assertThat;
import static org.assertj.core.api.Assertions.assertThatThrownBy;

import com.facebook.thrift.metrics.common.Clock;
import com.facebook.thrift.metrics.distribution.TestClock;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.concurrent.atomic.AtomicLong;
import org.junit.jupiter.api.Test;

public class CompositeMovingCounterTest {

  // Hardcoded layout in CompositeMovingCounter: 1s base tick, three 60-bucket rings at
  // 1s / 10s / 60s per bucket.
  private static final long TICK_NS = TimeUnit.SECONDS.toNanos(1L);
  private static final int NB = 60;

  private CompositeMovingCounter newCounter(Clock clock) {
    return new CompositeMovingCounter(clock);
  }

  // ---------------------------------------------------------------------------
  // 1. gap == 1 (fast path) — closes one base tick into the source slot of the 1m ring
  //    (and the same slot of the coarser rings, since ringTick=0 for all three at this point).
  // ---------------------------------------------------------------------------
  @Test
  public void fastPath_singleTickGap() {
    TestClock clock = new TestClock();
    CompositeMovingCounter c = newCounter(clock);

    c.add(7L);
    clock.incrementNs(TICK_NS);
    c.add(11L);

    // 1m ring: bucket[0] holds the closed delta for ring-tick 0 (= base tick 0).
    assertThat(c.oneMinuteRingRef().bucketTickAt(0)).isEqualTo(0L);
    assertThat(c.oneMinuteRingRef().bucketDeltaAt(0)).isEqualTo(7L);
    // Coarser rings also get the delta — same slot 0, same ring-tick 0 (since base tick 0
    // maps to ring-tick 0 in every ring).
    assertThat(c.tenMinuteRingRef().bucketTickAt(0)).isEqualTo(0L);
    assertThat(c.tenMinuteRingRef().bucketDeltaAt(0)).isEqualTo(7L);
    assertThat(c.oneHourRingRef().bucketTickAt(0)).isEqualTo(0L);
    assertThat(c.oneHourRingRef().bucketDeltaAt(0)).isEqualTo(7L);
    // All three rates see in-flight (11) + closed (7) for tick 0..1.
    assertThat(c.oneMinuteRate()).isEqualTo(7L + 11L);
    assertThat(c.tenMinuteRate()).isEqualTo(7L + 11L);
    assertThat(c.oneHourRate()).isEqualTo(7L + 11L);
  }

  // ---------------------------------------------------------------------------
  // 2. gap > 1 (slow path) — closed delta lands in source-tick slot of every ring.
  //    Intermediate ticks contribute 0 via the bucketTicks check (no slot bears their tick stamp).
  // ---------------------------------------------------------------------------
  @Test
  public void slowPath_multiTickGap_attributesDeltaToSourceBucket() {
    TestClock clock = new TestClock();
    CompositeMovingCounter c = newCounter(clock);

    c.add(13L);
    clock.incrementNs(5L * TICK_NS); // jump to base tick 5
    c.add(17L);

    // Source bucket (base tick 0) in each ring holds the closed delta. Same slot 0, same
    // ring-tick 0 in all three.
    assertThat(c.oneMinuteRingRef().bucketTickAt(0)).isEqualTo(0L);
    assertThat(c.oneMinuteRingRef().bucketDeltaAt(0)).isEqualTo(13L);
    assertThat(c.tenMinuteRingRef().bucketTickAt(0)).isEqualTo(0L);
    assertThat(c.tenMinuteRingRef().bucketDeltaAt(0)).isEqualTo(13L);
    assertThat(c.oneHourRingRef().bucketTickAt(0)).isEqualTo(0L);
    assertThat(c.oneHourRingRef().bucketDeltaAt(0)).isEqualTo(13L);

    // Behavioral guarantee: rates correctly include all closed + in-flight, with no zero-fill.
    assertThat(c.oneMinuteRate()).isEqualTo(13L + 17L);
    assertThat(c.tenMinuteRate()).isEqualTo(13L + 17L);
    assertThat(c.oneHourRate()).isEqualTo(13L + 17L);
  }

  // ---------------------------------------------------------------------------
  // 3. 1-minute ring: gap >= numBuckets (full lap) — old data rolls out.
  // ---------------------------------------------------------------------------
  @Test
  public void oneMinuteRate_fullLap_oldDataRollsOut() {
    TestClock clock = new TestClock();
    CompositeMovingCounter c = newCounter(clock);

    c.add(99L);
    // Jump past the entire 1m window; the original 99 must roll off oneMinuteRate.
    clock.incrementNs((long) (NB + 5) * TICK_NS);
    c.add(3L);

    // 1m: only the new in-flight contributes.
    assertThat(c.oneMinuteRate()).isEqualTo(3L);
    // 10m and 1hr still hold the original 99 — base tick 0 is in their windows.
    assertThat(c.tenMinuteRate()).isEqualTo(99L + 3L);
    assertThat(c.oneHourRate()).isEqualTo(99L + 3L);
  }

  // ---------------------------------------------------------------------------
  // 4. Reader-driven rotation (no further writes after the gap).
  // ---------------------------------------------------------------------------
  @Test
  public void readerDrivenRotation_withoutAdditionalWrites() {
    TestClock clock = new TestClock();
    CompositeMovingCounter c = newCounter(clock);

    c.add(42L);
    clock.incrementNs(3L * TICK_NS); // jump to base tick 3 with no further writes

    // First read drives the rotation.
    long sum = c.oneMinuteRate();
    assertThat(sum).isEqualTo(42L);

    // After the rotation: state is STABLE(3) and bucket 0 of every ring holds the closed delta.
    assertThat(CompositeMovingCounter.stateMode(c.stateValue()))
        .isEqualTo(CompositeMovingCounter.STABLE);
    assertThat(CompositeMovingCounter.stateTick(c.stateValue())).isEqualTo(3L);
    assertThat(c.oneMinuteRingRef().bucketTickAt(0)).isEqualTo(0L);
    assertThat(c.oneMinuteRingRef().bucketDeltaAt(0)).isEqualTo(42L);
    assertThat(c.tenMinuteRingRef().bucketTickAt(0)).isEqualTo(0L);
    assertThat(c.tenMinuteRingRef().bucketDeltaAt(0)).isEqualTo(42L);
    assertThat(c.oneHourRingRef().bucketTickAt(0)).isEqualTo(0L);
    assertThat(c.oneHourRingRef().bucketDeltaAt(0)).isEqualTo(42L);
  }

  // ---------------------------------------------------------------------------
  // 5. Stale ring slots (a): SENTINEL initialization protects against unread slots
  //    across all three rings.
  // ---------------------------------------------------------------------------
  @Test
  public void freshCounter_allRingsHaveSentinelSlots() {
    TestClock clock = new TestClock();
    CompositeMovingCounter c = newCounter(clock);

    for (int i = 0; i < NB; i++) {
      assertThat(c.oneMinuteRingRef().bucketTickAt(i))
          .as("1m slot %d", i)
          .isEqualTo(CompositeMovingCounter.SENTINEL_TICK);
      assertThat(c.tenMinuteRingRef().bucketTickAt(i))
          .as("10m slot %d", i)
          .isEqualTo(CompositeMovingCounter.SENTINEL_TICK);
      assertThat(c.oneHourRingRef().bucketTickAt(i))
          .as("1hr slot %d", i)
          .isEqualTo(CompositeMovingCounter.SENTINEL_TICK);
    }
    // All rates return 0 — nothing in the reservoir, all slot reads see SENTINEL != expected.
    assertThat(c.oneMinuteRate()).isEqualTo(0L);
    assertThat(c.tenMinuteRate()).isEqualTo(0L);
    assertThat(c.oneHourRate()).isEqualTo(0L);
  }

  // ---------------------------------------------------------------------------
  // 6. Stale ring slots (b) — load-bearing bucketTicks check on the 1-minute ring.
  // ---------------------------------------------------------------------------
  @Test
  public void bucketTicksCheck_excludesStaleOutOfWindowSlots() {
    TestClock clock = new TestClock();
    CompositeMovingCounter c = newCounter(clock);

    c.add(100L);
    clock.incrementNs(TICK_NS);
    c.add(200L);
    assertThat(c.oneMinuteRingRef().bucketTickAt(0)).isEqualTo(0L);
    assertThat(c.oneMinuteRingRef().bucketDeltaAt(0)).isEqualTo(100L);

    // Jump a full lap so tick 0 must roll out of the 1m window. Slow rotation closes tick 1
    // into slot 1 of the 1m ring; slot 0 is NOT touched (no zero-fill).
    clock.incrementNs((long) NB * TICK_NS);
    c.add(300L);

    // 1m slot 0 still holds its stale (tick=0, delta=100) — physically present in the ring.
    assertThat(c.oneMinuteRingRef().bucketTickAt(0)).isEqualTo(0L);
    assertThat(c.oneMinuteRingRef().bucketDeltaAt(0)).isEqualTo(100L);
    // 1m slot 1 holds tick 1's closed delta (also stale relative to the 1m window's expected
    // tick range at active=NB+1, but it's there).
    assertThat(c.oneMinuteRingRef().bucketTickAt(1)).isEqualTo(1L);
    assertThat(c.oneMinuteRingRef().bucketDeltaAt(1)).isEqualTo(200L);

    // Behavioral guarantee: stale slots do NOT pollute oneMinuteRate. Only the in-flight 300
    // contributes — the bucketTicks check rejects stale slot 0 when k=NB-1 (expected=NB), and
    // ticks 0/1 are out of the loop range entirely (k=NB and k=NB+1).
    assertThat(c.oneMinuteRate()).isEqualTo(300L);
  }

  // ---------------------------------------------------------------------------
  // 7. Negative input rejected.
  // ---------------------------------------------------------------------------
  @Test
  public void add_rejectsNegativeInput() {
    TestClock clock = new TestClock();
    CompositeMovingCounter c = newCounter(clock);
    assertThatThrownBy(() -> c.add(-1L)).isInstanceOf(IllegalArgumentException.class);
  }

  // ---------------------------------------------------------------------------
  // 8. Constructor validation — only null clock to reject (layout is hardcoded).
  // ---------------------------------------------------------------------------
  @Test
  public void constructor_rejectsNullClock() {
    assertThatThrownBy(() -> new CompositeMovingCounter(null))
        .isInstanceOf(NullPointerException.class);
  }

  // ---------------------------------------------------------------------------
  // 9. Concurrent writers preserve total — uses thread-safe AtomicTestClock.
  // ---------------------------------------------------------------------------
  @Test
  public void concurrentWriters_preserveTotal() throws Exception {
    AtomicTestClock clock = new AtomicTestClock();
    CompositeMovingCounter c = newCounter(clock);

    int threads = 8;
    int perThread = 100_000;
    int maxAdvances = NB - 2; // never roll the 1m window
    ExecutorService pool = Executors.newFixedThreadPool(threads);
    CountDownLatch ready = new CountDownLatch(threads);
    CountDownLatch writersDone = new CountDownLatch(threads);
    CountDownLatch start = new CountDownLatch(1);
    AtomicBoolean stopAdvancer = new AtomicBoolean(false);

    Thread advancer =
        new Thread(
            () -> {
              for (int i = 0; i < maxAdvances && !stopAdvancer.get(); i++) {
                try {
                  Thread.sleep(1L);
                } catch (InterruptedException e) {
                  Thread.currentThread().interrupt();
                  return;
                }
                clock.incrementNs(TICK_NS);
              }
            });
    advancer.setDaemon(true);

    for (int t = 0; t < threads; t++) {
      pool.submit(
          () -> {
            ready.countDown();
            try {
              start.await();
              for (int i = 0; i < perThread; i++) {
                c.add(1L);
              }
            } catch (InterruptedException e) {
              Thread.currentThread().interrupt();
            } finally {
              writersDone.countDown();
            }
          });
    }

    ready.await();
    advancer.start();
    start.countDown();
    writersDone.await();
    stopAdvancer.set(true);
    advancer.join();
    pool.shutdown();
    assertThat(pool.awaitTermination(30L, TimeUnit.SECONDS)).isTrue();

    long expected = (long) threads * (long) perThread;
    // No data has rolled out of any window, so all three rates return the full total.
    assertThat(c.oneMinuteRate()).isEqualTo(expected);
    assertThat(c.tenMinuteRate()).isEqualTo(expected);
    assertThat(c.oneHourRate()).isEqualTo(expected);
  }

  // ---------------------------------------------------------------------------
  // 10. Concurrent reader vs writers (smoke) — no spurious throws, final sums match.
  // ---------------------------------------------------------------------------
  @Test
  public void concurrentReaderAndWriters_noSpuriousErrors() throws Exception {
    AtomicTestClock clock = new AtomicTestClock();
    CompositeMovingCounter c = newCounter(clock);

    int writers = 4;
    int perWriter = 50_000;
    int maxAdvances = NB - 2;
    ExecutorService pool = Executors.newFixedThreadPool(writers + 2);
    CountDownLatch ready = new CountDownLatch(writers);
    CountDownLatch writersDone = new CountDownLatch(writers);
    CountDownLatch start = new CountDownLatch(1);
    AtomicBoolean stopReader = new AtomicBoolean(false);
    AtomicBoolean stopAdvancer = new AtomicBoolean(false);
    AtomicLong readerErrors = new AtomicLong(0L);

    // Reader spins reading all three rates. Asserts no negative results, no exceptions.
    pool.submit(
        () -> {
          while (!stopReader.get()) {
            try {
              long m = c.oneMinuteRate();
              long ten = c.tenMinuteRate();
              long hr = c.oneHourRate();
              if (m < 0L || ten < 0L || hr < 0L) {
                readerErrors.incrementAndGet();
              }
            } catch (RuntimeException e) {
              readerErrors.incrementAndGet();
            }
          }
        });

    pool.submit(
        () -> {
          for (int i = 0; i < maxAdvances && !stopAdvancer.get(); i++) {
            try {
              Thread.sleep(1L);
            } catch (InterruptedException e) {
              Thread.currentThread().interrupt();
              return;
            }
            clock.incrementNs(TICK_NS);
          }
        });

    for (int t = 0; t < writers; t++) {
      pool.submit(
          () -> {
            ready.countDown();
            try {
              start.await();
              for (int i = 0; i < perWriter; i++) {
                c.add(1L);
              }
            } catch (InterruptedException e) {
              Thread.currentThread().interrupt();
            } finally {
              writersDone.countDown();
            }
          });
    }

    ready.await();
    start.countDown();
    writersDone.await();
    stopAdvancer.set(true);
    stopReader.set(true);
    pool.shutdown();
    assertThat(pool.awaitTermination(30L, TimeUnit.SECONDS)).isTrue();

    assertThat(readerErrors.get()).as("reader errors").isEqualTo(0L);
    long expected = (long) writers * (long) perWriter;
    assertThat(c.oneMinuteRate()).isEqualTo(expected);
    assertThat(c.tenMinuteRate()).isEqualTo(expected);
    assertThat(c.oneHourRate()).isEqualTo(expected);
  }

  // ---------------------------------------------------------------------------
  // 11. Fast-path rotation winner re-validates after a late clock advance — verifies that
  //     the rotation fans out to ALL rings and the sample lands in the truly-current tick.
  // ---------------------------------------------------------------------------
  @Test
  public void rotationWinner_reValidatesAfterLateClockAdvance() {
    StagedClock clock = new StagedClock(0L, TICK_NS, 5L * TICK_NS);
    CompositeMovingCounter c = newCounter(clock);

    c.add(7L);

    // State has been advanced all the way to STABLE(5) by the winner re-loop.
    assertThat(CompositeMovingCounter.stateMode(c.stateValue()))
        .isEqualTo(CompositeMovingCounter.STABLE);
    assertThat(CompositeMovingCounter.stateTick(c.stateValue())).isEqualTo(5L);

    // Bucket 1 of the 1m ring was written by the slow rotation's source step with delta 0
    // (reservoir was empty when the slow rotation snapshotted). The 7 is NOT attributed there.
    assertThat(c.oneMinuteRingRef().bucketTickAt(1)).isEqualTo(1L);
    assertThat(c.oneMinuteRingRef().bucketDeltaAt(1)).isEqualTo(0L);
    // 10m / 1hr rings: the second rotation (slow, from tick 1 to tick 5) called
    // ring.add(1, 0). For 10m, ringTick = 0 → slot 0. For 1hr, ringTick = 0 → slot 0. Both
    // already had bucketTicks=0 from the first fast rotation, so the addAndGet path fires:
    // delta is added to existing bucket value (which was 0 from the first rotation, since
    // reservoir.sum() == snap at that point). Both should still be 0.
    assertThat(c.tenMinuteRingRef().bucketTickAt(0)).isEqualTo(0L);
    assertThat(c.tenMinuteRingRef().bucketDeltaAt(0)).isEqualTo(0L);
    assertThat(c.oneHourRingRef().bucketTickAt(0)).isEqualTo(0L);
    assertThat(c.oneHourRingRef().bucketDeltaAt(0)).isEqualTo(0L);

    // The 7 sits in the in-flight reservoir for tick 5; all rates pick it up.
    assertThat(c.oneMinuteRate()).isEqualTo(7L);
    assertThat(c.tenMinuteRate()).isEqualTo(7L);
    assertThat(c.oneHourRate()).isEqualTo(7L);
  }

  // ---------------------------------------------------------------------------
  // 12. Slow-path rotation winner re-validates — variant of #11 where the writer's first
  //     iteration goes directly into the slow path (gap > 1) rather than escalating from fast.
  // ---------------------------------------------------------------------------
  @Test
  public void slowRotationWinner_reValidatesAfterLateClockAdvance() {
    StagedClock clock = new StagedClock(0L, 3L * TICK_NS, 8L * TICK_NS);
    CompositeMovingCounter c = newCounter(clock);

    c.add(11L);

    assertThat(CompositeMovingCounter.stateMode(c.stateValue()))
        .isEqualTo(CompositeMovingCounter.STABLE);
    assertThat(CompositeMovingCounter.stateTick(c.stateValue())).isEqualTo(8L);

    // First slow rotation closed tick 0 with delta 0 (reservoir empty); second closed tick 3
    // with delta 0 too. The 11 sits in the in-flight reservoir for tick 8.
    assertThat(c.oneMinuteRingRef().bucketTickAt(0)).isEqualTo(0L);
    assertThat(c.oneMinuteRingRef().bucketDeltaAt(0)).isEqualTo(0L);
    assertThat(c.oneMinuteRingRef().bucketTickAt(3)).isEqualTo(3L);
    assertThat(c.oneMinuteRingRef().bucketDeltaAt(3)).isEqualTo(0L);
    assertThat(c.oneMinuteRate()).isEqualTo(11L);
    assertThat(c.tenMinuteRate()).isEqualTo(11L);
    assertThat(c.oneHourRate()).isEqualTo(11L);
  }

  // ---------------------------------------------------------------------------
  // 13. Adversarial: every 1m slot filled with a non-zero stale delta and a valid-looking
  //     tick stamp; after a long jump, oneMinuteRate must NOT leak any of it.
  // ---------------------------------------------------------------------------
  @Test
  public void fullStaleBuffer_doesNotPolluteNewSampleForOneMinute() {
    TestClock clock = new TestClock();
    CompositeMovingCounter c = newCounter(clock);

    // Phase 1: fill EVERY 1m slot 0..NB-1 with a closed delta of 1.
    for (int i = 0; i <= NB; i++) {
      c.add(1L);
      if (i < NB) {
        clock.incrementNs(TICK_NS);
      }
    }
    for (int i = 0; i < NB; i++) {
      assertThat(c.oneMinuteRingRef().bucketTickAt(i)).as("1m slot %d", i).isEqualTo((long) i);
      assertThat(c.oneMinuteRingRef().bucketDeltaAt(i)).as("1m slot %d", i).isEqualTo(1L);
    }

    // Phase 2: jump to base tick 4*NB so all existing 1m stamps (0..NB-1, plus the
    // soon-to-be-written stamp NB from the slow rotation's source step) are out of the new
    // 1m window. Then add one new sample.
    long farFutureTick = 4L * (long) NB;
    long advanceFromCurrent = farFutureTick - (long) NB;
    clock.incrementNs(advanceFromCurrent * TICK_NS);
    c.add(7L);

    // The 1m reader's expected ticks lie outside any 1m slot's stamp, so all are skipped.
    // Only the in-flight 7 contributes.
    assertThat(c.oneMinuteRate()).isEqualTo(7L);
  }

  // ---------------------------------------------------------------------------
  // 14. Rate interface scope: all three rates supported and return non-throwing values.
  // ---------------------------------------------------------------------------
  @Test
  public void rateInterface_allRatesSupported() {
    TestClock clock = new TestClock();
    CompositeMovingCounter c = newCounter(clock);
    c.add(5L);
    // No rotation yet; the in-flight tick is 0 with delta 5 from the reservoir, picked up by
    // every ring's window sum (each via its own inFlight contribution since the snapshot is
    // shared and equal to 0 at this point).
    assertThat(c.oneMinuteRate()).isEqualTo(5L);
    assertThat(c.tenMinuteRate()).isEqualTo(5L);
    assertThat(c.oneHourRate()).isEqualTo(5L);
  }

  // ===========================================================================
  // Tests for multi-ring behavior
  // ===========================================================================

  // ---------------------------------------------------------------------------
  // 15. One rotation atomically updates all three rings with the same delta.
  // ---------------------------------------------------------------------------
  @Test
  public void rotation_fansOutToAllRingsAtomically() {
    TestClock clock = new TestClock();
    CompositeMovingCounter c = newCounter(clock);

    c.add(50L);
    clock.incrementNs(TICK_NS); // active=0 → active=1, fast rotation
    c.add(0L); // any add triggers the rotation

    // All three rings received the closed delta for ring-tick 0 in the same rotation.
    assertThat(c.oneMinuteRingRef().bucketTickAt(0)).isEqualTo(0L);
    assertThat(c.oneMinuteRingRef().bucketDeltaAt(0)).isEqualTo(50L);
    assertThat(c.tenMinuteRingRef().bucketTickAt(0)).isEqualTo(0L);
    assertThat(c.tenMinuteRingRef().bucketDeltaAt(0)).isEqualTo(50L);
    assertThat(c.oneHourRingRef().bucketTickAt(0)).isEqualTo(0L);
    assertThat(c.oneHourRingRef().bucketDeltaAt(0)).isEqualTo(50L);
  }

  // ---------------------------------------------------------------------------
  // 16. 10m ring accumulates 10 base-tick deltas into one slot, then advances at the boundary.
  // ---------------------------------------------------------------------------
  @Test
  public void tenMinuteRing_accumulatesAndAdvancesAtCoarseBoundary() {
    TestClock clock = new TestClock();
    CompositeMovingCounter c = newCounter(clock);

    // Add a distinct value at each base tick 0..10. Each cross-tick triggers a fast rotation
    // that fans into every ring. For 10m: ring-tick = base/10, so ticks 0..9 all map to
    // ring-tick 0 (slot 0, accumulate). Tick 10 maps to ring-tick 1 (slot 1, fresh write).
    long[] values = {1L, 2L, 3L, 4L, 5L, 6L, 7L, 8L, 9L, 10L, 100L};
    for (int i = 0; i < values.length; i++) {
      c.add(values[i]);
      clock.incrementNs(TICK_NS);
    }
    // Force one more rotation so base tick 10's delta is closed into a slot rather than
    // sitting in the reservoir.
    c.add(0L);

    // 10m slot 0 holds the SUM of deltas from ring-tick 0 = base ticks 0..9 = 1+2+...+10 = 55.
    assertThat(c.tenMinuteRingRef().bucketTickAt(0)).isEqualTo(0L);
    assertThat(c.tenMinuteRingRef().bucketDeltaAt(0)).isEqualTo(55L);

    // 10m slot 1 holds the fresh-write delta for ring-tick 1 = base tick 10 = 100.
    assertThat(c.tenMinuteRingRef().bucketTickAt(1)).isEqualTo(1L);
    assertThat(c.tenMinuteRingRef().bucketDeltaAt(1)).isEqualTo(100L);

    // 1m ring still has per-base-tick granularity: each slot holds its respective delta.
    for (int i = 0; i < 10; i++) {
      assertThat(c.oneMinuteRingRef().bucketTickAt(i)).as("1m slot %d tick", i).isEqualTo((long) i);
      assertThat(c.oneMinuteRingRef().bucketDeltaAt(i))
          .as("1m slot %d delta", i)
          .isEqualTo(values[i]);
    }
    assertThat(c.oneMinuteRingRef().bucketTickAt(10)).isEqualTo(10L);
    assertThat(c.oneMinuteRingRef().bucketDeltaAt(10)).isEqualTo(100L);
  }

  // ---------------------------------------------------------------------------
  // 17. 1hr ring accumulates 60 base-tick deltas into one slot, then advances at the boundary.
  // ---------------------------------------------------------------------------
  @Test
  public void oneHourRing_accumulatesAndAdvancesAtCoarseBoundary() {
    TestClock clock = new TestClock();
    CompositeMovingCounter c = newCounter(clock);

    // Add 1 at each base tick 0..60 (61 adds total). Tick 60 starts the next 1hr ring-tick.
    for (int i = 0; i < 61; i++) {
      c.add(1L);
      clock.incrementNs(TICK_NS);
    }
    // Force one more rotation so base tick 60's delta is closed into a slot.
    c.add(0L);

    // 1hr slot 0 holds the SUM of deltas from ring-tick 0 = base ticks 0..59 = 60 × 1 = 60.
    assertThat(c.oneHourRingRef().bucketTickAt(0)).isEqualTo(0L);
    assertThat(c.oneHourRingRef().bucketDeltaAt(0)).isEqualTo(60L);

    // 1hr slot 1 holds the fresh-write delta for ring-tick 1 = base tick 60 = 1.
    assertThat(c.oneHourRingRef().bucketTickAt(1)).isEqualTo(1L);
    assertThat(c.oneHourRingRef().bucketDeltaAt(1)).isEqualTo(1L);

    // 10m ring also accumulated correctly: 6 fully-closed slots of 10 base ticks each, plus
    // slot 6 holding ring-tick 6's first base tick (= base tick 60 = delta 1).
    for (int slot = 0; slot < 6; slot++) {
      assertThat(c.tenMinuteRingRef().bucketTickAt(slot))
          .as("10m slot %d tick", slot)
          .isEqualTo((long) slot);
      assertThat(c.tenMinuteRingRef().bucketDeltaAt(slot))
          .as("10m slot %d delta", slot)
          .isEqualTo(10L);
    }
    assertThat(c.tenMinuteRingRef().bucketTickAt(6)).isEqualTo(6L);
    assertThat(c.tenMinuteRingRef().bucketDeltaAt(6)).isEqualTo(1L);
  }

  // ---------------------------------------------------------------------------
  // 18. tenMinuteRate includes the current coarse bucket plus in-flight without double-counting.
  // ---------------------------------------------------------------------------
  @Test
  public void tenMinuteRate_includesCurrentCoarseBucketPlusInFlight() {
    TestClock clock = new TestClock();
    CompositeMovingCounter c = newCounter(clock);

    // Add 10 at each of base ticks 0..14 (15 adds). 10m ring-tick layout:
    //   ticks 0..9 → ring-tick 0 (slot 0), accumulated delta = 100.
    //   ticks 10..14 → ring-tick 1 (slot 1), accumulated delta = 50 after closes.
    // Active will be 14 (last add) or 15 (after one more clock advance). Let's land at active=15
    // with one in-flight base tick of value 7.
    for (int i = 0; i < 15; i++) {
      c.add(10L);
      clock.incrementNs(TICK_NS);
    }
    c.add(7L); // in-flight at active=15

    // Expected oneMinuteRate: window covers base ticks ~0..15 (within 60). All adds are in
    // window. Total = 15*10 + 7 = 157.
    assertThat(c.oneMinuteRate()).isEqualTo(157L);

    // Expected tenMinuteRate: 10m ring slot 0 = ticks 0..9 deltas = 100 (closed), slot 1 =
    // ticks 10..14 deltas = 50 (closed and accumulated), in-flight = 7.
    // Total = 100 + 50 + 7 = 157.
    assertThat(c.tenMinuteRate()).isEqualTo(157L);

    // Expected oneHourRate: 1hr ring slot 0 = ticks 0..14 deltas = 150 (closed and accumulated),
    // in-flight = 7. Total = 157.
    assertThat(c.oneHourRate()).isEqualTo(157L);
  }

  // ---------------------------------------------------------------------------
  // 19. tenMinuteRate full lap — old data rolls out of the 10m window (600s).
  // ---------------------------------------------------------------------------
  @Test
  public void tenMinuteRate_fullLap_oldDataRollsOut() {
    TestClock clock = new TestClock();
    CompositeMovingCounter c = newCounter(clock);

    c.add(99L);
    // Jump past the full 10m window (600 seconds + buffer); the original 99 must roll out
    // of oneMinuteRate AND tenMinuteRate. 1hr still has 99 in window.
    clock.incrementNs(610L * TICK_NS);
    c.add(3L);

    assertThat(c.oneMinuteRate()).isEqualTo(3L);
    assertThat(c.tenMinuteRate()).isEqualTo(3L);
    // 1hr window covers 3600s; 610s ago is well within it.
    assertThat(c.oneHourRate()).isEqualTo(99L + 3L);
  }

  // ---------------------------------------------------------------------------
  // 20. oneHourRate full lap — old data rolls out of all three windows after a 1+ hour jump.
  // ---------------------------------------------------------------------------
  @Test
  public void oneHourRate_fullLap_oldDataRollsOut() {
    TestClock clock = new TestClock();
    CompositeMovingCounter c = newCounter(clock);

    c.add(99L);
    // Jump past the full 1hr window (3600 seconds + buffer); the 99 must roll out everywhere.
    clock.incrementNs(3700L * TICK_NS);
    c.add(3L);

    assertThat(c.oneMinuteRate()).isEqualTo(3L);
    assertThat(c.tenMinuteRate()).isEqualTo(3L);
    assertThat(c.oneHourRate()).isEqualTo(3L);
  }

  // ---------------------------------------------------------------------------
  // 21. Exact 10m boundary: tick 0 INCLUDED at active=599 (last base tick where ring-tick 0
  //     is in the 10m window's reader loop), EXCLUDED at active=600 (window has rolled past).
  // ---------------------------------------------------------------------------
  @Test
  public void tenMinuteRate_exactBoundary_includesAt599_excludesAt600() {
    TestClock clock = new TestClock();
    CompositeMovingCounter c = newCounter(clock);

    c.add(99L);
    // Advance to active=599. currentRingTick (10m) = 59. Reader's loop expected = 59..0,
    // and ring-tick 0's slot holds (0, 99) from the reader-driven slow rotation. Included.
    clock.incrementNs(599L * TICK_NS);
    assertThat(c.tenMinuteRate()).isEqualTo(99L);

    // Advance one more base tick. active=600 → currentRingTick=60 → expected covers 60..1.
    // Slot 0 still holds (0, 99) (the fast rotation 599→600 wrote slot 59, not slot 0), but
    // bucketTicks[0]=0 != 60 — the bucketTicks check rejects it. Tick 0 is now out of window.
    clock.incrementNs(TICK_NS);
    assertThat(c.tenMinuteRate()).isEqualTo(0L);
  }

  // ---------------------------------------------------------------------------
  // 22. Exact 1hr boundary: tick 0 INCLUDED at active=3599, EXCLUDED at active=3600.
  // ---------------------------------------------------------------------------
  @Test
  public void oneHourRate_exactBoundary_includesAt3599_excludesAt3600() {
    TestClock clock = new TestClock();
    CompositeMovingCounter c = newCounter(clock);

    c.add(99L);
    // Advance to active=3599. currentRingTick (1hr) = 59. Reader's loop expected = 59..0.
    // 1hr slot 0 holds (0, 99) from the reader-driven slow rotation. Included.
    clock.incrementNs(3599L * TICK_NS);
    assertThat(c.oneHourRate()).isEqualTo(99L);

    // Advance one more base tick. active=3600 → currentRingTick=60 → expected covers 60..1.
    // bucketTicks[0]=0 != 60 — the bucketTicks check rejects the now-stale tick 0 data.
    clock.incrementNs(TICK_NS);
    assertThat(c.oneHourRate()).isEqualTo(0L);
  }

  // ---------------------------------------------------------------------------
  // 23. Adversarial: every 10m slot filled with non-zero stale data (delta=10 each, valid
  //     stamp), then jump well past the 10m window. tenMinuteRate must NOT leak any of it.
  // ---------------------------------------------------------------------------
  @Test
  public void fullStaleBuffer_doesNotPolluteNewSampleForTenMinute() {
    TestClock clock = new TestClock();
    CompositeMovingCounter c = newCounter(clock);

    // Phase 1: 601 adds across base ticks 0..600. Each ring-tick X (0..59) of the 10m ring
    // accumulates 10 base-tick rotations of delta=1 → bucket[X] = (X, 10).
    for (int i = 0; i <= 600; i++) {
      c.add(1L);
      if (i < 600) {
        clock.incrementNs(TICK_NS);
      }
    }
    // Sanity: every 10m slot is fully populated with a valid-looking stamp + non-zero delta.
    for (int i = 0; i < NB; i++) {
      assertThat(c.tenMinuteRingRef().bucketTickAt(i))
          .as("10m slot %d stamp", i)
          .isEqualTo((long) i);
      assertThat(c.tenMinuteRingRef().bucketDeltaAt(i)).as("10m slot %d delta", i).isEqualTo(10L);
    }

    // Phase 2: jump to active=2400 (4× the 10m window in base ticks). currentRingTick=240;
    // reader's loop covers expected ring-ticks 240..181, none of which match the stale stamps
    // 0..60 living in any slot. All SKIP via bucketTicks check.
    long farFutureTick = 2400L;
    long advanceFromCurrent = farFutureTick - 600L;
    clock.incrementNs(advanceFromCurrent * TICK_NS);
    c.add(7L);

    // Only the in-flight 7 contributes.
    assertThat(c.tenMinuteRate()).isEqualTo(7L);
  }

  // ---------------------------------------------------------------------------
  // 24. Adversarial: every 1hr slot filled with non-zero stale data (delta=60 each, valid
  //     stamp), then jump well past the 1hr window. oneHourRate must NOT leak any of it.
  // ---------------------------------------------------------------------------
  @Test
  public void fullStaleBuffer_doesNotPolluteNewSampleForOneHour() {
    TestClock clock = new TestClock();
    CompositeMovingCounter c = newCounter(clock);

    // Phase 1: 3601 adds across base ticks 0..3600. Each ring-tick X (0..59) of the 1hr ring
    // accumulates 60 base-tick rotations of delta=1 → bucket[X] = (X, 60).
    for (int i = 0; i <= 3600; i++) {
      c.add(1L);
      if (i < 3600) {
        clock.incrementNs(TICK_NS);
      }
    }
    // Sanity: every 1hr slot is fully populated with a valid-looking stamp + non-zero delta.
    for (int i = 0; i < NB; i++) {
      assertThat(c.oneHourRingRef().bucketTickAt(i)).as("1hr slot %d stamp", i).isEqualTo((long) i);
      assertThat(c.oneHourRingRef().bucketDeltaAt(i)).as("1hr slot %d delta", i).isEqualTo(60L);
    }

    // Phase 2: jump to active=14400 (4× the 1hr window in base ticks). currentRingTick=240;
    // expected ring-ticks 240..181, none matching stale stamps 0..60. All SKIP.
    long farFutureTick = 14400L;
    long advanceFromCurrent = farFutureTick - 3600L;
    clock.incrementNs(advanceFromCurrent * TICK_NS);
    c.add(7L);

    assertThat(c.oneHourRate()).isEqualTo(7L);
  }

  // ---------------------------------------------------------------------------
  // Helpers
  // ---------------------------------------------------------------------------

  /**
   * Clock that returns a fixed schedule of values; once exhausted, returns the last value. Used to
   * simulate "the clock jumped between two consecutive calls in the same method" without an actual
   * thread preemption.
   */
  private static final class StagedClock implements Clock {
    private final long[] schedule;
    private final java.util.concurrent.atomic.AtomicInteger calls =
        new java.util.concurrent.atomic.AtomicInteger(0);

    StagedClock(long... schedule) {
      this.schedule = schedule;
    }

    @Override
    public long tickNanos() {
      int i = calls.getAndIncrement();
      return schedule[Math.min(i, schedule.length - 1)];
    }

    @Override
    public long tickMilis() {
      return tickNanos() / Clock.NANOS_PER_MS;
    }
  }

  /**
   * Thread-safe Clock for the concurrent tests. {@link TestClock}'s {@code now} field is a plain
   * long, so concurrent mutation has no happens-before edge.
   */
  private static final class AtomicTestClock implements Clock {
    private final AtomicLong nanos = new AtomicLong(0L);

    @Override
    public long tickNanos() {
      return nanos.get();
    }

    @Override
    public long tickMilis() {
      return nanos.get() / Clock.NANOS_PER_MS;
    }

    void incrementNs(long delta) {
      nanos.addAndGet(delta);
    }
  }
}
