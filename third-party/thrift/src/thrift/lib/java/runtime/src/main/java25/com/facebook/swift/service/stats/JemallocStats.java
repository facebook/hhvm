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

import com.facebook.thrift.malloc.jemalloc_h;
import com.google.common.collect.ImmutableMap;
import java.util.Map;
import java.util.Optional;
import java.util.OptionalLong;
import java.util.concurrent.atomic.AtomicBoolean;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

/**
 * Native allocator statistics for jemalloc, reported under the {@code memory.jemalloc.*} counter
 * family. Java 25 multi-release override of the Java 8 baseline.
 *
 * <p>jemalloc separates layers of memory that glibc's {@code mallinfo2} cannot distinguish, so
 * these counters are more precise than their {@link MallocStats} equivalents. In particular {@code
 * allocated_bytes} is tracked directly rather than derived as a residual, so unlike glibc's {@code
 * in_use_bytes} it does not silently absorb allocator bookkeeping or thread-cached frees.
 *
 * <table>
 *   <caption>counter to mallctl name</caption>
 *   <tr><th>Counter</th><th>mallctl</th><th>Meaning</th></tr>
 *   <tr><td>{@code allocated_bytes}</td><td>stats.allocated</td>
 *       <td>Bytes the application holds. Exact, not a residual</td></tr>
 *   <tr><td>{@code active_bytes}</td><td>stats.active</td>
 *       <td>Bytes in active pages; exceeds allocated by the rounding to page boundaries</td></tr>
 *   <tr><td>{@code resident_bytes}</td><td>stats.resident</td>
 *       <td>Physically backed bytes, i.e. what shows up in RSS</td></tr>
 *   <tr><td>{@code mapped_bytes}</td><td>stats.mapped</td><td>Address space mapped</td></tr>
 *   <tr><td>{@code retained_bytes}</td><td>stats.retained</td>
 *       <td>Address space kept but already returned to the OS; costs no RSS</td></tr>
 *   <tr><td>{@code metadata_bytes}</td><td>stats.metadata</td>
 *       <td>jemalloc's own bookkeeping</td></tr>
 *   <tr><td>{@code pinned_bytes}</td><td>stats.pinned</td><td>Bytes that cannot be returned</td></tr>
 *   <tr><td>{@code zero_reallocs}</td><td>stats.zero_reallocs</td>
 *       <td>Count of reallocs to size zero</td></tr>
 *   <tr><td>{@code fragmented_bytes}</td><td>active - allocated</td>
 *       <td>Derived. Space lost to page rounding inside active extents</td></tr>
 *   <tr><td>{@code overhead_bytes}</td><td>resident - allocated</td>
 *       <td>Derived. Everything resident that the application is not using</td></tr>
 *   <tr><td>{@code arena_count}</td><td>arenas.narenas</td><td>Arenas in existence</td></tr>
 *   <tr><td>{@code thread_count}</td><td>…nthreads</td><td>Threads bound to arenas</td></tr>
 *   <tr><td>{@code dirty_bytes}</td><td>…pdirty × page</td>
 *       <td>Freed but not yet purged; reclaimable by decay</td></tr>
 *   <tr><td>{@code muzzy_bytes}</td><td>…pmuzzy × page</td>
 *       <td>Lazily madvised; cheap to reuse</td></tr>
 *   <tr><td>{@code tcache_bytes}</td><td>…tcache_bytes</td>
 *       <td>Held in per-thread caches. Reported explicitly, unlike glibc</td></tr>
 *   <tr><td>{@code base_bytes} / {@code internal_bytes}</td><td>…base, …internal</td>
 *       <td>Allocator-internal structures</td></tr>
 *   <tr><td>{@code small_allocated_bytes} / {@code large_allocated_bytes}</td>
 *       <td>…small.allocated, …large.allocated</td><td>Split by size class</td></tr>
 *   <tr><td>{@code dirty_purge_count} / {@code muzzy_purge_count}</td>
 *       <td>…dirty_npurge, …muzzy_npurge</td><td>Cumulative purge runs</td></tr>
 * </table>
 *
 * <p>Per-arena figures are read through the merged {@code MALLCTL_ARENAS_ALL} index rather than
 * exported per arena: a process routinely runs hundreds of arenas, which is far too many counters.
 *
 * <p>For memory waste, {@code retained_bytes} is <em>not</em> waste — that space has already gone
 * back to the OS and only address space is held. The figures that cost real memory are {@code
 * fragmented_bytes} and {@code dirty_bytes}, and {@code overhead_bytes} bounds the total.
 *
 * <p>Collection is best effort. If jemalloc is not the active allocator, or any read fails, this
 * returns an empty map and logs once. It never throws and never prevents a server from starting.
 */
