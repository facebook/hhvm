/*
 * Copyright (c) 2017-2021 VMware Inc. or its affiliates, All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   https://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
package com.facebook.thrift.util.resources;

import static com.facebook.thrift.util.resources.ResourceConfiguration.capForkJoinThreads;
import static reactor.core.Exceptions.unwrap;

import java.util.ArrayDeque;
import java.util.Map;
import java.util.Objects;
import java.util.Queue;
import java.util.concurrent.Executor;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.ForkJoinPool;
import java.util.concurrent.ForkJoinTask;
import java.util.concurrent.ForkJoinWorkerThread;
import java.util.concurrent.RejectedExecutionException;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicLong;
import java.util.function.BiConsumer;
import java.util.function.BooleanSupplier;
import org.slf4j.LoggerFactory;
import reactor.core.Disposable;
import reactor.core.Exceptions;
import reactor.core.scheduler.Scheduler;
import reactor.core.scheduler.Schedulers;
import reactor.util.Logger;
import reactor.util.Loggers;

public final class ForkJoinPoolScheduler implements ThriftScheduler {
  private static final org.slf4j.Logger LOGGER =
      LoggerFactory.getLogger(ForkJoinPoolScheduler.class);

  private ForkJoinPoolSchedulerStats stats;

  private static volatile BiConsumer<Thread, ? super Throwable> onHandleErrorHook;

  /**
   * {@link Scheduler} that hosts a fixed pool of single-threaded ExecutorService-based workers and
   * is suited for parallel work.
   *
   * @param name Thread prefix
   * @return a new {@link Scheduler} that hosts a fixed pool of single-threaded
   *     ExecutorService-based workers and is suited for parallel work
   */
  public static ForkJoinPoolScheduler create(String name) {
    return create(name, Runtime.getRuntime().availableProcessors());
  }

  /**
   * {@link Scheduler} that utilizes a {@link ForkJoinPool} for workers and is suited for parallel
   * work. Since the ForkJoinPool does not support periodic or delayed scheduling, a single
   * Scheduler is used to enqueue any tasks that are delayed or periodic
   *
   * @param name Thread prefix
   * @param parallelism Number of worker threads
   * @return a new {@link Scheduler} that utilizes a ForkJoinPool
   */
  public static ForkJoinPoolScheduler create(String name, int parallelism) {
    return new ForkJoinPoolScheduler(
        name,
        parallelism,
        new SchedulerForkJoinWorkerThreadFactory(name, COUNTER),
        Schedulers.newSingle(name + "-timer", true),
        true);
  }

  static void handleError(Throwable ex) {
    Thread thread = Thread.currentThread();
    Throwable t = unwrap(ex);
    Exceptions.throwIfJvmFatal(t);
    Thread.UncaughtExceptionHandler x = thread.getUncaughtExceptionHandler();
    if (x != null) {
      x.uncaughtException(thread, t);
    } else {
      log.error("Scheduler worker failed with an uncaught exception", t);
    }
    if (onHandleErrorHook != null) {
      onHandleErrorHook.accept(thread, t);
    }
  }

  /**
   * Define a hook that is executed when a {@link Scheduler} has {@link #handleError(Throwable)
   * handled an error}. Note that it is executed after the error has been passed to the thread
   * uncaughtErrorHandler, which is not the case when a fatal error occurs (see {@link
   * Exceptions#throwIfJvmFatal(Throwable)}).
   *
   * @param c the new hook to set.
   */
  public static void onHandleError(BiConsumer<Thread, ? super Throwable> c) {
    log.info("Hooking new: onHandleError");
    onHandleErrorHook = Objects.requireNonNull(c, "onHandleError");
  }

  /** Reset the {@link #onHandleError(BiConsumer)} hook to the default no-op behavior. */
  public static void resetOnHandleError() {
    log.info("Reset to default: onHandleError");
    onHandleErrorHook = null;
  }

  private final ForkJoinPool pool;
  private final Scheduler scheduler;
  private final boolean disposeScheduler;

  /**
   * Construct a new instance
   *
   * @param parallelism Parallelism. Number of fork-join pool threads. See {@link ForkJoinPool}
   * @param workerThreadFactory Thread factory for fork-join worker threads
   * @param scheduler Scheduler to use for time-based scheduling
   */
  private ForkJoinPoolScheduler(
      String name,
      int parallelism,
      ForkJoinPool.ForkJoinWorkerThreadFactory workerThreadFactory,
      Scheduler scheduler,
      boolean disposeScheduler) {
    this.stats = new ForkJoinPoolSchedulerStats(name);
    this.scheduler = scheduler;
    this.disposeScheduler = disposeScheduler;
    this.pool =
        ForkJoinPoolFactory.create(
            parallelism, workerThreadFactory, this::uncaughtException, capForkJoinThreads);
  }

  public Map<String, Long> getStats() {
    return stats.getStats();
  }

  @Override
  public Scheduler.Worker createWorker() {
    return new Worker(stats, pool, scheduler);
  }

  @Override
  public void dispose() {
    if (disposeScheduler) {
      scheduler.dispose();
    }
    pool.shutdownNow();
  }

  @Override
  public boolean isDisposed() {
    return pool.isShutdown();
  }

  @Override
  public Disposable schedule(Runnable runnable) {
    // If ContextPropagation is enabled, wrap the runnable with ContextPropRunnable
    Runnable wrappedTask = runnable;
    if (ContextPropagationRegistry.isContextPropEnabled()) {
      wrappedTask = new ContextPropRunnable(runnable);
    }
    return new DisposableForkJoinTask(stats, pool.submit(wrappedTask));
  }

  @Override
  public Disposable schedule(Runnable task, long delay, TimeUnit unit) {
    if (delay == 0) {
      return schedule(task);
    }
    TrampolinedTask trampolinedTask = new TrampolinedTask(stats, pool, task, NO_PARENT);
    return new CompositeDisposable(
        scheduler.schedule(trampolinedTask, delay, unit), trampolinedTask);
  }

  @Override
  public Disposable schedulePeriodically(
      Runnable task, long initialDelay, long period, TimeUnit unit) {
    TrampolinedTask trampolinedTask = new TrampolinedTask(stats, pool, task, NO_PARENT);

    return new CompositeDisposable(
        scheduler.schedulePeriodically(trampolinedTask, initialDelay, period, unit),
        trampolinedTask);
  }

  private void uncaughtException(Thread t, Throwable e) {
    log.error(
        "Scheduler worker in group "
            + t.getThreadGroup().getName()
            + " failed with an uncaught exception",
        e);
  }

  @Override
  public ExecutorService getExecutor() {
    return pool;
  }

  private static class DisposableForkJoinTask implements Disposable {

    private final ForkJoinPoolSchedulerStats stats;
    private final ForkJoinTask<?> task;

    DisposableForkJoinTask(ForkJoinPoolSchedulerStats stats, ForkJoinTask<?> task) {
      this.stats = stats;
      this.task = task;
    }

    @Override
    public void dispose() {
      task.cancel(false);
    }

    @Override
    public boolean isDisposed() {
      return task.isDone();
    }
  }

  private static class TrampolinedTask implements Runnable, Disposable {
    private final ForkJoinPoolSchedulerStats stats;

    private final long start;
    private final Executor executor;
    private final Runnable task;
    private final BooleanSupplier isParentDisposed;
    private volatile boolean disposed;

    public TrampolinedTask(
        ForkJoinPoolSchedulerStats stats,
        Executor executor,
        Runnable task,
        BooleanSupplier isParentDisposed) {
      this.stats = stats;
      this.start = stats.incrementPendingAndStartRecordingTime();
      this.executor = executor;
      this.task = task;
      this.isParentDisposed = isParentDisposed;
    }

    @Override
    public void dispose() {
      disposed = true;
    }

    @Override
    public boolean isDisposed() {
      return disposed || isParentDisposed.getAsBoolean();
    }

    @Override
    public void run() {
      executor.execute(
          () -> {
            if (!isDisposed()) {
              try {
                stats.incrementActiveTasks();
                task.run();
              } finally {
                stats.incrementCompletedTasksAndRecordTime(start);
              }
            } else {
              stats.incrementDisposedTasksAndRecordTime(start);
            }
          });
    }
  }

  static final class CompositeDisposable implements Disposable {

    final Disposable a;
    final Disposable b;

    CompositeDisposable(Disposable a, Disposable b) {
      this.a = a;
      this.b = b;
    }

    @Override
    public void dispose() {
      a.dispose();
      b.dispose();
    }

    @Override
    public boolean isDisposed() {
      return a.isDisposed() && b.isDisposed();
    }
  }

  private static class Worker implements Scheduler.Worker {
    private final ForkJoinPoolSchedulerStats stats;

    private final Executor executor;
    private final Scheduler scheduler;
    private final Object lock = new Object();
    private final Queue<Runnable> tasks = new ArrayDeque<>();
    private boolean executing = false;
    private final Runnable processTaskQueue = this::processTaskQueue;
    private final Executor workerExecutor = this::execute;
    private volatile boolean shutdown;
    private final BooleanSupplier isDisposed = this::isDisposed;

    Worker(ForkJoinPoolSchedulerStats stats, Executor executor, Scheduler scheduler) {
      this.stats = stats;
      this.executor = executor;
      this.scheduler = scheduler;
    }

    @Override
    public void dispose() {
      if (shutdown) {
        return;
      }

      shutdown = true;

      synchronized (lock) {
        tasks.clear();
      }
    }

    @Override
    public boolean isDisposed() {
      return shutdown;
    }

    @Override
    public Disposable schedule(Runnable task) {
      if (shutdown) {
        throw Exceptions.failWithRejected();
      }

      DisposableWorkerTask workerTask = new DisposableWorkerTask(stats, task, isDisposed);

      try {
        execute(workerTask);
      } catch (RejectedExecutionException ignored) {
        // dispose the task since it made it into the queue
        workerTask.dispose();
        throw ignored;
      }

      return workerTask;
    }

    @Override
    public Disposable schedule(Runnable task, long delay, TimeUnit unit) {
      if (delay == 0) {
        return schedule(task);
      }

      if (shutdown) {
        throw Exceptions.failWithRejected();
      }

      TrampolinedTask trampolinedTask =
          new TrampolinedTask(stats, workerExecutor, task, isDisposed);
      return new CompositeDisposable(
          scheduler.schedule(trampolinedTask, delay, unit), trampolinedTask);
    }

    @Override
    public Disposable schedulePeriodically(
        Runnable task, long initialDelay, long period, TimeUnit unit) {
      if (shutdown) {
        throw Exceptions.failWithRejected();
      }

      TrampolinedTask trampolinedTask =
          new TrampolinedTask(stats, workerExecutor, task, isDisposed);

      return new CompositeDisposable(
          scheduler.schedulePeriodically(trampolinedTask, initialDelay, period, unit),
          trampolinedTask);
    }

    private void execute(Runnable command) {
      boolean schedule;
      synchronized (lock) {
        tasks.add(command);

        if (executing) {
          schedule = false;
        } else {
          executing = true;
          schedule = true;
        }
      }

      if (schedule) {
        executor.execute(processTaskQueue);
      }
    }

    private void processTaskQueue() {
      while (true) {
        Runnable task;
        synchronized (lock) {
          task = tasks.poll();
          if (task == null) {
            executing = false;
            return;
          }
        }

        try {
          task.run();
        } catch (Throwable ex) {
          handleError(ex);
        }
      }
    }
  }

  private static class DisposableWorkerTask implements Disposable, Runnable {
    private final ForkJoinPoolSchedulerStats stats;

    private final Runnable task;
    private final BooleanSupplier isParentDisposed;

    private final long start;
    private volatile boolean disposed;

    private DisposableWorkerTask(
        ForkJoinPoolSchedulerStats stats, Runnable task, BooleanSupplier disposed) {
      this.stats = stats;
      this.start = stats.incrementPendingAndStartRecordingTime();
      this.task = task;
      isParentDisposed = disposed;
    }

    @Override
    public void dispose() {
      disposed = true;
    }

    @Override
    public boolean isDisposed() {
      return disposed || isParentDisposed.getAsBoolean();
    }

    @Override
    public void run() {
      if (isDisposed()) {
        stats.incrementDisposedTasksAndRecordTime(start);
        return;
      }
      try {
        stats.incrementActiveTasks();
        task.run();
      } finally {
        stats.incrementCompletedTasksAndRecordTime(start);
      }
    }
  }

  private static final class SchedulerForkJoinWorkerThread extends FastThreadLocalThread {

    SchedulerForkJoinWorkerThread(String name, ForkJoinPool pool) {
      super(pool);
      setName(name);
    }
  }

  private static final class SchedulerForkJoinWorkerThreadFactory
      implements ForkJoinPool.ForkJoinWorkerThreadFactory {

    final String name;
    final AtomicLong COUNTER;

    SchedulerForkJoinWorkerThreadFactory(String name, AtomicLong COUNTER) {
      this.name = name;
      this.COUNTER = COUNTER;
    }

    @Override
    public ForkJoinWorkerThread newThread(ForkJoinPool pool) {
      return new SchedulerForkJoinWorkerThread(name + "-" + COUNTER.incrementAndGet(), pool);
    }
  }

  private static final AtomicLong COUNTER = new AtomicLong();
  private static final Logger log = Loggers.getLogger(Schedulers.class);
  private static final BooleanSupplier NO_PARENT = () -> false;
}
