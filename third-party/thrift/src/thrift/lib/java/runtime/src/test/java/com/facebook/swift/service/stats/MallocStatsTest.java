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

import static org.assertj.core.api.Assertions.assertThat;

import java.util.Map;
import org.junit.jupiter.api.Test;

/**
 * Covers the {@link MallocStats} contract that must hold on every Java version. This runs across
 * all supported runtimes, so it resolves to the Java 8 baseline on older ones and to the {@code
 * src/main/java25} override on newer ones, and asserts only what is true of both: collecting native
 * allocator stats never throws, never returns null, and never emits a counter outside its
 * namespace.
 *
 * <p>Behavior specific to the Java 25 override, which needs live {@code mallinfo2} data, is covered
 * by {@code MallocStatsJava25Test} in {@code src/test/java25}.
 */
class MallocStatsTest {

  private static final String PREFIX = "memory.malloc.";

  /**
   * The contract that matters most: collection degrades to an empty map rather than throwing when
   * the allocator cannot be inspected.
   */
  @Test
  void neverThrowsRegardlessOfAvailability() {
    MallocStats stats = new MallocStats();

    Map<String, Long> counters = stats.getMallocStats();

    assertThat(counters).isNotNull();
    if (!stats.isAvailable()) {
      assertThat(counters).isEmpty();
    }
  }

  @Test
  void collectionIsRepeatable() {
    MallocStats stats = new MallocStats();

    for (int i = 0; i < 100; i++) {
      assertThat(stats.getMallocStats()).isNotNull();
    }
  }

  @Test
  void everyCounterIsNamespacedAndNonNegative() {
    Map<String, Long> counters = new MallocStats().getMallocStats();

    assertThat(counters.keySet()).allSatisfy(key -> assertThat(key).startsWith(PREFIX));
    assertThat(counters.values()).allSatisfy(value -> assertThat(value).isNotNegative());
  }
}
