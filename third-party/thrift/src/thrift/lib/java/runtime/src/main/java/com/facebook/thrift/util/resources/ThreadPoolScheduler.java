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

package com.facebook.thrift.util.resources;

import io.airlift.stats.DecayCounter;
import io.airlift.stats.Distribution;
import io.airlift.stats.ExponentialDecay;
import io.netty.util.internal.PlatformDependent;
import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.ThreadFactory;
import java.util.concurrent.ThreadPoolExecutor;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.concurrent.atomic.AtomicInteger;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import reactor.core.Disposable;
import reactor.core.scheduler.Scheduler;
import reactor.core.scheduler.Schedulers;

/**
 * Scheduler that lets you wrap a ThreadPoolScheduler without throwing an exception. Unlike other
 * reactor-core schedulers this will *not* create a new Worker to assign work to a particular
 * thread. All tasks are submitted to the underlying executor.
 */
final class ThreadPoolScheduler extends AtomicBoolean implements StatsScheduler {
  private static final Logger LOGGER = LoggerFactory.getLogger(ThreadPoolScheduler.class);

  private final Distribution executionTime;
  private final Map<String, Distribution> perThreadExecutionTimes;

  private final Distribution poolSizeAvg;
  private final Distribution pendingTasksAvg;
  private final Distribution activeTasksAvg;
  private final DecayCounter completedTasksSum;

  private final ThreadPoolExecutor threadPoolExecutor;
  private final Worker worker;
  private final Scheduler scheduler;

  public ThreadPoolScheduler(
      int minNumThreads,
      int maxNumThreads,
      int maxPendingTasks,
      int minPendingTasksBeforeNewThread) {
    this.executionTime = new Distribution(ExponentialDecay.oneMinute());
    this.perThreadExecutionTimes = PlatformDependent.newConcurrentHashMap();

    this.poolSizeAvg = new Distribution(ExponentialDecay.oneMinute());
    this.pendingTasksAvg = new Distribution(ExponentialDecay.oneMinute());
    this.activeTasksAvg = new Distribution(ExponentialDecay.oneMinute());
    this.completedTasksSum = new DecayCounter(ExponentialDecay.oneMinute());

    ThreadPoolSchedulerThreadFactory threadFactory =
        new ThreadPoolSchedulerThreadFactory(
            "thrift-offloop",
            executionTime,
            perThreadExecutionTimes,
            (t, e) -> LOGGER.error("uncaught exception on thread {}", t.getName(), e));

    this.threadPoolExecutor =
        createThreadPoolExecutor(
            minNumThreads,
            maxNumThreads,
            minPendingTasksBeforeNewThread,
            threadFactory,
            maxPendingTasks);
    threadPoolExecutor.prestartAllCoreThreads();

    this.worker = new ThreadPoolSchedulerWorker();
    this.scheduler = Schedulers.newElastic("thrift-offloop-scheduler", 60, true);

    scheduler.schedulePeriodically(this::captureThreadPoolExecutorMetrics, 1, 1, TimeUnit.SECONDS);
  }

  private static ThreadPoolExecutor createThreadPoolExecutor(
      int minThreads,
      int maxNumThreads,
      int minPendingTasksBeforeNewThread,
      ThreadFactory threadFactory,
      int maxPendingTasks) {
    return new ThreadPoolExecutor(
        minThreads,
        maxNumThreads,
        60_000L,
        TimeUnit.MILLISECONDS,
        new ThreadPoolSchedulerQueue(minPendingTasksBeforeNewThread),
        threadFactory,
        (r, executor) -> {
          try {
            if (executor.getQueue().size() < maxPendingTasks) {
              executor.getQueue().put(r);
            } else {
              LOGGER.error("rejecting task because max pending tasks of " + maxPendingTasks);
            }
          } catch (InterruptedException e) {
            Thread.currentThread().interrupt();
          }
        });
  }

  private void captureThreadPoolExecutorMetrics() {
    poolSizeAvg.add(threadPoolExecutor.getPoolSize());
    pendingTasksAvg.add(threadPoolExecutor.getQueue().size());
    activeTasksAvg.add(threadPoolExecutor.getActiveCount());
    completedTasksSum.add(threadPoolExecutor.getCompletedTaskCount());
  }

