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

package com.facebook.thrift.util;

import com.facebook.thrift.util.resources.RpcResources;
import io.netty.util.Timeout;
import io.netty.util.Timer;
import io.netty.util.TimerTask;
import java.time.Duration;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Set;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.TimeoutException;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.concurrent.atomic.AtomicReference;
import java.util.concurrent.locks.LockSupport;
import org.junit.Assert;
import org.junit.Test;
import reactor.core.Disposable;
import reactor.core.publisher.Flux;
import reactor.core.publisher.Mono;
import reactor.core.publisher.Sinks;
import reactor.core.scheduler.Scheduler;
import reactor.core.scheduler.Schedulers;
import reactor.test.StepVerifier;
import reactor.util.context.Context;

public class MonoTimeoutTransformerTest {
  @Test
  public void testMultipleSubscriptionsToSingleTransformer() {
    MonoTimeoutTransformer<Integer> transformer =
        new MonoTimeoutTransformer<>(
            RpcResources.getClientOffLoopScheduler(), 1, TimeUnit.MILLISECONDS);

    StepVerifier.create(Mono.just(1).transform(transformer)).expectNext(1).verifyComplete();
    StepVerifier.create(Mono.delay(Duration.ofSeconds(1)).then(Mono.just(1)).transform(transformer))
        .verifyError(TimeoutException.class);
    StepVerifier.create(Mono.just(1).transform(transformer)).expectNext(1).verifyComplete();
    StepVerifier.create(Mono.delay(Duration.ofSeconds(1)).then(Mono.just(1)).transform(transformer))
        .verifyError(TimeoutException.class);
  }

  @Test
  public void testMonoTimeoutTransformerWhenFluxIsBlocked() {
    Mono<Object> transform =
        Mono.fromRunnable(LockSupport::park)
            .transform(
                new MonoTimeoutTransformer<>(
                    RpcResources.getClientOffLoopScheduler(), 1, TimeUnit.MILLISECONDS))
            .subscribeOn(RpcResources.getOffLoopScheduler());

    StepVerifier.create(transform).verifyError(TimeoutException.class);
  }

  @Test
  public void testMonoTimeoutTransformerBeingCalledParallel() {
    int count = 1_000;
    Mono<Integer> reduce =
        Flux.range(0, count)
            .parallel()
            .runOn(Schedulers.parallel())
            .flatMap(
                i -> {
                  if (i % 2 == 0) {
                    return Mono.just(1)
                        .transform(
                            new MonoTimeoutTransformer<>(
                                RpcResources.getClientOffLoopScheduler(),
                                1,
                                TimeUnit.MILLISECONDS));
                  } else {
                    return Mono.delay(Duration.ofSeconds(1))
                        .transform(
                            new MonoTimeoutTransformer<>(
                                RpcResources.getClientOffLoopScheduler(), 1, TimeUnit.MILLISECONDS))
                        .onErrorResume(throwable -> Mono.empty());
                  }
                })
            .sequential()
            .reduce(0, (integer, number) -> integer + 1);

    StepVerifier.create(reduce).expectNext(count / 2).verifyComplete();
  }

  @Test
  public void testShouldNotTimeout() {
    Mono<Integer> transform =
        Flux.range(0, 100_000)
            .ignoreElements()
            .transform(
                new MonoTimeoutTransformer<>(
                    RpcResources.getClientOffLoopScheduler(), 1, TimeUnit.DAYS));

    StepVerifier.create(transform).expectComplete().verify();
  }

  @Test
  public void testShouldTimeout() {
    Mono<Object> transform =
        Mono.never()
            .transform(
                new MonoTimeoutTransformer<>(
                    RpcResources.getClientOffLoopScheduler(), 1, TimeUnit.SECONDS));

    StepVerifier.create(transform).verifyError(TimeoutException.class);
  }

  @Test
  public void testShouldFallback() {
    Mono<String> transform =
        Mono.<String>never()
            .transform(
                new MonoTimeoutTransformer<>(
                    RpcResources.getClientOffLoopScheduler(),
                    1,
                    TimeUnit.SECONDS,
                    Mono.just("hello")));

    StepVerifier.create(transform).expectNext("hello").verifyComplete();
  }

  @Test
  public void testShouldFallbackMultipleTimes() {
    Mono<String> transform =
        Mono.<String>never()
            .transform(
                new MonoTimeoutTransformer<>(
                    RpcResources.getClientOffLoopScheduler(),
                    1,
                    TimeUnit.SECONDS,
                    Mono.just("hello")));

    Flux<String> flux = Flux.range(0, 3).flatMap(__ -> transform);

    StepVerifier.create(flux).expectNext("hello", "hello", "hello").verifyComplete();
  }

  @Test
  public void testShouldTimeOutWithInfiniteStream() {
    Mono<Long> infinite =
        Flux.interval(Duration.ofMillis(1), Schedulers.boundedElastic())
            .ignoreElements()
            .transform(
                new MonoTimeoutTransformer<>(
                    RpcResources.getClientOffLoopScheduler(), 1, TimeUnit.SECONDS));

    StepVerifier.create(infinite)
        .verifyErrorSatisfies(
            throwable -> {
              Assert.assertTrue(throwable instanceof TimeoutException);
              Thread thread = Thread.currentThread();
              Assert.assertTrue(
                  thread instanceof io.netty.util.concurrent.FastThreadLocalThread
                      || thread
                          instanceof com.facebook.thrift.util.resources.FastThreadLocalThread);
            });
  }

  @Test(timeout = 5_000)
  public void testShouldTimeOutWhenNeverRequested() {
    Mono<Object> never =
        Mono.never()
            .transform(
                new MonoTimeoutTransformer<>(
                    RpcResources.getClientOffLoopScheduler(), 1, TimeUnit.SECONDS));

    StepVerifier.create(never).verifyError();
  }

  // ===================================================================================
  // Validation Tests
  // ===================================================================================

  @Test(expected = NullPointerException.class)
  public void testNullSchedulerThrows() {
    new MonoTimeoutTransformer<>(null, 1, TimeUnit.SECONDS);
  }

  @Test(expected = NullPointerException.class)
  public void testNullTimeUnitThrows() {
    new MonoTimeoutTransformer<>(RpcResources.getClientOffLoopScheduler(), 1, null);
  }

  @Test(expected = IllegalArgumentException.class)
  public void testNegativeDelayThrows() {
    new MonoTimeoutTransformer<>(RpcResources.getClientOffLoopScheduler(), -1, TimeUnit.SECONDS);
  }

  @Test
  public void testZeroDelayIsAllowed() {
    MonoTimeoutTransformer<String> transformer =
        new MonoTimeoutTransformer<>(RpcResources.getClientOffLoopScheduler(), 0, TimeUnit.SECONDS);

    // Zero delay should immediately timeout for a slow source
    StepVerifier.create(
            Mono.delay(Duration.ofMillis(100)).then(Mono.just("late")).transform(transformer))
        .verifyError(TimeoutException.class);
  }

  // ===================================================================================
  // Empty Mono Tests
  // ===================================================================================

  @Test
  public void testEmptyMonoCompletes() {
    Mono<String> transform =
        Mono.<String>empty()
            .transform(
                new MonoTimeoutTransformer<>(
                    RpcResources.getClientOffLoopScheduler(), 1, TimeUnit.SECONDS));

    StepVerifier.create(transform).verifyComplete();
  }

  @Test
  public void testEmptyMonoWithFallbackStillCompletes() {
    // Empty mono should complete before timeout, so fallback is not used
    Mono<String> transform =
        Mono.<String>empty()
            .transform(
                new MonoTimeoutTransformer<>(
                    RpcResources.getClientOffLoopScheduler(),
                    1,
                    TimeUnit.SECONDS,
                    Mono.just("fallback")));

    StepVerifier.create(transform).verifyComplete();
  }

  // ===================================================================================
  // Error Propagation Tests
  // ===================================================================================