public class JemallocStats {

  private static final Logger LOGGER = LoggerFactory.getLogger(JemallocStats.class);

  private static final String PREFIX = "memory.jemalloc.";

  /** Merged view over every arena. */
  private static final String MERGED = "stats.arenas." + jemalloc_h.ARENAS_ALL + ".";

  private final AtomicBoolean loggedFailure = new AtomicBoolean();

  /** Whether jemalloc is the active allocator and its statistics can be read. */
  public boolean isAvailable() {
    return jemalloc_h.isAvailable();
  }

  /**
   * A snapshot of jemalloc allocator usage, keyed by counter name.
   *
   * @return the counters, or an empty map if jemalloc is unavailable; never null
   */
  public Map<String, Long> getJemallocStats() {
    if (!jemalloc_h.isAvailable()) {
      return ImmutableMap.of();
    }

    try {
      // Statistics are cached until the epoch is advanced, so without this every scrape would
      // return the values captured at the previous one.
      if (!jemalloc_h.refreshEpoch()) {
        return ImmutableMap.of();
      }

      ImmutableMap.Builder<String, Long> counters = ImmutableMap.builderWithExpectedSize(20);

      OptionalLong allocated = putLong(counters, "allocated_bytes", "stats.allocated");
      OptionalLong active = putLong(counters, "active_bytes", "stats.active");
      OptionalLong resident = putLong(counters, "resident_bytes", "stats.resident");
      putLong(counters, "mapped_bytes", "stats.mapped");
      putLong(counters, "retained_bytes", "stats.retained");
      putLong(counters, "metadata_bytes", "stats.metadata");
      putLong(counters, "pinned_bytes", "stats.pinned");
      putLong(counters, "zero_reallocs", "stats.zero_reallocs");

      // Emitted only when both inputs were genuinely read. Substituting zero for a failed read
      // would not omit the counter, it would publish the other operand whole: a failed
      // stats.allocated would report the entire active set as fragmentation. Both differences are
      // non-negative within one epoch snapshot; clamped anyway against a future accounting change.
      putDifference(counters, "fragmented_bytes", active, allocated);
      putDifference(counters, "overhead_bytes", resident, allocated);

      putUnsigned(counters, "arena_count", "arenas.narenas");
      putUnsigned(counters, "thread_count", MERGED + "nthreads");

      // pdirty and pmuzzy are page counts; convert so every byte counter reads in bytes.
      long pageBytes = jemalloc_h.readLong("arenas.page").orElse(0L);
      if (pageBytes > 0) {
        jemalloc_h
            .readLong(MERGED + "pdirty")
            .ifPresent(pages -> counters.put(PREFIX + "dirty_bytes", pages * pageBytes));
        jemalloc_h
            .readLong(MERGED + "pmuzzy")
            .ifPresent(pages -> counters.put(PREFIX + "muzzy_bytes", pages * pageBytes));
      }

      putLong(counters, "tcache_bytes", MERGED + "tcache_bytes");
      putLong(counters, "base_bytes", MERGED + "base");
      putLong(counters, "internal_bytes", MERGED + "internal");
      putLong(counters, "small_allocated_bytes", MERGED + "small.allocated");
      putLong(counters, "large_allocated_bytes", MERGED + "large.allocated");
      putLong(counters, "dirty_purge_count", MERGED + "dirty_npurge");
      putLong(counters, "muzzy_purge_count", MERGED + "muzzy_npurge");

      return counters.build();
    } catch (Throwable t) {
      if (loggedFailure.compareAndSet(false, true)) {
        LOGGER.error("Failed to collect jemalloc allocator stats; they will be omitted", t);
      }
      return ImmutableMap.of();
    }
  }