  @Override
  public Disposable schedule(Runnable task) {
    DisposableExecutionMeasuringRunnable runnable = new DisposableExecutionMeasuringRunnable(task);
    threadPoolExecutor.submit(runnable);
    return runnable;
  }

  @Override
  public Worker createWorker() {
    return worker;
  }

  @Override
  public Disposable schedule(Runnable task, long delay, TimeUnit unit) {
    if (delay <= 0) {
      return schedule(task);
    }

    return scheduler.schedule(task, delay, unit);
  }

  @Override
  public Disposable schedulePeriodically(
      Runnable task, long initialDelay, long period, TimeUnit unit) {
    if (initialDelay <= 0 && period <= 0) {
      return schedule(task);
    }

    return scheduler.schedulePeriodically(task, initialDelay, period, unit);
  }

  @Override
  public void dispose() {
    try {
      threadPoolExecutor.shutdown();
    } finally {
      scheduler.dispose();
    }
  }

  @Override
  public boolean isDisposed() {
    return threadPoolExecutor.isShutdown()
        || threadPoolExecutor.isTerminated()
        || threadPoolExecutor.isTerminating();
  }

  public Map<String, Long> getStats() {
    Map<String, Long> stats = new HashMap<>();

    double result = poolSizeAvg.getAvg();
    stats.put(
        "thrift_offloop.pool_size.avg.60", Double.isInfinite(result) ? 0L : Math.round(result));

    result = pendingTasksAvg.getAvg();
    stats.put(
        "thrift_offloop.pending_tasks.avg.60", Double.isInfinite(result) ? 0L : Math.round(result));

    result = activeTasksAvg.getAvg();
    stats.put(
        "thrift_offloop.active_tasks.avg.60", Double.isInfinite(result) ? 0L : Math.round(result));

    result = completedTasksSum.getCount();
    stats.put(
        "thrift_offloop.complete_tasks.sum.60",
        Double.isInfinite(result) ? 0L : Math.round(result));

    stats.put("thrift_offloop.execution_time.avg", (long) executionTime.getP50());
    stats.put("thrift_offloop.execution_time.p90", (long) executionTime.getP90());

    return stats;
  }

  private class ThreadPoolSchedulerWorker implements Worker {
    @Override
    public Disposable schedule(Runnable task) {
      return ThreadPoolScheduler.this.schedule(task);
    }

    @Override
    public Disposable schedule(Runnable task, long delay, TimeUnit unit) {
      return ThreadPoolScheduler.this.schedule(task, delay, unit);
    }

    @Override
    public Disposable schedulePeriodically(
        Runnable task, long initialDelay, long period, TimeUnit unit) {
      return ThreadPoolScheduler.this.schedulePeriodically(task, initialDelay, period, unit);
    }

    @Override
    public void dispose() {}
  }

  private static class ThreadPoolSchedulerThreadFactory extends AtomicInteger
      implements ThreadFactory {
    private final Distribution executionTime;
    private final Map<String, Distribution> perThreadExecutionTimes;
    private final Thread.UncaughtExceptionHandler uncaughtExceptionHandler;
    private final String prefix;

    public ThreadPoolSchedulerThreadFactory(
        String prefix,
        Distribution executionTime,
        Map<String, Distribution> perThreadExecutionTimes,
        Thread.UncaughtExceptionHandler uncaughtExceptionHandler) {
      this.prefix = prefix;
      this.executionTime = executionTime;
      this.perThreadExecutionTimes = perThreadExecutionTimes;
      this.uncaughtExceptionHandler = uncaughtExceptionHandler;
    }

    @Override
    public Thread newThread(Runnable r) {
      String name = prefix + getAndDecrement();
      Distribution perThreadExecutionTime = new Distribution(ExponentialDecay.oneMinute());
      perThreadExecutionTimes.put(name, perThreadExecutionTime);

      ExecutionRecordingThread t =
          new ExecutionRecordingThread(r, perThreadExecutionTime, executionTime);
      t.setUncaughtExceptionHandler(uncaughtExceptionHandler);
      t.setName(prefix + getAndDecrement());
      t.setDaemon(true);
      return t;
    }
  }
}
