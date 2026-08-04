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
import static org.junit.jupiter.api.Assumptions.assumeTrue;

import com.facebook.thrift.malloc.malloc_h;
import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import org.junit.jupiter.api.Test;

/**
 * Covers the Java 25 multi-release override of {@link MallocStats}, which reports live {@code
 * mallinfo2} data. Runs only on Java 25 and newer, where {@link malloc_h} exists; the
 * version-independent contract is covered by {@code MallocStatsTest} in {@code src/test/java}.
 *
 * <p>Being on Java 25 is necessary but not sufficient: {@code mallinfo2} is a glibc extension, so
 * tests needing real allocator data are still gated on {@link MallocStats#isAvailable()} and skip
 * under jemalloc, tcmalloc, musl, and on non-Linux platforms.
 */
class MallocStatsJava25Test {

  private static final String PREFIX = "memory.malloc.";

  private static final String IN_USE_BYTES = PREFIX + "in_use_bytes";
  private static final String FREE_BYTES = PREFIX + "free_bytes";
  private static final String HEAP_BYTES = PREFIX + "heap_bytes";
  private static final String MMAP_BYTES = PREFIX + "mmap_bytes";
  private static final String TOTAL_BYTES = PREFIX + "total_bytes";
  private static final String RELEASABLE_BYTES = PREFIX + "releasable_bytes";

  /** The override must report availability from the binding, not assume Java 25 implies glibc. */
  @Test
  void isAvailableTracksSymbolResolution() {
    assertThat(new MallocStats().isAvailable()).isEqualTo(malloc_h.isAvailable());
  }

  @Test
  void exportsAllExpectedCounters() {
    MallocStats stats = new MallocStats();
    assumeTrue(stats.isAvailable(), "mallinfo2 is not available on this platform");

    assertThat(stats.getMallocStats())
        .containsOnlyKeys(
            IN_USE_BYTES,
            FREE_BYTES,
            HEAP_BYTES,
            MMAP_BYTES,
            TOTAL_BYTES,
            RELEASABLE_BYTES,
            PREFIX + "free_chunk_count",
            PREFIX + "mmap_region_count",
            PREFIX + "fastbin_free_bytes",
            PREFIX + "fastbin_chunk_count");
  }

  @Test
  void countersAreInternallyConsistent() {
    MallocStats stats = new MallocStats();
    assumeTrue(stats.isAvailable(), "mallinfo2 is not available on this platform");

    Map<String, Long> counters = stats.getMallocStats();

    assertThat(counters.values()).allSatisfy(value -> assertThat(value).isNotNegative());
    assertThat(counters.get(TOTAL_BYTES))
        .isEqualTo(counters.get(HEAP_BYTES) + counters.get(MMAP_BYTES));
    // Everything handed out plus everything free must fit in what the allocator holds.
    assertThat(counters.get(IN_USE_BYTES) + counters.get(FREE_BYTES))
        .isLessThanOrEqualTo(counters.get(TOTAL_BYTES));
    assertThat(counters.get(RELEASABLE_BYTES)).isLessThanOrEqualTo(counters.get(TOTAL_BYTES));
  }

  @Test
  void repeatedCollectionIsStable() {
    MallocStats stats = new MallocStats();
    assumeTrue(stats.isAvailable(), "mallinfo2 is not available on this platform");

    for (int i = 0; i < 100; i++) {
      assertThat(stats.getMallocStats()).isNotEmpty();
    }
  }

  /**
   * A large native allocation must be visible in the reported totals.
   *
   * <p>Asserts on {@code total_bytes} rather than {@code in_use_bytes} because glibc routes an
   * allocation to an arena or to its own mmap depending on the mmap threshold, which is dynamic —
   * it starts at 128KB and grows as mmapped blocks are freed. Which of the two counters moves
   * therefore depends on allocation history, but {@code total_bytes} covers both and holds either
   * way.
   */
  @Test
  void reflectsNativeAllocations() {
    MallocStats stats = new MallocStats();
    assumeTrue(stats.isAvailable(), "mallinfo2 is not available on this platform");

    long before = stats.getMallocStats().get(TOTAL_BYTES);

    // Direct buffers land on the C heap via Unsafe.allocateMemory, i.e. glibc malloc.
    int chunkSize = 4 * 1024 * 1024;
    int chunks = 16;
    List<ByteBuffer> held = new ArrayList<>(chunks);
    try {
      for (int i = 0; i < chunks; i++) {
        ByteBuffer buffer = ByteBuffer.allocateDirect(chunkSize);
        buffer.put(0, (byte) 1);
        held.add(buffer);
      }

      assertThat(stats.getMallocStats().get(TOTAL_BYTES)).isGreaterThan(before);
    } finally {
      held.clear();
    }
  }

  /**
   * Guards the interposed-allocator case. glibc reports empty arenas rather than failing when
   * something else is servicing malloc, so an all-zero reading must be suppressed instead of
   * published as a full set of zeroed counters.
   */
  @Test
  void reportsNothingWhenGlibcIsNotTheActiveAllocator() {
    MallocStats stats = new MallocStats();
    assumeTrue(stats.isAvailable(), "mallinfo2 is not available on this platform");

    Map<String, Long> counters = stats.getMallocStats();

    // A live JVM has allocated from whichever allocator is active, so glibc reporting nothing at
    // all means it is not that allocator, and the map must be empty rather than a row of zeros.
    if (!counters.isEmpty()) {
      assertThat(counters.get(TOTAL_BYTES))
          .as("published counters must describe a real heap, not an interposed-away one")
          .isPositive();
    }
  }
}
