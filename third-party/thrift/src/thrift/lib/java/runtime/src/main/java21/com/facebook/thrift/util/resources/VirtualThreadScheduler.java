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

import com.facebook.thrift.metrics.rate.SlidingTimeWindowMovingCounter;
import io.micrometer.core.instrument.DistributionSummary;
import io.micrometer.core.instrument.MeterRegistry;
import io.micrometer.core.instrument.simple.SimpleMeterRegistry;
import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.Future;
import java.util.concurrent.Semaphore;
import java.util.concurrent.ThreadFactory;
import java.util.concurrent.TimeUnit;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import reactor.core.Disposable;
import reactor.core.Disposables;
import reactor.core.Exceptions;
import reactor.core.scheduler.Scheduler;
import reactor.core.scheduler.Schedulers;

/**
 * Stats exporting scheduler that allows work to be scheduled on virtual threads. These threads are
 * managed by the JVM's internal ForkJoinPool and tasks are executed on its carrier threads. Tasks
 * will never be rejected as this scheduler does not throw exceptions when submitting tasks. Tasks
 * that are not immediately runnable are queued for execution using the ThreadPerTaskExecutor.
 * Virtual threads will be created and will wait to run until there is an available carrier thread.
 * This allows for high scalability but may consume additional memory under high load.
 *
 * <p>Notes:
 *
 * <p>These virtual threads are managed by the JVM's internal ForkJoinPool. There are three
 * configurations options that allow you to configure the properties of this ForkJoinPool: max
 * parallelism, pool size and min runnable threads.
 *
 * <pre>
 *    -Djdk.virtualThreadScheduler.parallelism
 *    -Djdk.virtualThreadScheduler.maxPoolSize
 *    -Djdk.virtualThreadScheduler.minRunnable
 * </pre>
 *
 * <p>Keep in mind that these settings affect the entire JVM and all virtual threads whether they
 * were scheduled on this scheduler or not.
 *
 * <p>The VirtualThreadScheduler can be configured with the following flags:
 *
 * <pre>
 *     (string) -Dthrift.off-loop-scheduler(=virtual-thread)
 *     (boolean)-Dthrift.virtual-thread-limit-concurrency
 *     (int) -Dthrift.virtual-thread-max-concurrency
 * </pre>
 *
 * <p>set thrift.virtual-thread-scheduler to "true" to enable the virtual thread scheduler, this
 * will disable any other ForkJoin/ThreadPool schedulers. If you wish to limit concurrency set
 * thrift.virtual-thread-limit-concurrency to true and then set
 * thrift.virtual-thread-max-concurrency to the max number of concurrent tasks. The low level
 * implementation uses a Semaphore (with strict fairness) to gate the execution of the underlying
 * runnable. This means that regardless of how many carrier threads are available only N tasks will
 * run in parallel, where N = maxConcurrency. All other virtual threads will be queued in memory
 * until a lock can be acquired.
 *
 * <p>If you do not wish to limit concurrency do not set virtual-thread-limit-concurrency and the OS
 * will limit concurrency based on JVM settings and internal ForkJoinPool, typically
 * Runtime.getRuntime().availableProcessors()
 *
 * <p>Considerations: Be cautious when limiting concurrency. Virtual threads can become pinned to
 * carrier thread and starve the application of resources in certain circumstances. Instances where
 * virtual threads can become pinned are long-running or CPU intensive tasks, synchronized blocks
 * and JNI/FFI api calls. Virtual threads cannot yield if they do not block meaning they will
 * consume the CPU available until they complete. The synchronized keyword is also not compatible
 * with virtual threads at this time and yielding will fail on blocking calls inside synchronized
 * blocks. Avoid this at all cost, consider using ReentrantLock or other java concurrent mechanisms.
 * Virtual threads will also become pinned during any native calls either JNI or FFI calls.
 *
 * <p>Other considerations include avoiding ThreadLocals if they consume a lot of memory or are
 * costly to construct. Thread locals will be created for every instance of a Virtual thread and
 * will consume memory. Consider using ScopedValues if you need to propagate context between calls
 * in the virtual thread or consider using a shared resource pool if you want to avoid creating
 * expensive or large high memory objects that need to be used across threads.
 */
public final class VirtualThreadScheduler implements ThriftScheduler {
  private static final Logger LOGGER = LoggerFactory.getLogger(VirtualThreadScheduler.class);
  private static final MeterRegistry registry = new SimpleMeterRegistry();
  private static final DistributionSummary dist =
      DistributionSummary.builder("execution_time").publishPercentiles(0.95).register(registry);
  private final SlidingTimeWindowMovingCounter createdTasksSum =
      new SlidingTimeWindowMovingCounter();
  private final Semaphore semaphore;
  private final ExecutorService executor;
  private final Scheduler periodicScheduler;