  @Test
  public void testSourceErrorPropagates() {
    RuntimeException testError = new RuntimeException("test error");
    Mono<String> transform =
        Mono.<String>error(testError)
            .transform(
                new MonoTimeoutTransformer<>(
                    RpcResources.getClientOffLoopScheduler(), 1, TimeUnit.SECONDS));

    StepVerifier.create(transform).verifyErrorSatisfies(t -> Assert.assertEquals(testError, t));
  }

  @Test
  public void testSourceErrorBeforeTimeoutPropagates() {
    RuntimeException testError = new RuntimeException("test error");
    Mono<String> transform =
        Mono.delay(Duration.ofMillis(10))
            .<String>then(Mono.error(testError))
            .transform(
                new MonoTimeoutTransformer<>(
                    RpcResources.getClientOffLoopScheduler(), 1, TimeUnit.SECONDS));

    StepVerifier.create(transform).verifyErrorSatisfies(t -> Assert.assertEquals(testError, t));
  }

  @Test
  public void testFallbackErrorPropagates() {
    RuntimeException fallbackError = new RuntimeException("fallback error");
    Mono<String> transform =
        Mono.<String>never()
            .transform(
                new MonoTimeoutTransformer<>(
                    RpcResources.getClientOffLoopScheduler(),
                    100,
                    TimeUnit.MILLISECONDS,
                    Mono.error(fallbackError)));

    StepVerifier.create(transform).verifyErrorSatisfies(t -> Assert.assertEquals(fallbackError, t));
  }

  // ===================================================================================
  // Cancellation Tests
  // ===================================================================================

  @Test
  public void testCancelBeforeTimeout() throws InterruptedException {
    AtomicBoolean sourceSubscribed = new AtomicBoolean(false);
    AtomicBoolean sourceCancelled = new AtomicBoolean(false);
    CountDownLatch cancelLatch = new CountDownLatch(1);

    Mono<String> source =
        Mono.<String>never()
            .doOnSubscribe(s -> sourceSubscribed.set(true))
            .doOnCancel(
                () -> {
                  sourceCancelled.set(true);
                  cancelLatch.countDown();
                });

    Mono<String> transform =
        source.transform(
            new MonoTimeoutTransformer<>(
                RpcResources.getClientOffLoopScheduler(), 10, TimeUnit.SECONDS));

    Disposable disposable = transform.subscribe();

    // Wait a bit for subscription to propagate
    Thread.sleep(50);
    Assert.assertTrue("Source should be subscribed", sourceSubscribed.get());

    // Cancel before timeout
    disposable.dispose();

    // Verify source was cancelled
    Assert.assertTrue(
        "Cancel should complete within timeout", cancelLatch.await(1, TimeUnit.SECONDS));
    Assert.assertTrue("Source should be cancelled", sourceCancelled.get());
  }

  @Test
  public void testCancelAfterValueEmitted() throws InterruptedException {
    AtomicBoolean completed = new AtomicBoolean(false);
    CountDownLatch valueLatch = new CountDownLatch(1);

    Mono<String> transform =
        Mono.just("value")
            .delayElement(Duration.ofMillis(10))
            .transform(
                new MonoTimeoutTransformer<>(
                    RpcResources.getClientOffLoopScheduler(), 10, TimeUnit.SECONDS))
            .doOnNext(v -> valueLatch.countDown())
            .doOnTerminate(() -> completed.set(true));

    Disposable disposable = transform.subscribe();

    // Wait for value
    Assert.assertTrue("Value should be emitted", valueLatch.await(1, TimeUnit.SECONDS));

    // Cancel after value (should be a no-op since already completed)
    disposable.dispose();

    // Give time for completion
    Thread.sleep(100);
    Assert.assertTrue("Should complete after value", completed.get());
  }

  @Test
  public void testCancelDuringFallback() throws InterruptedException {
    AtomicBoolean fallbackSubscribed = new AtomicBoolean(false);
    AtomicBoolean fallbackCancelled = new AtomicBoolean(false);
    CountDownLatch fallbackLatch = new CountDownLatch(1);
    CountDownLatch cancelLatch = new CountDownLatch(1);

    // Slow fallback that we can cancel
    Mono<String> fallback =
        Mono.<String>never()
            .doOnSubscribe(
                s -> {
                  fallbackSubscribed.set(true);
                  fallbackLatch.countDown();
                })
            .doOnCancel(
                () -> {
                  fallbackCancelled.set(true);
                  cancelLatch.countDown();
                });

    Mono<String> transform =
        Mono.<String>never()
            .transform(
                new MonoTimeoutTransformer<>(
                    RpcResources.getClientOffLoopScheduler(), 50, TimeUnit.MILLISECONDS, fallback));

    Disposable disposable = transform.subscribe();

    // Wait for fallback to be subscribed (after timeout)
    Assert.assertTrue("Fallback should be subscribed", fallbackLatch.await(2, TimeUnit.SECONDS));
    Assert.assertTrue("Fallback subscribed flag should be set", fallbackSubscribed.get());

    // Cancel during fallback
    disposable.dispose();

    // Verify fallback was cancelled
    Assert.assertTrue("Fallback cancel should complete", cancelLatch.await(1, TimeUnit.SECONDS));
    Assert.assertTrue("Fallback should be cancelled", fallbackCancelled.get());
  }

  // ===================================================================================
  // Race Condition Tests
  // ===================================================================================

  @Test
  public void testRaceSourceVsTimeout() {
    // Test the race between source emitting and timeout firing
    // Run many iterations to increase likelihood of hitting the race
    int iterations = 1000;
    AtomicInteger successCount = new AtomicInteger(0);
    AtomicInteger timeoutCount = new AtomicInteger(0);

    for (int i = 0; i < iterations; i++) {
      // Source that emits around the same time as timeout
      Mono<String> source = Mono.delay(Duration.ofMillis(5)).thenReturn("value");

      Mono<String> transform =
          source.transform(
              new MonoTimeoutTransformer<>(
                  RpcResources.getClientOffLoopScheduler(), 5, TimeUnit.MILLISECONDS));

      try {
        String result = transform.block(Duration.ofSeconds(5));
        if ("value".equals(result)) {
          successCount.incrementAndGet();
        }
      } catch (Exception e) {
        if (e.getCause() instanceof TimeoutException || e instanceof TimeoutException) {
          timeoutCount.incrementAndGet();
        } else {
          throw e;
        }
      }
    }

    // Both outcomes are valid - just ensure we don't have errors
    Assert.assertTrue(
        "Should have some successes or timeouts",
        successCount.get() + timeoutCount.get() == iterations);
  }

  @Test
  public void testRaceCancelVsFallbackSetup() throws InterruptedException {
    // This test specifically targets the race condition where cancel() is called
    // while the fallback subscription is being set up
    int iterations = 100;
    List<Throwable> errors = Collections.synchronizedList(new ArrayList<>());

    for (int i = 0; i < iterations; i++) {
      CountDownLatch timeoutLatch = new CountDownLatch(1);
      AtomicReference<Disposable> disposableRef = new AtomicReference<>();

      // Fallback that signals when it's subscribed
      Mono<String> fallback =
          Mono.defer(
              () -> {
                timeoutLatch.countDown();
                return Mono.just("fallback");
              });

      Mono<String> transform =
          Mono.<String>never()
              .transform(
                  new MonoTimeoutTransformer<>(
                      RpcResources.getClientOffLoopScheduler(), 1, TimeUnit.MILLISECONDS, fallback))
              .doOnError(errors::add);

      disposableRef.set(transform.subscribe(v -> {}, errors::add, () -> {}));

      // Wait a tiny bit then cancel - trying to hit the race window
      timeoutLatch.await(1, TimeUnit.SECONDS);
      disposableRef.get().dispose();
    }

    // We shouldn't have any unexpected errors
    for (Throwable error : errors) {
      if (!(error instanceof TimeoutException)) {
        Assert.fail("Unexpected error: " + error);
      }
    }
  }