  /**
   * jemalloc's build version and tunable settings. These are fixed for the life of the process, so
   * they belong alongside the counters as fb303 attributes rather than as counters themselves.
   *
   * @return the settings, or an empty map if jemalloc is unavailable; never null
   */
  public Map<String, String> getJemallocAttributes() {
    if (!jemalloc_h.isAvailable()) {
      return ImmutableMap.of();
    }

    try {
      ImmutableMap.Builder<String, String> attributes = ImmutableMap.builderWithExpectedSize(10);
      putString(attributes, "version", "version");
      putString(attributes, "opt.metadata_thp", "opt.metadata_thp");
      putSigned(attributes, "opt.dirty_decay_ms", "opt.dirty_decay_ms");
      putSigned(attributes, "opt.muzzy_decay_ms", "opt.muzzy_decay_ms");
      putSigned(attributes, "opt.tcache_max", "opt.tcache_max");
      putSigned(attributes, "arenas.page", "arenas.page");
      putUnsignedAttribute(attributes, "arenas.nbins", "arenas.nbins");
      putBoolean(attributes, "opt.tcache", "opt.tcache");
      putBoolean(attributes, "opt.background_thread", "opt.background_thread");
      putBoolean(attributes, "opt.prof", "opt.prof");
      return attributes.build();
    } catch (Throwable t) {
      if (loggedFailure.compareAndSet(false, true)) {
        LOGGER.error("Failed to read jemalloc settings; they will be omitted", t);
      }
      return ImmutableMap.of();
    }
  }

  private static OptionalLong putLong(
      ImmutableMap.Builder<String, Long> counters, String counter, String mallctlName) {
    OptionalLong value = jemalloc_h.readLong(mallctlName);
    value.ifPresent(v -> counters.put(PREFIX + counter, v));
    return value;
  }

  /** Emits {@code minuend - subtrahend}, or nothing if either operand could not be read. */
  private static void putDifference(
      ImmutableMap.Builder<String, Long> counters,
      String counter,
      OptionalLong minuend,
      OptionalLong subtrahend) {
    if (minuend.isPresent() && subtrahend.isPresent()) {
      counters.put(PREFIX + counter, Math.max(0L, minuend.getAsLong() - subtrahend.getAsLong()));
    }
  }

  private static void putUnsigned(
      ImmutableMap.Builder<String, Long> counters, String counter, String mallctlName) {
    jemalloc_h.readUnsigned(mallctlName).ifPresent(value -> counters.put(PREFIX + counter, value));
  }

  private static void putString(
      ImmutableMap.Builder<String, String> attributes, String attribute, String mallctlName) {
    Optional<String> value = jemalloc_h.readString(mallctlName);
    value.ifPresent(v -> attributes.put(PREFIX + attribute, v));
  }

  private static void putSigned(
      ImmutableMap.Builder<String, String> attributes, String attribute, String mallctlName) {
    jemalloc_h
        .readLong(mallctlName)
        .ifPresent(value -> attributes.put(PREFIX + attribute, Long.toString(value)));
  }

  private static void putUnsignedAttribute(
      ImmutableMap.Builder<String, String> attributes, String attribute, String mallctlName) {
    jemalloc_h
        .readUnsigned(mallctlName)
        .ifPresent(value -> attributes.put(PREFIX + attribute, Long.toString(value)));
  }

  private static void putBoolean(
      ImmutableMap.Builder<String, String> attributes, String attribute, String mallctlName) {
    jemalloc_h
        .readBoolean(mallctlName)
        .ifPresent(value -> attributes.put(PREFIX + attribute, Boolean.toString(value)));
  }
}