  public VirtualThreadScheduler() {
    this(false, Runtime.getRuntime().availableProcessors());
  }

  public VirtualThreadScheduler(boolean limitConcurrency, int maxConcurrency) {
    if (limitConcurrency) {
      semaphore = new Semaphore(maxConcurrency, true);
    } else {
      semaphore = null;
    }

    // Thread factory use for both immediate and scheduled executors
    ThreadFactory threadFactory =
        Thread.ofVirtual()
            .inheritInheritableThreadLocals(true)
            .name("thrift-virtual-scheduler-", 0)
            .uncaughtExceptionHandler(
                (t, e) -> LOGGER.error("Uncaught exception on thread {}", t.getName(), e))
            .factory();

    // Configure immediate virtual thread executor
    this.executor = Executors.newThreadPerTaskExecutor(threadFactory);
    this.periodicScheduler = Schedulers.newSingle("periodic-scheduler", true);
  }

  @Override
  public ExecutorService getExecutor() {
    return executor;
  }

  @Override
  public Worker createWorker() {
    return new DelegatingSchedulerWorker(this);
  }

  private ConcurrencyLimmitingMeasuringRunnable wrapTaskIfNecessary(Runnable task) {
    if (task instanceof ConcurrencyLimmitingMeasuringRunnable wrapped) {
      return wrapped;
    } else {
      return new ConcurrencyLimmitingMeasuringRunnable(task, semaphore, dist);
    }
  }

  @Override
  public Disposable schedule(Runnable task) {
    // Wrap if necessary
    var wrapped = wrapTaskIfNecessary(task);
    createdTasksSum.add(1L);
    return new DisposableFuture(executor.submit(wrapped));
  }

  @Override
  public Disposable schedule(Runnable task, long delay, TimeUnit unit) {
    if (delay <= 0) {
      return schedule(task);
    }

    // Wrap if necessary
    var wrapped = wrapTaskIfNecessary(task);

    createdTasksSum.add(1L);

    TrampolinedTask trampolinedTask = new TrampolinedTask(executor, wrapped);
    return Disposables.composite(
        periodicScheduler.schedule(trampolinedTask, delay, unit), trampolinedTask);
  }

  @Override
  public Disposable schedulePeriodically(
      Runnable task, long initialDelay, long period, TimeUnit unit) {
    // Wrap if necessary
    var wrapped = wrapTaskIfNecessary(task);

    createdTasksSum.add(1L);

    TrampolinedTask trampolinedTask = new TrampolinedTask(executor, wrapped);
    return Disposables.composite(
        periodicScheduler.schedulePeriodically(trampolinedTask, initialDelay, period, unit),
        trampolinedTask);
  }

  @Override
  public void dispose() {
    executor.shutdown();
    periodicScheduler.dispose();
  }

  @Override
  public boolean isDisposed() {
    return (executor.isShutdown() || executor.isTerminated()) && (periodicScheduler.isDisposed());
  }

  public Map<String, Long> getStats() {
    Map<String, Long> stats = new HashMap<>();

    var snapshot = dist.takeSnapshot();

    stats.put("thrift_offloop.execution_time.avg", (long) snapshot.mean());
    stats.put("thrift_offloop.execution_time.p95", (long) snapshot.percentileValues()[0].value());
    stats.put("thrift_offloop.created_tasks.sum.60", createdTasksSum.oneMinuteRate());

    return stats;
  }

  /**
   * DisposableFuture creates a disposable from the Future returned from the underlying
   * ExecutorService. We delegate cancellation to the Future task and the executor framework. This
   * means tasks will not be executed if they haven't started execution yet and threads will be
   * interrupted if they're in a blocking state/
   *
   * @param future returned from executor service.submit(...)
   */
  private record DisposableFuture(Future<?> future) implements Disposable {
    @Override
    public void dispose() {
      // Todo: @jbahr consider if we should make interruption configurable
      future.cancel(true);
    }

    @Override
    public boolean isDisposed() {
      return future.isCancelled();
    }
  }

  /**
   * Delegating worker that simply delegates any scheduler calls to the VirtualThreadScheduler's
   * underlying executor. Workers are disposable, so we need to track tasks passed to this worker
   * and dispose of them when called. We track them via Disposable.Composite list, and on shutdown
   * we set the volatile shutdown boolean to true then dispose of all work that was added to the
   * list. This will ensure the runnable is disposed of it's not already been executed
   */
  private class DelegatingSchedulerWorker implements Worker {
    private final VirtualThreadScheduler scheduler;
    private final Disposable.Composite tasks = Disposables.composite();
    private volatile boolean shutdown;