  @Test
  public void testConcurrentCancellations() throws InterruptedException {
    // Test that concurrent cancellations don't cause issues
    int threads = 10;
    CountDownLatch startLatch = new CountDownLatch(1);
    CountDownLatch doneLatch = new CountDownLatch(threads);
    List<Throwable> errors = Collections.synchronizedList(new ArrayList<>());

    Mono<String> transform =
        Mono.<String>never()
            .transform(
                new MonoTimeoutTransformer<>(
                    RpcResources.getClientOffLoopScheduler(),
                    10,
                    TimeUnit.SECONDS,
                    Mono.just("fallback")));

    Disposable disposable = transform.subscribe(v -> {}, errors::add, () -> {});

    // Start multiple threads that all try to cancel
    for (int i = 0; i < threads; i++) {
      new Thread(
              () -> {
                try {
                  startLatch.await();
                  disposable.dispose();
                } catch (InterruptedException e) {
                  Thread.currentThread().interrupt();
                } finally {
                  doneLatch.countDown();
                }
              })
          .start();
    }

    // Release all threads at once
    startLatch.countDown();

    // Wait for all to complete
    Assert.assertTrue("All threads should complete", doneLatch.await(5, TimeUnit.SECONDS));

    // Should not have errors
    Assert.assertTrue("Should have no errors: " + errors, errors.isEmpty());
  }

  @Test
  public void testRapidSubscribeCancel() throws InterruptedException {
    // Rapidly subscribe and cancel to stress test cleanup
    int iterations = 1000;
    List<Throwable> errors = Collections.synchronizedList(new ArrayList<>());

    for (int i = 0; i < iterations; i++) {
      Mono<String> transform =
          Mono.<String>never()
              .transform(
                  new MonoTimeoutTransformer<>(
                      RpcResources.getClientOffLoopScheduler(), 100, TimeUnit.MILLISECONDS));

      Disposable disposable = transform.subscribe(v -> {}, errors::add, () -> {});

      // Immediately cancel
      disposable.dispose();
    }

    // Small delay to let any async errors propagate
    Thread.sleep(200);

    Assert.assertTrue("Should have no errors: " + errors, errors.isEmpty());
  }

  // ===================================================================================
  // Fusion Optimization Tests
  // ===================================================================================

  @Test
  public void testScalarSourceFusion() {
    // Mono.just should be fused and not use timer
    Mono<String> transform =
        Mono.just("immediate")
            .transform(
                new MonoTimeoutTransformer<>(
                    RpcResources.getClientOffLoopScheduler(),
                    1,
                    TimeUnit.DAYS)); // Very long timeout

    // Should complete immediately due to fusion
    StepVerifier.create(transform).expectNext("immediate").verifyComplete();
  }

  @Test
  public void testEmptySourceFusion() {
    // Mono.empty should be fused
    Mono<String> transform =
        Mono.<String>empty()
            .transform(
                new MonoTimeoutTransformer<>(
                    RpcResources.getClientOffLoopScheduler(), 1, TimeUnit.DAYS));

    StepVerifier.create(transform).verifyComplete();
  }

  @Test
  public void testScalarFallbackFusion() {
    // Mono.just as fallback should be fused
    Mono<String> transform =
        Mono.<String>never()
            .transform(
                new MonoTimeoutTransformer<>(
                    RpcResources.getClientOffLoopScheduler(),
                    10,
                    TimeUnit.MILLISECONDS,
                    Mono.just("fallback")));

    StepVerifier.create(transform).expectNext("fallback").verifyComplete();
  }

  // ===================================================================================
  // Context Propagation Tests
  // ===================================================================================

  @Test
  public void testContextPropagationToSource() {
    String contextKey = "testKey";
    String contextValue = "testValue";
    AtomicReference<String> capturedValue = new AtomicReference<>();

    Mono<String> source =
        Mono.deferContextual(
            ctx -> {
              capturedValue.set(ctx.getOrDefault(contextKey, "missing"));
              return Mono.just("result");
            });

    Mono<String> transform =
        source
            .transform(
                new MonoTimeoutTransformer<>(
                    RpcResources.getClientOffLoopScheduler(), 1, TimeUnit.SECONDS))
            .contextWrite(Context.of(contextKey, contextValue));

    StepVerifier.create(transform).expectNext("result").verifyComplete();

    Assert.assertEquals(
        "Context should be propagated to source", contextValue, capturedValue.get());
  }

  @Test
  public void testContextPropagationToFallback() {
    String contextKey = "testKey";
    String contextValue = "fallbackContext";
    AtomicReference<String> capturedValue = new AtomicReference<>();

    Mono<String> fallback =
        Mono.deferContextual(
            ctx -> {
              capturedValue.set(ctx.getOrDefault(contextKey, "missing"));
              return Mono.just("fallback");
            });

    Mono<String> transform =
        Mono.<String>never()
            .transform(
                new MonoTimeoutTransformer<>(
                    RpcResources.getClientOffLoopScheduler(), 10, TimeUnit.MILLISECONDS, fallback))
            .contextWrite(Context.of(contextKey, contextValue));

    StepVerifier.create(transform).expectNext("fallback").verifyComplete();

    Assert.assertEquals(
        "Context should be propagated to fallback", contextValue, capturedValue.get());
  }

  // ===================================================================================
  // Complex Fallback Tests
  // ===================================================================================

  @Test
  public void testNonScalarFallback() {
    // Fallback that is not a scalar (uses delay)
    Mono<String> fallback = Mono.delay(Duration.ofMillis(10)).thenReturn("delayed-fallback");

    Mono<String> transform =
        Mono.<String>never()
            .transform(
                new MonoTimeoutTransformer<>(
                    RpcResources.getClientOffLoopScheduler(), 10, TimeUnit.MILLISECONDS, fallback));

    StepVerifier.create(transform).expectNext("delayed-fallback").verifyComplete();
  }

  @Test
  public void testFallbackThatEmitsEmpty() {
    Mono<String> transform =
        Mono.<String>never()
            .transform(
                new MonoTimeoutTransformer<>(
                    RpcResources.getClientOffLoopScheduler(),
                    10,
                    TimeUnit.MILLISECONDS,
                    Mono.empty()));

    StepVerifier.create(transform).verifyComplete();
  }

  @Test
  public void testFallbackWithOwnTimeout() {
    // Fallback that also times out
    Mono<String> fallback = Mono.<String>never().timeout(Duration.ofMillis(50));

    Mono<String> transform =
        Mono.<String>never()
            .transform(
                new MonoTimeoutTransformer<>(
                    RpcResources.getClientOffLoopScheduler(), 10, TimeUnit.MILLISECONDS, fallback));

    StepVerifier.create(transform).verifyError(TimeoutException.class);
  }

  // ===================================================================================
  // Backpressure Tests
  // ===================================================================================

  @Test
  public void testBackpressure() {
    Mono<String> transform =
        Mono.just("value")
            .transform(
                new MonoTimeoutTransformer<>(
                    RpcResources.getClientOffLoopScheduler(), 1, TimeUnit.SECONDS));

    StepVerifier.create(transform, 0).thenRequest(1).expectNext("value").verifyComplete();
  }

  // ===================================================================================
  // Late Subscriber Tests
  // ===================================================================================

