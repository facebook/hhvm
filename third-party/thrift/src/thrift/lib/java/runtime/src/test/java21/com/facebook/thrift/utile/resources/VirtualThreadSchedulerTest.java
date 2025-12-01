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

import static org.assertj.core.api.Assertions.assertThat;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;

import java.time.Duration;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.concurrent.atomic.AtomicLong;
import java.util.concurrent.atomic.AtomicReference;
import org.junit.Test;
import reactor.core.Disposable;
import reactor.core.publisher.Flux;
import reactor.core.publisher.Mono;
import reactor.core.scheduler.Scheduler;
import reactor.test.StepVerifier;

public class VirtualThreadSchedulerTest extends AbstractSchedulerTest {

  @Test(timeout = 5000)
  public void scheduleThenDisposeOfScheduler() throws Exception {
    Scheduler s = new VirtualThreadScheduler();

    CountDownLatch latch = new CountDownLatch(1);

    s.schedule(latch::countDown, 200, TimeUnit.MILLISECONDS);

    assertThat(s.isDisposed()).isFalse();
    s.dispose();
    assertThat(s.isDisposed()).isTrue();
    assertThat(latch.await(100, TimeUnit.MILLISECONDS)).isFalse();
  }

  @Test
  public void scheduledDoesntReject() {
    Scheduler s = scheduler();

    assertThat(s.schedule(() -> {}, 100, TimeUnit.MILLISECONDS))
        .describedAs("direct delayed scheduling")
        .isNotNull();
    assertThat(s.schedulePeriodically(() -> {}, 100, 100, TimeUnit.MILLISECONDS))
        .describedAs("direct periodic scheduling")
        .isNotNull();

    Scheduler.Worker w = s.createWorker();
    assertThat(w.schedule(() -> {}, 100, TimeUnit.MILLISECONDS))
        .describedAs("worker delayed scheduling")
        .isNotNull();
    assertThat(w.schedulePeriodically(() -> {}, 100, 100, TimeUnit.MILLISECONDS))
        .describedAs("worker periodic scheduling")
        .isNotNull();
  }

  @Test
  public void smokeTestDelay() {
    for (int i = 0; i < 20; i++) {
      Scheduler s = scheduler();
      AtomicLong start = new AtomicLong();
      AtomicLong end = new AtomicLong();

      try {
        StepVerifier.create(
                Mono.delay(Duration.ofMillis(100), s)
                    .log()
                    .doOnSubscribe(sub -> start.set(System.nanoTime()))
                    .doOnSuccess(v -> end.set(System.nanoTime()))
                    .doOnError(e -> end.set(System.nanoTime())))
            .expectSubscription()
            .expectNext(0L)
            .verifyComplete();

        long endValue = end.longValue();
        long startValue = start.longValue();
        long measuredDelay = endValue - startValue;
        long measuredDelayMs = TimeUnit.NANOSECONDS.toMillis(measuredDelay);
        assertThat(measuredDelayMs)
            .as(
                "iteration %s, measured delay %s nanos, start at %s nanos, end at %s nanos",
                i, measuredDelay, startValue, endValue)
            .isGreaterThanOrEqualTo(100L)
            .isLessThan(200L);
      } finally {
        s.dispose();
      }
    }
  }

  @Test
  public void smokeTestInterval() {
    Scheduler s = scheduler();

    try {
      StepVerifier.create(Flux.interval(Duration.ofMillis(100), Duration.ofMillis(200), s))
          .expectSubscription()
          .expectNoEvent(Duration.ofMillis(100))
          .expectNext(0L)
          .expectNoEvent(Duration.ofMillis(200))
          .expectNext(1L)
          .expectNoEvent(Duration.ofMillis(200))
          .expectNext(2L)
          .thenCancel();
    } finally {
      s.dispose();
    }
  }

  @Test
  public void testDisposableWorkerDisposesOfTasks() throws InterruptedException {
    Scheduler.Worker worker = scheduler().createWorker();

    // Task is to count down this latch
    CountDownLatch latch = new CountDownLatch(1);
    Disposable disposable = worker.schedule(latch::countDown, 100, TimeUnit.MILLISECONDS);

    assertFalse(disposable.isDisposed());
    // Ensure tht the worker is disposed, including all tasks
    worker.dispose();
    // Task should be disposed of when worker is disposed of
    assertTrue(disposable.isDisposed());
    // assert task was not run
    assertEquals(latch.getCount(), 1);

    // Assert that when task never ran, even after waiting
    assertThat(latch.await(300, TimeUnit.MILLISECONDS)).isFalse();
  }

