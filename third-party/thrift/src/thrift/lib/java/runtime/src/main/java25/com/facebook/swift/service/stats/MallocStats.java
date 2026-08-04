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

import com.facebook.thrift.malloc.malloc_h;
import com.google.common.collect.ImmutableMap;
import java.lang.foreign.Arena;
import java.lang.foreign.MemorySegment;
import java.util.Map;
import java.util.concurrent.atomic.AtomicBoolean;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

/**
 * Native (glibc) allocator statistics, reported under the {@code memory.malloc.*} counter family.
 *
 * <p>Java 25 multi-release override of the Java 8 baseline. Reports whole-process glibc allocator
 * state via {@code mallinfo2(3)}, which aggregates every arena and so remains accurate for a
 * multi-threaded JVM. These counters sit alongside the JVM's own {@code memory.heap.*} figures and
 * cover what the JVM's view cannot: the C heap behind direct buffers, JNI, and the JVM's own
 * internal allocations.
 *
 * <p>The raw {@code struct mallinfo2} field names are cryptic, so each is republished under a
 * descriptive counter name:
 *
 * <table>
 *   <caption>mallinfo2 field to counter name</caption>
 *   <tr><th>Counter</th><th>mallinfo2 field</th><th>Meaning</th></tr>
 *   <tr><td>{@code in_use_bytes}</td><td>uordblks</td>
 *       <td>Bytes currently handed out to the application</td></tr>
 *   <tr><td>{@code free_bytes}</td><td>fordblks</td>
 *       <td>Bytes free inside the arenas, retained rather than returned to the OS</td></tr>
 *   <tr><td>{@code heap_bytes}</td><td>arena</td>
 *       <td>Bytes obtained from the OS in non-mmapped arenas</td></tr>
 *   <tr><td>{@code mmap_bytes}</td><td>hblkhd</td>
 *       <td>Bytes in mmapped regions (allocations above the mmap threshold)</td></tr>
 *   <tr><td>{@code total_bytes}</td><td>arena + hblkhd</td>
 *       <td>Total bytes the allocator holds from the OS</td></tr>
 *   <tr><td>{@code releasable_bytes}</td><td>keepcost</td>
 *       <td>Main arena's top chunk; a floor on what {@code malloc_trim} can return, not an
 *       estimate of it</td></tr>
 *   <tr><td>{@code free_chunk_count}</td><td>ordblks</td>
 *       <td>Number of free chunks; rising against flat free_bytes indicates fragmentation</td></tr>
 *   <tr><td>{@code mmap_region_count}</td><td>hblks</td><td>Number of live mmapped regions</td></tr>
 *   <tr><td>{@code fastbin_free_bytes}</td><td>fsmblks</td><td>Bytes in free fastbin chunks</td></tr>
 *   <tr><td>{@code fastbin_chunk_count}</td><td>smblks</td><td>Number of free fastbin chunks</td></tr>
 * </table>
 *
 * <p>{@code usmblks} is not exported: modern glibc always reports it as 0.
 *
 * <p>{@code free_bytes} is the arena-bloat figure: glibc computes it as the arena total minus what
 * is handed out, so {@code heap_bytes == in_use_bytes + free_bytes} exactly, and it counts every
 * arena's top chunk and free bins. Memory freed on one arena cannot satisfy a thread bound to
 * another, so this is the space charged to process RSS that the application cannot use. {@code
 * free_chunk_count} qualifies it: climbing while {@code free_bytes} stays flat means the free space
 * is scattered across many small chunks rather than coalesced into reclaimable spans.
 *
 * <p>These counters cannot show per-allocation metadata overhead. glibc derives the in-use figure
 * as {@code system_mem - avail}, so chunk headers and alignment padding are counted as in use
 * rather than as waste.
 *
 * <p>Collection is best effort. If {@code mallinfo2} is unavailable (jemalloc, tcmalloc, musl,
 * non-Linux, or native access denied) or the downcall fails, {@link #getMallocStats()} returns an
 * empty map and logs once. It never throws and never prevents a server from starting.
 */
public class MallocStats {

  private static final Logger LOGGER = LoggerFactory.getLogger(MallocStats.class);

