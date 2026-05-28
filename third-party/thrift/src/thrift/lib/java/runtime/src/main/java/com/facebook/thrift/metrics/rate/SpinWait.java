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

/**
 * Spin-wait primitive used inside {@link LockFreeSlidingTimeWindowMovingCounter}'s CAS loops.
 *
 * <p>This Java 8 baseline falls back to {@link Thread#yield()}. Java 11+ runtimes pick up the
 * override under {@code src/main/java11/}, which uses {@link Thread#onSpinWait()} — a JVM/CPU spin
 * hint that is typically much cheaper than a scheduler yield.
 *
 * <p>Spin paths in the counter fire in narrow circumstances:
 *
 * <ul>
 *   <li>Writers waiting on {@code ROTATING_SLOW} — only when there has been a multi-tick gap (no
 *       writes for &gt; 1 tick) followed by a burst.
 *   <li>Writers observing {@code ROTATING_FAST} with {@code nowTick > t + 1} — rare; the in-window
 *       add path bails before spinning.
 *   <li>Readers seeing any non-{@code STABLE} state during a rotation.
 * </ul>
 *
 * Rotation work itself is O(1) (no zero-fill of intermediate buckets), so the wait window is
 * normally a few microseconds at most and a single primitive (no bounded backoff) suffices. If
 * virtual-thread workloads later show carrier saturation, revisit a bounded backoff here.
 */
final class SpinWait {

  private SpinWait() {}

  static void spin() {
    Thread.yield();
  }
}
