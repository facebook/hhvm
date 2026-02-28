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

import com.google.common.collect.ImmutableMap;
import io.netty.buffer.AdaptiveByteBufAllocator;
import io.netty.buffer.ByteBufAllocator;
import io.netty.buffer.ByteBufAllocatorMetric;
import io.netty.buffer.PoolArenaMetric;
import io.netty.buffer.PooledByteBufAllocator;
import io.netty.buffer.PooledByteBufAllocatorMetric;
import io.netty.buffer.UnpooledByteBufAllocator;
import io.netty.util.internal.PlatformDependent;
import java.util.List;
import java.util.Map;
import java.util.function.ToLongFunction;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public class AllocatorStats {
  private static final Logger log = LoggerFactory.getLogger(AllocatorStats.class);

  /**
   * Retrieves a snapshot of the current memory usage for the process's default Netty allocator. It
   * inspects the type of ByteBufAllocator.DEFAULT and reports metrics accordingly.
   *
   * @return An immutable map where the key is a descriptive name of the memory pool and the value
   *     is the usage in bytes.
   */
  public Map<String, Long> getAllocatorStats() {

    ByteBufAllocator alloc = ByteBufAllocator.DEFAULT;

    if (alloc instanceof PooledByteBufAllocator) {
      return addPooledAllocatorMetrics((PooledByteBufAllocator) alloc);
    } else if (alloc instanceof AdaptiveByteBufAllocator) {
      return addAdaptiveAllocatorMetrics((AdaptiveByteBufAllocator) alloc);
    } else if (alloc instanceof UnpooledByteBufAllocator) {
      return addUnpooledAllocatorMetrics((UnpooledByteBufAllocator) alloc);
    } else {
      log.error("Unsupported ByteBufAllocator implementation: {}", alloc.getClass().getName());
      return ImmutableMap.of();
    }
  }

  /**
   * Adds metrics for PooledByteBufAllocator. This allocator type exposes its metrics directly by
   * implementing PooledByteBufAllocatorMetric.
   *
   * @return map of PooledByteBufAllocatorMetrics
   */
  private Map<String, Long> addPooledAllocatorMetrics(PooledByteBufAllocator allocator) {
    PooledByteBufAllocatorMetric metric = allocator.metric();

    ImmutableMap.Builder<String, Long> metrics = ImmutableMap.builderWithExpectedSize(50);
    addNettyDirectMemoryCounter(metrics);
    metrics.put("netty.pooled.usedHeapMemory", metric.usedHeapMemory());
    metrics.put("netty.pooled.usedDirectMemory", metric.usedDirectMemory());
    metrics.put("netty.pooled.activeHeapMemory", allocator.pinnedHeapMemory());
    metrics.put("netty.pooled.activeDirectMemory", allocator.pinnedDirectMemory());
    metrics.put("netty.pooled.numHeapArenas", (long) metric.numHeapArenas());
    metrics.put("netty.pooled.numDirectArenas", (long) metric.numDirectArenas());
    metrics.put("netty.pooled.numThreadLocalCaches", (long) metric.numThreadLocalCaches());
    metrics.put("netty.pooled.smallCacheSize", (long) metric.smallCacheSize());
    metrics.put("netty.pooled.normalCacheSize", (long) metric.normalCacheSize());
    metrics.put("netty.pooled.chunkSize", (long) metric.chunkSize());

    addArenaMetrics(metrics, "netty.pooled.heap", metric.heapArenas());
    addArenaMetrics(metrics, "netty.pooled.direct", metric.directArenas());
    return metrics.build();
  }

  private void addArenaMetrics(
      ImmutableMap.Builder<String, Long> metrics,
      String prefix,
      List<PoolArenaMetric> arenaMetrics) {
    if (arenaMetrics == null) {
      return;
    }

    metrics.put(
        prefix + ".arena.numActiveAllocations.avg",
        getAvg(arenaMetrics, PoolArenaMetric::numActiveAllocations));
    metrics.put(
        prefix + ".arena.numActiveBytes.avg",
        getAvg(arenaMetrics, PoolArenaMetric::numActiveBytes));
    metrics.put(
        prefix + ".arena.numAllocations.avg",
        getAvg(arenaMetrics, PoolArenaMetric::numAllocations));
    metrics.put(
        prefix + ".arena.numDeallocations.avg",
        getAvg(arenaMetrics, PoolArenaMetric::numDeallocations));
    metrics.put(
        prefix + ".arena.numActiveSmallAllocations.avg",
        getAvg(arenaMetrics, PoolArenaMetric::numActiveSmallAllocations));
    metrics.put(
        prefix + ".arena.numActiveNormalAllocations.avg",
        getAvg(arenaMetrics, PoolArenaMetric::numActiveNormalAllocations));
    metrics.put(
        prefix + ".arena.numActiveHugeAllocations.avg",
        getAvg(arenaMetrics, PoolArenaMetric::numActiveHugeAllocations));
    metrics.put(
        prefix + ".arena.numThreadCaches.avg",
        getAvg(arenaMetrics, PoolArenaMetric::numThreadCaches));
    metrics.put(
        prefix + ".arena.numSmallSubpages.avg",
        getAvg(arenaMetrics, PoolArenaMetric::numSmallSubpages));
    metrics.put(
        prefix + ".arena.numChunkLists.avg", getAvg(arenaMetrics, PoolArenaMetric::numChunkLists));
    metrics.put(
        prefix + ".arena.numSmallAllocations.avg",
        getAvg(arenaMetrics, PoolArenaMetric::numSmallAllocations));
    metrics.put(
        prefix + ".arena.numNormalAllocations.avg",
        getAvg(arenaMetrics, PoolArenaMetric::numNormalAllocations));
    metrics.put(
        prefix + ".arena.numHugeAllocations.avg",
        getAvg(arenaMetrics, PoolArenaMetric::numHugeAllocations));
    metrics.put(
        prefix + ".arena.numSmallDeallocations.avg",
        getAvg(arenaMetrics, PoolArenaMetric::numSmallDeallocations));
    metrics.put(
        prefix + ".arena.numNormalDeallocations.avg",
        getAvg(arenaMetrics, PoolArenaMetric::numNormalDeallocations));
    metrics.put(
        prefix + ".arena.numHugeDeallocations.avg",
        getAvg(arenaMetrics, PoolArenaMetric::numHugeDeallocations));
  }

  private long getAvg(List<PoolArenaMetric> arenaMetrics, ToLongFunction<PoolArenaMetric> mapper) {
    return (long) arenaMetrics.stream().mapToLong(mapper).average().orElse(0.0);
  }

  /**
   * Adds metrics for AdaptiveByteBufAllocator. This allocator type exposes its metrics directly by
   * implementing ByteBufAllocatorMetric.
   *
   * @return map of AdaptiveAllocatorMetrics
   */
  private Map<String, Long> addAdaptiveAllocatorMetrics(AdaptiveByteBufAllocator allocator) {
    ByteBufAllocatorMetric metric = allocator.metric();

    ImmutableMap.Builder<String, Long> metrics = ImmutableMap.builderWithExpectedSize(2);
    addNettyDirectMemoryCounter(metrics);
    metrics.put("netty.adaptive.usedHeapMemory", metric.usedHeapMemory());
    metrics.put("netty.adaptive.usedDirectMemory", metric.usedDirectMemory());

    return metrics.build();
  }

  /**
   * Adds metrics for UnpooledByteBufAllocator. This allocator type exposes its metrics directly.
   *
   * @return map of UnpooledAllocatorMetrics
   */
  private Map<String, Long> addUnpooledAllocatorMetrics(UnpooledByteBufAllocator allocator) {
    ByteBufAllocatorMetric metric = allocator.metric();

    ImmutableMap.Builder<String, Long> metrics = ImmutableMap.builderWithExpectedSize(2);
    addNettyDirectMemoryCounter(metrics);
    metrics.put("netty.unpooled.usedHeapMemory", metric.usedHeapMemory());
    metrics.put("netty.unpooled.usedDirectMemory", metric.usedDirectMemory());

    return metrics.build();
  }

  private void addNettyDirectMemoryCounter(ImmutableMap.Builder<String, Long> metrics) {
    metrics.put("netty.maxDirectMemory", PlatformDependent.maxDirectMemory());
    metrics.put(
        "netty.usedDirectMemory",
        PlatformDependent.usedDirectMemory() == -1 ? 0 : PlatformDependent.usedDirectMemory());
  }
}