  private static final String PREFIX = "memory.malloc.";

  private static final String IN_USE_BYTES = PREFIX + "in_use_bytes";
  private static final String FREE_BYTES = PREFIX + "free_bytes";
  private static final String HEAP_BYTES = PREFIX + "heap_bytes";
  private static final String MMAP_BYTES = PREFIX + "mmap_bytes";
  private static final String TOTAL_BYTES = PREFIX + "total_bytes";
  private static final String RELEASABLE_BYTES = PREFIX + "releasable_bytes";
  private static final String FREE_CHUNK_COUNT = PREFIX + "free_chunk_count";
  private static final String MMAP_REGION_COUNT = PREFIX + "mmap_region_count";
  private static final String FASTBIN_FREE_BYTES = PREFIX + "fastbin_free_bytes";
  private static final String FASTBIN_CHUNK_COUNT = PREFIX + "fastbin_chunk_count";

  private final AtomicBoolean loggedFailure = new AtomicBoolean();

  /** Whether {@code mallinfo2} was resolved and native allocator stats can be collected. */
  public boolean isAvailable() {
    return malloc_h.isAvailable();
  }

  /**
   * A snapshot of glibc allocator usage, keyed by counter name.
   *
   * @return the counters, or an empty map if native allocator stats are unavailable; never null
   */
  public Map<String, Long> getMallocStats() {
    if (!malloc_h.isAvailable()) {
      return ImmutableMap.of();
    }

    try (Arena arena = Arena.ofConfined()) {
      MemorySegment info = malloc_h.mallinfo2(arena);

      long heapBytes = malloc_h.arena(info);
      long mmapBytes = malloc_h.hblkhd(info);

      if (!isActiveAllocator(heapBytes, mmapBytes)) {
        return ImmutableMap.of();
      }

      return ImmutableMap.<String, Long>builderWithExpectedSize(10)
          .put(IN_USE_BYTES, malloc_h.uordblks(info))
          .put(FREE_BYTES, malloc_h.fordblks(info))
          .put(HEAP_BYTES, heapBytes)
          .put(MMAP_BYTES, mmapBytes)
          .put(TOTAL_BYTES, heapBytes + mmapBytes)
          .put(RELEASABLE_BYTES, malloc_h.keepcost(info))
          .put(FREE_CHUNK_COUNT, malloc_h.ordblks(info))
          .put(MMAP_REGION_COUNT, malloc_h.hblks(info))
          .put(FASTBIN_FREE_BYTES, malloc_h.fsmblks(info))
          .put(FASTBIN_CHUNK_COUNT, malloc_h.smblks(info))
          .build();
    } catch (Throwable t) {
      // Stats collection must never break a scrape, let alone startup. Log once so a persistent
      // failure is visible without flooding on every counter poll.
      if (loggedFailure.compareAndSet(false, true)) {
        LOGGER.error("Failed to collect glibc allocator stats; they will be omitted", t);
      }
      return ImmutableMap.of();
    }
  }

  /**
   * Whether glibc is the allocator actually servicing this process.
   *
   * <p>{@code mallinfo2} lives in libc, which is always loaded, so it resolves and answers even
   * when another allocator has interposed on {@code malloc}. It then truthfully reports glibc's own
   * arenas — which are empty, because nothing is allocating from them. Publishing that would emit a
   * full set of zeroed counters that is indistinguishable from an idle heap, so a dashboard reads
   * "no memory used" and an alert on growth never fires.
   *
   * <p>A live JVM has always obtained memory from its allocator, so both figures being zero can
   * only mean the allocations went somewhere else: jemalloc, tcmalloc, or anything else preloaded
   * ahead of libc. No allocator-specific knowledge is needed to detect it.
   *
   * <p>Deliberately re-evaluated on every collection rather than cached. When glibc is inactive its
   * arenas are empty, so the {@code mallinfo2} bin walk has nothing to traverse and the call costs
   * microseconds — there is no saving to justify the risk of latching a verdict formed during early
   * startup.
   */
  private static boolean isActiveAllocator(long heapBytes, long mmapBytes) {
    return heapBytes != 0 || mmapBytes != 0;
  }
}