  @Test(timeout = 10000)
  public void workerExecutesTasksSerially() throws Exception {
    Scheduler s = scheduler();
    Scheduler.Worker w = s.createWorker();

    try {
      AtomicInteger concurrentTasks = new AtomicInteger(0);
      AtomicInteger maxConcurrency = new AtomicInteger(0);
      List<Integer> executionOrder = Collections.synchronizedList(new ArrayList<>());
      CountDownLatch latch = new CountDownLatch(10);

      for (int i = 0; i < 10; i++) {
        final int taskId = i;
        w.schedule(
            () -> {
              int concurrent = concurrentTasks.incrementAndGet();
              maxConcurrency.updateAndGet(max -> Math.max(max, concurrent));
              executionOrder.add(taskId);

              try {
                // Simulate some work
                Thread.sleep(10);
              } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
              }

              concurrentTasks.decrementAndGet();
              latch.countDown();
            });
      }

      assertThat(latch.await(5, TimeUnit.SECONDS)).as("All tasks should complete").isTrue();

      // Verify serialization: max concurrency should be 1
      assertThat(maxConcurrency.get())
          .as("Tasks should execute serially (max concurrency = 1)")
          .isEqualTo(1);

      // Verify FIFO order
      assertThat(executionOrder)
          .as("Tasks should execute in FIFO order")
          .containsExactly(0, 1, 2, 3, 4, 5, 6, 7, 8, 9);
    } finally {
      w.dispose();
      s.dispose();
    }
  }

  @Test(timeout = 10000)
  public void workerDoesNotSpawnMultipleQueueProcessors() throws Exception {
    Scheduler s = scheduler();
    Scheduler.Worker w = s.createWorker();

    try {
      ConcurrentHashMap<Thread, Boolean> processorThreads = new ConcurrentHashMap<>();
      CountDownLatch allScheduled = new CountDownLatch(1);
      CountDownLatch startExecution = new CountDownLatch(1);
      CountDownLatch allComplete = new CountDownLatch(50);

      // Schedule many tasks rapidly
      for (int i = 0; i < 50; i++) {
        w.schedule(
            () -> {
              processorThreads.put(Thread.currentThread(), true);
              if (allScheduled.getCount() > 0) {
                allScheduled.countDown();
              }
              try {
                startExecution.await(5, TimeUnit.SECONDS);
              } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
              }
              allComplete.countDown();
            });
      }

      // Wait for at least one task to start
      assertThat(allScheduled.await(2, TimeUnit.SECONDS))
          .as("At least one task should start executing")
          .isTrue();

      // Give time for any potential additional processors to spawn
      Thread.sleep(100);

      // Verify only ONE queue processor thread was used
      assertThat(processorThreads.size())
          .as("Should only have one queue processor thread")
          .isEqualTo(1);

      // Let tasks complete
      startExecution.countDown();
      assertThat(allComplete.await(5, TimeUnit.SECONDS)).as("All tasks should complete").isTrue();
    } finally {
      w.dispose();
      s.dispose();
    }
  }

  @Test(timeout = 5000)
  public void tasksRunOnVirtualThreads() throws Exception {
    Scheduler s = scheduler();

    try {
      AtomicReference<Thread> threadRef = new AtomicReference<>();
      CountDownLatch latch = new CountDownLatch(1);

      s.schedule(
          () -> {
            threadRef.set(Thread.currentThread());
            latch.countDown();
          });

      assertThat(latch.await(2, TimeUnit.SECONDS)).as("Task should execute").isTrue();

      Thread t = threadRef.get();
      assertThat(t).as("Thread should not be null").isNotNull();

      // Verify it's a virtual thread
      assertThat(t.isVirtual()).as("Task should run on a virtual thread").isTrue();
    } finally {
      s.dispose();
    }
  }

  @Test(timeout = 10000)
  public void workerDelayedTasksMaintainSerialization() throws Exception {
    Scheduler s = scheduler();
    Scheduler.Worker w = s.createWorker();

    try {
      AtomicInteger concurrentTasks = new AtomicInteger(0);
      AtomicInteger maxConcurrency = new AtomicInteger(0);
      List<Integer> executionOrder = Collections.synchronizedList(new ArrayList<>());
      CountDownLatch latch = new CountDownLatch(5);

      // Mix immediate and delayed tasks
      w.schedule(
          () -> {
            trackExecution(0, concurrentTasks, maxConcurrency, executionOrder, latch);
          });

      w.schedule(
          () -> {
            trackExecution(1, concurrentTasks, maxConcurrency, executionOrder, latch);
          },
          50,
          TimeUnit.MILLISECONDS);

      w.schedule(
          () -> {
            trackExecution(2, concurrentTasks, maxConcurrency, executionOrder, latch);
          });

      w.schedule(
          () -> {
            trackExecution(3, concurrentTasks, maxConcurrency, executionOrder, latch);
          },
          30,
          TimeUnit.MILLISECONDS);

      w.schedule(
          () -> {
            trackExecution(4, concurrentTasks, maxConcurrency, executionOrder, latch);
          });

      assertThat(latch.await(5, TimeUnit.SECONDS)).as("All tasks should complete").isTrue();

      // Verify serialization: max concurrency should be 1
      assertThat(maxConcurrency.get())
          .as("Tasks (including delayed) should execute serially")
          .isEqualTo(1);

      // Immediate tasks (0, 2, 4) should run first in FIFO order,
      // then delayed tasks in their delay order (3@30ms, 1@50ms)
      assertThat(executionOrder)
          .as("Tasks should execute respecting both queue order and delays")
          .containsExactly(0, 2, 4, 3, 1);
    } finally {
      w.dispose();
      s.dispose();
    }
  }

  private void trackExecution(
      int taskId,
      AtomicInteger concurrentTasks,
      AtomicInteger maxConcurrency,
      List<Integer> executionOrder,
      CountDownLatch latch) {
    int concurrent = concurrentTasks.incrementAndGet();
    maxConcurrency.updateAndGet(max -> Math.max(max, concurrent));
    executionOrder.add(taskId);

    try {
      Thread.sleep(10);
    } catch (InterruptedException e) {
      Thread.currentThread().interrupt();
    }

    concurrentTasks.decrementAndGet();
    latch.countDown();
  }

  @Override
  protected Scheduler scheduler() {
    return new VirtualThreadScheduler();
  }

  @Override
  protected boolean shouldCheckInterrupted() {
    return true;
  }
}