    DelegatingSchedulerWorker(VirtualThreadScheduler scheduler) {
      this.scheduler = scheduler;
    }

    private ConcurrencyLimmitingMeasuringRunnable wrapTask(Runnable task) {
      return new ConcurrencyLimmitingMeasuringRunnable(task, semaphore, dist);
    }

    @Override
    public Disposable schedule(Runnable task) {
      var wrappedTask = wrapTask(task);

      if (shutdown) {
        throw Exceptions.failWithRejected();
      }

      Disposable disposable = scheduler.schedule(wrappedTask);
      tasks.add(disposable);
      return disposable;
    }

    @Override
    public Disposable schedule(Runnable task, long delay, TimeUnit unit) {
      var wrappedTask = wrapTask(task);

      if (shutdown) {
        throw Exceptions.failWithRejected();
      }

      Disposable disposable = scheduler.schedule(wrappedTask, delay, unit);
      tasks.add(disposable);
      return disposable;
    }

    @Override
    public Disposable schedulePeriodically(
        Runnable task, long initialDelay, long period, TimeUnit unit) {
      var wrappedTask = wrapTask(task);

      if (shutdown) {
        throw Exceptions.failWithRejected();
      }

      Disposable disposable =
          scheduler.schedulePeriodically(wrappedTask, initialDelay, period, unit);
      tasks.add(disposable);
      return disposable;
    }

    @Override
    public void dispose() {
      shutdown = true;
      tasks.dispose();
    }

    @Override
    public boolean isDisposed() {
      return shutdown && tasks.isDisposed();
    }
  }

  /**
   * Decorator for the internal runnable that performs a few operations .
   *
   * <pre>
   *     1) records execution time of the delegate runnable
   *     2) may limit concurrency by accessing semaphore before running
   * </pre>
   *
   * <p>Execution time is recorded to the DistributionSummary and will include blocking time. Access
   * to the concurrency is limited by the Semaphore, if null the lock will be ignored and run will
   * be invoked immediately. If
   */
  static class ConcurrencyLimmitingMeasuringRunnable implements Runnable {
    private final Runnable runnable;
    private final Semaphore semaphore;
    private final DistributionSummary recorder;
    private final long start;

    public ConcurrencyLimmitingMeasuringRunnable(
        Runnable runnable, Semaphore semaphore, DistributionSummary recorder) {
      this.runnable = runnable;
      this.semaphore = semaphore;
      this.recorder = recorder;
      this.start = System.nanoTime();
    }

    @Override
    public void run() {
      if (maybeAcquireLock()) {
        try {
          runnable.run();
        } finally {
          recordExecutionTimeNanos(TimeUnit.NANOSECONDS.toMicros(System.nanoTime() - start));
          maybeReleaseLock();
        }
      }
    }

    /**
     * Return true if no lock or if lock is acquired, false if failed due to interrupt will skip
     * task if thread is interrupted.
     *
     * @return true if we should continue to run when no lock or not interrupted
     */
    private boolean maybeAcquireLock() {
      if (semaphore != null) {
        try {
          semaphore.acquire();
        } catch (InterruptedException e) {
          Thread.currentThread().interrupt();
          LOGGER.error("Thread interrupted while waiting for semaphore", e);
          return false;
        }
      }

      return true;
    }

    /** always release lock in finally block to ensure return of permits to semaphore */
    private void maybeReleaseLock() {
      if (semaphore != null) {
        semaphore.release();
      }
    }

    private void recordExecutionTimeNanos(long micros) {
      recorder.record(micros);
    }
  }

  /**
   * A trampoline task is a disposable runnable that is submitted as a periodic task. When the
   * periodic task is scheduled to run it delegates to the underlying Virtual Thread Per Task
   * Executor effectively putting the runnable in a virtual thread context. This is needed because
   * scheduled executors don't schedule tasks on virtual threads. This trampoline ensures the thread
   * is bounced over to a virtual thread so that all real work happens there. This class also
   * implements disposable to ensure that tasks can be cancelled before being started.
   */
  private static class TrampolinedTask implements Runnable, Disposable {
    private final ExecutorService executor;
    private final Runnable task;
    private volatile boolean disposed;

    public TrampolinedTask(ExecutorService executor, Runnable task) {
      this.executor = executor;
      this.task = task;
    }

    @Override
    public void run() {
      executor.submit(
          () -> {
            if (!isDisposed()) {
              task.run();
            }
          });
    }

    @Override
    public void dispose() {
      disposed = true;
    }

    @Override
    public boolean isDisposed() {
      return disposed;
    }
  }
}
