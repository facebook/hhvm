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

import java.time.Duration;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicLong;
import org.junit.Ignore;
import org.junit.Test;
import reactor.core.publisher.Flux;
import reactor.core.publisher.Mono;
import reactor.core.scheduler.Scheduler;
import reactor.test.StepVerifier;

public class ForkJoinPoolSchedulerTest extends AbstractSchedulerTest {

  @Ignore("Flaky")
  @Test(timeout = 5000)
  public void scheduleThenDisposeOfScheduler() throws Exception {
    Scheduler s = ForkJoinPoolScheduler.create("test", 1);

    CountDownLatch latch = new CountDownLatch(1);

    s.schedule(latch::countDown, 200, TimeUnit.MILLISECONDS);

    assertThat(s.isDisposed()).isFalse();
    s.dispose();
    assertThat(s.isDisposed()).isTrue();
    assertThat(latch.await(100, TimeUnit.MILLISECONDS)).isFalse();
  }

  @Ignore("Flaky")
  @Test(timeout = 5000)
  public void scheduleThenDisposeOfWorker() throws Exception {
    Scheduler s = scheduler();

    CountDownLatch latch = new CountDownLatch(1);

    Scheduler.Worker worker = s.createWorker();

    worker.schedule(latch::countDown, 50, TimeUnit.MILLISECONDS);

    assertThat(worker.isDisposed()).isFalse();
    worker.dispose();
    assertThat(worker.isDisposed()).isTrue();
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

  @Ignore("Flaky")
  @Test
  public void smokeTestDelay() {
    for (int i = 0; i < 20; i++) {
      Scheduler s = ForkJoinPoolScheduler.create("test");
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

  @Ignore("Flaky")
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

  @Override
  protected Scheduler scheduler() {
    return ForkJoinPoolScheduler.create("test");
  }

  @Override
  protected boolean shouldCheckInterrupted() {
    return true;
  }
}
