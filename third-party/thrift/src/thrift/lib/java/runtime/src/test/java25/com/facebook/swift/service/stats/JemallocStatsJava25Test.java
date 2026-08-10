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
import static org.junit.jupiter.api.Assumptions.assumeFalse;
import static org.junit.jupiter.api.Assumptions.assumeTrue;

import com.facebook.thrift.malloc.jemalloc_h;
import java.util.Map;
import org.junit.jupiter.api.Test;

/**
 * Covers the Java 25 {@link JemallocStats} override. Which half of this runs depends on the
 * allocator the test JVM is linked against, and both halves matter: under glibc it pins the
 * degrade-to-empty path, and under jemalloc it checks the real counters.
 */
class JemallocStatsJava25Test {

  private static final String PREFIX = "memory.jemalloc.";

  private static final String ALLOCATED_BYTES = PREFIX + "allocated_bytes";
  private static final String ACTIVE_BYTES = PREFIX + "active_bytes";
  private static final String RESIDENT_BYTES = PREFIX + "resident_bytes";
  private static final String MAPPED_BYTES = PREFIX + "mapped_bytes";
  private static final String FRAGMENTED_BYTES = PREFIX + "fragmented_bytes";
  private static final String OVERHEAD_BYTES = PREFIX + "overhead_bytes";
  private static final String ARENA_COUNT = PREFIX + "arena_count";

  @Test
  void isAvailableTracksSymbolResolution() {
    assertThat(new JemallocStats().isAvailable()).isEqualTo(jemalloc_h.isAvailable());
  }

  /**
   * The path that matters on a glibc-linked binary, which is most of them: a missing {@code
   * mallctl} must degrade to reporting nothing, not throw. Also guards the weak-symbol case, where
   * the symbol can resolve to a null address rather than be absent.
   */
  @Test
  void reportsNothingWhenJemallocIsAbsent() {
    JemallocStats stats = new JemallocStats();
    assumeFalse(stats.isAvailable(), "jemalloc is the active allocator on this runtime");

    assertThat(stats.getJemallocStats()).isNotNull().isEmpty();
    assertThat(stats.getJemallocAttributes()).isNotNull().isEmpty();
    assertThat(jemalloc_h.refreshEpoch()).isFalse();
    assertThat(jemalloc_h.readLong("stats.allocated")).isEmpty();
    assertThat(jemalloc_h.readUnsigned("arenas.narenas")).isEmpty();
    assertThat(jemalloc_h.readBoolean("opt.tcache")).isEmpty();
    assertThat(jemalloc_h.readString("version")).isEmpty();
  }

  @Test
  void exportsCoreCountersWhenJemallocIsPresent() {
    JemallocStats stats = new JemallocStats();
    assumeTrue(stats.isAvailable(), "jemalloc is not the active allocator on this runtime");

    assertThat(stats.getJemallocStats())
        .containsKeys(
            ALLOCATED_BYTES,
            ACTIVE_BYTES,
            RESIDENT_BYTES,
            MAPPED_BYTES,
            PREFIX + "retained_bytes",
            PREFIX + "metadata_bytes",
            FRAGMENTED_BYTES,
            OVERHEAD_BYTES,
            ARENA_COUNT,
            PREFIX + "tcache_bytes");
  }

  /**
   * jemalloc's own layering: what the application holds fits inside the active pages, which fit
   * inside what is resident, which fits inside what is mapped. A width mismatch on any read would
   * produce a garbage value that breaks the chain.
   */
  @Test
  void countersRespectTheAllocatorLayering() {
    JemallocStats stats = new JemallocStats();
    assumeTrue(stats.isAvailable(), "jemalloc is not the active allocator on this runtime");

    Map<String, Long> counters = stats.getJemallocStats();

    assertThat(counters.values()).allSatisfy(value -> assertThat(value).isNotNegative());
    assertThat(counters.get(ALLOCATED_BYTES)).isLessThanOrEqualTo(counters.get(ACTIVE_BYTES));
    assertThat(counters.get(ACTIVE_BYTES)).isLessThanOrEqualTo(counters.get(RESIDENT_BYTES));
    assertThat(counters.get(RESIDENT_BYTES)).isLessThanOrEqualTo(counters.get(MAPPED_BYTES));
    assertThat(counters.get(FRAGMENTED_BYTES))
        .isEqualTo(counters.get(ACTIVE_BYTES) - counters.get(ALLOCATED_BYTES));
    assertThat(counters.get(OVERHEAD_BYTES))
        .isEqualTo(counters.get(RESIDENT_BYTES) - counters.get(ALLOCATED_BYTES));
    assertThat(counters.get(ARENA_COUNT)).isGreaterThanOrEqualTo(1L);
  }

  /** Stats are cached until the epoch advances, so a scrape that skipped it would go stale. */
  @Test
  void epochRefreshSucceedsWhenJemallocIsPresent() {
    assumeTrue(jemalloc_h.isAvailable(), "jemalloc is not the active allocator on this runtime");

    assertThat(jemalloc_h.refreshEpoch()).isTrue();
  }

  /**
   * Guards the type table. mallctl rejects a mismatched width with EINVAL, which this binding maps
   * to an absent value, so a wrong type would silently drop the counter rather than fail loudly.
   */
  @Test
  void everyDeclaredTypeMatchesJemalloc() {
    assumeTrue(jemalloc_h.isAvailable(), "jemalloc is not the active allocator on this runtime");
    jemalloc_h.refreshEpoch();

    String merged = "stats.arenas." + jemalloc_h.ARENAS_ALL + ".";
    for (String eightByte :
        new String[] {
          "stats.allocated",
          "stats.active",
          "stats.resident",
          "stats.mapped",
          "stats.retained",
          "stats.metadata",
          "stats.pinned",
          "stats.zero_reallocs",
          "arenas.page",
          merged + "pdirty",
          merged + "pmuzzy",
          merged + "tcache_bytes",
          merged + "base",
          merged + "internal",
          merged + "small.allocated",
          merged + "large.allocated",
          merged + "dirty_npurge",
          merged + "muzzy_npurge",
          "opt.dirty_decay_ms",
          "opt.muzzy_decay_ms",
          "opt.tcache_max"
        }) {
      assertThat(jemalloc_h.readLong(eightByte)).as("8-byte read of %s", eightByte).isPresent();
    }
    for (String fourByte : new String[] {"arenas.narenas", "arenas.nbins", merged + "nthreads"}) {
      assertThat(jemalloc_h.readUnsigned(fourByte)).as("4-byte read of %s", fourByte).isPresent();
    }
    for (String flag : new String[] {"opt.tcache", "opt.background_thread", "opt.prof"}) {
      assertThat(jemalloc_h.readBoolean(flag)).as("bool read of %s", flag).isPresent();
    }
    for (String text : new String[] {"version", "opt.metadata_thp"}) {
      assertThat(jemalloc_h.readString(text)).as("string read of %s", text).isPresent();
    }
  }

  @Test
  void exportsSettingsAsAttributesWhenJemallocIsPresent() {
    JemallocStats stats = new JemallocStats();
    assumeTrue(stats.isAvailable(), "jemalloc is not the active allocator on this runtime");

    assertThat(stats.getJemallocAttributes())
        .containsKeys(
            PREFIX + "version",
            PREFIX + "opt.dirty_decay_ms",
            PREFIX + "opt.muzzy_decay_ms",
            PREFIX + "opt.tcache",
            PREFIX + "opt.background_thread");
  }
}