  @Test
  public void testLateSourceSignalAfterTimeout() {
    // Source that tries to emit after timeout
    Sinks.One<String> sink = Sinks.one();

    Mono<String> transform =
        sink.asMono()
            .transform(
                new MonoTimeoutTransformer<>(
                    RpcResources.getClientOffLoopScheduler(), 50, TimeUnit.MILLISECONDS));

    StepVerifier.create(transform)
        .then(
            () -> {
              // Emit after timeout should have fired
              try {
                Thread.sleep(100);
              } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
              }
              sink.tryEmitValue("late");
            })
        .verifyError(TimeoutException.class);
  }

  // ===================================================================================
  // Thread Safety Tests
  // ===================================================================================

  @Test
  public void testManyParallelSubscriptions() {
    int parallelism = 100;

    Flux<String> flux =
        Flux.range(0, parallelism)
            .parallel()
            .runOn(Schedulers.parallel())
            .flatMap(
                i -> {
                  Mono<String> source = i % 2 == 0 ? Mono.just("fast-" + i) : Mono.<String>never();

                  return source.transform(
                      new MonoTimeoutTransformer<>(
                          RpcResources.getClientOffLoopScheduler(),
                          50,
                          TimeUnit.MILLISECONDS,
                          Mono.just("fallback-" + i)));
                })
            .sequential();

    StepVerifier.create(flux).expectNextCount(parallelism).verifyComplete();
  }

  // ===================================================================================
  // TIMEOUT_MARKER Fix Tests (Race 3)
  // These tests verify that cancel() after timeout but before scheduler executes
  // properly prevents signal emission.
  // ===================================================================================

  @Test
  public void testCancelAfterTimeoutBeforeSchedulerExecutes_NoFallback()
      throws InterruptedException {
    // This test verifies Race 3 handling for the no-fallback path.
    // The TIMEOUT_MARKER fix ensures that if cancel() is called after timeout fires
    // but before the scheduler executes run(), no signal is emitted.

    int iterations = 100;
    AtomicInteger errorCount = new AtomicInteger(0);
    AtomicInteger cancelledCount = new AtomicInteger(0);
    List<Throwable> unexpectedErrors = Collections.synchronizedList(new ArrayList<>());

    for (int i = 0; i < iterations; i++) {
      CountDownLatch subscribed = new CountDownLatch(1);
      AtomicReference<Disposable> disposableRef = new AtomicReference<>();

      Mono<String> transform =
          Mono.<String>never()
              .doOnSubscribe(s -> subscribed.countDown())
              .transform(
                  new MonoTimeoutTransformer<>(
                      RpcResources.getClientOffLoopScheduler(), 1, TimeUnit.MILLISECONDS));

      disposableRef.set(
          transform.subscribe(
              v -> {},
              e -> {
                if (e instanceof TimeoutException) {
                  errorCount.incrementAndGet();
                } else {
                  unexpectedErrors.add(e);
                }
              },
              () -> {}));

      // Wait for subscription
      subscribed.await(1, TimeUnit.SECONDS);

      // Wait around timeout time, then cancel
      // This tries to hit the window between timeout firing and scheduler executing
      Thread.sleep(1);
      disposableRef.get().dispose();
      cancelledCount.incrementAndGet();
    }

    // Small delay to let any async work complete
    Thread.sleep(50);

    // We should not have any unexpected errors
    Assert.assertTrue(
        "Should have no unexpected errors: " + unexpectedErrors, unexpectedErrors.isEmpty());

    // Due to the race, some iterations may timeout before cancel, others may be cancelled
    // The key is that we don't have any unexpected behavior
    System.out.println(
        "Cancel after timeout test: "
            + errorCount.get()
            + " timeouts, "
            + (cancelledCount.get() - errorCount.get())
            + " cancelled before error");
  }

  @Test
  public void testCancelAfterTimeoutBeforeSchedulerExecutes_WithFallback()
      throws InterruptedException {
    // This test verifies Race 3 handling for the standard fallback path.

    int iterations = 100;
    AtomicInteger fallbackCount = new AtomicInteger(0);
    AtomicInteger cancelledBeforeFallbackCount = new AtomicInteger(0);
    List<Throwable> unexpectedErrors = Collections.synchronizedList(new ArrayList<>());

    for (int i = 0; i < iterations; i++) {
      CountDownLatch subscribed = new CountDownLatch(1);
      AtomicBoolean fallbackSubscribed = new AtomicBoolean(false);
      AtomicReference<Disposable> disposableRef = new AtomicReference<>();

      // Non-scalar fallback to test the standard path
      Mono<String> fallback =
          Mono.defer(
              () -> {
                fallbackSubscribed.set(true);
                return Mono.just("fallback");
              });

      Mono<String> transform =
          Mono.<String>never()
              .doOnSubscribe(s -> subscribed.countDown())
              .transform(
                  new MonoTimeoutTransformer<>(
                      RpcResources.getClientOffLoopScheduler(),
                      1,
                      TimeUnit.MILLISECONDS,
                      fallback));

      disposableRef.set(
          transform.subscribe(
              v -> {
                if ("fallback".equals(v)) {
                  fallbackCount.incrementAndGet();
                }
              },
              unexpectedErrors::add,
              () -> {}));

      // Wait for subscription
      subscribed.await(1, TimeUnit.SECONDS);

      // Wait around timeout time, then cancel
      Thread.sleep(1);
      disposableRef.get().dispose();

      if (!fallbackSubscribed.get()) {
        cancelledBeforeFallbackCount.incrementAndGet();
      }
    }

    // Small delay to let any async work complete
    Thread.sleep(50);

    Assert.assertTrue(
        "Should have no unexpected errors: " + unexpectedErrors, unexpectedErrors.isEmpty());

    System.out.println(
        "Cancel after timeout with fallback test: "
            + fallbackCount.get()
            + " fallbacks executed, "
            + cancelledBeforeFallbackCount.get()
            + " cancelled before fallback");
  }

  @Test
  public void testCancelAfterTimeoutBeforeSchedulerExecutes_ScalarFallback()
      throws InterruptedException {
    // This test verifies Race 3 handling for the scalar fallback path.

    int iterations = 100;
    AtomicInteger fallbackCount = new AtomicInteger(0);
    List<Throwable> unexpectedErrors = Collections.synchronizedList(new ArrayList<>());

    for (int i = 0; i < iterations; i++) {
      CountDownLatch subscribed = new CountDownLatch(1);
      AtomicReference<Disposable> disposableRef = new AtomicReference<>();

      // Scalar fallback (Mono.just)
      Mono<String> transform =
          Mono.<String>never()
              .doOnSubscribe(s -> subscribed.countDown())
              .transform(
                  new MonoTimeoutTransformer<>(
                      RpcResources.getClientOffLoopScheduler(),
                      1,
                      TimeUnit.MILLISECONDS,
                      Mono.just("scalar-fallback")));

      disposableRef.set(
          transform.subscribe(
              v -> {
                if ("scalar-fallback".equals(v)) {
                  fallbackCount.incrementAndGet();
                }
              },
              unexpectedErrors::add,
              () -> {}));

      // Wait for subscription
      subscribed.await(1, TimeUnit.SECONDS);

      // Wait around timeout time, then cancel
      Thread.sleep(1);
      disposableRef.get().dispose();
    }

    // Small delay to let any async work complete
    Thread.sleep(50);

    Assert.assertTrue(
        "Should have no unexpected errors: " + unexpectedErrors, unexpectedErrors.isEmpty());

    System.out.println(
        "Cancel after timeout with scalar fallback test: "
            + fallbackCount.get()
            + " fallbacks executed out of "
            + iterations);
  }

  // ===================================================================================
  // Upstream Cancellation Tests
  // ===================================================================================

  @Test
  public void testUpstreamCancelledWhenTimeoutFires() throws InterruptedException {
    AtomicBoolean upstreamCancelled = new AtomicBoolean(false);
    CountDownLatch cancelLatch = new CountDownLatch(1);

    Mono<String> source =
        Mono.<String>never()
            .doOnCancel(
                () -> {
                  upstreamCancelled.set(true);
                  cancelLatch.countDown();
                });

    Mono<String> transform =
        source.transform(
            new MonoTimeoutTransformer<>(
                RpcResources.getClientOffLoopScheduler(), 50, TimeUnit.MILLISECONDS));

    StepVerifier.create(transform).verifyError(TimeoutException.class);

    // Verify upstream was cancelled
    Assert.assertTrue(
        "Upstream should be cancelled when timeout fires", cancelLatch.await(1, TimeUnit.SECONDS));
    Assert.assertTrue("Upstream cancelled flag should be set", upstreamCancelled.get());
  }

  @Test
  public void testUpstreamCancelledWhenTimeoutFires_WithFallback() throws InterruptedException {
    AtomicBoolean upstreamCancelled = new AtomicBoolean(false);
    CountDownLatch cancelLatch = new CountDownLatch(1);

    Mono<String> source =
        Mono.<String>never()
            .doOnCancel(
                () -> {
                  upstreamCancelled.set(true);
                  cancelLatch.countDown();
                });

    Mono<String> transform =
        source.transform(
            new MonoTimeoutTransformer<>(
                RpcResources.getClientOffLoopScheduler(),
                50,
                TimeUnit.MILLISECONDS,
                Mono.just("fallback")));

    StepVerifier.create(transform).expectNext("fallback").verifyComplete();

    // Verify upstream was cancelled
    Assert.assertTrue(
        "Upstream should be cancelled when timeout fires", cancelLatch.await(1, TimeUnit.SECONDS));
    Assert.assertTrue("Upstream cancelled flag should be set", upstreamCancelled.get());
  }

  // ===================================================================================
  // State Transition Tests
  // ===================================================================================

  @Test
  public void testStateTransition_SourceEmitsBeforeTimeout() {
    // INIT -> VALUE_EMITTED -> TERMINATED
    AtomicInteger onNextCount = new AtomicInteger(0);
    AtomicInteger onCompleteCount = new AtomicInteger(0);

    Mono<String> transform =
        Mono.just("value")
            .transform(
                new MonoTimeoutTransformer<>(
                    RpcResources.getClientOffLoopScheduler(), 1, TimeUnit.SECONDS))
            .doOnNext(v -> onNextCount.incrementAndGet())
            .doOnTerminate(() -> onCompleteCount.incrementAndGet());

    StepVerifier.create(transform).expectNext("value").verifyComplete();

    Assert.assertEquals("Should receive exactly one onNext", 1, onNextCount.get());
    Assert.assertEquals("Should receive exactly one onComplete", 1, onCompleteCount.get());
  }

  @Test
  public void testStateTransition_SourceCompletesEmpty() {
    // INIT -> TERMINATED (via onComplete with no value)
    AtomicInteger onNextCount = new AtomicInteger(0);
    AtomicInteger onCompleteCount = new AtomicInteger(0);

    Mono<String> transform =
        Mono.<String>empty()
            .transform(
                new MonoTimeoutTransformer<>(
                    RpcResources.getClientOffLoopScheduler(), 1, TimeUnit.SECONDS))
            .doOnNext(v -> onNextCount.incrementAndGet())
            .doOnTerminate(() -> onCompleteCount.incrementAndGet());

    StepVerifier.create(transform).verifyComplete();

    Assert.assertEquals("Should receive no onNext", 0, onNextCount.get());
    Assert.assertEquals("Should receive exactly one onComplete", 1, onCompleteCount.get());
  }

  @Test
  public void testStateTransition_SourceErrors() {
    // INIT -> TERMINATED (via onError)
    RuntimeException testError = new RuntimeException("test");
    AtomicInteger onErrorCount = new AtomicInteger(0);

    Mono<String> transform =
        Mono.<String>error(testError)
            .transform(
                new MonoTimeoutTransformer<>(
                    RpcResources.getClientOffLoopScheduler(), 1, TimeUnit.SECONDS))
            .doOnError(e -> onErrorCount.incrementAndGet());

    StepVerifier.create(transform).verifyError(RuntimeException.class);

    Assert.assertEquals("Should receive exactly one onError", 1, onErrorCount.get());
  }

  @Test
  public void testStateTransition_TimeoutFires() {
    // INIT -> TERMINATED (via timeout)
    AtomicInteger onErrorCount = new AtomicInteger(0);

    Mono<String> transform =
        Mono.<String>never()
            .transform(
                new MonoTimeoutTransformer<>(
                    RpcResources.getClientOffLoopScheduler(), 50, TimeUnit.MILLISECONDS))
            .doOnError(e -> onErrorCount.incrementAndGet());

    StepVerifier.create(transform).verifyError(TimeoutException.class);

    Assert.assertEquals("Should receive exactly one onError", 1, onErrorCount.get());
  }

  @Test
  public void testStateTransition_Cancel() throws InterruptedException {
    // INIT -> TERMINATED (via cancel)
    AtomicInteger signalCount = new AtomicInteger(0);
    CountDownLatch subscribed = new CountDownLatch(1);

    Mono<String> transform =
        Mono.<String>never()
            .doOnSubscribe(s -> subscribed.countDown())
            .transform(
                new MonoTimeoutTransformer<>(
                    RpcResources.getClientOffLoopScheduler(), 10, TimeUnit.SECONDS))
            .doOnSuccess(v -> signalCount.incrementAndGet())
            .doOnError(e -> signalCount.incrementAndGet());

    Disposable disposable = transform.subscribe();

    subscribed.await(1, TimeUnit.SECONDS);
    disposable.dispose();

    // Small delay to ensure no signals are sent
    Thread.sleep(100);

    Assert.assertEquals("Should receive no signals after cancel", 0, signalCount.get());
  }

  // ===================================================================================
  // Scalar Fallback Edge Cases
  // ===================================================================================

  @Test
  public void testScalarFallbackWithEmptyMono() {
    // Mono.empty() as scalar fallback
    Mono<String> transform =
        Mono.<String>never()
            .transform(
                new MonoTimeoutTransformer<>(
                    RpcResources.getClientOffLoopScheduler(),
                    50,
                    TimeUnit.MILLISECONDS,
                    Mono.empty()));

    StepVerifier.create(transform).verifyComplete();
  }

  @Test
  public void testScalarFallbackThatThrows() {
    // ScalarCallable that throws an exception
    RuntimeException fallbackError = new RuntimeException("fallback failed");

    Mono<String> throwingFallback =
        Mono.fromCallable(
            () -> {
              throw fallbackError;
            });

    Mono<String> transform =
        Mono.<String>never()
            .transform(
                new MonoTimeoutTransformer<>(
                    RpcResources.getClientOffLoopScheduler(),
                    50,
                    TimeUnit.MILLISECONDS,
                    throwingFallback));

    StepVerifier.create(transform).verifyErrorSatisfies(e -> Assert.assertEquals(fallbackError, e));
  }

  // ===================================================================================
  // Double Cancel / Idempotency Tests
  // ===================================================================================

  @Test
  public void testDoubleCancelIsIdempotent() throws InterruptedException {
    AtomicInteger cancelCount = new AtomicInteger(0);
    List<Throwable> errors = Collections.synchronizedList(new ArrayList<>());
    CountDownLatch subscribed = new CountDownLatch(1);

    Mono<String> transform =
        Mono.<String>never()
            .doOnSubscribe(s -> subscribed.countDown())
            .doOnCancel(cancelCount::incrementAndGet)
            .transform(
                new MonoTimeoutTransformer<>(
                    RpcResources.getClientOffLoopScheduler(), 10, TimeUnit.SECONDS));

    Disposable disposable = transform.subscribe(v -> {}, errors::add, () -> {});

    subscribed.await(1, TimeUnit.SECONDS);

    // Cancel multiple times
    disposable.dispose();
    disposable.dispose();
    disposable.dispose();

    // Small delay
    Thread.sleep(50);

    // Should have no errors
    Assert.assertTrue("Should have no errors: " + errors, errors.isEmpty());

    // Note: cancelCount may be 1 or more depending on how cancellation propagates
    // The key is no errors and no exceptions
  }

  // ===================================================================================
  // Timer Cancellation Verification
  // ===================================================================================

  @Test
  public void testTimerCancelledWhenSourceEmits() throws InterruptedException {
    // If source emits before timeout, timer should be cancelled (no timeout error later)
    AtomicInteger errorCount = new AtomicInteger(0);

    Mono<String> transform =
        Mono.just("value")
            .transform(
                new MonoTimeoutTransformer<>(
                    RpcResources.getClientOffLoopScheduler(), 100, TimeUnit.MILLISECONDS))
            .doOnError(e -> errorCount.incrementAndGet());

    StepVerifier.create(transform).expectNext("value").verifyComplete();

    // Wait longer than the timeout to ensure timer doesn't fire
    Thread.sleep(200);

    Assert.assertEquals("Timer should not fire after source emits", 0, errorCount.get());
  }

  @Test
  public void testTimerCancelledWhenSourceCompletes() throws InterruptedException {
    AtomicInteger errorCount = new AtomicInteger(0);

    Mono<String> transform =
        Mono.<String>empty()
            .transform(
                new MonoTimeoutTransformer<>(
                    RpcResources.getClientOffLoopScheduler(), 100, TimeUnit.MILLISECONDS))
            .doOnError(e -> errorCount.incrementAndGet());

    StepVerifier.create(transform).verifyComplete();

    Thread.sleep(200);

    Assert.assertEquals("Timer should not fire after source completes", 0, errorCount.get());
  }

  @Test
  public void testTimerCancelledWhenSourceErrors() throws InterruptedException {
    AtomicInteger timeoutCount = new AtomicInteger(0);

    Mono<String> transform =
        Mono.<String>error(new RuntimeException("source error"))
            .transform(
                new MonoTimeoutTransformer<>(
                    RpcResources.getClientOffLoopScheduler(), 100, TimeUnit.MILLISECONDS))
            .doOnError(
                e -> {
                  if (e instanceof TimeoutException) {
                    timeoutCount.incrementAndGet();
                  }
                });

    StepVerifier.create(transform).verifyError(RuntimeException.class);

    Thread.sleep(200);

    Assert.assertEquals("Timer should not fire after source errors", 0, timeoutCount.get());
  }

  // ===================================================================================
  // Request Forwarding Tests
  // ===================================================================================

  @Test
  public void testRequestForwardedToUpstream() {
    AtomicInteger requestedCount = new AtomicInteger(0);

    Mono<String> source =
        Mono.just("value")
            .doOnRequest(
                n -> {
                  if (n == Long.MAX_VALUE) {
                    requestedCount.incrementAndGet();
                  }
                });

    Mono<String> transform =
        source.transform(
            new MonoTimeoutTransformer<>(
                RpcResources.getClientOffLoopScheduler(), 1, TimeUnit.SECONDS));

    StepVerifier.create(transform).expectNext("value").verifyComplete();

    Assert.assertEquals(
        "Request should be forwarded to upstream with Long.MAX_VALUE", 1, requestedCount.get());
  }

  @Test
  public void testRequestForwardedToFallback() {
    AtomicInteger requestedCount = new AtomicInteger(0);

    Mono<String> fallback =
        Mono.defer(
                () ->
                    Mono.just("fallback")
                        .doOnRequest(
                            n -> {
                              if (n == Long.MAX_VALUE) {
                                requestedCount.incrementAndGet();
                              }
                            }))
            .delaySubscription(Duration.ofMillis(1)); // Make it non-scalar

    Mono<String> transform =
        Mono.<String>never()
            .transform(
                new MonoTimeoutTransformer<>(
                    RpcResources.getClientOffLoopScheduler(), 50, TimeUnit.MILLISECONDS, fallback));

    StepVerifier.create(transform).expectNext("fallback").verifyComplete();

    Assert.assertEquals(
        "Request should be forwarded to fallback with Long.MAX_VALUE", 1, requestedCount.get());
  }

  // ===================================================================================
  // Edge Cases for Source Error With Fallback
  // ===================================================================================

  @Test
  public void testSourceErrorBeforeTimeout_WithFallbackConfigured() {
    // When source errors before timeout, fallback should NOT be used
    // (fallback is only for timeout, not source errors)
    RuntimeException sourceError = new RuntimeException("source failed");

    Mono<String> transform =
        Mono.<String>error(sourceError)
            .transform(
                new MonoTimeoutTransformer<>(
                    RpcResources.getClientOffLoopScheduler(),
                    1,
                    TimeUnit.SECONDS,
                    Mono.just("fallback")));

    StepVerifier.create(transform).verifyErrorSatisfies(e -> Assert.assertEquals(sourceError, e));
  }

  // ===================================================================================
  // Deterministic Race Condition Tests using ControllableTimer
  // These tests use ControllableTimer to precisely control timeout firing and
  // achieve 100% deterministic race condition testing.
  // ===================================================================================

  @Test
  public void testDeterministicRace3_StandardFallback_CancelBeforeRun()
      throws InterruptedException {
    // This test deterministically verifies Race 3 handling for standard fallback.
    // Sequence:
    // 1. Subscribe - timer task is registered
    // 2. Fire timeout - run(Timeout) executes, sets S=TIMEOUT_MARKER, schedules run()
    // 3. Cancel - before scheduler executes run(), cancel() is called
    // 4. Scheduler executes run() - should detect cancellation and NOT subscribe to fallback

    CountDownLatch schedulerExecuted = new CountDownLatch(1);
    AtomicBoolean fallbackSubscribed = new AtomicBoolean(false);
    AtomicReference<Disposable> disposableRef = new AtomicReference<>();

    // Fallback that tracks subscription
    Mono<String> fallback =
        Mono.defer(
            () -> {
              fallbackSubscribed.set(true);
              return Mono.just("fallback");
            });

    // Use a scheduler that we can control
    CountDownLatch runScheduled = new CountDownLatch(1);
    AtomicReference<Runnable> scheduledRunnable = new AtomicReference<>();
    Scheduler controlledScheduler = createControllableScheduler(scheduledRunnable, runScheduled);

    // Create timer for the actual test
    ControllableTimer controllableTimer = new ControllableTimer();

    Mono<String> transform =
        Mono.<String>never()
            .transform(
                new MonoTimeoutTransformer<>(
                    controlledScheduler, 1, TimeUnit.HOURS, fallback, () -> controllableTimer));

    disposableRef.set(transform.subscribe());

    // Step 2: Fire timeout - run(Timeout) executes and schedules run()
    controllableTimer.fireAllTimeouts();

    // Wait for run() to be scheduled
    Assert.assertTrue("run() should be scheduled", runScheduled.await(1, TimeUnit.SECONDS));

    // Step 3: Cancel BEFORE run() executes
    disposableRef.get().dispose();

    // Step 4: Now execute run()
    Runnable runMethod = scheduledRunnable.get();
    Assert.assertNotNull("run() should have been captured", runMethod);
    runMethod.run();

    // Verify: Fallback should NOT be subscribed because we cancelled
    Assert.assertFalse("Fallback should NOT be subscribed after cancel", fallbackSubscribed.get());
  }

  @Test
  public void testDeterministicRace3_ScalarFallback_CancelBeforeEmit() throws InterruptedException {
    // Similar to above but for scalar fallback path

    CountDownLatch runScheduled = new CountDownLatch(1);
    AtomicReference<Runnable> scheduledRunnable = new AtomicReference<>();
    AtomicInteger valueReceived = new AtomicInteger(0);
    AtomicReference<Disposable> disposableRef = new AtomicReference<>();

    Scheduler controlledScheduler = createControllableScheduler(scheduledRunnable, runScheduled);

    ControllableTimer controllableTimer = new ControllableTimer();

    Mono<String> transform =
        Mono.<String>never()
            .transform(
                new MonoTimeoutTransformer<>(
                    controlledScheduler,
                    1,
                    TimeUnit.HOURS,
                    Mono.just("scalar-fallback"), // Scalar callable
                    () -> controllableTimer));

    // Step 1: Subscribe
    disposableRef.set(transform.subscribe(v -> valueReceived.incrementAndGet(), e -> {}, () -> {}));

    // Step 2: Fire timeout
    controllableTimer.fireAllTimeouts();

    // Wait for run() to be scheduled
    Assert.assertTrue("run() should be scheduled", runScheduled.await(1, TimeUnit.SECONDS));

    // Step 3: Cancel before run() executes
    disposableRef.get().dispose();

    // Step 4: Execute run()
    scheduledRunnable.get().run();

    // Verify: Value should NOT be emitted
    Assert.assertEquals("Scalar fallback value should NOT be emitted", 0, valueReceived.get());
  }

  @Test
  public void testDeterministicRace3_NoFallback_CancelBeforeError() throws InterruptedException {
    // Test for no-fallback path: cancel after timeout fires but before error emission

    CountDownLatch runScheduled = new CountDownLatch(1);
    AtomicReference<Runnable> scheduledRunnable = new AtomicReference<>();
    AtomicInteger errorReceived = new AtomicInteger(0);
    AtomicReference<Disposable> disposableRef = new AtomicReference<>();

    Scheduler controlledScheduler = createControllableScheduler(scheduledRunnable, runScheduled);

    ControllableTimer controllableTimer = new ControllableTimer();

    Mono<String> transform =
        Mono.<String>never()
            .transform(
                new MonoTimeoutTransformer<>(
                    controlledScheduler,
                    1,
                    TimeUnit.HOURS,
                    null, // No fallback - will emit TimeoutException
                    () -> controllableTimer));

    // Step 1: Subscribe
    disposableRef.set(
        transform.subscribe(
            v -> {},
            e -> {
              if (e instanceof TimeoutException) {
                errorReceived.incrementAndGet();
              }
            },
            () -> {}));

    // Step 2: Fire timeout
    controllableTimer.fireAllTimeouts();

    // Wait for run() to be scheduled
    Assert.assertTrue("run() should be scheduled", runScheduled.await(1, TimeUnit.SECONDS));

    // Step 3: Cancel before run() executes
    disposableRef.get().dispose();

    // Step 4: Execute run()
    scheduledRunnable.get().run();

    // Verify: TimeoutException should NOT be emitted
    Assert.assertEquals("TimeoutException should NOT be emitted", 0, errorReceived.get());
  }

  @Test
  public void testControllableTimer_TimeoutFiresNormally() {
    // Verify that ControllableTimer works correctly when not cancelled
    ControllableTimer timer = new ControllableTimer();
    AtomicBoolean taskExecuted = new AtomicBoolean(false);

    Timeout timeout = timer.newTimeout(t -> taskExecuted.set(true), 1, TimeUnit.SECONDS);

    Assert.assertFalse("Task should not execute before fire", taskExecuted.get());
    Assert.assertFalse("Should not be expired", timeout.isExpired());
    Assert.assertFalse("Should not be cancelled", timeout.isCancelled());

    // Fire the timeout
    ((ControllableTimeout) timeout).fire();

    Assert.assertTrue("Task should execute after fire", taskExecuted.get());
    Assert.assertTrue("Should be expired", timeout.isExpired());
    Assert.assertFalse("Should not be cancelled", timeout.isCancelled());
  }

  @Test
  public void testControllableTimer_CancelPreventsExecution() {
    // Verify that cancelled timeouts don't fire
    ControllableTimer timer = new ControllableTimer();
    AtomicBoolean taskExecuted = new AtomicBoolean(false);

    Timeout timeout = timer.newTimeout(t -> taskExecuted.set(true), 1, TimeUnit.SECONDS);

    // Cancel the timeout
    boolean cancelled = timeout.cancel();
    Assert.assertTrue("Cancel should succeed", cancelled);
    Assert.assertTrue("Should be cancelled", timeout.isCancelled());

    // Try to fire - should do nothing
    ((ControllableTimeout) timeout).fire();

    Assert.assertFalse("Task should NOT execute after cancel", taskExecuted.get());
    Assert.assertFalse("Should not be expired after cancel", timeout.isExpired());
  }

  // ===================================================================================
  // FallbackSubscriber Defensive Checks Tests
  // ===================================================================================

  @Test
  public void testFallbackSubscriber_DropsSignalsAfterCancel() throws InterruptedException {
    // This test verifies that FallbackSubscriber properly drops signals
    // that arrive after cancellation.

    Sinks.Many<String> sink = Sinks.many().multicast().onBackpressureBuffer();
    CountDownLatch firstValueReceived = new CountDownLatch(1);
    CountDownLatch fallbackSubscribed = new CountDownLatch(1);
    AtomicInteger valueCount = new AtomicInteger(0);
    AtomicReference<Disposable> disposableRef = new AtomicReference<>();
    List<Throwable> errors = Collections.synchronizedList(new ArrayList<>());

    // Non-scalar fallback that we control via sink
    Mono<String> fallback = sink.asFlux().next().doOnSubscribe(s -> fallbackSubscribed.countDown());

    Mono<String> transform =
        Mono.<String>never()
            .transform(
                new MonoTimeoutTransformer<>(
                    RpcResources.getClientOffLoopScheduler(), 10, TimeUnit.MILLISECONDS, fallback));

    disposableRef.set(
        transform.subscribe(
            v -> {
              valueCount.incrementAndGet();
              firstValueReceived.countDown();
            },
            errors::add,
            () -> {}));

    // Wait for fallback to be subscribed (after timeout)
    Assert.assertTrue(
        "Fallback should be subscribed", fallbackSubscribed.await(2, TimeUnit.SECONDS));

    // Emit first value - this should be received
    sink.tryEmitNext("first");

    // Wait for first value to be received
    Assert.assertTrue(
        "First value should be received", firstValueReceived.await(1, TimeUnit.SECONDS));

    // Now cancel the subscription
    disposableRef.get().dispose();

    // Try to emit more values - these should be dropped by FallbackSubscriber
    sink.tryEmitNext("second");
    sink.tryEmitNext("third");

    // Small delay to ensure any async processing completes
    Thread.sleep(50);

    // Verify only one value was received
    Assert.assertEquals(
        "Only the first value should be received (before cancel)", 1, valueCount.get());

    // Verify no errors
    Assert.assertTrue("Should have no errors: " + errors, errors.isEmpty());
  }

  @Test
  public void testFallbackSubscriber_DropsErrorAfterCancel() throws InterruptedException {
    // This test verifies that FallbackSubscriber drops errors that arrive after cancel

    Sinks.One<String> sink = Sinks.one();
    CountDownLatch fallbackSubscribed = new CountDownLatch(1);
    AtomicInteger errorCount = new AtomicInteger(0);
    AtomicReference<Disposable> disposableRef = new AtomicReference<>();

    Mono<String> fallback = sink.asMono().doOnSubscribe(s -> fallbackSubscribed.countDown());

    Mono<String> transform =
        Mono.<String>never()
            .transform(
                new MonoTimeoutTransformer<>(
                    RpcResources.getClientOffLoopScheduler(), 10, TimeUnit.MILLISECONDS, fallback));

    disposableRef.set(transform.subscribe(v -> {}, e -> errorCount.incrementAndGet(), () -> {}));

    // Wait for fallback to be subscribed
    Assert.assertTrue(
        "Fallback should be subscribed", fallbackSubscribed.await(2, TimeUnit.SECONDS));

    // Cancel before any signal from fallback
    disposableRef.get().dispose();

    // Try to emit error - should be dropped
    sink.tryEmitError(new RuntimeException("late error"));

    Thread.sleep(50);

    // Verify no error was received
    Assert.assertEquals("Error after cancel should be dropped", 0, errorCount.get());
  }

  @Test
  public void testFallbackSubscriber_DropsCompleteAfterCancel() throws InterruptedException {
    // This test verifies that FallbackSubscriber drops onComplete after cancel

    Sinks.One<String> sink = Sinks.one();
    CountDownLatch fallbackSubscribed = new CountDownLatch(1);
    AtomicBoolean completed = new AtomicBoolean(false);
    AtomicReference<Disposable> disposableRef = new AtomicReference<>();

    Mono<String> fallback = sink.asMono().doOnSubscribe(s -> fallbackSubscribed.countDown());

    Mono<String> transform =
        Mono.<String>never()
            .transform(
                new MonoTimeoutTransformer<>(
                    RpcResources.getClientOffLoopScheduler(), 10, TimeUnit.MILLISECONDS, fallback));

    disposableRef.set(transform.subscribe(v -> {}, e -> {}, () -> completed.set(true)));

    // Wait for fallback to be subscribed
    Assert.assertTrue(
        "Fallback should be subscribed", fallbackSubscribed.await(2, TimeUnit.SECONDS));

    // Cancel before completion
    disposableRef.get().dispose();

    // Try to complete - should be dropped
    sink.tryEmitEmpty();

    Thread.sleep(50);

    // Verify onComplete was not called
    Assert.assertFalse("onComplete after cancel should be dropped", completed.get());
  }

  // ===================================================================================
  // Misbehaving Publisher Tests
  // ===================================================================================

  @Test
  public void testMisbehavingFallback_ContinuesEmittingAfterCancel() throws InterruptedException {
    // This test uses a misbehaving publisher that continues to emit after cancel().
    // The FallbackSubscriber should drop all signals after cancellation.

    AtomicInteger valuesReceived = new AtomicInteger(0);
    CountDownLatch fallbackSubscribed = new CountDownLatch(1);
    AtomicReference<Disposable> mainDisposable = new AtomicReference<>();
    List<Throwable> errors = Collections.synchronizedList(new ArrayList<>());

    // Create a misbehaving flux that ignores cancellation
    Sinks.Many<String> sink = Sinks.many().multicast().directBestEffort();

    Mono<String> misbehavingFallback =
        sink.asFlux().next().doOnSubscribe(s -> fallbackSubscribed.countDown());

    Mono<String> transform =
        Mono.<String>never()
            .transform(
                new MonoTimeoutTransformer<>(
                    RpcResources.getClientOffLoopScheduler(),
                    10,
                    TimeUnit.MILLISECONDS,
                    misbehavingFallback));

    mainDisposable.set(
        transform.subscribe(v -> valuesReceived.incrementAndGet(), errors::add, () -> {}));

    // Wait for fallback to be subscribed
    Assert.assertTrue(
        "Fallback should be subscribed", fallbackSubscribed.await(2, TimeUnit.SECONDS));

    // Cancel the main subscription
    mainDisposable.get().dispose();

    // Now emit values - a misbehaving publisher would continue emitting
    // but FallbackSubscriber should drop them
    sink.tryEmitNext("value1");
    sink.tryEmitNext("value2");
    sink.tryEmitNext("value3");

    Thread.sleep(50);

    // Verify no values were received (all dropped after cancel)
    Assert.assertEquals("All values after cancel should be dropped", 0, valuesReceived.get());

    // Verify no unexpected errors
    Assert.assertTrue("Should have no errors: " + errors, errors.isEmpty());
  }

  @Test
  public void testMisbehavingFallback_ValueThenCancelThenMoreValues() throws InterruptedException {
    // Tests: value received -> cancel -> more values (should be dropped)

    Sinks.Many<String> sink = Sinks.many().multicast().onBackpressureBuffer();
    CountDownLatch fallbackSubscribed = new CountDownLatch(1);
    CountDownLatch firstValueLatch = new CountDownLatch(1);
    List<String> receivedValues = Collections.synchronizedList(new ArrayList<>());
    AtomicReference<Disposable> mainDisposable = new AtomicReference<>();

    Mono<String> fallback = sink.asFlux().next().doOnSubscribe(s -> fallbackSubscribed.countDown());

    Mono<String> transform =
        Mono.<String>never()
            .transform(
                new MonoTimeoutTransformer<>(
                    RpcResources.getClientOffLoopScheduler(), 10, TimeUnit.MILLISECONDS, fallback));

    mainDisposable.set(
        transform.subscribe(
            v -> {
              receivedValues.add(v);
              firstValueLatch.countDown();
            },
            e -> {},
            () -> {}));

    // Wait for fallback subscription
    Assert.assertTrue(
        "Fallback should be subscribed", fallbackSubscribed.await(2, TimeUnit.SECONDS));

    // Emit first value
    sink.tryEmitNext("first");
    Assert.assertTrue("First value should be received", firstValueLatch.await(1, TimeUnit.SECONDS));

    // Cancel
    mainDisposable.get().dispose();

    // Emit more values after cancel
    sink.tryEmitNext("second");
    sink.tryEmitNext("third");

    Thread.sleep(50);

    // Only first value should be in the list
    Assert.assertEquals("Should receive exactly one value", 1, receivedValues.size());
    Assert.assertEquals("First value should be 'first'", "first", receivedValues.get(0));
  }

  // ===================================================================================
  // Helper methods and classes are grouped together at the very end of the file.
  // They are declared private static.
  // ===================================================================================

  /**
   * Creates a controllable scheduler that captures scheduled runnables for manual execution. Used
   * by deterministic race condition tests to control exactly when scheduled tasks run.
   *
   * @param scheduledRunnable AtomicReference to capture the scheduled runnable
   * @param runScheduled CountDownLatch to signal when a runnable has been scheduled
   * @return A Scheduler that captures runnables instead of executing them
   */
  private static Scheduler createControllableScheduler(
      AtomicReference<Runnable> scheduledRunnable, CountDownLatch runScheduled) {
    return new Scheduler() {
      @Override
      public Disposable schedule(Runnable task) {
        scheduledRunnable.set(task);
        runScheduled.countDown();
        return () -> {};
      }

      @Override
      public Worker createWorker() {
        return new Worker() {
          @Override
          public Disposable schedule(Runnable task) {
            scheduledRunnable.set(task);
            runScheduled.countDown();
            return () -> {};
          }

          @Override
          public void dispose() {}
        };
      }

      @Override
      public void dispose() {}
    };
  }

  /**
   * A controllable Timer implementation that allows tests to precisely control when timeouts fire.
   * This enables truly deterministic testing of race conditions between timeout firing,
   * cancellation, and scheduler execution.
   */
  private static class ControllableTimer implements Timer {
    private final Set<ControllableTimeout> pendingTimeouts = ConcurrentHashMap.newKeySet();
    private volatile boolean stopped = false;

    @Override
    public Timeout newTimeout(TimerTask task, long delay, TimeUnit unit) {
      if (stopped) {
        throw new IllegalStateException("Timer has been stopped");
      }
      ControllableTimeout timeout = new ControllableTimeout(task, this);
      pendingTimeouts.add(timeout);
      return timeout;
    }

    @Override
    public Set<Timeout> stop() {
      stopped = true;
      Set<Timeout> remaining = ConcurrentHashMap.newKeySet();
      remaining.addAll(pendingTimeouts);
      pendingTimeouts.clear();
      return remaining;
    }

    /** Fire all pending timeouts synchronously on the calling thread. */
    public void fireAllTimeouts() {
      for (ControllableTimeout timeout : pendingTimeouts) {
        timeout.fire();
      }
    }

    /** Get the first pending timeout (for single-timeout scenarios). */
    public ControllableTimeout getFirstTimeout() {
      return pendingTimeouts.stream().findFirst().orElse(null);
    }

    /** Check if there are any pending timeouts. */
    public boolean hasPendingTimeouts() {
      return !pendingTimeouts.isEmpty();
    }

    void removeTimeout(ControllableTimeout timeout) {
      pendingTimeouts.remove(timeout);
    }
  }

  /** A controllable Timeout that can be fired on demand. */
  private static class ControllableTimeout implements Timeout {
    private final TimerTask task;
    private final ControllableTimer timer;
    private volatile boolean cancelled = false;
    private volatile boolean expired = false;

    ControllableTimeout(TimerTask task, ControllableTimer timer) {
      this.task = task;
      this.timer = timer;
    }

    @Override
    public Timer timer() {
      return timer;
    }

    @Override
    public TimerTask task() {
      return task;
    }

    @Override
    public boolean isExpired() {
      return expired;
    }

    @Override
    public boolean isCancelled() {
      return cancelled;
    }

    @Override
    public boolean cancel() {
      if (!cancelled && !expired) {
        cancelled = true;
        timer.removeTimeout(this);
        return true;
      }
      return false;
    }

    /**
     * Fire the timeout synchronously. This simulates the timer thread calling run(Timeout). The
     * timeout task is executed on the calling thread.
     */
    public void fire() {
      if (!cancelled && !expired) {
        expired = true;
        timer.removeTimeout(this);
        try {
          task.run(this);
        } catch (Exception e) {
          // Ignore - in real timer, exceptions are logged
        }
      }
    }
  }
}
