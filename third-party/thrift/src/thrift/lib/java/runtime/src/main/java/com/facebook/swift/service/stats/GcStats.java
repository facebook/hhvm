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

import com.facebook.thrift.metrics.distribution.Quantile;
import com.facebook.thrift.metrics.distribution.SingleWindowDistribution;
import com.facebook.thrift.metrics.rate.SlidingTimeWindowMovingCounter;
import com.google.common.collect.ImmutableMap;
import com.google.common.collect.ImmutableSet;
import com.google.common.util.concurrent.AtomicDouble;
import com.sun.management.GarbageCollectionNotificationInfo;
import java.lang.management.BufferPoolMXBean;
import java.lang.management.GarbageCollectorMXBean;
import java.lang.management.ManagementFactory;
import java.lang.management.MemoryMXBean;
import java.lang.management.MemoryUsage;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.atomic.AtomicLong;
import javax.management.NotificationEmitter;
import javax.management.NotificationListener;
import javax.management.openmbean.CompositeData;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public class GcStats {
  private static final Logger logger = LoggerFactory.getLogger(GcStats.class);

  private final Map<String, SlidingTimeWindowMovingCounter> counters = new ConcurrentHashMap<>();
  private final Map<String, SingleWindowDistribution> distributions = new ConcurrentHashMap<>();

  public static enum Collector {
    G1,
    GENERATIONAL_ZGC,
    ZGC,
    PARALLEL,
    UNKNOWN;
  };

  private static enum ParEventType {
    MARK_SWEEP,
    SCAVENGE,
    UNKNOWN,
  }

  private static enum G1EventType {
    FULL,
    MIXED,
    YOUNG,
    CONCURRENT,
    UNKNOWN;
  }

  private static enum GenerationalZGCEventType {
    MAJOR_CYCLE,
    MAJOR_PAUSE,
    MINOR_CYCLE,
    MINOR_PAUSE,
    UNKNOWN;
  }

  private static enum ZGCEventType {
    CYCLE,
    PAUSE,
    UNKNOWN;
  }

  private final MemoryMXBean memoryMXBean;

  // GC state variables
  private final AtomicDouble heapUsedAfterGcPct = new AtomicDouble(0.0);
  private final AtomicLong heapUsedAfterGc = new AtomicLong(0L);

  private Collector collector = Collector.UNKNOWN;

  public GcStats() {
    this.memoryMXBean = ManagementFactory.getMemoryMXBean();

    for (GarbageCollectorMXBean gcBean : ManagementFactory.getGarbageCollectorMXBeans()) {
      String gcName = gcBean.getName();

      // Collector names are acquired via manual inspection using test script P1730483553
      if (ImmutableSet.of(
              "ZGC Minor Cycles", "ZGC Minor Pauses", "ZGC Major Cycles", "ZGC Major Pauses")
          .contains(gcName)) {
        collector = Collector.GENERATIONAL_ZGC;
        break;
      } else if (ImmutableSet.of("G1 Young Generation", "G1 Concurrent GC", "G1 Old Generation")
          .contains(gcName)) {
        collector = Collector.G1;
        break;
      } else if (ImmutableSet.of("ZGC Cycles", "ZGC Pauses").contains(gcName)) {
        collector = Collector.ZGC;
        break;
      } else if (ImmutableSet.of("PS MarkSweep", "PS Scavenge").contains(gcName)) {
        collector = Collector.PARALLEL;
        break;
      } else {
        collector = Collector.UNKNOWN;
      }
    }

    for (GarbageCollectorMXBean mbean : ManagementFactory.getGarbageCollectorMXBeans()) {
      if (mbean instanceof NotificationEmitter) {
        // Register callback for GC notifications
        ((NotificationEmitter) mbean)
            .addNotificationListener(
                getListenerForCollector(collector),
                notification ->
                    notification
                        .getType()
                        .equals(GarbageCollectionNotificationInfo.GARBAGE_COLLECTION_NOTIFICATION),
                null);
      }
    }
  }

  private NotificationListener getListenerForCollector(Collector collector) {
    NotificationListener listener;
    switch (collector) {
      case G1:
        listener = getG1NotifcationListener();
        break;
      case ZGC:
        listener = getZGCNotificationListener();
        break;
      case GENERATIONAL_ZGC:
        listener = getGenerationalZGCNotificationListener();
        break;
      case PARALLEL:
        listener = getPSNotificationListener();
        break;
      case UNKNOWN:
      default:
        listener =
            (notification, ref) -> {
              logger.error("No GCNotificationListener registered");
            };
        break;
    }

    return listener;
  }

  private NotificationListener getGenerationalZGCNotificationListener() {
    return (notification, ref) -> {
      try {
        final String type = notification.getType();
        final CompositeData data = (CompositeData) notification.getUserData();
        final GarbageCollectionNotificationInfo info = GarbageCollectionNotificationInfo.from(data);
        final String name = info.getGcName();
        final long id = info.getGcInfo().getId();
        final String action = info.getGcAction();
        final String cause = info.getGcCause();
        final long startTime = info.getGcInfo().getStartTime();
        final long endTime = info.getGcInfo().getEndTime();
        final long duration = info.getGcInfo().getDuration();

        final MemoryUsage youngGenBefore =
            info.getGcInfo().getMemoryUsageBeforeGc().get("ZGC Young Generation");
        final MemoryUsage youngGenAfter =
            info.getGcInfo().getMemoryUsageAfterGc().get("ZGC Young Generation");
        final MemoryUsage oldGenBefore =
            info.getGcInfo().getMemoryUsageBeforeGc().get("ZGC Old Generation");
        final MemoryUsage oldGenAfter =
            info.getGcInfo().getMemoryUsageAfterGc().get("ZGC Old Generation");

        /*
         Generational ZGC has two event types and two collectors
           ZGC Minor Cycles
           ZGC Minor Pauses
           ZGC Major Cycles
           ZGC Major Pauses
         Pause time is only represented by Major/Minor Pauses event type
         Memory is only emitted on the Major/Minor Cycles event type

         Note: Pause time is emitted in milliseconds and will often be 0
        */
        GenerationalZGCEventType gcEventType = GenerationalZGCEventType.UNKNOWN;

        if ("ZGC Minor Cycles".equals(name)) {
          gcEventType = GenerationalZGCEventType.MINOR_CYCLE;
        } else if ("ZGC Minor Pauses".equals(name)) {
          gcEventType = GenerationalZGCEventType.MINOR_PAUSE;
        } else if ("ZGC Major Cycles".equals(name)) {
          gcEventType = GenerationalZGCEventType.MAJOR_CYCLE;
        } else if ("ZGC Major Pauses".equals(name)) {
          gcEventType = GenerationalZGCEventType.MAJOR_PAUSE;
        }

        logger.debug(name + " (" + type + ")");
        logger.debug("                  Id: " + id);
        logger.debug("              Action: " + action);
        logger.debug("               Cause: " + cause);
        logger.debug("           StartTime: " + startTime);
        logger.debug("             EndTime: " + endTime);
        logger.debug("            Duration: " + duration);
        logger.debug("        Young Before: " + oldGenBefore.getUsed());
        logger.debug("         Young After: " + youngGenAfter.getUsed());
        logger.debug("          Old Before: " + oldGenBefore.getUsed());
        logger.debug("           Old After: " + oldGenAfter.getUsed());
        logger.debug("       GC Event Type: " + gcEventType.name());

        if (gcEventType == GenerationalZGCEventType.MINOR_PAUSE
            || gcEventType == GenerationalZGCEventType.MAJOR_PAUSE) {
          bumpGcDurationStats(Collector.GENERATIONAL_ZGC.name(), gcEventType.name(), duration);
        }

        if (gcEventType == GenerationalZGCEventType.MINOR_CYCLE
            || gcEventType == GenerationalZGCEventType.MAJOR_CYCLE) {
          long usedBefore = youngGenBefore.getUsed() + oldGenBefore.getUsed();
          long usedAfter = youngGenAfter.getUsed() + oldGenAfter.getUsed();
          long maxAfter = oldGenAfter.getMax();

          heapUsedAfterGc.set(usedAfter);
          heapUsedAfterGcPct.set((double) usedAfter / (double) maxAfter);

          long memoryCollected = usedBefore - usedAfter;
          double percentCollected = (((double) memoryCollected) / ((double) usedBefore)) * 100;
          double utilization = (((double) usedAfter) / ((double) maxAfter)) * 100;

          bumpGcMemoryStats(
              Collector.GENERATIONAL_ZGC.name(),
              gcEventType.name(),
              (long) utilization,
              (long) percentCollected);
        }
      } catch (Exception e) {
        logger.error(e.toString(), e);
      }
    };
  }

  private NotificationListener getZGCNotificationListener() {
    return (notification, ref) -> {
      final String type = notification.getType();
      final CompositeData data = (CompositeData) notification.getUserData();
      final GarbageCollectionNotificationInfo info = GarbageCollectionNotificationInfo.from(data);
      final String name = info.getGcName();
      final long id = info.getGcInfo().getId();
      final String action = info.getGcAction();
      final String cause = info.getGcCause();
      final long startTime = info.getGcInfo().getStartTime();
      final long endTime = info.getGcInfo().getEndTime();
      final long duration = info.getGcInfo().getDuration();

      final MemoryUsage heapBefore = info.getGcInfo().getMemoryUsageBeforeGc().get("ZHeap");
      final MemoryUsage heapAfter = info.getGcInfo().getMemoryUsageAfterGc().get("ZHeap");

      /*
       ZGC has two possible event types:
         ZGC Pauses
         ZGC Cycles
       Pause time is only represented by the "ZGC Pauses" event type
       Memory is only emitted on the "ZGC Cycles" event type

       Note: Pause time is emitted in milliseconds and will often be 0
      */
      ZGCEventType gcEventType = ZGCEventType.UNKNOWN;

      if ("ZGC Pauses".equals(name)) {
        gcEventType = ZGCEventType.PAUSE;
      } else if ("ZGC Cycles".equals(name)) {
        gcEventType = ZGCEventType.CYCLE;
      }

      logger.debug(name + " (" + type + ")");
      logger.debug("                  Id: " + id);
      logger.debug("              Action: " + action);
      logger.debug("               Cause: " + cause);
      logger.debug("           StartTime: " + startTime);
      logger.debug("             EndTime: " + endTime);
      logger.debug("            Duration: " + duration);
      logger.debug("    Heap Used Before: " + heapBefore.getUsed());
      logger.debug("     Heap Used After: " + heapAfter.getUsed());
      logger.debug("       GC Event Type: " + gcEventType.name());

      if (gcEventType == ZGCEventType.PAUSE) {
        bumpGcDurationStats(Collector.ZGC.name(), gcEventType.name(), duration);
      }

      if (gcEventType == ZGCEventType.CYCLE) {
        long usedBefore = heapBefore.getUsed();
        long usedAfter = heapAfter.getUsed();
        long maxAfter = heapAfter.getMax();

        heapUsedAfterGc.set(usedAfter);
        heapUsedAfterGcPct.set((double) usedAfter / (double) maxAfter);

        long memoryCollected = usedBefore - usedAfter;
        double percentCollected = (((double) memoryCollected) / ((double) usedBefore)) * 100;
        double utilization = (((double) usedAfter) / ((double) maxAfter)) * 100;

        bumpGcMemoryStats(
            Collector.ZGC.name(), gcEventType.name(), (long) utilization, (long) percentCollected);
      }
    };
  }

  private NotificationListener getG1NotifcationListener() {
    return (notification, ref) -> {
      final String type = notification.getType();
      final CompositeData data = (CompositeData) notification.getUserData();
      final GarbageCollectionNotificationInfo info = GarbageCollectionNotificationInfo.from(data);
      final String name = info.getGcName();
      final long id = info.getGcInfo().getId();
      final String action = info.getGcAction();
      final String cause = info.getGcCause();
      final long startTime = info.getGcInfo().getStartTime();
      final long endTime = info.getGcInfo().getEndTime();
      final long duration = info.getGcInfo().getDuration();

      final MemoryUsage edenSpaceBefore =
          info.getGcInfo().getMemoryUsageBeforeGc().get("G1 Eden Space");
      final MemoryUsage edenSpaceAfter =
          info.getGcInfo().getMemoryUsageAfterGc().get("G1 Eden Space");
      final MemoryUsage survivorSpaceBefore =
          info.getGcInfo().getMemoryUsageBeforeGc().get("G1 Survivor Space");
      final MemoryUsage survivorSpaceAfter =
          info.getGcInfo().getMemoryUsageAfterGc().get("G1 Survivor Space");
      final MemoryUsage oldGenBefore = info.getGcInfo().getMemoryUsageBeforeGc().get("G1 Old Gen");
      final MemoryUsage oldGenAfter = info.getGcInfo().getMemoryUsageAfterGc().get("G1 Old Gen");

      G1EventType gcEventType = G1EventType.UNKNOWN;

      /*
      Possible GC types are as follows:
          G1 Young Generation
          G1 Concurrent GC
          G1 Old Generation

          G1 Young Generation is either Young or Mixed by whether the old generation has been collected.
       */
      if ("G1 Young Generation".equals(name) && oldGenBefore.getUsed() <= oldGenAfter.getUsed()) {
        gcEventType = G1EventType.YOUNG;
      } else if ("G1 Young Generation".equals(name)
          && oldGenBefore.getUsed() > oldGenAfter.getUsed()) {
        gcEventType = G1EventType.MIXED;
      } else if ("G1 Old Generation".equals(name)) {
        gcEventType = G1EventType.FULL;
      } else if ("G1 Concurrent GC".equals(name)) {
        gcEventType = G1EventType.CONCURRENT;
      }

      logger.debug(name + " (" + type + ")");
      logger.debug("                       Id: " + id);
      logger.debug("                   Action: " + action);
      logger.debug("                    Cause: " + cause);
      logger.debug("                StartTime: " + startTime);
      logger.debug("                  EndTime: " + endTime);
      logger.debug("                 Duration: " + duration);
      logger.debug("     Eden Space Before GC: " + edenSpaceBefore.getUsed());
      logger.debug("      Eden Space After GC: " + edenSpaceAfter.getUsed());
      logger.debug(" Survivor Space Before GC: " + survivorSpaceBefore.getUsed());
      logger.debug("  Survivor Space After GC: " + survivorSpaceAfter.getUsed());
      logger.debug(" Old Generation Before GC: " + oldGenBefore.getUsed());
      logger.debug("  Old Generation After GC: " + oldGenAfter.getUsed());
      logger.debug("            GC Event Type: " + gcEventType.name());

      long usedAfter =
          edenSpaceAfter.getUsed() + survivorSpaceAfter.getUsed() + oldGenAfter.getUsed();

      long maxAfter = oldGenAfter.getMax();

      heapUsedAfterGc.set(usedAfter);
      heapUsedAfterGcPct.set((double) usedAfter / (double) maxAfter);

      if (gcEventType == G1EventType.YOUNG
          || gcEventType == G1EventType.MIXED
          || gcEventType == G1EventType.FULL) {
        long usedBefore =
            edenSpaceBefore.getUsed() + survivorSpaceBefore.getUsed() + oldGenBefore.getUsed();

        long memoryCollected = usedBefore - usedAfter;
        double percentCollected = (((double) memoryCollected) / ((double) usedBefore)) * 100;
        double utilization = (((double) usedAfter) / ((double) maxAfter)) * 100;

        bumpGcMemoryStats(
            Collector.G1.name(), gcEventType.name(), (long) utilization, (long) percentCollected);
        bumpGcDurationStats(Collector.G1.name(), gcEventType.name(), duration);
      }
    };
  }

  private NotificationListener getPSNotificationListener() {
    return (notification, ref) -> {
      final String type = notification.getType();
      final CompositeData data = (CompositeData) notification.getUserData();
      final GarbageCollectionNotificationInfo info = GarbageCollectionNotificationInfo.from(data);
      final String name = info.getGcName();
      final long id = info.getGcInfo().getId();
      final String action = info.getGcAction();
      final String cause = info.getGcCause();
      final long startTime = info.getGcInfo().getStartTime();
      final long endTime = info.getGcInfo().getEndTime();
      final long duration = info.getGcInfo().getDuration();

      final MemoryUsage edenSpaceBefore =
          info.getGcInfo().getMemoryUsageBeforeGc().get("PS Eden Space");
      final MemoryUsage edenSpaceAfter =
          info.getGcInfo().getMemoryUsageAfterGc().get("PS Eden Space");

      final MemoryUsage survivorSpaceBefore =
          info.getGcInfo().getMemoryUsageBeforeGc().get("PS Survivor Space");
      final MemoryUsage survivorSpaceAfter =
          info.getGcInfo().getMemoryUsageAfterGc().get("PS Survivor Space");

      final MemoryUsage oldGenBefore = info.getGcInfo().getMemoryUsageBeforeGc().get("PS Old Gen");
      final MemoryUsage oldGenAfter = info.getGcInfo().getMemoryUsageAfterGc().get("PS Old Gen");

      ParEventType gcEventType = ParEventType.UNKNOWN;

      /*
      Possible GC types are as follows:
          PS MarkSweep
          PS Scavenge
       */
      if ("PS MarkSweep".equals(name)) {
        gcEventType = ParEventType.MARK_SWEEP;
      } else if ("PS Scavenge".equals(name)) {
        gcEventType = ParEventType.SCAVENGE;
      }

      logger.debug(name + " (" + type + ")");
      logger.debug("                       Id: " + id);
      logger.debug("                   Action: " + action);
      logger.debug("                    Cause: " + cause);
      logger.debug("                StartTime: " + startTime);
      logger.debug("                  EndTime: " + endTime);
      logger.debug("                 Duration: " + duration);
      logger.debug("     Eden Space Before GC: " + edenSpaceBefore.getUsed());
      logger.debug("      Eden Space After GC: " + edenSpaceAfter.getUsed());
      logger.debug(" Survivor Space Before GC: " + survivorSpaceBefore.getUsed());
      logger.debug("  Survivor Space After GC: " + survivorSpaceAfter.getUsed());
      logger.debug(" Old Generation Before GC: " + oldGenBefore.getUsed());
      logger.debug("  Old Generation After GC: " + oldGenAfter.getUsed());
      logger.debug("            GC Event Type: " + gcEventType.name());

      long usedAfter =
          edenSpaceAfter.getUsed() + survivorSpaceAfter.getUsed() + oldGenAfter.getUsed();

      // -Xmx seems be equal to sum of eden, survivor and old generation
      long maxAfter = edenSpaceAfter.getMax() + survivorSpaceAfter.getMax() + oldGenAfter.getMax();

      heapUsedAfterGc.set(usedAfter);
      heapUsedAfterGcPct.set((double) usedAfter / (double) maxAfter);

      if (gcEventType == ParEventType.MARK_SWEEP || gcEventType == ParEventType.SCAVENGE) {
        long usedBefore =
            edenSpaceBefore.getUsed() + survivorSpaceBefore.getUsed() + oldGenBefore.getUsed();

        long memoryCollected = usedBefore - usedAfter;
        double percentCollected = (((double) memoryCollected) / ((double) usedBefore)) * 100;
        double utilization = (((double) usedAfter) / ((double) maxAfter)) * 100;

        bumpGcMemoryStats(
            Collector.PARALLEL.name(),
            gcEventType.name(),
            (long) utilization,
            (long) percentCollected);
        bumpGcDurationStats(Collector.PARALLEL.name(), gcEventType.name(), duration);
      }
    };
  }

  private void bumpGcMemoryStats(
      String collector, String event, long utilization, long percentCollected) {
    String prefix = "GC." + collector + "." + event;
    String heapUseAfter = prefix + ".heap_used_after_gc";
    String heapUseAfterPct = prefix + ".heap_used_after_gc_pct";
    String utilizationString = prefix + ".utilization";
    String percentCollectedString = prefix + ".percent_collected";

    incrementCounter(prefix, 1);
    incrementDistribution(heapUseAfterPct, (long) (100 * heapUsedAfterGcPct.get()));
    incrementDistribution(heapUseAfter, heapUsedAfterGc.get());
    incrementDistribution(utilizationString, utilization);
    incrementDistribution(percentCollectedString, percentCollected);
  }

  private void bumpGcDurationStats(String collector, String event, long duration) {
    String prefix = "GC." + collector + "." + event;
    String durationString = prefix + ".duration";

    incrementCounter(prefix, 1);

    incrementDistribution(durationString, duration);
  }

  private void incrementCounter(String key, long value) {
    counters.computeIfAbsent(key, (k) -> new SlidingTimeWindowMovingCounter()).add(value);
  }

  private void incrementDistribution(String key, long value) {
    distributions
        .computeIfAbsent(key, (k) -> new SingleWindowDistribution())
        .add(Math.max(value, 0));
  }

  public Map<String, Long> getCounters() {
    ImmutableMap.Builder<String, Long> metrics = ImmutableMap.builder();

    counters.forEach(
        (key, counter) -> {
          metrics.put(key + ".sum.60", counter.oneMinuteRate());
        });

    distributions.forEach(
        (key, dist) -> {
          for (Quantile q : Quantile.values()) {
            metrics.put(key + "." + q.getKey() + ".60", dist.getOneMinuteQuantiles().get(q));
          }
        });

    addBufferPoolCounters(metrics);
    addMemoryCounters(metrics);
    return metrics.build();
  }

  private void addBufferPoolCounters(ImmutableMap.Builder<String, Long> counters) {
    for (BufferPoolMXBean pool : ManagementFactory.getPlatformMXBeans(BufferPoolMXBean.class)) {
      counters.put(
          String.format("buffer_pools.%s.memory_used", pool.getName()), pool.getMemoryUsed());
      counters.put(
          String.format("buffer_pools.%s.capacity", pool.getName()), pool.getTotalCapacity());
    }
  }

  private void addMemoryCounters(ImmutableMap.Builder<String, Long> counters) {
    counters.put("memory.heap.used", memoryMXBean.getHeapMemoryUsage().getUsed());
    counters.put("memory.heap.committed", memoryMXBean.getHeapMemoryUsage().getCommitted());
    counters.put("memory.heap.total", memoryMXBean.getHeapMemoryUsage().getMax());

    counters.put("memory.nonheap.used", memoryMXBean.getNonHeapMemoryUsage().getUsed());
    counters.put("memory.nonheap.committed", memoryMXBean.getNonHeapMemoryUsage().getCommitted());
    counters.put("memory.nonheap.total", memoryMXBean.getNonHeapMemoryUsage().getMax());
  }
}
