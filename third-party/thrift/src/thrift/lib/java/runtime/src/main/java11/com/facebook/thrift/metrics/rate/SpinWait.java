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
 * Java 11+ override of {@link SpinWait}. Uses {@link Thread#onSpinWait()}, a JVM/CPU spin hint that
 * is typically much cheaper than a scheduler yield. See the Java 8 baseline class doc for when the
 * counter's spin paths actually fire — they are narrow burst-after-quiet cases, and rotation work
 * itself is O(1), so a single primitive (no bounded backoff) is sufficient. If a virtual-thread
 * workload later shows carrier saturation, revisit a bounded backoff here.
 */
final class SpinWait {

  private SpinWait() {}

  static void spin() {
    Thread.onSpinWait();
  }
}
