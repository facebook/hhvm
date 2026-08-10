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
 * Covers the {@link JemallocStats} contract that must hold on every Java version. This runs across
 * all supported runtimes, resolving to the Java 8 baseline on older ones and to the {@code
 * src/main/java25} override on newer ones, and asserts only what is true of both: collecting
 * allocator statistics never throws, never returns null, and never emits a counter outside its
 * namespace.
 *
 * <p>Most importantly it pins the behaviour when jemalloc is <em>not</em> the allocator, which is
 * the common case: report nothing rather than fail.
 */
class JemallocStatsTest {

  private static final String PREFIX = "memory.jemalloc.";

  @Test
  void neverThrowsRegardlessOfAvailability() {
    JemallocStats stats = new JemallocStats();

    Map<String, Long> counters = stats.getJemallocStats();

    assertThat(counters).isNotNull();
    if (!stats.isAvailable()) {
      assertThat(counters).isEmpty();
    }
  }

  @Test
  void attributesNeverThrowRegardlessOfAvailability() {
    JemallocStats stats = new JemallocStats();

    Map<String, String> attributes = stats.getJemallocAttributes();

    assertThat(attributes).isNotNull();
    if (!stats.isAvailable()) {
      assertThat(attributes).isEmpty();
    }
  }

  @Test
  void collectionIsRepeatable() {
    JemallocStats stats = new JemallocStats();

    for (int i = 0; i < 50; i++) {
      assertThat(stats.getJemallocStats()).isNotNull();
    }
  }

  @Test
  void everyCounterIsNamespacedAndNonNegative() {
    Map<String, Long> counters = new JemallocStats().getJemallocStats();

    assertThat(counters.keySet()).allSatisfy(key -> assertThat(key).startsWith(PREFIX));
    assertThat(counters.values()).allSatisfy(value -> assertThat(value).isNotNegative());
  }

  @Test
  void everyAttributeIsNamespaced() {
    Map<String, String> attributes = new JemallocStats().getJemallocAttributes();

    assertThat(attributes.keySet()).allSatisfy(key -> assertThat(key).startsWith(PREFIX));
  }
}
